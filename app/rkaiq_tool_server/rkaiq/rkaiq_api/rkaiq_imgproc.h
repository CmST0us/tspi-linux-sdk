#ifndef _TOOL_RKAIQ_API_IMGPROC_H_
#define _TOOL_RKAIQ_API_IMGPROC_H_

#include "rk_aiq_user_api_imgproc.h"
#include "rkaiq_socket.h"

class RKAiqToolImgProc
{
  public:
    RKAiqToolImgProc();
    virtual ~RKAiqToolImgProc();

    int SetGrayMode(rk_aiq_gray_mode_t mode, int cmdID);
    int GetGrayMode(rk_aiq_gray_mode_t* mode, int cmdID);
};

#endif // _TOOL_RKAIQ_API_IMGPROC_H_
