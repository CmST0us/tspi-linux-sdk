// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_SERVER_H_
#define _RK_DBUS_SERVER_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dbus-c++/dbus.h>

#include "dbus_dbserver.h"
#include "dbus_dispatcher.h"
#include "dbus_media_control.h"
#include "dbus_storage_manager.h"

#include "server.h"
#include "thread.h"

namespace rockchip {
namespace aiserver {

class DBusServer : public Service {
public:
  DBusServer() = delete;
  DBusServer(bool session, bool need_dbserver);
  virtual ~DBusServer();
  virtual void start(void) override;
  virtual void stop(void) override;
  ThreadStatus status(void);
  int RegisterMediaControl(RTGraphListener *listener);
  int UnRegisterMediaControl();
  std::shared_ptr<DBusDbServer> GetDBserverProxy() { return dbserver_proxy_; }
  std::shared_ptr<DBusDbEvent> GetDBEventProxy() { return dbevent_proxy_; }
  std::shared_ptr<DBusStorageManager> GetStorageProxy() {
    return strorage_proxy_;
  }

private:
  Thread::UniquePtr service_thread_;
  bool session_;
  bool need_dbserver_;

  std::unique_ptr<DBusMediaControl> media_control_;
  std::shared_ptr<DBusDbServer> dbserver_proxy_;
  std::unique_ptr<DBusDbListener> dbserver_listen_;
  std::shared_ptr<DBusDbEvent> dbevent_proxy_;
  std::unique_ptr<DBusDbEventListener> dbevent_listen_;
  std::shared_ptr<DBusStorageManager> strorage_proxy_;
  std::unique_ptr<DBusStorageManagerListen> strorage_listen_;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_SERVER_H_