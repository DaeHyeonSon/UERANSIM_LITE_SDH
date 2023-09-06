//
// Created by Hoonyong Park on 5/10/23.
//

#include <sstream>

#include <CU/types.hpp>
#include <utils/common.hpp>

namespace nr::CU
{


Json ToJson(const CUConfig &v)
{
    return Json::Obj({
        {"name", v.name},
        {"nci", v.nci},
        {"plmn", ToJson(v.plmn)},
        {"tac", v.tac},
        {"nssai", ToJson(v.nssai)},
        {"paging-drx", ToJson(v.pagingDrx)},
        {"ignore-sctp-id", v.ignoreStreamIds},
    });
}

Json ToJson(const EPagingDrx &v)
{
    switch (v)
    {
    case EPagingDrx::V32:
        return "v32";
    case EPagingDrx::V64:
        return "v64";
    case EPagingDrx::V128:
        return "v128";
    case EPagingDrx::V256:
        return "v256";
    default:
        return "?";
    }
}

Json ToJson(const SctpAssociation &v)
{
    return Json::Obj({{"id", v.associationId}, {"rx-num", v.inStreams}, {"tx-num", v.outStreams}});
}

} // namespace nr::CU
