
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_ahdr.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_ahdr_ptl.h"

XCamReturn
rk_aiq_user_api_ahdr_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ahdr_attrib_t attr) {
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ahdr_SetAttrib);
    return 0;
}
#endif
