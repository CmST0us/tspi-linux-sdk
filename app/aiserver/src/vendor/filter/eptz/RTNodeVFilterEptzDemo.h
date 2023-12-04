/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTEREPTZDEMO_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTEREPTZDEMO_H_

#include "RTTaskNode.h"
#include "RTMediaRockx.h"
#include "RTAIDetectResults.h"
#include "eptz_algorithm.h"

class RTNodeVFilterEptz : public RTTaskNode {
 public:
    RTNodeVFilterEptz();
    virtual ~RTNodeVFilterEptz();

    virtual RT_RET open(RTTaskNodeContext *context);
    virtual RT_RET process(RTTaskNodeContext *context);
    virtual RT_RET close(RTTaskNodeContext *context);
 protected:
    virtual RT_RET invokeInternal(RtMetaData *meta);

 private:
    RtMutex        *mLock;
    RTRect          mRoiRegion;
    INT32           mSrcWidth;
    INT32           mSrcHeight;
    INT32           mEptzWidth;
    INT32           mEptzHeight;
    float           mClipRatio;
    INT32           mClipWidth;
    INT32           mClipHeight;
    INT32           mLastXY[4];
    INT32           mTempXY[4];
    INT32           mSequeFrame;
    INT32           mSequeEptz;
    EptzInitInfo    mEptzInfo;
};

#endif  // SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTEREPTZDEMO_H_
