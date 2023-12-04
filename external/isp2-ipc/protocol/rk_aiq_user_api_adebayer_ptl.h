#ifndef _RK_AIQ_USER_API_ADEBAYER_PTL_H_
#define _RK_AIQ_USER_API_ADEBAYER_PTL_H_

//#include "adebayer/rk_aiq_uapi_adebayer_int.h"
#include "rk_aiq_user_api_adebayer.h"

typedef struct rk_aiq_user_api_adebayer_SetAttrib {
    rk_aiq_sys_ctx_t* sys_ctx;
    adebayer_attrib_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_adebayer_SetAttrib_t;

#endif
