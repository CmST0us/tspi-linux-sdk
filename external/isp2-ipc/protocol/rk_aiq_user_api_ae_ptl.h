#ifndef __RK_AIQ_USER_API_AE_PTL_H__
#define __RK_AIQ_USER_API_AE_PTL_H__

//#include "ae/rk_aiq_uapi_ae_int.h"
#include "rk_aiq_user_api_ae.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct rk_aiq_user_api_ae_setExpSwAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_ExpSwAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_setExpSwAttr_t;

typedef struct rk_aiq_user_api_ae_getExpSwAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_ExpSwAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getExpSwAttr_t;

typedef struct rk_aiq_user_api_ae_setLinAeDayRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
     Uapi_LinAeRouteAttr_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ae_setLinAeDayRouteAttr_t;

typedef struct rk_aiq_user_api_ae_getLinAeDayRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_LinAeRouteAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getLinAeDayRouteAttr_t;

typedef struct rk_aiq_user_api_ae_setLinAeNightRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
     Uapi_LinAeRouteAttr_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ae_setLinAeNightRouteAttr_t;

typedef struct rk_aiq_user_api_ae_getLinAeNightRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_LinAeRouteAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getLinAeNightRouteAttr_t;

typedef struct rk_aiq_user_api_ae_setHdrAeDayRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
     Uapi_HdrAeRouteAttr_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ae_setHdrAeDayRouteAttr_t;

typedef struct rk_aiq_user_api_ae_getHdrAeDayRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_HdrAeRouteAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getHdrAeDayRouteAttr_t;

typedef struct rk_aiq_user_api_ae_setHdrNightAeRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
     Uapi_HdrAeRouteAttr_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ae_setHdrAeNightRouteAttr_t;

typedef struct rk_aiq_user_api_ae_getHdrAeNightRouteAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_HdrAeRouteAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getHdrAeNightRouteAttr_t;

typedef struct rk_aiq_user_api_ae_queryExpResInfo {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_ExpQueryInfo_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_queryExpResInfo_t;

typedef struct rk_aiq_user_api_ae_setLinExpAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
     Uapi_LinExpAttr_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ae_setLinExpAttr_t;

typedef struct rk_aiq_user_api_ae_getLinExpAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_LinExpAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getLinExpAttr_t;

typedef struct rk_aiq_user_api_ae_setHdrExpAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
     Uapi_HdrExpAttr_t attr;
     XCamReturn returnvalue;
} rk_aiq_user_api_ae_setHdrExpAttr_t;

typedef struct rk_aiq_user_api_ae_getHdrExpAttr {
    rk_aiq_sys_ctx_t*   sys_ctx;
    Uapi_HdrExpAttr_t attr;
    XCamReturn returnvalue;
} rk_aiq_user_api_ae_getHdrExpAttr_t;

#ifdef __cplusplus
}
#endif
#endif /__RK_AIQ_USER_API_AE_H__/
