#ifndef _TOOL_RKAIQ_API_SHARP_H_
#define _TOOL_RKAIQ_API_SHARP_H_

#include "rk_aiq_user_api_asharp.h"
#include "rkaiq_cmdid.h"

class RKAiqToolSharp
{
  public:
    RKAiqToolSharp();
    virtual ~RKAiqToolSharp();

    int SetAttrib(rk_aiq_sharp_attrib_t* attr, int cmdID);
    int GetAttrib(rk_aiq_sharp_attrib_t* attr, int cmdID);
    int SetIQPara(void* para, int cmdID);
    int GetIQPara(void* para, int cmdID);
    int SetIQEFPara(rk_aiq_sharp_IQpara_t* para, int cmdID);
    int GetIQEFPara(rk_aiq_sharp_IQpara_t* para, int cmdID);
    int SetStrength(float fPercent, int cmdID);
    int GetStrength(float* pPercent, int cmdID);
};

#endif // _TOOL_RKAIQ_API_SHARP_H_
