#include "rkaiq_imgproc.h"

RKAiqToolImgProc::RKAiqToolImgProc()
{
}

RKAiqToolImgProc::~RKAiqToolImgProc()
{
}

int RKAiqToolImgProc::SetGrayMode(rk_aiq_gray_mode_t mode, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&mode, sizeof(rk_aiq_gray_mode_t));
}

int RKAiqToolImgProc::GetGrayMode(rk_aiq_gray_mode_t* mode, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)mode, sizeof(rk_aiq_gray_mode_t));
}
