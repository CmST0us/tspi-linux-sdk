
#include "config.h"
#if ENABLE_ALL
#include "adehaze/rk_aiq_uapi_adehaze_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_adehaze_ptl.h"

XCamReturn  rk_aiq_user_api_adehaze_setSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_t attr) {
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_adehaze_setSwAttrib);
}


XCamReturn  rk_aiq_user_api_adehaze_getSwAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adehaze_sw_t *attr) {
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_adehaze_getSwAttrib);
}
#endif













