#ifndef __RK_AIQ_USER_API_AE_IPC_H__
#define __RK_AIQ_USER_API_AE_IPC_H__

#include "rk_aiq_user_api_ae.h"

#ifdef __cplusplus
extern "C"
{
#endif

XCamReturn rk_aiq_user_api_ae_setExpSwAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getExpSwAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_setLinAeDayRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getLinAeDayRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_setLinAeNightRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getLinAeNightRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_setHdrAeDayRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getHdrAeDayRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_setHdrAeNightRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getHdrAeNightRouteAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_queryExpResInfo_ipc(void* args);

XCamReturn rk_aiq_user_api_ae_setLinExpAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getLinExpAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_setHdrExpAttr_ipc(void* args);
XCamReturn rk_aiq_user_api_ae_getHdrExpAttr_ipc(void* args);


#ifdef __cplusplus
}
#endif





#endif /*__RK_AIQ_USER_API_AE_H__*/
