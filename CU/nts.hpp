//
// Created by Hoonyong Park on 5/11/23.
//
#pragma once

#include "types.hpp"

#include <utility>

#include <lib/app/cli_base.hpp>
#include <lib/app/cli_cmd.hpp>
#include <lib/asn/utils.hpp>
#include <lib/rls/rls_base.hpp>
#include <lib/rrc/rrc.hpp>
#include <lib/sctp/sctp.hpp>
#include <utils/network.hpp>
#include <utils/nts.hpp>
#include <utils/octet_string.hpp>
#include <utils/unique_buffer.hpp>

extern "C"
{
    struct ASN_NGAP_FiveG_S_TMSI;
    struct ASN_NGAP_TAIListForPaging;
}

namespace nr::CU
{

struct NmCUF1apToRrc : NtsMessage
{
    enum PR
    {
        RECEIVE_CCCH_Message,
        RECEIVE_DCCH_Message
    } present;

    int duId{};
    int gNB_DU_ID{};

    std::string data;

    explicit NmCUF1apToRrc(PR present) : NtsMessage(NtsMessageType::CU_F1AP_TO_RRC), present(present)
    {

    }
};

struct NmCURrcToF1ap : NtsMessage
{
    enum PR
    {
        SEND_MESSAGE,
        UE_CONTEXT_SETUP_REQUEST,
        UE_CONTEXT_RELEASE_COMMAND,
    } present;

    // SEND_MESSAGE
    // UE_CONTEXT_SETUP_REQUEST
    int duId{};

    // SEND_MESSAGE
    rrc::RrcChannel rrcChannel{};

    // SEND_MESSAGE
    // UE_CONTEXT_SETUP_REQUEST
    std::string data;

    // UE_CONTEXT_SETUP_REQUEST
    int ueId{};
    int GNB_DU_UE_ID;
    long sourcePCI;
    long targetPCI;

    explicit NmCURrcToF1ap(PR present) : NtsMessage(NtsMessageType::CU_RRC_TO_F1AP), present(present)
    {

    }
};

struct NmCUSctp : NtsMessage
{
    enum PR
    {
        CONNECTION_REQUEST,
        CONNECTION_CLOSE,
        ASSOCIATION_SETUP,
        ASSOCIATION_SHUTDOWN,
        RECEIVE_MESSAGE,
        SEND_MESSAGE,
        UNHANDLED_NOTIFICATION,
    } present;

    // CONNECTION_REQUEST
    // CONNECTION_CLOSE
    // ASSOCIATION_SETUP
    // ASSOCIATION_SHUTDOWN
    // RECEIVE_MESSAGE
    // SEND_MESSAGE
    // UNHANDLED_NOTIFICATION
    int clientId{};

    // CONNECTION_REQUEST
    std::string localAddress{};
    uint16_t localPort{};
    std::string remoteAddress{};
    uint16_t remotePort{};
    sctp::PayloadProtocolId ppid{};
    NtsTask *associatedTask{};

    // ASSOCIATION_SETUP
    int associationId{};
    int inStreams{};
    int outStreams{};

    // RECEIVE_MESSAGE
    // SEND_MESSAGE
    UniqueBuffer buffer{};
    uint16_t stream{};

    explicit NmCUSctp(PR present) : NtsMessage(NtsMessageType::CU_SCTP), present(present)
    {
    }
};

struct NmCUCliCommand : NtsMessage
{
    std::unique_ptr<app::CUCliCommand> cmd;
    InetAddress address;

    NmCUCliCommand(std::unique_ptr<app::CUCliCommand> cmd, InetAddress address)
        : NtsMessage(NtsMessageType::CU_CLI_COMMAND), cmd(std::move(cmd)), address(address)
    {
    }
};

} // namespace nr::CU