//
// Created by Hoonyong Park on 6/13/23.
//

#include "task.hpp"

#include <DU/f1ap/task.hpp>
#include <lib/rrc/encode.hpp>

#include <asn/rrc/ASN_RRC_MeasurementReport.h>
#include <asn/rrc/ASN_RRC_MeasurementReport-IEs.h>
#include <asn/rrc/ASN_RRC_MeasResultServMO.h>
#include <asn/rrc/ASN_RRC_MeasQuantityResults.h>
#include <asn/rrc/ASN_RRC_MeasResultNR.h>
#include <asn/rrc/ASN_RRC_MeasResultListNR.h>
#include <asn/rrc/ASN_RRC_RRCReconfiguration.h>
#include <asn/rrc/ASN_RRC_RRCReconfiguration-IEs.h>
#include <asn/rrc/ASN_RRC_DL-DCCH-Message.h>
#include <asn/rrc/ASN_RRC_CellGroupConfig.h>
#include <asn/rrc/ASN_RRC_SpCellConfig.h>
#include <asn/rrc/ASN_RRC_ReconfigurationWithSync.h>
#include <asn/rrc/ASN_RRC_ServingCellConfigCommon.h>

namespace nr::DU
{

void DURrcTask::receiveMeasurementReport(int ueId, const ASN_RRC_MeasurementReport &msg)
{
    auto *ue = findUe(ueId);
    if (!ue)
        return;

    std::string buffer = "MeasurementReport|" + std::to_string(ueId) + "|";
    auto MR = msg.criticalExtensions.choice.measurementReport;
    auto servMR = MR->measResults.measResultServingMOList.list.array[0]->measResultServingCell;
    long servPCI = *servMR.physCellId;
    long servDbm = *servMR.measResult.cellResults.resultsSSB_Cell->rsrp;

    buffer = buffer + std::to_string(servPCI) + "|" + std::to_string(servDbm) + "|";

    auto neighMRCount = MR->measResults.measResultNeighCells->choice.measResultListNR->list.count;

    buffer = buffer + std::to_string(neighMRCount);

    for(int i = 0; i < neighMRCount; i++)
    {
        auto measResultNR = MR->measResults.measResultNeighCells->choice.measResultListNR->list.array[i];
        auto neighPCI = measResultNR->physCellId;
        auto neighDbm = measResultNR->measResult.cellResults.resultsSSB_Cell->rsrp;

        buffer = buffer + "|" + std::to_string(*neighPCI) + "|" + std::to_string(*neighDbm);
    }

    m_logger->debug("Received MeasurementReport From [%d]", ueId);

    auto msg1 = std::make_unique<NmDURrcToF1ap>(NmDURrcToF1ap::UL_RRC_TRANSFER);
    msg1->rrcChannel = rrc::RrcChannel::UL_DCCH;
    msg1->buffer = buffer;
    m_base->f1apTask->push(std::move(msg1));
}

void DURrcTask::handleUEContextSetupRequest(std::vector<std::string> msg)
{
    auto ue = createUe(++ueIdentifier);

    msg.push_back(std::to_string(ue->ueId));

    auto w = std::make_unique<NmDURrcToF1ap>(NmDURrcToF1ap::UE_CONTEXT_SETUP_RESPONSE);
    w->msg = msg;
    m_base->f1apTask->push(std::move(w));
}

void DURrcTask::sendRRCReconfiguration(int ueId, long targetPCI)
{
    auto *ue = findUe(ueId);
    if (!ue)
        return;

    auto *pdu = asn::New<ASN_RRC_DL_DCCH_Message>();
    pdu->message.present = ASN_RRC_DL_DCCH_MessageType_PR_c1;
    pdu->message.choice.c1 = asn::NewFor(pdu->message.choice.c1);
    pdu->message.choice.c1->present = ASN_RRC_DL_DCCH_MessageType__c1_PR_rrcReconfiguration;
    auto &rrcReconfiguration = pdu->message.choice.c1->choice.rrcReconfiguration = asn::New<ASN_RRC_RRCReconfiguration>();
    rrcReconfiguration->rrc_TransactionIdentifier = getNextTid();
    rrcReconfiguration->criticalExtensions.present = ASN_RRC_RRCReconfiguration__criticalExtensions_PR_rrcReconfiguration;
    rrcReconfiguration->criticalExtensions.choice.rrcReconfiguration = asn::New<ASN_RRC_RRCReconfiguration_IEs>();

    sendRrcMessage(ueId, pdu);
    asn::Free(asn_DEF_ASN_RRC_DL_DCCH_Message, pdu);
}

void DURrcTask::receiveRrcReconfigurationComplete(int ueId, int64_t sti, const ASN_RRC_RRCReconfigurationComplete &msg)
{
    std::string buffer = "RRCReconfigurationComplete|" + std::to_string(ueId) + "|" + std::to_string(sti);

    createUe(ueId);

    auto msg1 = std::make_unique<NmDURrcToF1ap>(NmDURrcToF1ap::UL_RRC_TRANSFER);
    msg1->rrcChannel = rrc::RrcChannel::UL_DCCH;
    msg1->buffer = buffer;
    m_base->f1apTask->push(std::move(msg1));
}

void DURrcTask::sendUEContextReleaseComplete(int ueId)
{
    m_ueCtx.erase(ueId);

    auto msg1 = std::make_unique<NmDURrcToF1ap>(NmDURrcToF1ap::UE_CONTEXT_RELEASE_COMPLETE);
    msg1->ueId = ueId;
    m_base->f1apTask->push(std::move(msg1));
}

}