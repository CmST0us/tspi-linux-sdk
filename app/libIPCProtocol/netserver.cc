// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
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

#include "netserver.h"
#include "netserver_proxy.h"
#include "dbus_connection.h"
#include "json-c/json.h"
#include "libipcpro_log_control.h"

extern int ipc_pro_log_ctl;

#define NETSERVER_DBUSSEND(FUNC) \
    char *ret = NULL; \
    DBusNetServer* netserver_proxy_ = NULL; \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        netserver_proxy_ = new DBusNetServer(conn, NETSERVER_PATH, NETSERVER_BUS_NAME, NETSERVER_INTERFACE); \
        auto config = netserver_proxy_->FUNC(json); \
        ret = g_strdup(config.c_str()); \
        delete netserver_proxy_; \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
        if (NULL != netserver_proxy_) { \
            delete netserver_proxy_; \
        } \
    } \
    dbus_mutex_unlock(); \
    return ret;

void dbus_netserver_scan_wifi(void)
{
    dbus_mutex_lock();
    try {
        DBus::Connection conn = get_dbus_conn();
        DBusNetServer* netserver_proxy_ = new DBusNetServer(conn, NETSERVER_PATH, NETSERVER_BUS_NAME, NETSERVER_INTERFACE);
        netserver_proxy_->ScanWifi();
        delete netserver_proxy_;
    } catch (DBus::Error err) {
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what());
    }
    dbus_mutex_unlock();
}

char *dbus_netserver_get_service(char *json)
{
    NETSERVER_DBUSSEND(GetService);
}

char *dbus_netserver_get_config(char *json)
{
    NETSERVER_DBUSSEND(GetConfig);
}

char *dbus_netserver_get_networkip(char *json)
{
    NETSERVER_DBUSSEND(GetNetworkIP);
}

extern "C" void netserver_scan_wifi(void)
{
    dbus_netserver_scan_wifi();
}

extern "C" char *netserver_get_service(char *type)
{
    return dbus_netserver_get_service(type);
}

extern "C" char *netserver_get_config(char *service)
{
    return dbus_netserver_get_config(service);
}

extern "C" char *netserver_get_networkip(char *interface)
{
    return dbus_netserver_get_networkip(interface);
}

extern "C" void netserver_get_service_by_ssid(char *ssid, char *service)
{
    char *json_str = netserver_get_service((char *)"wifi");
    json_object *j_array = json_tokener_parse(json_str);
    int num = json_object_array_length(j_array);
    for (int i = 0; i < num; i++) {
        json_object *j_cfg = json_object_array_get_idx(j_array, i);
        char *sName = (char *)json_object_get_string(json_object_object_get(j_cfg, "sName"));
        // ssid may be null
        if ((!sName) && (!ssid)) {
            strcpy(service, json_object_get_string(json_object_object_get(j_cfg, "sService")));
            break;
        }

        if ((!sName) || (!ssid))
            continue;
        // can't compare null
        if (!strcmp(sName, ssid)) {
            strcpy(service, json_object_get_string(json_object_object_get(j_cfg, "sService")));
            break;
        }
    }
    json_object_put(j_array);
    g_free(json_str);
}
