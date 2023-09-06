//
// Created by Hoonyong Park on 6/2/23.
//
#include "task.hpp"

#include <CU/sctpServer/sctp_task.hpp>

namespace nr::CU
{

void F1apTask::handleAssociationSetup(int duId, int inCount, int outCount)
{

}

void F1apTask::handleAssociationShutdown(int duId)
{
    auto *du = findDuContext(duId);
    if (du == nullptr)
        return;

    m_logger->err("Association terminated for DU[%d]", duId);
    m_logger->debug("Removing DU context[%d]", duId);

    auto w = std::make_unique<NmCUSctp>(NmCUSctp::CONNECTION_CLOSE);
    w->clientId = duId;
    m_base->sctpServerTask->push(std::move(w));

    deleteDuContext(duId);
}

void F1apTask::receiveF1SetupRequest(int duId, int gNB_DU_ID /*, int cellId*/)
{
    createDuContext(duId, gNB_DU_ID /*, cellId*/);

    sendF1SetupResponse(duId);
}

void F1apTask::sendF1SetupResponse(int duId)
{
    m_logger->debug("Sending F1 Setup Response To DU[%d]", duId);

    auto *du = findDuContext(duId);
    if (du == nullptr)
        return;

    std::string *pdu = new std::string();

    *pdu = "F1SetupResponse|" + std::to_string(m_base->config->getCUId());

    //        auto *pdu = asn::f1ap::NewMessagePdu<F1SetupRequest>(
    //            {ieGlobalCUId, ieRanNodeName, ieSupportedTaList, iePagingDrx});

    sendF1ap(duId, pdu);
}

void F1apTask::sendDLRrcMessageTransfer(int duId, rrc::RrcChannel rrcChannel, std::string msg)
{
    auto *du = findDuContext(duId);
    if (du == nullptr)
        return;

    m_logger->debug("Sending DLRrcMessageTransfer To DU[%d]", duId);

    std::string *pdu = new std::string();

    *pdu = "DLRrcMessageTransfer|";

    switch (rrcChannel)
    {
    case rrc::RrcChannel::DL_CCCH: {
        *pdu = *pdu + "DL_CCCH|";
        break;
    }
    case rrc::RrcChannel::DL_DCCH: {
        *pdu = *pdu + "DL_DCCH|";
        break;
    }
    default:
        break;
    }

    *pdu = *pdu + msg;

    sendF1ap(duId, pdu);
}

void F1apTask::sendUEContextSetupRequest(int duId, int ueId, int GNB_DU_UE_ID,long sourcePCI, long targetPCI)
{
    auto *du = findDuContextByGNB_DU_ID(targetPCI);
    if (du == nullptr)
        return;

    m_logger->debug("Sending UEContextSetupRequest To PCI[%d]", targetPCI);

    std::string *pdu = new std::string();

    *pdu = "UEContextSetupRequest|" + std::to_string(ueId) + "|" + std::to_string(GNB_DU_UE_ID) + "|" + std::to_string(sourcePCI) + "|" + std::to_string(targetPCI);

    sendF1ap(du->ctxId, pdu);
}

void F1apTask::receiveUEContextSetupResponse(int duId, std::vector<std::string> msg)
{
    auto *du = findDuContext(duId);
    if (du == nullptr)
        return;

    m_logger->debug("Receiving UEContextSetupResponse From DU[%d]", du->ctxId);

    auto *sourceDU = findDuContextByGNB_DU_ID(std::stoi(msg.at(3)));

    std::string *pdu = new std::string();

    msg.erase(msg.begin());
    msg.insert(msg.begin(), "UEContextModificationRequest");

    *pdu = Merge(msg);

    sendF1ap(sourceDU->ctxId, pdu);
}

void F1apTask::sendUEContextReleaseCommand(int duId, int GNB_DU_UE_ID)
{
    auto *du = findDuContextByGNB_DU_ID(duId);
    if (du == nullptr)
        return;

    m_logger->debug("Sending UEContextReleaseCommand About UE[%d] To DU[%d]", GNB_DU_UE_ID, du->ctxId);

    std::string *pdu = new std::string();

    *pdu = "UEContextReleaseCommand|" + std::to_string(GNB_DU_UE_ID);

    sendF1ap(du->ctxId, pdu);
}

void F1apTask::receiveUEContextReleaseComplete(int duId)
{
    auto *du = findDuContext(duId);
    if (du == nullptr)
        return;

    m_logger->debug("Receiving UEContextReleaseComplete From DU[%d]", du->ctxId);
}


}