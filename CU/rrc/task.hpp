//
// Created by Hoonyong Park on 5/11/23.
//

#pragma once

#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include <CU/nts.hpp>
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
}

namespace nr::CU
{

class NgapTask;

class CURrcTask : public NtsTask
{
  private:
    TaskBase *m_base;
    CUConfig *m_config;
    std::unique_ptr<Logger> m_logger;
    std::string excutionTime;

    std::unordered_map<int, RrcUeContext *> m_ueCtx;
    int m_tidCounter;

    bool m_isBarred = true;
    bool m_cellReserved = false;
    UacAiBarringSet m_aiBarringSet = {};
    bool m_intraFreqReselectAllowed = true;

    int ueIdentifier = 0;

    friend class CUCmdHandler;

  public:
    explicit CURrcTask(TaskBase *base);
    ~CURrcTask() override = default;

  protected:
    void onStart() override;
    void onLoop() override;
    void onQuit() override;

  private:
    /* Management */
    int getNextTid();

    /* Utility functions */
    std::vector<std::string> split(std::string input, char delimiter);
    std::string Merge(std::vector<std::string> vec);

    /* Handlers */
    void handleUplinkRrc(int ueId, rrc::RrcChannel channel, const OctetString &rrcPdu);
    void releaseConnection(int ueId);
    void handleUplinkRrcCCCH(int duId, int gNB_DU_ID, std::string data);
    void handleUplinkRrcDCCH(int duId, int gNB_DU_ID, std::string data);

    /* RRC channel send message */
    void sendRrcMessage(ASN_RRC_BCCH_BCH_Message *msg);
    void sendRrcMessage(ASN_RRC_BCCH_DL_SCH_Message *msg);
    void sendRrcMessage(int ueId, ASN_RRC_DL_CCCH_Message *msg);
    void sendRrcMessage(int ueId, ASN_RRC_DL_DCCH_Message *msg);
    void sendRrcMessage(ASN_RRC_PCCH_Message *msg);

    /* RRC channel receive message */
    void receiveRrcMessage(int ueId, ASN_RRC_BCCH_BCH_Message *msg);
    void receiveRrcMessage(int ueId, ASN_RRC_UL_CCCH_Message *msg);
    void receiveRrcMessage(int ueId, ASN_RRC_UL_CCCH1_Message *msg);
    void receiveRrcMessage(int ueId, ASN_RRC_UL_DCCH_Message *msg);

    /* System Information Broadcast related */
    void onBroadcastTimerExpired();
    void triggerSysInfoBroadcast();

    /* Service Access Point */
    //void handleRlsSapMessage(NmCURlsToRrc &msg);

    /* UE Management */
    RrcUeContext *createUe(int id);
    RrcUeContext *tryFindUe(int id);
    RrcUeContext *findUe(int id);
    RrcUeContext *findUeByGNB_DU_UE_ID(int gNB_DU_ID, int GNB_DU_UE_ID);
    RrcUeContext *findUeBySti(int64_t sti);

    /* Connection Control */
    void receiveRrcSetupRequest(int duId, int gNB_DU_ID, std::string msg);
    void receiveRrcSetupComplete(int duId, int gNB_DU_ID, std::string msg);

    /* Handover Control */
    void receiveMeasurementReport(int duId, int gNB_DU_ID, std::string msg);
    void TriggerInterDuHandover(int, int, long);
    void receiveRrcReconfigurationComplete(int duId, int gNB_DU_ID, std::string data);
};

} // namespace nr::CU