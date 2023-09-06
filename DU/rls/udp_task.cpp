//
// Created by Hoonyong Park on 6/3/23.
//

#include "udp_task.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <set>
#include <iostream>
#include <iomanip>

#include <DU/nts.hpp>
#include <utils/common.hpp>
#include <utils/constants.hpp>
#include <utils/libc_error.hpp>

#define m_frequency 28.0e9
#define M_C 3.0e8
#define avg_building_h 5

static constexpr const int BUFFER_SIZE = 16384;

static constexpr const int LOOP_PERIOD = 1000;
static constexpr const int RECEIVE_TIMEOUT = 200;
static constexpr const int HEARTBEAT_THRESHOLD = 2000; // (LOOP_PERIOD + RECEIVE_TIMEOUT)'dan büyük olmalı

static constexpr const double MIN_ALLOWED_DBM = -120.0;

static constexpr const double txPower = 30.0;

void printHex(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(data[i]);
        if (i != length - 1) {
            std::cout << " ";
        }
    }
    std::cout << "\n";
}

double Calculate2dDistance(Vector3 a, Vector3 b) {
    double x = a.x - b.x;
    double y = a.y - b.y;
    double distance2D = std::sqrt(x * x + y * y);

    return distance2D;
}

static double CalculateDistance(Vector3 a, Vector3 b)
{
    double x1, y1, z1;
    double x2, y2, z2;

    x1 = a.x;
    y1 = a.y;
    z1 = a.z;

    x2 = b.x;
    y2 = b.y;
    z2 = b.z;

    return std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2) + std::pow(z2 - z1, 2));
}

static double GetBpDistance(double frequency, double hA, double hB)
{
    double distanceBp = 2.0 * M_PI * hA * hB * frequency / M_C;

    return distanceBp;
}

static double Pl1(double frequency, double distance3d, double h, double w)
{
    double loss = 20.0 * std::log10(40.0 * M_PI * distance3d * frequency / 1e9 / 3.0) +
                  std::min(0.03 * pow(h, 1.72), 10.0) * std::log10(distance3d) - std::min(0.044 * pow(h, 1.72), 14.77) +
                  0.002 * std::log10(h) * distance3d;

    return loss;
}

static double GetLossLos(double distance2d, double distance3d, double hUt, double hBs)
{
    if (hUt < 1.0 or hUt > 10.0)
        throw std::out_of_range("The height of the UT should be between 1 and 10 m");

    if (hBs < 10.0 or hBs > 150.0)
        throw std::out_of_range("The height of the BS should be between 10 and 150 m");

    double distanceBp = GetBpDistance(m_frequency, hBs, hUt);

//    if (distance2d < 10.0 or distance2d > 10.0e3)
//        throw std::out_of_range("The 2D distance is outside the validity range, the pathloss value may not be accurate");

    double loss = 0;

    if (distance2d <= distanceBp)
        loss = Pl1(m_frequency, distance3d, avg_building_h, 0);
    else
        loss = Pl1(m_frequency, distanceBp, avg_building_h, 0) + 40 * std::log10(distance3d / distanceBp);

    return loss;
}

static double GetLoss(double distance2d, double distance3d, double hUt, double hBs)
{
    double loss = GetLossLos(distance2d, distance3d, hUt, hBs);

    return loss;
}

static double DoCalcRxPower(Vector3 uePos, Vector3 bsPos)
{
    double distance2d = Calculate2dDistance(uePos, bsPos);

    double distance3d = CalculateDistance(uePos, bsPos);

    double heights[2] = {uePos.z, bsPos.z};

    double rxPow = txPower;

    rxPow = rxPow - GetLoss(distance2d, distance3d, heights[0], heights[1]);

    return rxPow;
}

static double EstimateSimulatedDbm(const Vector3 &myPos, const Vector3 &uePos)
{
    //std::cout << "x: " << uePos.x << " y: " << uePos.y << " z: " << uePos.z << std::endl;

    if (uePos.z < 1.0 or uePos.z > 10.0)
        return MIN_ALLOWED_DBM;

    if (myPos.z < 10.0 or myPos.z > 150.0)
        return MIN_ALLOWED_DBM;

    return DoCalcRxPower(uePos, myPos);
}

