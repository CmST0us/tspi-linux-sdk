// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_AI_TASK_HANDLER_H_
#define _RK_AI_TASK_HANDLER_H_

#include "RTMediaBuffer.h"

namespace rockchip {
namespace aiserver {

class AITaskHandler {
 public:
    AITaskHandler() {}
    virtual ~AITaskHandler() {}

    virtual int32_t processAIData(RTMediaBuffer *buffer) = 0;
    virtual int32_t processAIMatting(RTMediaBuffer *buffer) = 0;
    virtual int32_t processAIFeature(RTMediaBuffer *buffer) = 0;
    virtual int32_t convertDetectType(int32_t detectType) = 0;

};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_AI_TASK_HANDLER_H_
