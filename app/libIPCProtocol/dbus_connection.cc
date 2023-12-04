// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
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
#include <iostream>
#include <cstring>
#include <cstdio>

#include "dbus_connection.h"

DBus::BusDispatcher dbus_dispatcher;
DBusConn* dbus_conn = nullptr;
int dbus_conn_flag = 0;
static pthread_mutex_t dbus_mutex = PTHREAD_MUTEX_INITIALIZER;

DBus::Connection &get_dbus_conn(void)
{
    if (dbus_conn_flag == 0) {
        DBus::_init_threading();
        DBus::default_dispatcher = &dbus_dispatcher;
        new DBus::DefaultTimeout(100, false, &dbus_dispatcher);
        DBus::Connection conn = DBus::Connection::SystemBus();
        dbus_conn = new DBusConn(conn);
        dbus_conn_flag = 1;
    }

    return dbus_conn->conn();
}

void dbus_mutex_lock(void)
{
    pthread_mutex_lock(&dbus_mutex);
}

void dbus_mutex_unlock(void)
{
    pthread_mutex_unlock(&dbus_mutex);
}