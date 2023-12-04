// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_AI_TASK_FACTORY_H_
#define _RK_AI_TASK_FACTORY_H_

#include "ai_task_handler.h"

namespace rockchip {
namespace aiserver {

class AITaskFactory {
  public:
    static AITaskHandler* createHandler();

};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_AI_TASK_FACTORY_H_

