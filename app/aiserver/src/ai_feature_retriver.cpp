// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ai_feature_retriver.h"
#include "logger/log.h"
#include "nn_data.pb.h"
#include "shm_control_feature.h"
#include "RTMediaBuffer.h"
#include "RTMediaMetaKeys.h"
#include "RTVideoFrame.h"

namespace rockchip {
namespace aiserver {

// send NN data to SmartDisplayService
RT_RET AIFeatureRetriver::ai_feature_output_callback(RTMediaBuffer *buffer) {
    int32_t ret = 0;

    ret = mAITaskManager->processAIFeature(buffer);
    if (ret < 0) {
        mAIGraph->recognize(buffer);
    }

    return RT_OK;
}

AIFeatureRetriver::AIFeatureRetriver() {
    mAIGraph = RT_NULL;
}

AIFeatureRetriver::~AIFeatureRetriver() {
    if (mAIGraph != nullptr) {
        delete mAIGraph;
        mAIGraph = nullptr;
    }
}

INT32 AIFeatureRetriver::setup(AITaskManager *taskManager) {
    RT_RET err = RT_OK;

    std::lock_guard<std::mutex> lock(mOpMutex);
    mAITaskManager = taskManager;
    if (mAIGraph == NULL) {
        mAIGraph = new RTAIGraph("ai_graph");
        mAIGraph->observeOutputStream(std::bind(&AIFeatureRetriver::ai_feature_output_callback, this, std::placeholders::_1));
        mAIGraph->prepare();
        preload();
    }

    return 0;
}

INT32 AIFeatureRetriver::preload() {
#if PRELOAD_HANDLE_FEATURE
    RtMetaData meta;
    meta.setCString("preload_handle", "scene_pic");
    LOG_INFO("preload scene pic resource in\n");
    mAIGraph->preload(&meta);
    LOG_INFO("preload scene pic resource ok\n");
#endif
    return 0;
}

INT32 AIFeatureRetriver::start() {
    LOG_INFO("start ai graph(%p)\n", mAIGraph);
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (mAIGraph != NULL) {
        mAIGraph->start();
    }

    return 0;
}

INT32 AIFeatureRetriver::stop() {
    LOG_INFO("stop ai graph(%p)\n", mAIGraph);
    std::lock_guard<std::mutex> lock(mOpMutex);
    if (mAIGraph != NULL) {
        mAIGraph->stop();
        mAIGraph->waitUntilDone();
    }

    return 0;
}

#define USE_SHM_DATA   1

INT32 AIFeatureRetriver::runTaskOnce(void *params) {
    RTImage         imgQueue;
    std::string     imgRaw;

    if (mAIGraph == NULL) {
        LOG_ERROR("failed to get ai graph\n");
        return -1;
    }

    char *cmdParams = (char *)params;
    char name[16] = {0};
    char type[16] = {0};
    char uuid[64] = {0};
    sscanf(cmdParams, "%[^:]:%[^:]:%[^:]", name, type, uuid);
    LOG_INFO("runTaskOnce name: %s, type %s, uuid %s\n", name, type, uuid);

#if USE_SHM_DATA
    shm_queue_recv_buffer(&imgRaw);
    if (imgRaw.length() > 0) {
        imgQueue.ParseFromString(imgRaw);
    }

    LOG_INFO("nanlyse_ipc_client received_image(%p) size=%zu\n", imgRaw, imgRaw.length());
    LOG_INFO("nanlyse_ipc_client parsed image_queue((%dx%d)x%d)\n", \
                                    imgQueue.width(), imgQueue.height(), \
                                    imgQueue.data_size());

    if (imgQueue.data_size() > 0) {
        for (INT32 idx = 0; idx < imgQueue.data_size(); idx++) {
            INT32 imgSize = imgQueue.data(idx).length();
            RTVideoFrame *buffer = new RTVideoFrame(imgSize);
            UINT8 *imgData = (UINT8 *)buffer->getData();
            memcpy(imgData, imgQueue.data(idx).c_str(), imgSize);

            buffer->setWidth(imgQueue.width());
            buffer->setHeight(imgQueue.height());
            buffer->setPixelFormat(RT_FMT_YUV420SP_VU);
            buffer->getMetaData()->setCString("stream_uuid", uuid);
            buffer->getMetaData()->setInt32("detect_type", mAITaskManager->convertDetectType(atoi(type)));
            LOG_INFO("ready to recognize image(buf=%p,data=%p,size=%d)\n", buffer, imgData, imgSize);
            mAIGraph->recognize(buffer);
        }
    } else {
        RTVideoFrame *buffer = new RTVideoFrame();
        buffer->getMetaData()->setCString("stream_uuid", uuid);
        mAITaskManager->processAIFeature(buffer);
        LOG_INFO("reply empty nn data with empty input data, uuid %s\n", uuid);
    }
#else
    UINT32 imgSize = 1280 * 720 * 3 / 2;
    RTVideoFrame *buffer = new RTVideoFrame(imgSize);
    buffer->getMetaData()->setCString("stream_uuid", uuid);
    UINT8 *imgData = (UINT8 *)buffer->getData();
    FILE* imgFile = fopen("/oem/usr/bin/test_image.nv21", "rb");
    if (RT_NULL == imgFile) {
        LOG_ERROR("failed to read local image\n");
    } else {
        fread(imgData, 1, imgSize, imgFile);
        fclose(imgFile);
        LOG_INFO("succeed to read local image\n");
    }

    buffer->setWidth(1280);
    buffer->setHeight(720);
    buffer->setPixelFormat(RT_FMT_YUV420SP_VU);
    LOG_INFO("ready to recognize image(buf=%p,data=%p,size=%d)\n", buffer, imgData, imgSize);
    mAIGraph->recognize(buffer);

#endif
    return 0;
}

} // namespace aiserver
} // namespace rockchip
