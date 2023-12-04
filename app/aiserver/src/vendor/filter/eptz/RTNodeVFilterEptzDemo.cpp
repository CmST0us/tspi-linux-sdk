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
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include "RTNodeVFilterEptzDemo.h"          // NOLINT
#include "RTNodeCommon.h"
#include "RTVideoFrame.h"
#include "RTTaskNodePrivKey.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RTNodeVFilterEptz"
#define kStubRockitEPTZDemo                MKTAG('e', 'p', 'd', 'm')
#define UVC_DYNAMIC_DEBUG_USE_TIME_CHECK "/tmp/uvc_use_time"
RTNodeVFilterEptz::RTNodeVFilterEptz() {
    mLock = new RtMutex();
    RT_ASSERT(RT_NULL != mLock);
}

RTNodeVFilterEptz::~RTNodeVFilterEptz() {
    rt_safe_delete(mLock);
}

RT_RET RTNodeVFilterEptz::open(RTTaskNodeContext *context) {
    RtMetaData* inputMeta   = context->options();
    RT_RET err              = RT_OK;

    RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_WIDTH, &mSrcWidth));
    RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_HEIGHT, &mSrcHeight));
    RT_ASSERT(inputMeta->findInt32(OPT_EPTZ_CLIP_WIDTH, &mClipWidth));
    RT_ASSERT(inputMeta->findInt32(OPT_EPTZ_CLIP_HEIGHT, &mClipHeight));

    mRoiRegion.x = 0;
    mRoiRegion.y = 0;
    mRoiRegion.w = mSrcWidth;
    mRoiRegion.h = mSrcHeight;

    mEptzInfo.eptz_src_width = mSrcWidth;
    mEptzInfo.eptz_src_height = mSrcHeight;
    mEptzInfo.eptz_dst_width = mSrcWidth;
    mEptzInfo.eptz_dst_height = mSrcHeight;
    mEptzInfo.camera_dst_width = mClipWidth;
    mEptzInfo.camera_dst_height = mClipHeight;
    //2K以上sensor建议使用1280x720数据,低于2K使用640x360
    mEptzInfo.eptz_npu_width = 640;
    mEptzInfo.eptz_npu_height = 360;
    //V2远距离建议0.4，V3近距离建议0.6
    mEptzInfo.eptz_facedetect_score_shold = 0.40;
    mEptzInfo.eptz_zoom_speed = 2;
    mEptzInfo.eptz_fast_move_frame_judge = 5;
    mEptzInfo.eptz_zoom_frame_judge = 10;
    mEptzInfo.eptz_threshold_x = 120;
    mEptzInfo.eptz_threshold_y = 100;
    mEptzInfo.eptz_animation = EPTZ_NONLINEAR;
    if (mEptzInfo.camera_dst_width >= 1920) {
        mEptzInfo.eptz_iterate_x = 12;
        mEptzInfo.eptz_iterate_y = 6;
    } else if (mEptzInfo.camera_dst_width >= 1280) {
        mEptzInfo.eptz_iterate_x = 10;
        mEptzInfo.eptz_iterate_y = 5;
    } else {
        mEptzInfo.eptz_iterate_x = 8;
        mEptzInfo.eptz_iterate_y = 4;
    }
    RT_LOGD("eptz_info src_wh [%d %d] dst_wh[%d %d], threshold_xy[%d %d] "
           "iterate_xy[%d %d] ratio[%.2f] \n",
           mEptzInfo.eptz_src_width, mEptzInfo.eptz_src_height,
           mEptzInfo.eptz_dst_width, mEptzInfo.eptz_dst_height, mEptzInfo.eptz_threshold_x,
           mEptzInfo.eptz_threshold_y, mEptzInfo.eptz_iterate_x, mEptzInfo.eptz_iterate_y, mClipRatio);

    mLastXY[0] = 0;
    mLastXY[1] = 0;
    mLastXY[2] = mEptzInfo.eptz_dst_width;
    mLastXY[3] = mEptzInfo.eptz_dst_height;
    eptzConfigInit(&mEptzInfo);
    mSequeFrame = 0;
    mSequeEptz = 0;

    return RT_OK;
}

