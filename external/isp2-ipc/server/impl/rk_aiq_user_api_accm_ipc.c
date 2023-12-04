
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_accm_ipc.h"
#include "../../protocol/rk_aiq_user_api_accm_ptl.h"

XCamReturn
rk_aiq_user_api_accm_SetAttrib_ipc(void *args){
    CALL_SET_AIQ(rk_aiq_user_api_accm_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_accm_GetAttrib_ipc(void *args){
    CALL_GET_AIQ(rk_aiq_user_api_accm_GetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_accm_QueryCcmInfo_ipc(void *args){
    CALL_GET_AIQ(rk_aiq_user_api_accm_QueryCcmInfo);    
    return 0;
}

#endif