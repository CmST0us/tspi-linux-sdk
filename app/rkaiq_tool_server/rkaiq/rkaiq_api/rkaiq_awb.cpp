#include "rkaiq_awb.h"

RKAiqToolAWB::RKAiqToolAWB()
{
}

RKAiqToolAWB::~RKAiqToolAWB()
{
}

int RKAiqToolAWB::SetAttrib(rk_aiq_wb_attrib_t attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&attr, sizeof(rk_aiq_wb_attrib_t));
}

int RKAiqToolAWB::GetAttrib(rk_aiq_wb_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_wb_attrib_t));
}

int RKAiqToolAWB::GetCCT(rk_aiq_wb_cct_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_wb_cct_t));
}

int RKAiqToolAWB::QueryWBInfo(rk_aiq_wb_querry_info_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_wb_querry_info_t));
}