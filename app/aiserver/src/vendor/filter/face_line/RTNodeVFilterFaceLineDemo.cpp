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
#include <math.h>
#include <stdlib.h>

#include "RTNodeVFilterFaceLineDemo.h"          // NOLINT
#include "RTNodeCommon.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RTNodeVFilter"
#define kStubRockitFaceLineDemo                MKTAG('f', 'a', 'l', 'i')
#define UVC_DYNAMIC_DEBUG_USE_TIME_CHECK "/tmp/uvc_use_time"


RTNodeVFilterFaceLine::RTNodeVFilterFaceLine()
{
    mLock = new RtMutex();
    RT_ASSERT(RT_NULL != mLock);
}

RTNodeVFilterFaceLine::~RTNodeVFilterFaceLine()
{
    rt_safe_delete(mLock);
}

/* method to keep osd start*/
INT32 RTNodeVFilterFaceLine::detect_rcd_list_init() {
  for (int i = 0; i < MAX_DRAW_NUM; i++) {
    memset(&mFaceAiDataList.face_data[i], 0, sizeof(FaceData));
    mFaceAiDataList.face_data[i].id = -1;
  }
  return 0;
}

INT32 RTNodeVFilterFaceLine::clear_detect_rcd() {
  INT32 ret = 0;
  for (int i = 0; i < MAX_DRAW_NUM; i++) {
    if (mFaceAiDataList.face_data[i].id > 0 && mFaceAiDataList.face_data[i].activate <= 0) {
      memset(&mFaceAiDataList.face_data[i], 0, sizeof(FaceData));
      mFaceAiDataList.face_data[i].id = -1;
    } else if (mFaceAiDataList.face_data[i].id > 0 && mFaceAiDataList.face_data[i].activate > 0) {
      ret = 1;
    }
  }
  return ret;
}

INT32 RTNodeVFilterFaceLine::update_detect_rcd(INT32 id, INT32 top, INT32 bottom, INT32 left, INT32 right,
                            INT32 x_rate, INT32 y_rate, INT32 w_max, INT32 h_max,
                            INT32 bAllowNew) {
  INT32 empty_order = -1;
  INT32 match_order = -1;
  INT32 ret = 0;
  for (INT32 i = 0; i < MAX_DRAW_NUM; i++) {
    if (mFaceAiDataList.face_data[i].id == id) {
      match_order = i;
      break;
    } else if (bAllowNew && mFaceAiDataList.face_data[i].id < 0 && empty_order < 0) {
      empty_order = i;
    }
  }
  if (match_order < 0 && empty_order < 0) {
    if (bAllowNew)
      RT_LOGD("over max rcd num:%d\n", MAX_DRAW_NUM);
    return -1;
  } else if (match_order < 0) {
    match_order = empty_order;
  }
  mFaceAiDataList.face_data[match_order].activate = DETECT_ACTIVE_CNT;
  mFaceAiDataList.face_data[match_order].id = id;
  if (abs(top - mFaceAiDataList.face_data[match_order].top) > DETECT_RECT_DIFF ||
      abs(bottom - mFaceAiDataList.face_data[match_order].bottom) > DETECT_RECT_DIFF ||
      abs(left - mFaceAiDataList.face_data[match_order].left) > DETECT_RECT_DIFF ||
      abs(right - mFaceAiDataList.face_data[match_order].right) > DETECT_RECT_DIFF) {
    mFaceAiDataList.face_data[match_order].top = top;
    mFaceAiDataList.face_data[match_order].bottom = bottom;
    mFaceAiDataList.face_data[match_order].left = left;
    mFaceAiDataList.face_data[match_order].right = right;
    int x = left * x_rate;
    int y = top * y_rate;
    int w = (right - left) * x_rate;
    int h = (bottom - top) * y_rate;
    if (x < 0)
      x = 0;
    if (y < 0)
      y = 0;
    while ((x + w) >= w_max) {
      w -= 16;
    }
    while ((y + h) >= h_max) {
      h -= 16;
    }
    mFaceAiDataList.face_data[match_order].width = UPALIGNTO(w, 2);
    mFaceAiDataList.face_data[match_order].height = UPALIGNTO(h, 2);
    mFaceAiDataList.face_data[match_order].x = UPALIGNTO(x, 2);
    mFaceAiDataList.face_data[match_order].y = UPALIGNTO(y, 2);
    ret = 1;
  }
  return ret;
}

