#ifndef _RK_AIQ_USER_API_AGAMMA_PTL_H_
#define _RK_AIQ_USER_API_AGAMMA_PTL_H_

//#include "agamma/rk_aiq_uapi_agamma_int.h"
#include "rk_aiq_user_api_agamma.h"
typedef struct rk_aiq_user_api_agamma_SetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_gamma_attrib_t attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_agamma_SetAttrib_t;

typedef struct rk_aiq_user_api_agamma_GetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_gamma_attrib_t attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_agamma_GetAttrib_t;

#endif
