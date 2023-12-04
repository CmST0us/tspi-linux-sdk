/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFACELINEDEMO_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFACELINEDEMO_H_

#include "RTTaskNode.h"
#include "RTMediaRockx.h"
#include "RTAIDetectResults.h"
#include <unistd.h>
#include "face_line_type.h"

#include <rga/im2d.h>
#include <rga/rga.h>

#define DRAW_INDEX 0
#define RK_NN_INDEX 1
#define MAX_RKNN_LIST_NUM 10
#define UPALIGNTO(value, align) ((value + align - 1) & (~(align - 1)))
#define UPALIGNTO16(value) UPALIGNTO(value, 16)
#define MAX_DRAW_NUM 10
#define DETECT_RECT_DIFF 25
#define DETECT_ACTIVE_CNT 3

class RTNodeVFilterFaceLine : public RTTaskNode
{
public:
    RTNodeVFilterFaceLine();
    virtual ~RTNodeVFilterFaceLine();

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
    INT32           mLineSize;
    float           mClipRatioW;
    float           mClipRatioH;
    INT32           mNeedDraw;
    INT32           mSequeFrame;
    FaceAiData      mFaceAiDataList;
    RtMutex         *mLock;
    INT32 update_detect_rcd(INT32 id, INT32 top, INT32 bottom, INT32 left,
                        INT32 right,INT32 x_rate, INT32 y_rate,
                        INT32 w_max, INT32 h_max,INT32 bAllowNew);
    INT32 detect_rcd_list_init();
    INT32 clear_detect_rcd();
    INT32 rga_nv12_detect(rga_buffer_t buf, INT32 x, INT32 y,
                        INT32 width, INT32 height, INT32 line_pixel,INT32 color);
};

#endif  // SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFACELINEDEMO_H_
