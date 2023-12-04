// Copyright 2021 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <glib.h>
#include <dbus-c++/dbus.h>

#include "ispserver.h"
#include "ispserver_proxy.h"
#include "dbus_connection.h"
#include "json-c/json.h"
#include "libipcpro_log_control.h"

extern int ipc_pro_log_ctl;

#define ISPSERVER_DBUSSEND(FUNC) \
    char *ret = NULL; \
    DBusNetServer* ispserver_proxy_ = NULL; \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        ispserver_proxy_ = new DBusNetServer(conn, ISPSERVER_PATH, ISPSERVER_BUS_NAME, ISPSERVER_INTERFACE); \
        auto config = ispserver_proxy_->FUNC(json); \
        ret = g_strdup(config.c_str()); \
        delete ispserver_proxy_; \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
        if (NULL != ispserver_proxy_) { \
            delete ispserver_proxy_; \
        } \
    } \
    dbus_mutex_unlock(); \
    return ret;

void ispserver_turnoff_signal_send(void)
{
    dbus_mutex_lock();
    try {
        DBus::Connection conn = get_dbus_conn();
        DBusNetServer* ispserver_proxy_ = new DBusNetServer(conn, ISPSERVER_PATH, ISPSERVER_BUS_NAME, ISPSERVER_INTERFACE);
        ispserver_proxy_->SendTurnoffIspSignal();
        delete ispserver_proxy_;
    } catch (DBus::Error err) {
        printf("DBus::Error - %s\n", err.what());
    }
    dbus_mutex_unlock();
}

char *ispserver_expo_info_get(char *json)
{
    ISPSERVER_DBUSSEND(GetDumpExposureInfo);
}

extern "C" char *ispserver_expo_info_get()
{
    return ispserver_expo_info_get((char *)"");
}
