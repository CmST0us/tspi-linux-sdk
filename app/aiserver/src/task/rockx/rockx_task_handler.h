// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_ROCKX_TASK_HANDLER_H_
#define _RK_ROCKX_TASK_HANDLER_H_

#include <mutex>
#include <stdint.h>

#include "ai_task_handler.h"
#include "shm_control_nn.h"
#include "nn_data.pb.h"

namespace rockchip {
namespace aiserver {

class RockxTaskHandler : public AITaskHandler {
 public:
    RockxTaskHandler();
    virtual ~RockxTaskHandler();

    virtual int32_t processAIData(RTMediaBuffer *buffer);
    virtual int32_t processAIMatting(RTMediaBuffer *buffer);
    virtual int32_t processAIFeature(RTMediaBuffer *buffer);
    virtual int32_t convertDetectType(int32_t detectType);

  private:
    void    postNNData(void *nnResult);
    void    pushFaceDetectInfo(NNData *nnData, void *bufptr, int32_t size);
    void    pushPoseBodyInfo(NNData *nnData, void *bufptr, int32_t size);
    void    pushLandMarkInfo(NNData *nnData, void *bufptr, int32_t size);
    void    pushFingerDetectInfo(NNData *nnData, void *bufptr, int32_t size);
    int32_t setNNGeneralInfos(NNData *nnData, void *bufptr, int32_t size);

  private:
    std::mutex       opMutex;
    ShmNNController *mShmNNcontroller;
};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_ROCKX_TASK_HANDLER_H_

