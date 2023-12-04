#ifndef _TOOL_RKAIQ_API_ENGINE_H_
#define _TOOL_RKAIQ_API_ENGINE_H_

#include <memory>
#include <thread>

#include "camera_memory.h"
#include "logger/log.h"
#include "rkaiq_imgproc.h"
#include "rkaiq_media.h"

typedef enum rk_aiq_state_e
{
    AIQ_STATE_INVALID = 0,
    AIQ_STATE_INITIALIZED = 1,
    AIQ_STATE_PREPARED = 2,
    AIQ_STATE_RUNNING = 3,
    AIQ_STATE_STOPPED = 4,
    AIQ_STATE_MAX
} rk_aiq_state_t;

class RKAiqEngine
{
  public:
    RKAiqEngine();
    virtual ~RKAiqEngine();
    static void RKAiqEngineLoop(void* arg);
    int InitEngine();
    int InitEngine(int mode);
    int StartEngine();
    int StopEngine();
    int DeInitEngine();

    friend class RKAiqToolManager;

  private:
    std::string iqfiles_path_;
    rk_aiq_working_mode_t mode_;
    std::string sensor_entity_name_;
    int width_;
    int height_;
};

#endif // _TOOL_RKAIQ_API_ENGINE_H_