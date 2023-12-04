
#include "config.h"
#if ENABLE_ALL
#include "adebayer/rk_aiq_uapi_adebayer_int.h"
#include "../protocol/rk_aiq_user_api_adebayer_ptl.h"
#include "rk_aiq_user_api_sysctl.h"

XCamReturn
rk_aiq_user_api_adebayer_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, adebayer_attrib_t attr) {
   CLIENT_CALL_SET_AIQ(rk_aiq_user_api_adebayer_SetAttrib);
}

#endif

