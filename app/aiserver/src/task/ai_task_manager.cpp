// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ai_task_manager.h"
#include "ai_task_factory.h"

namespace rockchip {
namespace aiserver {

AITaskManager::AITaskManager() {
    mTaskHandler = AITaskFactory::createHandler();
}

AITaskManager::~AITaskManager() {
    if (mTaskHandler != nullptr) {
        delete mTaskHandler;
        mTaskHandler = nullptr;
    }
}

int32_t AITaskManager::processAIData(RTMediaBuffer *buffer) {
    return mTaskHandler->processAIData(buffer);
}

int32_t AITaskManager::processAIMatting(RTMediaBuffer *buffer) {
    return mTaskHandler->processAIMatting(buffer);
}

int32_t AITaskManager::processAIFeature(RTMediaBuffer *buffer) {
    return mTaskHandler->processAIFeature(buffer);
}

int32_t AITaskManager::convertDetectType(int32_t detectType) {
    return mTaskHandler->convertDetectType(detectType);
}

} // namespace aiserver
} // namespace rockchip
