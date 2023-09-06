//
// This file is a part of UERANSIM open source project.
// Copyright (c) 2021 ALİ GÜNGÖR.
//
// The software and all associated files are licensed under GPL-3.0
// and subject to the terms and conditions defined in LICENSE file.
//

#include "task.hpp"

#include <CU/f1ap/task.hpp>
#include <lib/rrc/encode.hpp>

#include <asn/ngap/ASN_NGAP_FiveG-S-TMSI.h>
#include <asn/rrc/ASN_RRC_BCCH-BCH-Message.h>
#include <asn/rrc/ASN_RRC_BCCH-DL-SCH-Message.h>
#include <asn/rrc/ASN_RRC_CellGroupConfig.h>
#include <asn/rrc/ASN_RRC_DL-CCCH-Message.h>
#include <asn/rrc/ASN_RRC_DL-DCCH-Message.h>
#include <asn/rrc/ASN_RRC_DLInformationTransfer-IEs.h>
#include <asn/rrc/ASN_RRC_DLInformationTransfer.h>
#include <asn/rrc/ASN_RRC_PCCH-Message.h>
#include <asn/rrc/ASN_RRC_Paging.h>
#include <asn/rrc/ASN_RRC_PagingRecord.h>
#include <asn/rrc/ASN_RRC_PagingRecordList.h>
#include <asn/rrc/ASN_RRC_RRCRelease-IEs.h>
#include <asn/rrc/ASN_RRC_RRCRelease.h>
#include <asn/rrc/ASN_RRC_RRCSetup-IEs.h>
#include <asn/rrc/ASN_RRC_RRCSetup.h>
#include <asn/rrc/ASN_RRC_RRCSetupComplete-IEs.h>
#include <asn/rrc/ASN_RRC_RRCSetupComplete.h>
#include <asn/rrc/ASN_RRC_RRCSetupRequest.h>
#include <asn/rrc/ASN_RRC_UL-CCCH-Message.h>
#include <asn/rrc/ASN_RRC_UL-CCCH1-Message.h>
#include <asn/rrc/ASN_RRC_UL-DCCH-Message.h>
#include <asn/rrc/ASN_RRC_ULInformationTransfer-IEs.h>
#include <asn/rrc/ASN_RRC_ULInformationTransfer.h>

using namespace std;

namespace nr::CU
{

void CURrcTask::receiveRrcSetupRequest(int duId, int gNB_DU_ID, string data)
{
    auto msg = split(data, '|');

    auto *ue = createUe(ueIdentifier++);

    ue->gNB_DU_ID = gNB_DU_ID;
    ue->GNB_DU_UE_ID = stoi(msg.at(1));

    ue->state = ERrcState::RRC_IDLE;

    if (msg.at(2) == "TRUE")
    {
        ue->initialId = stoll(msg.at(3));
        ue->isInitialIdSTmsi = true;
    }
    else
    {
        ue->initialId = stoll(msg.at(3));
        ue->isInitialIdSTmsi = false;
    }

    ue->establishmentCause = stoll(msg.at(4));

    ue->sti = stoll(msg.at(5));

    // Prepare RRC Setup
    m_logger->info("RRC Setup for UE[%d]", ue->ueId);

    string pdu = "RRCSetup|" + to_string(ue->GNB_DU_UE_ID) + "|" + to_string(getNextTid());

    auto w = std::make_unique<NmCURrcToF1ap>(NmCURrcToF1ap::SEND_MESSAGE);
    w->duId = duId;
    w->rrcChannel = rrc::RrcChannel::DL_CCCH;
    w->data = pdu;
    m_base->f1apTask->push(std::move(w));
}

void CURrcTask::receiveRrcSetupComplete(int duId, int gNB_DU_ID, string data)
{
    auto msg = split(data, '|');

    auto DU_UE_ID = stoi(msg.at(1));
    auto *ue = findUeByGNB_DU_UE_ID(gNB_DU_ID, DU_UE_ID);

    ue->state = ERrcState::RRC_CONNECTED;

    // Send Measurement Report
    string pdu = "UECapabilityEnquiry|" + to_string(ue->GNB_DU_UE_ID) + "|" + to_string(getNextTid());

    auto w = std::make_unique<NmCURrcToF1ap>(NmCURrcToF1ap::SEND_MESSAGE);
    w->duId = duId;
    w->rrcChannel = rrc::RrcChannel::DL_DCCH;
    w->data = pdu;
    m_base->f1apTask->push(std::move(w));
}

} // namespace nr::CU