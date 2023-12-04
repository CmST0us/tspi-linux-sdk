// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_AI_TASK_MANAGER_H_
#define _RK_AI_TASK_MANAGER_H_

#include "ai_task_handler.h"

namespace rockchip {
namespace aiserver {

class AITaskManager {
  public:
    AITaskManager();
    virtual ~AITaskManager();

    int32_t processAIData(RTMediaBuffer *buffer);
    int32_t processAIMatting(RTMediaBuffer *buffer);
    int32_t processAIFeature(RTMediaBuffer *buffer);
    int32_t convertDetectType(int32_t detectType);

  private:
    AITaskHandler *mTaskHandler;

};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_AI_TASK_MANAGER_H_

