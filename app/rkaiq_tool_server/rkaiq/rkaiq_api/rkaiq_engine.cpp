#include "rkaiq_engine.h"

#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG "aiqtool"

int RKAiqEngine::InitEngine()
{
    return 0;
}

int RKAiqEngine::InitEngine(int mode)
{
    if (mode == 0) {
        mode_ = RK_AIQ_WORKING_MODE_NORMAL;
    } else if (mode == 1) {
        mode_ = RK_AIQ_WORKING_MODE_ISP_HDR2;
    } else if (mode == 2) {
        mode_ = RK_AIQ_WORKING_MODE_ISP_HDR3;
    }
    InitEngine();
    return 0;
}

int RKAiqEngine::StartEngine()
{
    return 0;
}

int RKAiqEngine::StopEngine()
{
    return 0;
}

int RKAiqEngine::DeInitEngine()
{
    return 0;
}

RKAiqEngine::RKAiqEngine()
{
}

RKAiqEngine::~RKAiqEngine()
{
    StopEngine();
    DeInitEngine();
}
