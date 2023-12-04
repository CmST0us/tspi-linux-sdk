
#include "config.h"
#if ENABLE_ALL
#include "awb/rk_aiq_uapi_awb_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_awb_ptl.h"

XCamReturn
rk_aiq_user_api_awb_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_attrib_t attr) {
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_awb_SetAttrib);
}
XCamReturn
rk_aiq_user_api_awb_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_attrib_t *attr) {
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_awb_GetAttrib); 
}
XCamReturn
rk_aiq_user_api_awb_GetCCT(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_cct_t *cct){
    rk_aiq_wb_cct_t *attr = cct;
   CLIENT_CALL_GET_AIQ(rk_aiq_user_api_awb_GetCCT); 
    return 0;
}
XCamReturn
rk_aiq_user_api_awb_QueryWBInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_querry_info_t *wb_querry_info) {
    rk_aiq_wb_querry_info_t *attr = wb_querry_info;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_awb_QueryWBInfo); 
    return 0;
}

#endif

