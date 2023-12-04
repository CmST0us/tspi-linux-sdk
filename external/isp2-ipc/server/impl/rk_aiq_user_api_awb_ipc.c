
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_awb_ipc.h"
#include "../../protocol/rk_aiq_user_api_awb_ptl.h"



XCamReturn
rk_aiq_user_api_awb_SetAttrib_ipc(void *args){

    CALL_SET_AIQ(rk_aiq_user_api_awb_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_awb_GetAttrib_ipc(void *args){
    CALL_GET_AIQ(rk_aiq_user_api_awb_GetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_awb_GetCCT_ipc(void *args){
    
    CALL_GET_AIQ(rk_aiq_user_api_awb_GetCCT);
    return 0;
}
XCamReturn
rk_aiq_user_api_awb_QueryWBInfo_ipc(void *args){

    CALL_GET_AIQ(rk_aiq_user_api_awb_QueryWBInfo);
    return 0;
}

#endif