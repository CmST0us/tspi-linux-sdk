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

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERZOOM_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERZOOM_H_

#include "RTTaskNode.h"

class RTNodeVFilterZoom : public RTTaskNode {
 public:
    RTNodeVFilterZoom();
    virtual ~RTNodeVFilterZoom();

    virtual RT_RET open(RTTaskNodeContext *context);
    virtual RT_RET process(RTTaskNodeContext *context);
    virtual RT_RET close(RTTaskNodeContext *context);

 private:
    RtMutex        *mLock;
    RTRect          mRoiRegion;
    INT32           mEptzOffsetX;
    INT32           mEptzOffsetY;
    INT32           mEptzWidth;
    INT32           mEptzHeight;
    INT32           mSrcWidth;
    INT32           mSrcHeight;
    INT32           mDstWidth;
    INT32           mDstHeight;
    float           mZoomValue;
    float           mZoomValueNow;
    INT32           mPanValue;
    INT32           mPanValueNow;
    INT32           mTiltValue;
    INT32           mTiltValueNow;
    bool            mIsZoomSet;
    INT32           mResult[4];
    void            RTZoomCalculate();

protected:
    virtual RT_RET invokeInternal(RtMetaData *meta);
};

#endif  // SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERZOOM_H_
