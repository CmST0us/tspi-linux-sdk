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

#include "RTNodeVFilterZoom.h"          // NOLINT
#include "RTNodeCommon.h"
#include "RTMediaBuffer.h"
#include "RTMediaMetaKeys.h"
#include "RTVideoFrame.h"
#include "RTTaskNodePrivKey.h"

#include <sys/mman.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RTNodeVFilterZoom"
#define kStubRockitZoom                MKTAG('z', 'o', 'o', 'm')

#define UVC_EPTZ_PAN_MAX             20.0
#define UVC_EPTZ_PAN_COUNT           40.0
#define UVC_EPTZ_TILT_MAX            20.0
#define UVC_EPTZ_TILT_COUNT          40.0

#define ALIGN2(x)                   (x- x % 2)

#define REQUEST16B9                  1


RTNodeVFilterZoom::RTNodeVFilterZoom() {
    mLock = new RtMutex();
    RT_ASSERT(RT_NULL != mLock);
    mZoomValue = 10;
    mZoomValueNow = 10;
    mPanValue = 0;
    mPanValueNow = 0;
    mTiltValue = 0;
    mTiltValueNow = 0;
    mIsZoomSet = false;
}

RTNodeVFilterZoom::~RTNodeVFilterZoom() {
    rt_safe_delete(mLock);
}

RT_RET RTNodeVFilterZoom::open(RTTaskNodeContext *context) {
    RtMetaData* inputMeta   = context->options();
    RT_RET err              = RT_OK;
    RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_WIDTH, &mSrcWidth));
    RT_ASSERT(inputMeta->findInt32(OPT_VIDEO_HEIGHT, &mSrcHeight));
    RT_ASSERT(inputMeta->findInt32(OPT_EPTZ_CLIP_WIDTH, &mDstWidth));
    RT_ASSERT(inputMeta->findInt32(OPT_EPTZ_CLIP_HEIGHT, &mDstHeight));
    mEptzOffsetX = 0;
    mEptzOffsetY = 0;
    mEptzWidth = mSrcWidth;
    mEptzHeight = mSrcHeight;
    return RT_OK;
}

RT_RET RTNodeVFilterZoom::process(RTTaskNodeContext *context) {
    RT_RET         err       = RT_OK;
    RTVideoFrame  *srcVFrame = RT_NULL;
    RTVideoFrame  *dstVFrame = RT_NULL;
    RTCropInfo    *cropInfo = RT_NULL;

    if (!context->inputIsEmpty()) {
        srcVFrame = reinterpret_vframe(context->dequeInputBuffer());
        RT_ASSERT(srcVFrame != RT_NULL);

        if (srcVFrame->getOpRect().w != 0 && srcVFrame->getOpRect().h != 0) {
            mEptzOffsetX = srcVFrame->getOpRect().x;
            mEptzOffsetY = srcVFrame->getOpRect().y;
            mEptzWidth = srcVFrame->getOpRect().w;
            mEptzHeight = srcVFrame->getOpRect().h;
        }
        mSrcWidth = srcVFrame->getVirWidth();
        mSrcHeight = srcVFrame->getVirHeight();

        //RT_LOGE("zoom get format[%d]",  format);

        RTZoomCalculate();

        RTRect srcRect;
        RTCropInfo dstCropInfo;

        srcRect.x = mResult[0];
        srcRect.y = mResult[1];
        srcRect.w = mResult[2];
        srcRect.h = mResult[3];
        dstCropInfo.region.x = 0;
        dstCropInfo.region.y = 0;
        dstCropInfo.region.w = mDstWidth;
        dstCropInfo.region.h = mDstHeight;
        dstCropInfo.virWidth = mDstWidth;
        dstCropInfo.virHeight = mDstHeight;

        dstVFrame = clone_vframe(srcVFrame);
        dstVFrame->setOpRect(srcRect);
        dstVFrame->setVirWidth(mSrcWidth);
        dstVFrame->setVirHeight(mSrcHeight);
        dstVFrame->getMetaData()->setStructData(KEY_COMM_DST_CROP, &dstCropInfo, sizeof(RTCropInfo));
        dstVFrame->setSeq(srcVFrame->getSeq());
        dstVFrame->setPts(srcVFrame->getPts());
        dstVFrame->setPixelFormat(srcVFrame->getPixelFormat());
        context->queueOutputBuffer(dstVFrame);
        srcVFrame->release();
        srcVFrame = RT_NULL;
    }

    return err;
}

