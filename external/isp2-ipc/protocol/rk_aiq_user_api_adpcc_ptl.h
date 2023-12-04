#ifndef _RK_AIQ_USER_API_ADPCC_PTL_H_
#define _RK_AIQ_USER_API_ADPCC_PTL_H_

//#include "adpcc/rk_aiq_uapi_adpcc_int.h"
#include "rk_aiq_user_api_adpcc.h"
typedef struct rk_aiq_user_api_adpcc_SetAttrib {
    rk_aiq_sys_ctx_t*  sys_ctx;
    rk_aiq_dpcc_attrib_t attr;
	XCamReturn returnvalue;
} rk_aiq_user_api_adpcc_SetAttrib_t;

typedef struct rk_aiq_user_api_adpcc_GetAttrib {
    rk_aiq_sys_ctx_t*    sys_ctx;
    rk_aiq_dpcc_attrib_t attr;
	XCamReturn returnvalue;
} rk_aiq_user_api_adpcc_GetAttrib_t;

#endif

