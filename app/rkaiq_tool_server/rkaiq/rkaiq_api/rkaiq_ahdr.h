#ifndef _TOOL_RKAIQ_API_AHDR_H_
#define _TOOL_RKAIQ_API_AHDR_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_ahdr.h"
#include "rkaiq_socket.h"

class RKAiqToolAHDR
{
  public:
    RKAiqToolAHDR();
    virtual ~RKAiqToolAHDR();

    int SetAttrib(const ahdr_attrib_t attr, int cmdID);
    int GetAttrib(ahdr_attrib_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_AHDR_H_
