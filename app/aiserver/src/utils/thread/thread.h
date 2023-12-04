// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RK_UTILS_THREAD_H_
#define RK_UTILS_THREAD_H_

#include <stdio.h>

#include <memory>
#include <string>
#include <thread>

namespace rockchip {

typedef void *(*pthread_func_t)(void *arg);

enum ThreadStatus {
  kThreadUninited = 1,
  kThreadRunning = 2,
  kThreadWaiting = 3,
  kThreadStopping = 4,
};

class Thread {
public:
  Thread(Thread &) = delete;
  Thread(const Thread &) = delete;
  Thread() : status_(kThreadUninited) {}
  explicit Thread(pthread_func_t func, void *arg);
  virtual ~Thread();

  typedef std::unique_ptr<Thread> UniquePtr;
  typedef std::shared_ptr<Thread> SharedPtr;

  bool joinable(void) const { return thread_->joinable(); }

  void join(void) { thread_->join(); }

  void detach(void) { thread_->detach(); }

  void set_status(const ThreadStatus status) { status_ = status; }

  const ThreadStatus status(void) const { return status_; }

private:
  std::thread *thread_;
  ThreadStatus status_;
  pthread_func_t func_;
  void *arg_;
};

} // namespace rockchip

#endif // RK_UTILS_THREAD_H_