void RTNodeVFilterZoom::RTZoomCalculate(){
    RT_LOGV("RTZoomCalculate enter , mZoomValue %.2f mZoomValueNow %.2f mPanValue %d mPanValueNow %d mTiltValue %d mTiltValueNow %d",
    mZoomValue, mZoomValueNow, mPanValue, mPanValueNow, mTiltValue, mTiltValueNow);
    if (mZoomValue > 10 || mZoomValueNow > 10) {
        mIsZoomSet = true;
        if (mZoomValueNow > mZoomValue) {
            if (mZoomValueNow - 10 > mZoomValue)
                mZoomValueNow -= 8;
            else if (mZoomValueNow - 5 > mZoomValue)
                mZoomValueNow -= 4;
            else
                mZoomValueNow -= 1;
        } else if (mZoomValueNow < mZoomValue) {
            if (mZoomValueNow + 10 < mZoomValue)
                mZoomValueNow += 8;
            else if (mZoomValueNow + 5 < mZoomValue)
                mZoomValueNow += 4;
            else
                mZoomValueNow += 1;
        }
    }else {
        mIsZoomSet = false;
    }

    int zoomIndex = (mZoomValueNow - 10);
    float ratio = (float)(mEptzWidth) / (float)(mSrcWidth);
    float zoomPerCrease = mSrcWidth * 1.0 / 16.0 / 80.0 * ratio;

    RT_LOGV("zoomIndex %d , ratio %.2f zoomPerCrease %.2f", zoomIndex, ratio, zoomPerCrease);
    RT_LOGV("mEPTZ Orignal xywh[%d %d %d %d] srcwh[%d %d] dstwh[%d %d]",
    mEptzOffsetX, mEptzOffsetY, mEptzWidth, mEptzHeight,
    mSrcWidth, mSrcHeight, mDstWidth, mDstHeight);

    float mEptzWidthF, mEptzHeightF, mEptzOffsetXF, mEptzOffsetYF;
    mEptzWidthF = (float)(mEptzWidth) - zoomIndex * zoomPerCrease * 16;
    mEptzHeightF = (float)(mEptzHeight) - zoomIndex * zoomPerCrease * 9;
    // mEptzOffsetXF = (float)(mEptzOffsetX) + (zoomIndex * zoomPerCrease * 16) / 2;
    // mEptzOffsetYF = (float)(mEptzOffsetY) + (zoomIndex * zoomPerCrease * 9) / 2;

    RT_LOGV("mEPTZ After ZOOM xywh[%.3f %.3f %.3f %.3f]", mEptzOffsetXF, mEptzOffsetYF, mEptzWidthF, mEptzHeightF);

    if (mIsZoomSet == false) {
        if (mPanValueNow > mPanValue) {
            if (mPanValueNow - 8 > mPanValue)
                mPanValueNow -= 6;
            else if (mPanValueNow - 5 > mPanValue)
                mPanValueNow -= 3;
            else
                mPanValueNow -= 1;
        }else if (mPanValueNow < mPanValue) {
            if (mPanValueNow + 8 < mPanValue)
                mPanValueNow += 6;
            else if (mPanValueNow + 5 < mPanValue)
                mPanValueNow += 3;
            else
                mPanValueNow += 1;
        }

        if (mTiltValueNow > mTiltValue) {
            if (mTiltValueNow - 8 > mTiltValue)
                mTiltValueNow -= 6;
            else if (mTiltValueNow - 5 > mTiltValue)
                mTiltValueNow -= 3;
            else
                mTiltValueNow -= 1;
        }else if (mTiltValueNow < mTiltValue) {
            if (mTiltValueNow + 8 < mTiltValue)
                mTiltValueNow += 6;
            else if (mTiltValueNow + 5 < mTiltValue)
                 mTiltValueNow += 3;
            else
                mTiltValueNow += 1;
        }
    }else {
        mPanValueNow = mPanValue;
        mTiltValueNow = mTiltValue;
    }

    float pan_value = (zoomIndex * zoomPerCrease * 16) * (mPanValueNow + UVC_EPTZ_PAN_MAX) / UVC_EPTZ_PAN_COUNT;
    float tilt_value = (zoomIndex * zoomPerCrease * 9) * (mTiltValueNow + UVC_EPTZ_TILT_MAX) / UVC_EPTZ_TILT_COUNT;
    mEptzOffsetXF = (float)(mEptzOffsetX) + pan_value;
    mEptzOffsetYF = (float)(mEptzOffsetY) + tilt_value;

    if(REQUEST16B9 && ((mDstWidth == 1024 && mDstHeight == 768) ||
        (mDstWidth == 640 && mDstHeight == 480) ||
        (mDstWidth == 320 && mDstHeight == 240))){
        float modify_x = mEptzWidthF -  (float)(mEptzHeightF) * 4 / 3;
        mEptzWidthF = mEptzWidthF - modify_x;
        mEptzOffsetXF = mEptzOffsetXF + modify_x / 2;
        RT_LOGV("640x480 2 16:9 modify_x %.3f mEptzWidth %.3f mEptzOffsetX %.3f", modify_x, mEptzWidthF, mEptzOffsetXF);
    }
    mResult[0] = ALIGN2((int)mEptzOffsetXF);
    mResult[1] = ALIGN2((int)mEptzOffsetYF);
    mResult[2] = ALIGN2((int)mEptzWidthF);
    mResult[3] = ALIGN2((int)mEptzHeightF);
    RT_LOGV("mEPTZ ZOOM pt[%.3f %.3f] 2Align xywh[%d %d %d %d]", pan_value, tilt_value, mResult[0], mResult[1], mResult[2], mResult[3]);
}

