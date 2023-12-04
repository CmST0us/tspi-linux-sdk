
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_adebayer_ipc.h"
#include "../../protocol/rk_aiq_user_api_adebayer_ptl.h"

XCamReturn
rk_aiq_user_api_adebayer_SetAttrib_ipc(void* args){
    CALL_SET_AIQ(rk_aiq_user_api_adebayer_SetAttrib);
    return 0;
}

#endif