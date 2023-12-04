
#include "config.h"
#if ENABLE_ALL
#include "asharp/rk_aiq_uapi_asharp_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_asharp_ptl.h"

XCamReturn
rk_aiq_user_api_asharp_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_t *attr) {
    CLIENT_CALL_SET_AIQ_P(rk_aiq_user_api_asharp_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_asharp_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_sharp_attrib_t *attr) {
      CLIENT_CALL_GET_AIQ(rk_aiq_user_api_asharp_GetAttrib);  
    return 0;
}
#endif


