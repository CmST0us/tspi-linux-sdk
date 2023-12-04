#ifndef _TOOL_RKAIQ_API_SYSCTL_H_
#define _TOOL_RKAIQ_API_SYSCTL_H_

#include "rk_aiq.h"
#include "rk_aiq_user_api_sysctl.h"

class RKAiqToolSysCtl
{
  public:
    RKAiqToolSysCtl();
    virtual ~RKAiqToolSysCtl();

    int SetCpsLtCfg(rk_aiq_cpsl_cfg_t* cfg, int cmdID);
    int GetCpsLtInfo(rk_aiq_cpsl_info_t* info, int cmdID);
    int QueryCpsLtCap(rk_aiq_cpsl_cap_t* cap, int cmdID);
    int SetWorkingModeDyn(rk_aiq_working_mode_t mode, int cmdID);
    int GetVersionInfo(rk_aiq_ver_info_t* vers, int cmdID);
    int EnumStaticMetas(rk_aiq_static_info_t* static_info, int cmdID);
};

#endif // _TOOL_RKAIQ_API_SYSCTL_H_
