#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_a3dlut_ipc.h"
#include "../../protocol/rk_aiq_user_api_a3dlut_ptl.h"


XCamReturn
rk_aiq_user_api_a3dlut_SetAttrib_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_a3dlut_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_a3dlut_GetAttrib_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_a3dlut_GetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_a3dlut_Query3dlutInfo_ipc(void* args){
    CALL_GET_AIQ(rk_aiq_user_api_a3dlut_Query3dlutInfo);
    return 0;
}

#endif
