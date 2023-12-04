// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_AI_FEATURE_RETRIVER_H_
#define _RK_AI_FEATURE_RETRIVER_H_

#include <mutex>
#include "ai_task_manager.h"
#include "RTAIGraph.h"

#define PRELOAD_HANDLE_FEATURE   1
#define PRELOAD_HANDLE_AI        1

namespace rockchip {
namespace aiserver {

class AIFeatureRetriver {
 public:
    AIFeatureRetriver();
    ~AIFeatureRetriver();

 public:
    INT32  setup(AITaskManager *taskManager);
    INT32  start();
    INT32  stop();
    INT32  runTaskOnce(void *params);

 private:
    INT32  preload();
    RT_RET ai_feature_output_callback(RTMediaBuffer *buffer);

 private:
    std::mutex     mOpMutex;
    RTAIGraph     *mAIGraph;
    AITaskManager *mAITaskManager;
};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_AI_FEATURE_RETRIVER_H_
