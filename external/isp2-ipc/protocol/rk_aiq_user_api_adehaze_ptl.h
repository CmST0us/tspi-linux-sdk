#ifndef _RK_AIQ_USER_API_ADEHAZE_PTL_H_
#define _RK_AIQ_USER_API_ADEHAZE_PTL_H_

//#include "adehaze/rk_aiq_uapi_adehaze_int.h"
#include "rk_aiq_user_api_adehaze.h"

typedef struct rk_aiq_user_api_adehaze_setSwAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    adehaze_sw_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_adehaze_setSwAttrib_t;

typedef struct rk_aiq_user_api_adehaze_getSwAttrib {
    rk_aiq_sys_ctx_t*  sys_ctx;
    adehaze_sw_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_adehaze_getSwAttrib_t;

#endif

