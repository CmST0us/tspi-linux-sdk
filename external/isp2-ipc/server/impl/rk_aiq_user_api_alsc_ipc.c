
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_alsc_ipc.h"
#include "../../protocol/rk_aiq_user_api_alsc_ptl.h"

XCamReturn
rk_aiq_user_api_alsc_SetAttrib_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_alsc_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_alsc_GetAttrib_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_alsc_GetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_alsc_QueryLscInfo_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_alsc_QueryLscInfo);
    return 0;
}

#endif