namespace nr::DU
{

RlsUdpTask::RlsUdpTask(TaskBase *base, uint64_t sti, Vector3 phyLocation)
    : m_server{}, m_ctlTask{}, m_sti{sti}, m_phyLocation{phyLocation}, m_lastLoop{}, m_stiToUe{}, m_ueMap{}, m_newIdCounter{}
{
    m_logger = base->logBase->makeUniqueLogger("rls-udp");

    try
    {
        m_server = new udp::UdpServer(base->config->linkIp, cons::RadioLinkPort);
    }
    catch (const LibError &e)
    {
        m_logger->err("RLS failure [%s]", e.what());
        quit();
        return;
    }
}

void RlsUdpTask::onStart()
{
}

void RlsUdpTask::onLoop()
{
    auto current = utils::CurrentTimeMillis();
    if (current - m_lastLoop > LOOP_PERIOD)
    {
        m_lastLoop = current;
        heartbeatCycle(current);
    }

    uint8_t buffer[BUFFER_SIZE];
    InetAddress peerAddress;

    int size = m_server->Receive(buffer, BUFFER_SIZE, RECEIVE_TIMEOUT, peerAddress);
    if (size > 0)
    {
//        std::cout << "Received: ";
//        printHex(buffer, size);
        auto rlsMsg = rls::DecodeRlsMessage(OctetView{buffer, static_cast<size_t>(size)});
        if (rlsMsg == nullptr)
            m_logger->err("Unable to decode RLS message");
        else
            receiveRlsPdu(peerAddress, std::move(rlsMsg));
    }
}

void RlsUdpTask::onQuit()
{
    delete m_server;
}

void RlsUdpTask::receiveRlsPdu(const InetAddress &addr, std::unique_ptr<rls::RlsMessage> &&msg)
{
    if (msg->msgType == rls::EMessageType::HEARTBEAT)
    {

        double dbm = EstimateSimulatedDbm(m_phyLocation, ((const rls::RlsHeartBeat &)*msg).simPos);
        if (dbm < MIN_ALLOWED_DBM)
        {
            m_logger->err("if the simulated signal strength is such low, then ignore this message: %f", dbm);
            return;
        }

        if (m_stiToUe.count(msg->sti))
        {
            int ueId = m_stiToUe[msg->sti];
            m_ueMap[ueId].sti = msg->sti;
            m_ueMap[ueId].address = addr;
            m_ueMap[ueId].lastSeen = utils::CurrentTimeMillis();
        }
        else
        {
            int ueId = ++m_newIdCounter;

            m_stiToUe[msg->sti] = ueId;
            m_ueMap[ueId].sti = msg->sti;
            m_ueMap[ueId].address = addr;
            m_ueMap[ueId].lastSeen = utils::CurrentTimeMillis();

            auto w = std::make_unique<NmDURlsToRls>(NmDURlsToRls::SIGNAL_DETECTED);
            w->ueId = ueId;
            m_ctlTask->push(std::move(w));
        }

        rls::RlsHeartBeatAck ack{m_sti};
        ack.dbm = dbm;

        sendRlsPdu(addr, ack);
        return;
    }

    if (!m_stiToUe.count(msg->sti))
    {
        // if no HB received yet, and the message is not HB, then ignore the message
        return;
    }

    auto w = std::make_unique<NmDURlsToRls>(NmDURlsToRls::RECEIVE_RLS_MESSAGE);
    w->ueId = m_stiToUe[msg->sti];
    w->sti = msg->sti;
    w->msg = std::move(msg);
    m_ctlTask->push(std::move(w));
}

void RlsUdpTask::sendRlsPdu(const InetAddress &addr, const rls::RlsMessage &msg)
{
    OctetString stream;
    rls::EncodeRlsMessage(msg, stream);

//    std::cout << "Send: ";
//    printHex(stream.data(), static_cast<size_t>(stream.length()));

    m_server->Send(addr, stream.data(), static_cast<size_t>(stream.length()));
}

void RlsUdpTask::heartbeatCycle(int64_t time)
{
    std::set<int> lostUeId{};
    std::set<uint64_t> lostSti{};

    for (auto &item : m_ueMap)
    {
        if (time - item.second.lastSeen > HEARTBEAT_THRESHOLD)
        {
            lostUeId.insert(item.first);
            lostSti.insert(item.second.sti);
        }
    }

    for (uint64_t sti : lostSti)
        m_stiToUe.erase(sti);

    for (int ueId : lostUeId)
        m_ueMap.erase(ueId);

    for (int ueId : lostUeId)
    {
        auto w = std::make_unique<NmDURlsToRls>(NmDURlsToRls::SIGNAL_LOST);
        w->ueId = ueId;
        m_ctlTask->push(std::move(w));
    }
}

void RlsUdpTask::initialize(NtsTask *ctlTask)
{
    m_ctlTask = ctlTask;
}

void RlsUdpTask::send(int ueId, const rls::RlsMessage &msg)
{
    if (ueId == 0)
    {
        for (auto &ue : m_ueMap)
            send(ue.first, msg);
        return;
    }

    if (!m_ueMap.count(ueId))
    {
        // ignore the message
        return;
    }

    sendRlsPdu(m_ueMap[ueId].address, msg);
}

} // namespace nr::DU
