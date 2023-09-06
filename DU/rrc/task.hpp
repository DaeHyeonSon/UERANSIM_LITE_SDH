//
// Created by Hpoonyong Park on 6/3/23.
//

#pragma once

#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <DU/nts.hpp>
#include <utils/logger.hpp>
#include <utils/nts.hpp>

extern "C"
{
    struct ASN_RRC_BCCH_BCH_Message;
    struct ASN_RRC_BCCH_DL_SCH_Message;
    struct ASN_RRC_DL_CCCH_Message;
    struct ASN_RRC_DL_DCCH_Message;
    struct ASN_RRC_PCCH_Message;
    struct ASN_RRC_UL_CCCH_Message;
    struct ASN_RRC_UL_CCCH1_Message;
    struct ASN_RRC_UL_DCCH_Message;

    struct ASN_RRC_RRCSetupRequest;
    struct ASN_RRC_RRCSetupComplete;
    struct ASN_RRC_ULInformationTransfer;
    struct ASN_RRC_MeasurementReport;
    struct ASN_RRC_RRCReconfigurationComplete;
}

namespace nr::DU
{

class DURrcTask : public NtsTask
{
  private:
    TaskBase *m_base;
    DUConfig *m_config;
    std::unique_ptr<Logger> m_logger;

    std::unordered_map<int, RrcUeContext *> m_ueCtx;
    bool m_isBarred = true;
    bool m_cellReserved = false;
    UacAiBarringSet m_aiBarringSet = {};
    bool m_intraFreqReselectAllowed = true;

    int m_tidCounter = 0;

    int ueIdentifier = 0;

    friend class CUCmdHandler;

  public:
    explicit DURrcTask(TaskBase *base);
    ~DURrcTask() override = default;

  protected:
    void onStart() override;
    void onLoop() override;
    void onQuit() override;

  private:
    int getNextTid();

    /* Handlers */
    void handleUplinkRrc(int ueId, int64_t sti, rrc::RrcChannel channel, const OctetString &rrcPdu);
    void releaseConnection(int ueId);
    void handleDownlinkRrcTransfer(std::vector<std::string> msg);
    void handleDLCCCHMessage(std::vector<std::string> msg);
    void handleDLDCCHMessage(std::vector<std::string> msg);

    /* RRC channel send message */
    void sendRrcMessage(ASN_RRC_BCCH_BCH_Message *msg);
    void sendRrcMessage(ASN_RRC_BCCH_DL_SCH_Message *msg);
    void sendRrcMessage(int ueId, ASN_RRC_DL_CCCH_Message *msg);
    void sendRrcMessage(int ueId, ASN_RRC_DL_DCCH_Message *msg);
    void sendRrcMessage(ASN_RRC_PCCH_Message *msg);


    /* RRC channel receive message */
    void receiveRrcMessage(int ueId, ASN_RRC_BCCH_BCH_Message *msg);
    void receiveRrcMessage(int ueId, int64_t sti, ASN_RRC_UL_CCCH_Message *msg);
    void receiveRrcMessage(int ueId, ASN_RRC_UL_CCCH1_Message *msg);
    void receiveRrcMessage(int ueId, int64_t sti, ASN_RRC_UL_DCCH_Message *msg);

    /* Service Access Point */
    void handleRlsSapMessage(NmDURlsToRrc &msg);

    /* System Information Broadcast related */
    void onBroadcastTimerExpired();
    void triggerSysInfoBroadcast();

    /* UE Management */
    RrcUeContext *createUe(int id);
    RrcUeContext *tryFindUe(int id);
    RrcUeContext *findUe(int id);

    /* Connection Control */
    void receiveRrcSetupRequest(int ueId, int64_t sti, const ASN_RRC_RRCSetupRequest &msg);
    void receiveRrcSetupComplete(int ueId, const ASN_RRC_RRCSetupComplete &msg);
    void sendRRCSetup(std::vector<std::string> msg);
    void sendUECapabilityEnquiry(std::vector<std::string> msg);

    /* handover Contorl */
    void receiveMeasurementReport(int ueId, const ASN_RRC_MeasurementReport &msg);
    void handleUEContextSetupRequest(std::vector<std::string> msg);
    void sendRRCReconfiguration(int ueId, long targetPCI);
    void receiveRrcReconfigurationComplete(int ueId, int64_t sti, const ASN_RRC_RRCReconfigurationComplete &msg);
    void sendUEContextReleaseComplete(int ueId);




};

}