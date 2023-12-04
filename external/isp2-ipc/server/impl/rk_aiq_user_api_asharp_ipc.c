
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_asharp_ipc.h"
#include "../../protocol/rk_aiq_user_api_asharp_ptl.h"

XCamReturn
rk_aiq_user_api_asharp_SetAttrib_ipc(void *args){
    CALL_SET_AIQ_P(rk_aiq_user_api_asharp_SetAttrib);
    return 0;
}
XCamReturn
rk_aiq_user_api_asharp_GetAttrib_ipc(void *args){

    CALL_GET_AIQ(rk_aiq_user_api_asharp_GetAttrib);
    return 0;

}

#endif
