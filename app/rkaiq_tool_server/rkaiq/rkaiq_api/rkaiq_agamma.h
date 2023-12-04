#ifndef _TOOL_RKAIQ_API_AGAMMA_H_
#define _TOOL_RKAIQ_API_AGAMMA_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_agamma.h"
#include "rkaiq_socket.h"

class RKAiqToolAGamma
{
  public:
    RKAiqToolAGamma();
    virtual ~RKAiqToolAGamma();

    int SetAttrib(const rk_aiq_gamma_attrib_t attr, int cmdID);
    int GetAttrib(rk_aiq_gamma_attrib_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_AGAMMA_H_