/* method to keep osd end */
INT32 RTNodeVFilterFaceLine::rga_nv12_detect(rga_buffer_t buf, INT32 x, INT32 y,
                                 INT32 width, INT32 height, INT32 line_pixel,
                                 INT32 color) {
  im_rect rect_up = {x, y, width, line_pixel};
  im_rect rect_buttom = {x, y + height - line_pixel, width, line_pixel};
  im_rect rect_left = {x, y, line_pixel, height};
  im_rect rect_right = {x + width - line_pixel, y, line_pixel, height};
  imfill(buf, rect_up, color);
  imfill(buf, rect_buttom, color);
  imfill(buf, rect_left, color);
  imfill(buf, rect_right, color);
  return 0;
}
RT_RET RTNodeVFilterFaceLine::open(RTTaskNodeContext *context)
{
    RtMetaData* inputMeta   = context->options();
    RT_RET err              = RT_OK;

    RT_ASSERT(inputMeta->findInt32("opt_clip_width", &mSrcWidth));
    RT_ASSERT(inputMeta->findInt32("opt_clip_height", &mSrcHeight));

    mClipRatioW =  mSrcWidth/640;
    mClipRatioH =  mSrcHeight/360;
    RT_LOGD("hjc face_info src_wh [%d %d] ,ratio_wh[%f %f]\n",
            mSrcWidth, mSrcHeight,mClipRatioW,mClipRatioH);
    mNeedDraw = 0;
    mFaceAiDataList.face_data = (FaceData *)malloc(MAX_DRAW_NUM * sizeof(FaceData));

    return RT_OK;
}

