#ifndef _TOOL_RKAIQ_API_BAYERNR_H_
#define _TOOL_RKAIQ_API_BAYERNR_H_

#include "logger/log.h"
#include "rk_aiq_user_api_anr.h"
#include "rkaiq_cmdid.h"
#include "rkaiq_socket.h"

class RKAiqToolANR
{
  public:
    RKAiqToolANR();
    virtual ~RKAiqToolANR();

    int SetAttrib(rk_aiq_nr_attrib_t* attr, int cmdID);
    int GetAttrib(rk_aiq_nr_attrib_t* attr, int cmdID);
    int SetIQPara(void* attr, int cmdID);
    int GetIQPara(void* attr, int cmdID);
    int SetLumaSFStrength(float fPercnt, int cmdID);
    int SetLumaTFStrength(float fPercnt, int cmdID);
    int GetLumaSFStrength(float* pPercnt, int cmdID);
    int GetLumaTFStrength(float* pPercnt, int cmdID);
    int SetChromaSFStrength(float fPercnt, int cmdID);
    int SetChromaTFStrength(float fPercnt, int cmdID);
    int GetChromaSFStrength(float* pPercnt, int cmdID);
    int GetChromaTFStrength(float* pPercnt, int cmdID);
    int SetRawnrSFStrength(float fPercnt, int cmdID);
    int GetRawnrSFStrength(float* pPercnt, int cmdID);
};

#endif // _TOOL_RKAIQ_API_BAYERNR_H_
