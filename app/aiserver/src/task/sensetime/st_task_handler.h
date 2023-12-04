// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_ST_TASK_HANDLER_H_
#define _RK_ST_TASK_HANDLER_H_

#include <mutex>
#include <stdint.h>

#include "ai_task_handler.h"
#include "shm_control_nn.h"
#include "kk_data.pb.h"
#include "nn_vision_rockx.h"
#include "st_asteria_api.h"
#include "st_asteria_common.h"

namespace rockchip {
namespace aiserver {

typedef struct {
    int32_t picID;
    int32_t faceID;
    int32_t repeatCnt;
    int32_t sended;
    int64_t recTime;
    int32_t dataSize;
    unsigned char *faceData;
    RTKKMattingFaceInfo *faceInfo;
} MattingFaceHolder;

class STTaskHandler : public AITaskHandler {
 public:
    STTaskHandler();
    virtual ~STTaskHandler();

    virtual int32_t processAIData(RTMediaBuffer *buffer);
    virtual int32_t processAIMatting(RTMediaBuffer *buffer);
    virtual int32_t processAIFeature(RTMediaBuffer *buffer);
    virtual int32_t convertDetectType(int32_t detectType);

  private:
    void    postNNData(void *nnResult);
    void    pushDetectInfo(KKAIData *mNNData, STDetectResult *detectResult, int32_t size);

    void    postAIMattingData(void *mattingBuffer, void *imgData);
    void    doPostMattingFace(MattingFaceHolder *holder);
    void    freeMattingFaceInfo(MattingFaceHolder* holder);
    MattingFaceHolder* saveMattingFaceInfo(RTKKMattingFaceInfo *faceInfo, void *imgData);

    void    postFeatureData(void *nnResult, const char *uuid);
    void    postEmptyFeatureData(const char* uuid);

  private:
    std::mutex       mOpMutex;
    ShmNNController *mShmNNcontroller;

};

} // namespace aiserver
} // namespace rockchip


#endif // _RK_ST_TASK_HANDLER_H_

