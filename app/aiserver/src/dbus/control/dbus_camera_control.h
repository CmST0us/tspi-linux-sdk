// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_CAMERA_CONTROL_H_
#define _RK_DBUS_CAMERA_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <memory>

#include "dbus_dispatcher.h"

namespace rockchip {
namespace aiserver {

class FlowUnit;
class DBusCameraControl : public control::camera_adaptor,
                          public DBus::IntrospectableAdaptor,
                          public DBus::ObjectAdaptor {
public:
  DBusCameraControl() = delete;
  DBusCameraControl(DBus::Connection &connection);
  virtual ~DBusCameraControl();

  int32_t SetFrameRate(const int32_t &id, const int32_t &param);
  int32_t SetResolution(const int32_t &id, const int32_t &param1, const int32_t &param2);
  int32_t StartCamera(const int32_t &id);
  int32_t StopCamera(const int32_t &id);
  int32_t TakePicture(const int32_t &id, const int32_t &count);
  int32_t TakePicture(const std::string &id_count);
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_CAMERA_CONTROL_H_