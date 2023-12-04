#include "rkaiq_ccm.h"

RKAiqToolCCM::RKAiqToolCCM()
{
}

RKAiqToolCCM::~RKAiqToolCCM()
{
}

int RKAiqToolCCM::SetAttrib(rk_aiq_ccm_attrib_t attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&attr, sizeof(rk_aiq_ccm_attrib_t));
}

int RKAiqToolCCM::GetAttrib(rk_aiq_ccm_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_ccm_attrib_t));
}

int RKAiqToolCCM::QueryCCMInfo(rk_aiq_ccm_querry_info_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_ccm_querry_info_t));
}