#ifndef _TOOL_RKAIQ_API_CCM_H_
#define _TOOL_RKAIQ_API_CCM_H_

#include "rk_aiq_user_api_accm.h"
#include "rkaiq_socket.h"

class RKAiqToolCCM
{
  public:
    RKAiqToolCCM();
    virtual ~RKAiqToolCCM();

    int SetAttrib(rk_aiq_ccm_attrib_t attr, int cmdID);
    int GetAttrib(rk_aiq_ccm_attrib_t* attr, int cmdID);
    int QueryCCMInfo(rk_aiq_ccm_querry_info_t* attr, int cmdID);
};

#endif // _TOOL_RKAIQ_API_CCM_H_
