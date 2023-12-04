
#include "config.h"
#if ENABLE_ALL
#include "alsc/rk_aiq_uapi_alsc_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_alsc_ptl.h"

XCamReturn
rk_aiq_user_api_alsc_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lsc_attrib_t attr) {
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_alsc_SetAttrib);
}
XCamReturn
rk_aiq_user_api_alsc_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lsc_attrib_t *attr) {
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_alsc_GetAttrib); 
}
XCamReturn
rk_aiq_user_api_alsc_QueryLscInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lsc_querry_info_t *lsc_querry_info) {
    rk_aiq_lsc_querry_info_t *attr = lsc_querry_info;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_alsc_QueryLscInfo); 
}

#endif

