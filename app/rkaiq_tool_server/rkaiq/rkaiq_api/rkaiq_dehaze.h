#ifndef _TOOL_RKAIQ_API_DEHAZE_H_
#define _TOOL_RKAIQ_API_DEHAZE_H_

#include "rk_aiq_user_api_adehaze.h"
#include "rkaiq_socket.h"

class RKAiqToolDehaze
{
  public:
    RKAiqToolDehaze();
    virtual ~RKAiqToolDehaze();

    int SetAttrib(adehaze_sw_t attr, int cmdID);
    int GetAttrib(adehaze_sw_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_DEHAZE_H_
