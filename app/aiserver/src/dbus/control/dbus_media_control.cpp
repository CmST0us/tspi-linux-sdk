// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#include "dbus_media_control.h"

namespace rockchip {
namespace aiserver {

int32_t DBusMediaControl::listenMediaCtrl(DBus::Connection &connection, RTGraphListener* listener) {
    camera_controls_  = std::make_shared<DBusCameraControl>(connection);
    encoder_controls_ = std::make_shared<DBusEncoderControl>(connection);
    graph_controls_   = std::make_shared<DBusGraphControl>(connection, listener);
    return 0;
}

} // namespace aiserver
} // namespace rockchip