RT_RET RTNodeVFilterEptz::process(RTTaskNodeContext *context) {
    RT_RET         err       = RT_OK;
    RTMediaBuffer *srcBuffer = RT_NULL;
    RTMediaBuffer *dstBuffer = RT_NULL;
    RtMutex::RtAutolock autoLock(mLock);

    if (!access("/tmp/eptz_mode1", 0)){
        system("rm /tmp/eptz_mode1");
        mEptzInfo.eptz_fast_move_frame_judge = 5;
        mEptzInfo.eptz_zoom_frame_judge = 10;
        setEptzMode(1);
    }

    if (!access("/tmp/eptz_mode2", 0)){
        system("rm /tmp/eptz_mode2");
        mEptzInfo.eptz_fast_move_frame_judge = 10;
        mEptzInfo.eptz_zoom_frame_judge = 15;
        setEptzMode(2);
    }

    // 此处是上级NN人脸检测节点输出人脸区域信息，SDK默认数据流路径是scale1->NN->EPTZ
    if (context->hasInputStream("image:rect")) {
        INT32 count = context->inputQueueSize("image:rect");
        if(count == 0 && mSequeEptz < mSequeFrame){
            EptzAiData eptz_ai_data;
            eptz_ai_data.face_count = 0;
            calculateClipRect(&eptz_ai_data, mLastXY, true, 5);
            mSequeEptz++;
        }
        while (count) {
            dstBuffer = context->dequeInputBuffer("image:rect");
            if (dstBuffer == RT_NULL)
                continue;

            count--;
            void* result = getAIDetectResults(dstBuffer);
            RTRknnAnalysisResults *nnResult  = reinterpret_cast<RTRknnAnalysisResults *>(result);
            if (nnResult != RT_NULL && mSequeEptz < mSequeFrame) {
                RTRect result;
                INT32 faceCount = nnResult->counter;
                RT_RET ret = RT_OK;
                EptzAiData eptz_ai_data;
                eptz_ai_data.face_data = (FaceData *)malloc(faceCount * sizeof(FaceData));
                eptz_ai_data.face_count = faceCount;
                if(eptz_ai_data.face_data){
                  for(int i = 0; i<faceCount; i++){
                      eptz_ai_data.face_data[i].left = nnResult->results[i].face_info.object.box.left;
                      eptz_ai_data.face_data[i].top = nnResult->results[i].face_info.object.box.top;
                      eptz_ai_data.face_data[i].right = nnResult->results[i].face_info.object.box.right;
                      eptz_ai_data.face_data[i].bottom = nnResult->results[i].face_info.object.box.bottom;
                      eptz_ai_data.face_data[i].score = nnResult->results[i].face_info.object.score;
                  }
                }
                calculateClipRect(&eptz_ai_data, mLastXY);
                mSequeEptz++;
                if(eptz_ai_data.face_data)
                  free(eptz_ai_data.face_data);
            }
            dstBuffer->release();
        }
    } else {
        RT_LOGE("don't has nn data stream!");
    }

    // 此处是用于预览的原始YUV数据，SDK默认数据流路径是bypass->EPTZ->RGA(输出给下级节点裁剪)
    INT32 count = context->inputQueueSize("image:nv12");
    while (count) {
        srcBuffer = context->dequeInputBuffer("image:nv12");
        if (srcBuffer == RT_NULL)
            continue;
        count--;
        mSequeFrame++;
        RTRect srcRect;
        RTCropInfo dstCropInfo;
        RTVideoFrame *srcVFrame = RT_NULL;
        srcVFrame = reinterpret_vframe(srcBuffer);
        RTVideoFrame *cloneDstBuffer = clone_vframe(srcVFrame);

        srcRect.x = (INT32)mLastXY[0] % 2 != 0 ? (INT32)mLastXY[0] - 1 : (INT32)mLastXY[0];
        srcRect.y = (INT32)mLastXY[1] % 2 != 0 ? (INT32)mLastXY[1] - 1 : (INT32)mLastXY[1];
        srcRect.w = (INT32)mLastXY[2] % 2 != 0 ? (INT32)mLastXY[2] - 1 : (INT32)mLastXY[2];
        srcRect.h = (INT32)mLastXY[3] % 2 != 0 ? (INT32)mLastXY[3] - 1 : (INT32)mLastXY[3];
        cloneDstBuffer->setOpRect(srcRect);
        cloneDstBuffer->setVirWidth(mSrcWidth);
        cloneDstBuffer->setVirHeight(mSrcHeight);
        dstCropInfo.region.x = 0;
        dstCropInfo.region.y = 0;
        dstCropInfo.region.w = mClipWidth;
        dstCropInfo.region.h = mClipHeight;
        dstCropInfo.virWidth = mClipWidth;
        dstCropInfo.virHeight = mClipHeight;
        cloneDstBuffer->getMetaData()->setStructData(KEY_COMM_DST_CROP, &dstCropInfo, sizeof(RTCropInfo));
        cloneDstBuffer->setSeq(srcVFrame->getSeq());
        cloneDstBuffer->setPts(srcVFrame->getPts());
        context->queueOutputBuffer(cloneDstBuffer);
        srcBuffer->release();
        srcBuffer = RT_NULL;
        if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0)) {
            int32_t use_time_us, now_time_us;
            struct timespec now_tm = {0, 0};
            clock_gettime(CLOCK_MONOTONIC, &now_tm);
            now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us
            use_time_us = now_time_us - cloneDstBuffer->getPts();
            RT_LOGE("isp->aiserver seq:%ld latency time:%d us, %d ms\n",
                     cloneDstBuffer->getSeq(), use_time_us, use_time_us / 1000);
        }
    }

    return err;
}

