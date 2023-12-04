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

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFACEAEDEMO_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFACEAEDEMO_H_

#include "RTTaskNode.h"
#include "RTMediaRockx.h"
#include "RTAIDetectResults.h"
#include "faceae_type.h"

class RTNodeVFilterFaceAE : public RTTaskNode
{
public:
    RTNodeVFilterFaceAE();
    virtual ~RTNodeVFilterFaceAE();

    virtual RT_RET open(RTTaskNodeContext *context);
    virtual RT_RET process(RTTaskNodeContext *context);
    virtual RT_RET close(RTTaskNodeContext *context);
protected:
    virtual RT_RET invokeInternal(RtMetaData *meta);


private:
    INT32           mSrcWidth;
    INT32           mSrcHeight;
    INT32           mVirWidth;
    INT32           mVirHeight;
    INT32           mEvbias;
    INT32           mFastMoveCount = 0;
    INT32           mNoPersonCount = 0;
    FaceAeInitInfo    mFaceAeInfo;
    RtMutex         *mLock;
    RT_RET calculatePersonRect(FaceAeAiData *faceae_ai_data,
                               FaceAeRect *result_person);
    RT_RET calculateResultRect(FaceAeRect rect);
    RT_RET faceAE(FaceAeAiData *faceae_ai_data, int delay_time);
    RT_RET doIspProcess(const char* buf,int evbias);
};

#endif  // SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFACEAEDEMO_H_
