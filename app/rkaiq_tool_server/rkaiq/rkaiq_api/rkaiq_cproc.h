#ifndef _TOOL_RKAIQ_API_CPROC_H_
#define _TOOL_RKAIQ_API_CPROC_H_

#include "rk_aiq_user_api_acp.h"
#include "rkaiq_socket.h"

class RKAiqToolCPROC
{
  public:
    RKAiqToolCPROC();
    virtual ~RKAiqToolCPROC();

    int SetAttrib(acp_attrib_t attr, int cmdID);
    int GetAttrib(acp_attrib_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_CPROC_H_
