#ifndef _RK_AIQ_USER_API_AHDR_PTL_H_
#define _RK_AIQ_USER_API_AHDR_PTL_H_

//#include "ahdr/rk_aiq_uapi_ahdr_int.h"
#include "rk_aiq_user_api_ahdr.h"
typedef struct rk_aiq_user_api_ahdr_SetAttrib {
    rk_aiq_sys_ctx_t*  sys_ctx;
    ahdr_attrib_t attr;
    XCamReturn  returnvalue;
} rk_aiq_user_api_ahdr_SetAttrib_t;

#endif
