#include "rkaiq_ahdr.h"

RKAiqToolAHDR::RKAiqToolAHDR()
{
}

RKAiqToolAHDR::~RKAiqToolAHDR()
{
}

int RKAiqToolAHDR::SetAttrib(const ahdr_attrib_t attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&attr, sizeof(ahdr_attrib_t));
}

int RKAiqToolAHDR::GetAttrib(ahdr_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(ahdr_attrib_t));
}