RT_RET RTNodeVFilterZoom::invokeInternal(RtMetaData *meta) {
    const char *command;
    if (RT_NULL == meta) {
        return RT_ERR_NULL_PTR;
    }
    RtMutex::RtAutolock autoLock(mLock);
    meta->findCString(kKeyPipeInvokeCmd, &command);
    RT_LOGD("invoke(%s) internally.", command);
    RTSTRING_SWITCH(command) {
      RTSTRING_CASE("set_zoom"):
            RT_ASSERT(meta->findFloat("value", &mZoomValue));
            mZoomValue = mZoomValue * 10;
            break;
        RTSTRING_CASE("set_pan"):
            RT_ASSERT(meta->findInt32("value", &mPanValue));
            mPanValue = mPanValue * 2;
            break;
        RTSTRING_CASE("set_tilt"):
            RT_ASSERT(meta->findInt32("value", &mTiltValue));
            mTiltValue = mTiltValue * 2;
            break;
        RTSTRING_CASE("reset_position"):
          RT_LOGD("zoom reset position [0 0 %d %d]", mSrcWidth, mSrcHeight);
          mEptzOffsetX = 0;
          mEptzOffsetY = 0;
          mEptzWidth = mSrcWidth;
          mEptzHeight = mSrcHeight;
        break;
      default:
        RT_LOGD("unsupported command=%d", command);
        break;
    }

    RT_LOGD("invokeInternal, mZoomValue %.2f mPanValue %d mTiltValue %d mDstWH[%d %d]",
             mZoomValue, mPanValue, mTiltValue, mDstWidth, mDstHeight);

    return RT_OK;
}

RT_RET RTNodeVFilterZoom::close(RTTaskNodeContext *context) {
    RT_RET err = RT_OK;
    return err;
}


static RTTaskNode* createZoomFilter() {
    return new RTNodeVFilterZoom();
}

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_zoom_demo {
    .mUid          = kStubRockitZoom,
    .mName         = "rkzoom",
    .mVersion      = "v1.0",
    .mCreateObj    = createZoomFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL}, },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_zoom_demo);
