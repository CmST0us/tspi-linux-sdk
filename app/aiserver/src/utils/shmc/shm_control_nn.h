// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHM_CONTROL_NN_H_
#define SHM_CONTROL_NN_H_

#include <mutex>
#include <shmc/shm_queue.h>
#include <stdint.h>

namespace {
constexpr const char *kShmNNKey       = "0x10007";
constexpr size_t      kNNQueueBufSize = 1024 * 1024 * 1;
} // namespace

using namespace shmc;

namespace rockchip {
namespace aiserver {

class ShmNNController {
  public:
    ShmNNController();
   ~ShmNNController();

    void initialize();
    void send(std::string &buf);

  private:
    std::mutex            opMutex;
    ShmQueue<shmc::SVIPC> shmNNQueue;
    int32_t               shmWriteInited;
};

} // namespace aiserver
} // namespace rockchip


#endif // SHM_CONTROL_NN_H_