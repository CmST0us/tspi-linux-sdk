
#include "config.h"

#if ENABLE_ALL
#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_agamma_ptl.h"

XCamReturn
rk_aiq_user_api_agamma_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_gamma_attrib_t attr) {
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_agamma_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_agamma_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_gamma_attrib_t *attr) {
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_agamma_GetAttrib); 
    return 0;
}
#endif

