#ifndef _RK_AIQ_USER_API_ABLC_PTL_H_
#define _RK_AIQ_USER_API_ABLC_PTL_H_

#include "ablc/rk_aiq_types_ablc_algo_int.h"
#include "rk_aiq_user_api_sysctl.h"

typedef struct
    rk_aiq_user_api_ablc_SetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_blc_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ablc_SetAttrib_t;

typedef struct
    rk_aiq_user_api_ablc_GetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_blc_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ablc_GetAttrib_t;

#endif
