//
// Created by Hoonyong Park on 5/11/23.
//

#include "task.hpp"

#include <CU/nts.hpp>
//#include <CU/rls/task.hpp>
#include <lib/rrc/encode.hpp>

#include <asn/rrc/ASN_RRC_DLInformationTransfer-IEs.h>
#include <asn/rrc/ASN_RRC_DLInformationTransfer.h>

static constexpr const int TIMER_ID_SI_BROADCAST = 1;
static constexpr const int TIMER_PERIOD_SI_BROADCAST = 10'000;

static std::string get_time_stamp()
{
    const auto now = std::chrono::system_clock::now();
    time_t tm_now = std::chrono::system_clock::to_time_t(now);
    struct tm tstruct = *localtime(&tm_now);

    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;

    char temp[128];
    snprintf(temp, sizeof(temp), "%04d%02d%02d_%02d:%02d:%02d.%03ld",
             tstruct.tm_year + 1900, tstruct.tm_mon + 1, tstruct.tm_mday,
             tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec, millis.count());

    return std::string(temp);
}

namespace nr::CU
{

CURrcTask::CURrcTask(TaskBase *base) : m_base{base}, m_ueCtx{}, m_tidCounter{}
{
    excutionTime = get_time_stamp();
    m_logger = base->logBase->makeUniqueLogger("rrc");
    m_config = m_base->config;
}

void CURrcTask::onStart()
{
    setTimer(TIMER_ID_SI_BROADCAST, TIMER_PERIOD_SI_BROADCAST);
}

void CURrcTask::onQuit()
{
    // todo
}

void CURrcTask::onLoop()
{
    auto msg = take();
    if (!msg)
        return;

    switch (msg->msgType)
    {
    case NtsMessageType::CU_F1AP_TO_RRC: {
        auto &w = dynamic_cast<NmCUF1apToRrc &>(*msg);
        switch (w.present)
        {
        case NmCUF1apToRrc::RECEIVE_CCCH_Message: {
            handleUplinkRrcCCCH(w.duId, w.gNB_DU_ID, w.data);
            break;
        }
        case NmCUF1apToRrc::RECEIVE_DCCH_Message: {
            handleUplinkRrcDCCH(w.duId, w.gNB_DU_ID, w.data);
            break;
        }
        }
        //handleRlsSapMessage(dynamic_cast<NmCURlsToRrc &>(*msg));
        break;
    }
    case NtsMessageType::TIMER_EXPIRED: {
        auto w = dynamic_cast<NmTimerExpired &>(*msg);
        if (w.timerId == TIMER_ID_SI_BROADCAST)
        {
            setTimer(TIMER_ID_SI_BROADCAST, TIMER_PERIOD_SI_BROADCAST);
            onBroadcastTimerExpired();
        }
        break;
    }
    default:
        m_logger->unhandledNts(*msg);
        break;
    }
}

} // namespace nr::CU
