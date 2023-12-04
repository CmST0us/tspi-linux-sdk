#include "rkaiq_anr.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

RKAiqToolANR::RKAiqToolANR()
{
}

RKAiqToolANR::~RKAiqToolANR()
{
}

int RKAiqToolANR::SetAttrib(rk_aiq_nr_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)attr, sizeof(rk_aiq_nr_attrib_t));
}

int RKAiqToolANR::GetAttrib(rk_aiq_nr_attrib_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(rk_aiq_nr_attrib_t));
}

int RKAiqToolANR::SetIQPara(void* attr, int cmdID)
{
    int paramSize = 0;
    if (cmdID == ENUM_ID_ANR_SETBAYERNRATTR) {
        paramSize = sizeof(CalibDb_BayerNr_t);
    } else if (cmdID == ENUM_ID_ANR_SETMFNRATTR) {
        paramSize = sizeof(CalibDb_MFNR_t);
    } else if (cmdID == ENUM_ID_ANR_SETUVNRATTR) {
        paramSize = sizeof(CalibDb_UVNR_t);
    } else if (cmdID == ENUM_ID_ANR_SETYNRATTR) {
        paramSize = sizeof(CalibDb_YNR_t);
    } else {
        return 1;
    }
    return RkAiqSocketClientINETSend(cmdID, attr, paramSize);
}

int RKAiqToolANR::GetIQPara(void* attr, int cmdID)
{
    int paramSize = 0;
    if (cmdID == ENUM_ID_ANR_GETBAYERNRATTR) {
        paramSize = sizeof(CalibDb_BayerNr_t);
    } else if (cmdID == ENUM_ID_ANR_GETMFNRATTR) {
        paramSize = sizeof(CalibDb_MFNR_t);
    } else if (cmdID == ENUM_ID_ANR_GETUVNRATTR) {
        paramSize = sizeof(CalibDb_UVNR_t);
    } else if (cmdID == ENUM_ID_ANR_GETYNRATTR) {
        paramSize = sizeof(CalibDb_YNR_t);
    } else {
        return 1;
    }
    return RkAiqSocketClientINETReceive(cmdID, attr, paramSize);
}

int RKAiqToolANR::SetLumaSFStrength(float fPercnt, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&fPercnt, sizeof(float));
}

int RKAiqToolANR::GetLumaSFStrength(float* pPercnt, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pPercnt, sizeof(float));
}

int RKAiqToolANR::SetLumaTFStrength(float fPercnt, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&fPercnt, sizeof(float));
}

int RKAiqToolANR::GetLumaTFStrength(float* pPercnt, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pPercnt, sizeof(float));
}

int RKAiqToolANR::SetChromaSFStrength(float fPercnt, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&fPercnt, sizeof(float));
}

int RKAiqToolANR::GetChromaSFStrength(float* pPercnt, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pPercnt, sizeof(float));
}

int RKAiqToolANR::SetChromaTFStrength(float fPercnt, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&fPercnt, sizeof(float));
}

int RKAiqToolANR::GetChromaTFStrength(float* pPercnt, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pPercnt, sizeof(float));
}

int RKAiqToolANR::SetRawnrSFStrength(float fPercnt, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&fPercnt, sizeof(float));
}

int RKAiqToolANR::GetRawnrSFStrength(float* pPercnt, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)pPercnt, sizeof(float));
}
