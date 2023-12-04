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
#include "dbus_encoder_control.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_encoder_control"

namespace rockchip {
namespace aiserver {

DBusEncoderControl::DBusEncoderControl(DBus::Connection &connection)
    : DBus::ObjectAdaptor(connection, MEDIA_CONTROL_PATH_ENCODER) {}

DBusEncoderControl::~DBusEncoderControl() {}

int32_t DBusEncoderControl::SetBitRate(const int32_t &id,
                                       const int32_t &param) {
    LOG_INFO("DBusEncoderControl::SetBitRate id %d param %d\n", id, param);
    return 0;
}

int32_t DBusEncoderControl::SetFrameRate(const int32_t &id,
                                         const int32_t &param) {
    LOG_INFO("DBusEncoderControl::SetFrameRate id %d param %d\n", id, param);
    return 0;
}

int32_t DBusEncoderControl::SetForceIdrFrame(const int32_t &id) {
    LOG_INFO("DBusEncoderControl::SetForceIdrFrame id %d\n", id);
    return 0;
}

int32_t DBusEncoderControl::SetQP(const int32_t &id, const int32_t &param) {
    LOG_INFO("DBusEncoderControl::SetQP id %d param %d\n", id, param);
    // TODO
    return 0;
}

int32_t DBusEncoderControl::SetOSDData(const int32_t &id,
                                       const std::string &param) {
    LOG_INFO("DBusEncoderControl::SetOSDData id %d param %s\n", id, param.c_str());
    // TODO
    return 0;
}

} // namespace aiserver
} // namespace rockchip
