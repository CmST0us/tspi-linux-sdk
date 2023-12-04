#ifndef _RK_AIQ_USER_API_A3DLUT_PTL_H_
#define _RK_AIQ_USER_API_A3DLUT_PTL_H_

#include "a3dlut/rk_aiq_uapi_a3dlut_int.h"
#include "rk_aiq_user_api_a3dlut.h"
typedef struct
    rk_aiq_user_api_a3dlut_SetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_lut3d_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_a3dlut_SetAttrib_t;

typedef struct
    rk_aiq_user_api_a3dlut_GetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_lut3d_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_a3dlut_GetAttrib_t;

typedef struct
    rk_aiq_user_api_a3dlut_Query3dlutInfo {
    rk_aiq_sys_ctx_t* sys_ctx;
    rk_aiq_lut3d_querry_info_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_a3dlut_Query3dlutInfo_t;

#endif
