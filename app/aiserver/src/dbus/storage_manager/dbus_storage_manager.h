// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_STORAGE_MANAGER_H_
#define _RK_DBUS_STORAGE_MANAGER_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_dispatcher.h"
#include "dbus_storage_manager_proxy.h"
#include <dbus-c++/dbus.h>

namespace rockchip {
namespace aiserver {

#define SM_GET_DISK_STATUS "{ \"sMountPath\": \"\" }"

class DBusStorageManager : public rockchip::StorageManager::file_proxy,
                           public DBus::IntrospectableProxy,
                           public DBus::ObjectProxy {
public:
  DBusStorageManager(DBus::Connection &connection, const char *adaptor_path,
                     const char *adaptor_name);
  virtual ~DBusStorageManager();

  std::string GetStorageStatus();
  std::string GetStoragePath();

  void UpdateDisksStatus(const std::string &param);
  void UpdateMediaPath(const std::string &param);
  void FreeSizeNotice(const std::string &param);

private:
  DBus::Pipe *pipe_;
};

class DBusStorageManagerListen : public DBus::InterfaceProxy,
                                 public DBus::ObjectProxy {
public:
  DBusStorageManagerListen(DBus::Connection &connection);
  virtual ~DBusStorageManagerListen();
  void UpdateDisksStatusChanged(const DBus::SignalMessage &sig);
  void UpdateMediaPathChanged(const DBus::SignalMessage &sig);
  void FreeSizeNoticeChanged(const DBus::SignalMessage &sig);
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_MEDIA_CONTROL_H_
