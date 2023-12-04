#ifndef _TOOL_RKAIQ_API_AWB_H_
#define _TOOL_RKAIQ_API_AWB_H_

#include "rk_aiq_user_api_awb.h"
#include "rkaiq_socket.h"

class RKAiqToolAWB
{
  public:
    RKAiqToolAWB();
    virtual ~RKAiqToolAWB();

    int SetAttrib(rk_aiq_wb_attrib_t attr, int cmdID);
    int GetAttrib(rk_aiq_wb_attrib_t* attr, int cmdID);
    int GetCCT(rk_aiq_wb_cct_t* attr, int cmdID);
    int QueryWBInfo(rk_aiq_wb_querry_info_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_AWB_H_
