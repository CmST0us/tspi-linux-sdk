// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_MEDIA_CONTROL_H_
#define _RK_DBUS_MEDIA_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_dispatcher.h"
#include "dbus_camera_control.h"
#include "dbus_encoder_control.h"
#include "dbus_graph_control.h"

namespace rockchip {
namespace aiserver {

class DBusMediaControl {
 public:
    DBusMediaControl(){};
    virtual ~DBusMediaControl() {}
    int32_t listenMediaCtrl(DBus::Connection &connection, RTGraphListener* listener);

private:
   std::shared_ptr<DBusEncoderControl> encoder_controls_;
   std::shared_ptr<DBusCameraControl> camera_controls_;
   std::shared_ptr<DBusGraphControl> graph_controls_;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_MEDIA_CONTROL_H_