
#include "config.h"
#if ENABLE_ALL
#include "config.h"
#include "rk_aiq_user_api_ahdr_ipc.h"
#include "../../protocol/rk_aiq_user_api_ahdr_ptl.h"

XCamReturn
rk_aiq_user_api_ahdr_SetAttrib_ipc(void *args){
    
    CALL_SET_AIQ(rk_aiq_user_api_ahdr_SetAttrib);
    return 0;
}


#endif