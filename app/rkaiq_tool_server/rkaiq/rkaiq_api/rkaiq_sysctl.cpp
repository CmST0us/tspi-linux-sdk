#include "rkaiq_sysctl.h"

#include "rkaiq_socket.h"

RKAiqToolSysCtl::RKAiqToolSysCtl()
{
}

RKAiqToolSysCtl::~RKAiqToolSysCtl()
{
}

int RKAiqToolSysCtl::SetCpsLtCfg(rk_aiq_cpsl_cfg_t* cfg, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)cfg, sizeof(rk_aiq_cpsl_cfg_t));
}

int RKAiqToolSysCtl::GetCpsLtInfo(rk_aiq_cpsl_info_t* info, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)info, sizeof(rk_aiq_cpsl_info_t));
}

int RKAiqToolSysCtl::QueryCpsLtCap(rk_aiq_cpsl_cap_t* cap, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)cap, sizeof(rk_aiq_cpsl_cap_t));
}

int RKAiqToolSysCtl::SetWorkingModeDyn(rk_aiq_working_mode_t mode, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&mode, sizeof(rk_aiq_working_mode_t));
}

int RKAiqToolSysCtl::GetVersionInfo(rk_aiq_ver_info_t* vers, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)vers, sizeof(rk_aiq_ver_info_t));
}

int RKAiqToolSysCtl::EnumStaticMetas(rk_aiq_static_info_t* static_info, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)static_info, sizeof(rk_aiq_static_info_t));
}

int RKAiqToolSysCtl::EnqueueRKRawFile(char* filePath, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)filePath, strlen(filePath));
}