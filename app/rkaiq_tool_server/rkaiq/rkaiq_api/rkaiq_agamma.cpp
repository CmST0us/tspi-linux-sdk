#include "rkaiq_agamma.h"

RKAiqToolAGamma::RKAiqToolAGamma()
{
}

RKAiqToolAGamma::~RKAiqToolAGamma()
{
}

int RKAiqToolAGamma::SetAttrib(const rk_aiq_gamma_attrib_t attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&attr, sizeof(rk_aiq_gamma_attrib_t));
}

int RKAiqToolAGamma::GetAttrib(rk_aiq_gamma_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_gamma_attrib_t));
}