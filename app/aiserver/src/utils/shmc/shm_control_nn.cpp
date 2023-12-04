// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shm_control_nn.h"
#include "logger/log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "shm_control_nn"

namespace rockchip {
namespace aiserver {

ShmNNController::ShmNNController() {
    shmWriteInited = 0;
    initialize();
}

ShmNNController::~ShmNNController() {

}

void ShmNNController::initialize() {
    if (shmWriteInited) {
        return;
    }

#ifdef ENABLE_SHM_SERVER
    shmc::SetLogHandler(shmc::kDebug, [](shmc::LogLevel lv, const char *s) {
        LOG_INFO("[%d] %s\n", lv, s);
    });
    shmWriteInited = shmNNQueue.InitForWrite(kShmNNKey, kNNQueueBufSize);
#endif
}

void ShmNNController::send(std::string &buf) {
    if (!shmWriteInited) {
        initialize();
    }

    if (shmWriteInited) {
        shmNNQueue.Push(buf);
    } else {
        LOG_ERROR("shm nn init for write failed");
    }
}

}
}
// namespace ShmControl
