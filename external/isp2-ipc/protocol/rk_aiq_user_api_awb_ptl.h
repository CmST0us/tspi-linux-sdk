#ifndef _RK_AIQ_USER_API_AWB_PTL_H_
#define _RK_AIQ_USER_API_AWB_PTL_H_

//#include "awb/rk_aiq_uapi_awb_int.h"
#include "rk_aiq_user_api_awb.h"

typedef struct rk_aiq_user_api_awb_SetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_attrib_t attr;
	XCamReturn returnvalue;
} rk_aiq_user_api_awb_SetAttrib_t;

typedef struct rk_aiq_user_api_awb_GetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_attrib_t attr;
	XCamReturn returnvalue;
} rk_aiq_user_api_awb_GetAttrib_t;

typedef struct rk_aiq_user_api_awb_GetCCT {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_cct_t attr;
	XCamReturn returnvalue;
} rk_aiq_user_api_awb_GetCCT_t;

typedef struct rk_aiq_user_api_awb_QueryWBInfo {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_wb_querry_info_t attr;
	XCamReturn returnvalue;
} rk_aiq_user_api_awb_QueryWBInfo_t;

#endif
