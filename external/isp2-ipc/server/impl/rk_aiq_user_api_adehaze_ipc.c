
#include "config.h"
#if ENABLE_ALL
#include "rk_aiq_user_api_adehaze_ipc.h"
#include "../../protocol/rk_aiq_user_api_adehaze_ptl.h"


XCamReturn  rk_aiq_user_api_adehaze_setSwAttrib_ipc(void *args){
    CALL_SET_AIQ(rk_aiq_user_api_adehaze_setSwAttrib);
    return 0;
}


XCamReturn  rk_aiq_user_api_adehaze_getSwAttrib_ipc(void *args){
   CALL_GET_AIQ(rk_aiq_user_api_adehaze_getSwAttrib);
    return 0;
}


#endif










