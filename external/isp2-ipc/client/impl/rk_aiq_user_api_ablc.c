
#include "config.h"
#if ENABLE_ALL
#include "ablc/rk_aiq_uapi_ablc_int.h"
#include "../protocol/rk_aiq_user_api_ablc_ptl.h"
#include "rk_aiq_user_api_sysctl.h"


XCamReturn
rk_aiq_user_api_ablc_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, \
                               rk_aiq_blc_attrib_t *attr) {
    CLIENT_CALL_SET_AIQ_P(rk_aiq_user_api_ablc_SetAttrib);
}
XCamReturn
rk_aiq_user_api_ablc_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx,\
                                rk_aiq_blc_attrib_t *attr) {
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ablc_GetAttrib); 
}
#endif


