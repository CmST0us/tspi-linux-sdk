#include "rkaiq_cproc.h"

RKAiqToolCPROC::RKAiqToolCPROC()
{
}

RKAiqToolCPROC::~RKAiqToolCPROC()
{
}

int RKAiqToolCPROC::SetAttrib(acp_attrib_t attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&attr, sizeof(acp_attrib_t));
}

int RKAiqToolCPROC::GetAttrib(acp_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(acp_attrib_t));
}