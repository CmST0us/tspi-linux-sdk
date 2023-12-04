
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_ae_ipc.h"
#include "../../protocol/rk_aiq_user_api_ae_ptl.h"

#ifdef __cplusplus
extern "C"
{
#endif

XCamReturn rk_aiq_user_api_ae_setExpSwAttr_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_ae_setExpSwAttr);
    return 0;
}
XCamReturn rk_aiq_user_api_ae_getExpSwAttr_ipc(void* args){
    //TOD QIUEN
    CALL_GET_AIQ(rk_aiq_user_api_ae_getExpSwAttr);
    return 0;
}
XCamReturn rk_aiq_user_api_ae_queryExpResInfo_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_ae_queryExpResInfo);
    return 0;
}

XCamReturn rk_aiq_user_api_ae_setLinExpAttr_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_ae_setLinExpAttr);
    return 0;
}
XCamReturn rk_aiq_user_api_ae_getLinExpAttr_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_ae_getLinExpAttr);
    return 0;
}

XCamReturn rk_aiq_user_api_ae_setHdrExpAttr_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_ae_setHdrExpAttr);
    return 0;
}

XCamReturn rk_aiq_user_api_ae_getHdrExpAttr_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_ae_getHdrExpAttr);
    return 0;
}


#ifdef __cplusplus
}
#endif


#endif