RT_RET RTNodeVFilterEptz::close(RTTaskNodeContext *context) {
    RT_RET err = RT_OK;

    return err;
}

RT_RET RTNodeVFilterEptz::invokeInternal(RtMetaData *meta) {
    const char *command;
    if (RT_NULL == meta) {
        return RT_ERR_NULL_PTR;
    }
    RtMutex::RtAutolock autoLock(mLock);
    meta->findCString(kKeyPipeInvokeCmd, &command);
    RT_LOGD("invoke(%s) internally.", command);
    RTSTRING_SWITCH(command) {
      RTSTRING_CASE("set_eptz_config"):
        RT_ASSERT(meta->findInt32(OPT_VIDEO_WIDTH, &mEptzInfo.eptz_src_width));
        RT_ASSERT(meta->findInt32(OPT_VIDEO_HEIGHT, &mEptzInfo.eptz_src_height));
        mEptzInfo.eptz_dst_width = mEptzInfo.eptz_src_width;
        mEptzInfo.eptz_dst_height = mEptzInfo.eptz_src_height;
        mSrcWidth = mEptzInfo.eptz_src_width;
        mSrcHeight = mEptzInfo.eptz_src_height;
        mLastXY[0] = 0;
        mLastXY[1] = 0;
        mLastXY[2] = mEptzInfo.eptz_dst_width;
        mLastXY[3] = mEptzInfo.eptz_dst_height;
        RT_LOGE("mEptzInfo.eptz_src_width=%d mEptzInfo.eptz_src_height=%d mClipWidth=%d mClipHeight=%d",
                 mEptzInfo.eptz_src_width, mEptzInfo.eptz_src_height, mClipWidth, mClipHeight);
        eptzConfigInit(&mEptzInfo);
        break;
      default:
        RT_LOGD("unsupported command=%d", command);
        break;
    }

    return RT_OK;
}

static RTTaskNode* createEptzFilter() {
    return new RTNodeVFilterEptz();
}

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_eptz_demo {
    .mUid          = kStubRockitEPTZDemo,
    .mName         = "rkeptz",
    .mVersion      = "v1.0",
    .mCreateObj    = createEptzFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_eptz_demo);
