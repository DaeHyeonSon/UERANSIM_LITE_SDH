//
// Created by Hoonyong Park on 5/10/23.
//

#pragma once

#include <set>
#include <Eigen/Dense>

#include <lib/app/monitor.hpp>
#include <lib/asn/utils.hpp>
#include <utils/common_types.hpp>
#include <utils/logger.hpp>
#include <utils/network.hpp>
#include <utils/nts.hpp>
#include <utils/octet_string.hpp>

#include <utils/scoped_thread.hpp>
#include <asn/ngap/ASN_NGAP_QosFlowSetupRequestList.h>
#include <asn/rrc/ASN_RRC_InitialUE-Identity.h>

namespace nr::CU
{

class CUAppTask;
class CURrcTask;
class F1apTask;
class SctpServerTask;

struct SctpAssociation
{
    int associationId{};
    int inStreams{};
    int outStreams{};
};

struct F1apDuContext
{
    int ctxId{};
    int32_t gNB_DU_ID{};
//    int cellID;
};

enum class ERrcState
{
    RRC_IDLE,
    RRC_CONNECTED,
    RRC_CONNECTED_HANDOVER,
    RRC_INACTIVE,
};

struct RrcUeContext
{
    const int ueId{};

    int32_t gNB_DU_ID{};
    int GNB_DU_UE_ID{};
    int64_t initialId = -1; // 39-bit value, or -1
    bool isInitialIdSTmsi{}; // TMSI-part-1 or a random value
    int64_t establishmentCause{};
    std::optional<GutiMobileIdentity> sTmsi{};
    ERrcState state{};

    int64_t sti;


    explicit RrcUeContext(const int ueId) : ueId(ueId)
    {
    }
};

struct CUConfig
{
    /* Read from config file */
    int64_t nci{};     // 36-bit
    int CUIdLength{}; // 22..32 bit
    Plmn plmn{};
    int tac{};
    NetworkSlice nssai{};
    std::string f1apIp{};
    uint16_t f1apPort{};
    bool ignoreStreamIds{};

    /* Assigned by program */
    std::string name{};
    EPagingDrx pagingDrx{};
    Vector3 phyLocation{};

    [[nodiscard]] inline uint32_t getCUId() const
    {
        return static_cast<uint32_t>((nci & 0xFFFFFFFFFLL) >> (36LL - static_cast<int64_t>(CUIdLength)));
    }

    [[nodiscard]] inline int getCellId() const
    {
        return static_cast<int>(nci & static_cast<uint64_t>((1 << (36 - CUIdLength)) - 1));
    }
};

struct TaskBase
{
    CUConfig *config{};
    LogBase *logBase{};
    app::INodeListener *nodeListener{};
    NtsTask *cliCallbackTask{};

    CUAppTask *appTask{};
    CURrcTask *rrcTask{};
    F1apTask *f1apTask{};
    SctpServerTask *sctpServerTask{};
    //CURlsTask *rlsTask{};
};

Json ToJson(const CUConfig &v);
Json ToJson(const EPagingDrx &v);
Json ToJson(const SctpAssociation &v);

} // namespace nr::CU