// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "thread.h"

#include <sys/prctl.h>

namespace rockchip {

Thread::Thread(pthread_func_t func, void *arg)
    : func_(func), arg_(arg), status_(kThreadUninited) {
  thread_ = new std::thread(func_, arg_);
}

Thread::~Thread() {
  if (joinable()) {
    join();
    delete thread_;
    // std::terminate();
  }
}

} // namespace rockchip
