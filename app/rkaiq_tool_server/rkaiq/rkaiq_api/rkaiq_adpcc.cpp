#include "rkaiq_adpcc.h"

RKAiqToolADPCC::RKAiqToolADPCC()
{
}

RKAiqToolADPCC::~RKAiqToolADPCC()
{
}

int RKAiqToolADPCC::SetAttrib(rk_aiq_dpcc_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)attr, sizeof(rk_aiq_dpcc_attrib_t));
}

int RKAiqToolADPCC::GetAttrib(rk_aiq_dpcc_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_dpcc_attrib_t));
}