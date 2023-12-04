// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_AI_SCENE_DIRECTOR_H_
#define _RK_AI_SCENE_DIRECTOR_H_

#include "dbus_graph_control.h"
#include "ai_feature_retriver.h"
#include "ai_task_manager.h"
#include "shmc/shm_control_uvc.h"
#include "ai_uvc_graph.h"

namespace rockchip {
namespace aiserver {

typedef enum _RockxTaskMode {
    ROCKX_TASK_MODE_SINGLE = 0,
    ROCKX_TASK_MODE_COMPLEX,
    ROCKX_TASK_MODE_MAX,
} RockxTaskMode;

/*
 * 1. run task graph for generic ai scene.
 * 2. output NN vision result to SHMC(ipc)
 * 3. provide a minimalist interface to aiserver
 */
class AISceneDirector : public RTGraphListener {
 public:
    AISceneDirector();
    ~AISceneDirector();

 public:
    int32_t setup();
    int32_t prepareUVCGraph();
    int32_t interrupt();
    int32_t waitUntilDone();

 public:
    // overide RTGraphListener control task graph
    virtual int32_t start(const std::string &appName);
    virtual int32_t stop(const std::string &appName);
    virtual int32_t observeGraphOutput(const std::string &appName, const int32_t &enable);

    virtual int32_t setEPTZ(const AI_UVC_EPTZ_MODE &mode, const int32_t &enabled);
    virtual int32_t setZoom(const double &val);
    virtual int32_t setFaceAE(const int32_t &enabled);
    virtual int32_t setFaceLine(const int32_t &enabled);

    virtual int32_t enableAIAlgorithm(const std::string &type);
    virtual int32_t disableAIAlgorithm(const std::string &type);
    virtual int32_t updateAIAlgorithmParams(const std::string &type);

    virtual int32_t openAIMatting();
    virtual int32_t closeAIMatting();

    virtual int32_t invoke(const std::string &appName, const std::string &actionName, void *params);
    virtual int32_t ctrlSubGraph(const char* nnName, int32_t enable);

 private:
    int32_t invokeFeature(const std::string &actionName, void *params);
    int32_t invokeUVC(const std::string &actionName, void *params);

    RT_RET  nn_data_output_callback(RTMediaBuffer *buffer);
    RT_RET  ai_matting_output_callback(RTMediaBuffer *buffer);
    RT_RET  uvc_data_output_callback(RTMediaBuffer *buffer);

 public:
    ShmUVCController *mUVCController;
    AITaskManager    *mAITaskManager;

 private:
    std::mutex   mOpMutex;
    AIUVCGraph  *mUVCGraph;
    AIFeatureRetriver *mAIFeatureRetriver;
    int32_t      mUVCGraphRef         = 0;
    int32_t      mEnableUVC           = 0;
    int32_t      mEnableNN            = 0;
};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_AI_SCENE_DIRECTOR_H_



