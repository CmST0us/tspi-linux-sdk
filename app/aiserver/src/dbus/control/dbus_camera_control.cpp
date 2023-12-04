// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "logger/log.h"
#include "dbus_camera_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_camera_ctl"

namespace rockchip {
namespace aiserver {

DBusCameraControl::DBusCameraControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_PATH_CAMERA) {}

DBusCameraControl::~DBusCameraControl() {}

int32_t DBusCameraControl::SetFrameRate(const int32_t &id,
                                        const int32_t &param) {
  LOG_INFO("DBusCameraControl::SetFrameRate\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::SetResolution(const int32_t &id,
                                         const int32_t &param1,
                                         const int32_t &param2) {
  LOG_INFO("DBusCameraControl::SetResolution\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::StartCamera(const int32_t &id) {
  LOG_INFO("DBusCameraControl::StartCamera\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::StopCamera(const int32_t &id) {
  LOG_INFO("DBusCameraControl::StopCamera\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::TakePicture(const int32_t &id,
                                       const int32_t &count) {
  LOG_INFO("DBusCameraControl::TakePicture\n");
  // TODO
  return 0;
}

int32_t DBusCameraControl::TakePicture(const std::string &id_count) {
  LOG_INFO("DBusCameraControl::TakePicture\n");
  // TODO
  return 0;
}

} // namespace aiserver
} // namespace rockchip
