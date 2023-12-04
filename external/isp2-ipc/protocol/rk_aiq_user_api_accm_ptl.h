#ifndef _RK_AIQ_USER_API_PTL_ACCM_H_
#define _RK_AIQ_USER_API_PTL_ACCM_H_

#include "accm/rk_aiq_uapi_accm_int.h"
#include "rk_aiq_user_api_accm.h"

typedef struct rk_aiq_user_api_accm_SetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_ccm_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_accm_SetAttrib_t;

typedef struct rk_aiq_user_api_accm_GetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_ccm_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_accm_GetAttrib_t;

typedef struct rk_aiq_user_api_accm_QueryCcmInfo {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_ccm_querry_info_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_accm_QueryCcmInfo_t;

#endif
