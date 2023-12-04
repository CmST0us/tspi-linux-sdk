// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/prctl.h>

#include "dbus_server.h"
#include "logger/log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_server.cpp"

namespace rockchip {
namespace aiserver {

DBusServer::DBusServer(bool session, bool need_dbserver)
      : session_(session), need_dbserver_(need_dbserver) {
    LOG_DEBUG("dbus server setup\n");

    media_control_.reset(new DBusMediaControl());
}

DBusServer::~DBusServer() {
    UnRegisterMediaControl();
    media_control_.reset(nullptr);
}

static void *ServerProcess(void *arg) {
    char thread_name[40];
    snprintf(thread_name, sizeof(thread_name), "DBusServer");
    prctl(PR_SET_NAME, thread_name);
    DBus::default_dispatcher->enter();
    return nullptr;
}

void DBusServer::start(void) {
    LOG_DEBUG("dbus server start\n");
    service_thread_.reset(new Thread(ServerProcess, this));
    service_thread_->set_status(kThreadRunning);
}

void DBusServer::stop(void) {
    DBus::default_dispatcher->leave();
    service_thread_->set_status(kThreadStopping);
    media_control_.reset();

    LOG_DEBUG("dbus server stop\n");
}

ThreadStatus DBusServer::status(void) {
    if (service_thread_)
      return service_thread_->status();
    else
      return kThreadRunning;
}

int DBusServer::RegisterMediaControl(RTGraphListener *listener) {
    DbusDispatcher(100);
    DBus::Connection conn =
        session_ ? DBus::Connection::SessionBus() : DBus::Connection::SystemBus();
    conn.request_name(MEDIA_CONTROL_BUS_NAME);
  

    media_control_->listenMediaCtrl(conn, listener);
    return 0;
}

int DBusServer::UnRegisterMediaControl() {
    return 0;
}

} // namespace aiserver
} // namespace rockchip
