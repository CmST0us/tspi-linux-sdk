#include "config.h"
#if ENABLE_ALL
#include "config.h"
#include "rk_aiq_user_api_agamma_ipc.h"
#include "../../protocol/rk_aiq_user_api_agamma_ptl.h"

XCamReturn
rk_aiq_user_api_agamma_SetAttrib_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_agamma_SetAttrib);
    return 0;
}

XCamReturn
rk_aiq_user_api_agamma_GetAttrib_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_agamma_GetAttrib);
    return 0;
}

#endif