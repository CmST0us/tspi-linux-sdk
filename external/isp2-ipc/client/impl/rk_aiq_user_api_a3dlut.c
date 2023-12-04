#include "config.h"
#if ENABLE_ALL
#include "a3dlut/rk_aiq_uapi_a3dlut_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_a3dlut_ptl.h"

XCamReturn
rk_aiq_user_api_a3dlut_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_attrib_t attr) {
   CLIENT_CALL_SET_AIQ(rk_aiq_user_api_a3dlut_SetAttrib);
}
XCamReturn
rk_aiq_user_api_a3dlut_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_attrib_t *attr) {
   CLIENT_CALL_GET_AIQ(rk_aiq_user_api_a3dlut_GetAttrib); 
}
XCamReturn
rk_aiq_user_api_a3dlut_Query3dlutInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_lut3d_querry_info_t *lut3d_querry_info) {
   rk_aiq_lut3d_querry_info_t *attr = lut3d_querry_info;
   CLIENT_CALL_GET_AIQ(rk_aiq_user_api_a3dlut_Query3dlutInfo); 
}
#endif

