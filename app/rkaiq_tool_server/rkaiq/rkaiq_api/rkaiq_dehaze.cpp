#include "rkaiq_dehaze.h"

RKAiqToolDehaze::RKAiqToolDehaze()
{
}

RKAiqToolDehaze::~RKAiqToolDehaze()
{
}

int RKAiqToolDehaze::SetAttrib(adehaze_sw_t attr, int cmdID)
{
    return RkAiqSocketClientINETSend(cmdID, (void*)&attr, sizeof(adehaze_sw_t));
}

int RKAiqToolDehaze::GetAttrib(adehaze_sw_t* attr, int cmdID)
{
    return RkAiqSocketClientINETReceive(cmdID, (void*)attr, sizeof(adehaze_sw_t));
}