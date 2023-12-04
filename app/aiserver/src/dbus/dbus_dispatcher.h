// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_DISPATCHER_H_
#define _RK_DBUS_DISPATCHER_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_media_control_adaptor.h"
#include <dbus-c++/dbus.h>

namespace rockchip {
namespace aiserver {

#define MEDIA_CONTROL_BUS_NAME     "rockchip.aiserver.control"
#define MEDIA_CONTROL_PATH_ENCODER "/rockchip/aiserver/control/encoder"
#define MEDIA_CONTROL_PATH_CAMERA  "/rockchip/aiserver/control/camera"
#define MEDIA_CONTROL_PATH_AUDIO   "/rockchip/aiserver/control/audio"
#define MEDIA_CONTROL_PATH_GRAPH   "/rockchip/aiserver/control/graph"
#define MEDIA_CONTROL_PATH_FEATURE "/rockchip/aiserver/control/feature"

// define by package dbserver
#define DBSERVER_BUS_NAME "rockchip.dbserver"
#define DBSERVER_PATH "/"
#define DBSERVER_MEDIA_INTERFACE DBSERVER_BUS_NAME ".media"
#define DBSERVER_DBCHANGE_INTERFACE DBSERVER_MEDIA_INTERFACE
#define DBSERVER_EVENT_INTERFACE DBSERVER_BUS_NAME ".event"
#define DBSERVER_DBEVENT_CHANGE_INTERFACE DBSERVER_EVENT_INTERFACE

// define by package storage_manager
#define STORAGE_MANAGER_BUS_NAME "rockchip.StorageManager"
#define STORAGE_MANAGER_PATH "/"
#define STORAGE_MANAGER_MEDIA_INTERFACE STORAGE_MANAGER_BUS_NAME ".file"
#define STORAGE_MANAGER_DBCHANGE_INTERFACE STORAGE_MANAGER_MEDIA_INTERFACE

class DbusDispatcher {
public:
  DbusDispatcher() = delete;
  DbusDispatcher(int timeout_ms);
  virtual ~DbusDispatcher() {}
  friend class DBusDbServer;

private:
  static DBus::BusDispatcher dispatcher_;
  int timeout_ms_;
};
} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_INTERFACE_H_
