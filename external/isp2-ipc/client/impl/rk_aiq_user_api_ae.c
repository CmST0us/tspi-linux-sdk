
#include "config.h"
#if ENABLE_ALL
#include "ae/rk_aiq_uapi_ae_int.h"
#include "rk_aiq_user_api_sysctl.h"
#include "../protocol/rk_aiq_user_api_ae_ptl.h"

#ifdef __cplusplus
extern "C"
{
#endif

XCamReturn rk_aiq_user_api_ae_setExpSwAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_ExpSwAttr_t pExpSwAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_HdrAeRouteAttr_t *attr = &pExpSwAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setExpSwAttr);
}
XCamReturn rk_aiq_user_api_ae_getExpSwAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_ExpSwAttr_t* pExpSwAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_ExpSwAttr_t* attr = pExpSwAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getExpSwAttr);  
}

XCamReturn rk_aiq_user_api_ae_setLinAeDayRouteAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_LinAeRouteAttr_t pLinAeRouteAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_LinAeRouteAttr_t* attr = &pLinAeRouteAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setLinAeDayRouteAttr);
}
XCamReturn rk_aiq_user_api_ae_getLinAeDayRouteAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr) {
     rk_aiq_sys_ctx_t* sys_ctx = ctx;
     Uapi_LinAeRouteAttr_t*attr =  pLinAeRouteAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getLinAeDayRouteAttr); 
}
XCamReturn rk_aiq_user_api_ae_setLinAeNightRouteAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_LinAeRouteAttr_t pLinAeRouteAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_LinAeRouteAttr_t* attr = &pLinAeRouteAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setLinAeNightRouteAttr);
}
XCamReturn rk_aiq_user_api_ae_getLinAeNightRouteAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_LinAeRouteAttr_t* pLinAeRouteAttr) {
     rk_aiq_sys_ctx_t* sys_ctx = ctx;
     Uapi_LinAeRouteAttr_t*attr =  pLinAeRouteAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getLinAeNightRouteAttr); 
}
XCamReturn rk_aiq_user_api_ae_setHdrAeDayRouteAttr(const rk_aiq_sys_ctx_t* ctx, \
                                               const Uapi_HdrAeRouteAttr_t pHdrAeRouteAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_HdrAeRouteAttr_t* attr = &pHdrAeRouteAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setHdrAeDayRouteAttr);
}

XCamReturn rk_aiq_user_api_ae_getHdrAeDayRouteAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_HdrAeRouteAttr_t*attr =  pHdrAeRouteAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getHdrAeDayRouteAttr); 
}
XCamReturn rk_aiq_user_api_ae_setHdrAeNightRouteAttr(const rk_aiq_sys_ctx_t* ctx, \
                                               const Uapi_HdrAeRouteAttr_t pHdrAeRouteAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_HdrAeRouteAttr_t* attr = &pHdrAeRouteAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setHdrAeNightRouteAttr);
}

XCamReturn rk_aiq_user_api_ae_getHdrAeNightRouteAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_HdrAeRouteAttr_t* pHdrAeRouteAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_HdrAeRouteAttr_t*attr =  pHdrAeRouteAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getHdrAeNightRouteAttr); 
}
XCamReturn rk_aiq_user_api_ae_queryExpResInfo(const rk_aiq_sys_ctx_t* ctx, Uapi_ExpQueryInfo_t* pExpResInfo) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_ExpQueryInfo_t* attr = pExpResInfo;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_queryExpResInfo);
}

XCamReturn rk_aiq_user_api_ae_setLinExpAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_LinExpAttr_t pLinExpAttr) {
   rk_aiq_sys_ctx_t* sys_ctx = ctx;
   Uapi_LinExpAttr_t* attr = &pLinExpAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setLinExpAttr);
}
XCamReturn rk_aiq_user_api_ae_getLinExpAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_LinExpAttr_t* pLinExpAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_LinExpAttr_t*attr =  pLinExpAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getLinExpAttr);
}

XCamReturn rk_aiq_user_api_ae_setHdrExpAttr(const rk_aiq_sys_ctx_t* ctx, const Uapi_HdrExpAttr_t pHdrExpAttr) {
    rk_aiq_sys_ctx_t* sys_ctx = ctx;
    Uapi_HdrExpAttr_t*attr =  &pHdrExpAttr;
    CLIENT_CALL_SET_AIQ(rk_aiq_user_api_ae_setHdrExpAttr);
}
XCamReturn rk_aiq_user_api_ae_getHdrExpAttr(const rk_aiq_sys_ctx_t* ctx, Uapi_HdrExpAttr_t* pHdrExpAttr) {
   rk_aiq_sys_ctx_t* sys_ctx = ctx;
   Uapi_HdrExpAttr_t* attr = pHdrExpAttr;
    CLIENT_CALL_GET_AIQ(rk_aiq_user_api_ae_getHdrExpAttr);
}


#ifdef __cplusplus
}
#endif

#endif




