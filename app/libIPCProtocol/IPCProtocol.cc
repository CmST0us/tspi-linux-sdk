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
#include <stddef.h>
#include <memory>
#include <iostream>
#include <map>
#include <algorithm>

#include "IPCProtocol.h"
#include "dbus_connection.h"

int IPCProtocol_init(void)
{
    int ret = -1;
    try {
        DBus::Connection con = get_dbus_conn();
        ret = 0;
    } catch (DBus::Error err) {
        printf("DBus::Error - %s\n", err.what());
    }

    return ret;
}

void IPCProtocol_deinit(void)
{

}