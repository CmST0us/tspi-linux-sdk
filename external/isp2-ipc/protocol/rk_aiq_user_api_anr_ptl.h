#ifndef _RK_AIQ_USER_API_ANR_PTL_H_
#define _RK_AIQ_USER_API_ANR_PTL_H_

//#include "anr/rk_aiq_uapi_anr_int.h"
#include "rk_aiq_user_api_anr.h"
typedef struct rk_aiq_user_api_anr_SetAttrib {
    rk_aiq_sys_ctx_t*   sys_ctx;
    rk_aiq_nr_attrib_t attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_anr_SetAttrib_t;

typedef struct rk_aiq_user_api_anr_GetAttrib {
    rk_aiq_sys_ctx_t*  sys_ctx;
    rk_aiq_nr_attrib_t  attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_anr_GetAttrib_t;

#endif

