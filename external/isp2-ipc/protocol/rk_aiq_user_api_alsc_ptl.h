#ifndef _RK_AIQ_USER_API_ALSC_PTL_H_
#define _RK_AIQ_USER_API_ALSC_PTL_H_

//#include "alsc/rk_aiq_uapi_alsc_int.h"
#include "rk_aiq_user_api_alsc.h"
typedef struct rk_aiq_user_api_alsc_SetAttrib {
    rk_aiq_sys_ctx_t*  sys_ctx;
    rk_aiq_lsc_attrib_t attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_alsc_SetAttrib_t;

typedef struct rk_aiq_user_api_alsc_GetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_lsc_attrib_t attr;
    XCamReturn  returnvalue;

} rk_aiq_user_api_alsc_GetAttrib_t;

typedef struct rk_aiq_user_api_alsc_QueryLscInfo {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_lsc_querry_info_t attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_alsc_QueryLscInfo_t;


#endif
