// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_UTILS_SERVICE_H_
#define _RK_UTILS_SERVICE_H_

namespace rockchip {

class Service {
public:
  Service() {}
  virtual ~Service() {}

  virtual void start(void) = 0;
  virtual void stop(void) = 0;

  bool is_running(void) const { return is_running_; }

private:
  bool is_running_;
};

} // namespace rockchip

#endif // _RK_UTILS_SERVICE_H_
