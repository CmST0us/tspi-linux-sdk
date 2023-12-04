
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_accm_ptl.h"

XCamReturn
rk_aiq_user_api_accm_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ccm_attrib_t attr) {
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_accm_SetAttrib);
}

XCamReturn
rk_aiq_user_api_accm_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ccm_attrib_t *attr) {
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_accm_GetAttrib);
}

XCamReturn
rk_aiq_user_api_accm_QueryCcmInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_ccm_querry_info_t *ccm_querry_info) {
    rk_aiq_ccm_querry_info_t *attr = ccm_querry_info;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_accm_QueryCcmInfo);
}
#endif
