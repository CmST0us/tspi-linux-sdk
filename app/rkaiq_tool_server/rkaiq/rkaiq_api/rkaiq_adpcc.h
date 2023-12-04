#ifndef _TOOL_RKAIQ_API_ADPCC_H_
#define _TOOL_RKAIQ_API_ADPCC_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_adpcc.h"
#include "rkaiq_socket.h"

class RKAiqToolADPCC
{
  public:
    RKAiqToolADPCC();
    virtual ~RKAiqToolADPCC();

    int SetAttrib(rk_aiq_dpcc_attrib_t* attr, int cmdID);
    int GetAttrib(rk_aiq_dpcc_attrib_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_ADPCC_H_