RT_RET RTNodeVFilterFaceLine::process(RTTaskNodeContext *context)
{
    RT_RET         err       = RT_OK;
    RTMediaBuffer *srcBuffer = RT_NULL;
    RTMediaBuffer *dstBuffer = RT_NULL;

    // 此处是上级NN人脸检测节点输出人脸区域信息，SDK默认数据流路径是scale1->NN->faceline
    if (context->hasInputStream("image:rect"))
    {
        INT32 count = context->inputQueueSize("image:rect");
        if (count == 0)
        {
            FaceAiData face_ai_data;
            face_ai_data.face_count = 0;
        }
        while (count)
        {
            dstBuffer = context->dequeInputBuffer("image:rect");
            if (dstBuffer == RT_NULL)
                continue;

            count--;

            void* result = getAIDetectResults(dstBuffer);
            RTRknnAnalysisResults *nnResult  = reinterpret_cast<RTRknnAnalysisResults *>(result);
            if (nnResult != RT_NULL)
            {
                RTRect result;
                INT32 faceCount = nnResult->counter;
                RT_RET ret = RT_OK;
                detect_rcd_list_init();
                mFaceAiDataList.face_count = faceCount;
                if (mFaceAiDataList.face_data)
                {
                    for (int i = 0; i<faceCount; i++)
                    {
                        int update_rcd_ret = 0;
                        if (nnResult->results[i].face_info.object.score <= 0.40f) {
                            update_rcd_ret = update_detect_rcd(
                                nnResult->results[i].face_info.object.id, nnResult->results[i].face_info.object.box.top,
                                nnResult->results[i].face_info.object.box.bottom,
                                nnResult->results[i].face_info.object.box.left, nnResult->results[i].face_info.object.box.right,
                                mClipRatioW, mClipRatioH, mSrcWidth,mSrcHeight, RT_FALSE);
                        } else {
                            update_rcd_ret = update_detect_rcd(
                                nnResult->results[i].face_info.object.id, nnResult->results[i].face_info.object.box.top,
                                nnResult->results[i].face_info.object.box.bottom,
                                nnResult->results[i].face_info.object.box.left, nnResult->results[i].face_info.object.box.right,
                                mClipRatioW, mClipRatioH, mSrcWidth,mSrcHeight, RT_TRUE);
                        }
                        if (0)
                           RT_LOGD("update_board_rcd: %d\n", update_rcd_ret);

                        mFaceAiDataList.face_data[i].score = nnResult->results[i].face_info.object.score;
                    }
                    mNeedDraw = clear_detect_rcd();
                }
            }
            dstBuffer->release();
        }
    }
    else
    {
        RT_LOGE("don't has nn data stream!");
    }

    // 此处是用于预览的原始YUV数据，SDK默认数据流路径是bypass->EPTZ->RGA(输出给下级节点裁剪)
    INT32 count = context->inputQueueSize("image:nv12");
    while (count) {
        srcBuffer = context->dequeInputBuffer("image:nv12");
        if (srcBuffer == RT_NULL)
            continue;
        count--;

        // draw
        if (mNeedDraw) {
            rga_buffer_t src = wrapbuffer_fd(
            srcBuffer->getFd(), mSrcWidth,mSrcHeight, RK_FORMAT_YCbCr_420_SP);
            for (int k = 0; k < MAX_DRAW_NUM; k++) {
                if (mFaceAiDataList.face_data[k].id >= 0 && mFaceAiDataList.face_data[k].activate) {
                    rga_nv12_detect(src, mFaceAiDataList.face_data[k].x, mFaceAiDataList.face_data[k].y,
                          mFaceAiDataList.face_data[k].width, mFaceAiDataList.face_data[k].height, 4,
                          255);
                    mFaceAiDataList.face_data[k].activate--;
                }
           }
        }
        INT32 streamId = context->getInputInfo()->streamId();
        RtMetaData *inputMeta = srcBuffer->extraMeta(streamId);
        int64_t pts = 0;
        int32_t seq = 0;
        inputMeta->findInt64(kKeyFramePts, &pts);
        inputMeta->findInt32(kKeyFrameSequence, &seq);

        streamId = context->getOutputInfo()->streamId();
        if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0)) {
            int32_t use_time_us, now_time_us;
            struct timespec now_tm = {0, 0};
            clock_gettime(CLOCK_MONOTONIC, &now_tm);
            now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us
            use_time_us = now_time_us - pts;
            RT_LOGE("isp->aiserver seq:%ld latency time:%d us, %d ms\n",seq, use_time_us, use_time_us / 1000);
        }
        dstBuffer = srcBuffer;
        context->queueOutputBuffer(dstBuffer);
    }
    return err;
}
RT_RET RTNodeVFilterFaceLine::invokeInternal(RtMetaData *meta)
{
    const char *command;
    int enable = 0;
    if ((RT_NULL == meta))
    {
        return RT_ERR_NULL_PTR;
    }

    RtMutex::RtAutolock autoLock(mLock);
    meta->findCString(kKeyPipeInvokeCmd, &command);
    RT_LOGD("invoke(%s) internally.", command);
    RTSTRING_SWITCH(command)
    {
        RTSTRING_CASE("set_faceline_config"):
            if (meta->findInt32("enable", &enable))
        {
            if (enable == 0)
            {
              RT_LOGD("set_faceline_config enable=%d", enable);
            }
        }
        break;
    default:
        RT_LOGD("unsupported command=%d", command);
        break;
    }
    return RT_OK;
}

RT_RET RTNodeVFilterFaceLine::close(RTTaskNodeContext *context)
{
    RT_RET err = RT_OK;
    if (mFaceAiDataList.face_data)
        free(mFaceAiDataList.face_data);

    return err;
}

static RTTaskNode* createFaceLineFilter()
{
    return new RTNodeVFilterFaceLine();
}

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_face_line_demo
{
    .mUid          = kStubRockitFaceLineDemo,
    .mName         = "faceline",
    .mVersion      = "v1.0",
    .mCreateObj    = createFaceLineFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL}, },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_face_line_demo);
