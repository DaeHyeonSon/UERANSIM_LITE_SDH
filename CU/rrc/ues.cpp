//
// This file is a part of UERANSIM open source project.
// Copyright (c) 2021 ALİ GÜNGÖR.
//
// The software and all associated files are licensed under GPL-3.0
// and subject to the terms and conditions defined in LICENSE file.
//

#include "task.hpp"

#include <lib/rrc/encode.hpp>

namespace nr::CU
{

RrcUeContext *CURrcTask::createUe(int id)
{
    auto *ctx = new RrcUeContext(id);
    m_ueCtx[id] = ctx;
    return ctx;
}

RrcUeContext *CURrcTask::tryFindUe(int id)
{
    if (m_ueCtx.count(id))
        return m_ueCtx[id];
    return nullptr;
}

RrcUeContext *CURrcTask::findUe(int id)
{
    auto *ue = tryFindUe(id);
    if (ue == nullptr)
    {
        m_logger->err("UE context with ID[%d] not found", id);
        return ue;
    }
    return ue;
}

RrcUeContext *CURrcTask::findUeByGNB_DU_UE_ID(int gNB_DU_ID, int GNB_DU_UE_ID)
{
    RrcUeContext *ue = nullptr;

    for (auto& it : m_ueCtx)
    {
        if((it.second->gNB_DU_ID == gNB_DU_ID) and (it.second->GNB_DU_UE_ID == GNB_DU_UE_ID))
        {
            ue = it.second;
            break;
        }
    }

    if (ue == nullptr)
    {
        m_logger->err("UE context with DU[%d], DU_UE_ID[%d] not found", gNB_DU_ID, GNB_DU_UE_ID);
        return ue;
    }

    return ue;
}

RrcUeContext *CURrcTask::findUeBySti(int64_t sti)
{
    RrcUeContext *ue = nullptr;

    for (auto& it : m_ueCtx)
    {
        if(it.second->sti == sti)
        {
            ue = it.second;
            break;
        }
    }

    if (ue == nullptr)
    {
        m_logger->err("UE context with sti[%d] not found", sti);
        return ue;
    }

    return ue;
}

} // namespace nr::CU