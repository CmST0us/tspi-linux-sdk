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

#include "json-c/json.h"
#include "system_manager.h"
#include "system_manager_proxy.h"
#include "dbus_connection.h"
#include "libipcpro_log_control.h"

extern int ipc_pro_log_ctl;

#define SYSTEMMANAGER_DBUSSEND_1(FUNC) \
    DBusSystemManager* system_manager_proxy_ = NULL; \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        system_manager_proxy_ = new DBusSystemManager(conn, SYSTEM_MANAGER_PATH, SYSTEM_MANAGER, SYSTEM_MANAGER_INTERFACE); \
        system_manager_proxy_->FUNC(); \
        delete system_manager_proxy_; \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
        if (NULL != system_manager_proxy_) { \
            delete system_manager_proxy_; \
        } \
    } \
    dbus_mutex_unlock(); \
    return NULL;

#define SYSTEMMANAGER_DBUSSEND_2(FUNC) \
    char *ret = NULL; \
    DBusSystemManager* system_manager_proxy_ = NULL; \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        system_manager_proxy_ = new DBusSystemManager(conn, SYSTEM_MANAGER_PATH, SYSTEM_MANAGER, SYSTEM_MANAGER_INTERFACE); \
        auto config = system_manager_proxy_->FUNC(json); \
        ret = g_strdup(config.c_str()); \
        delete system_manager_proxy_; \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
        if (NULL != system_manager_proxy_) { \
            delete system_manager_proxy_; \
        } \
    } \
    dbus_mutex_unlock(); \
    return ret;

char *dbus_system_reboot(void)
{
    SYSTEMMANAGER_DBUSSEND_1(Reboot);
}

char *dbus_system_factory_reset(void)
{
    SYSTEMMANAGER_DBUSSEND_1(FactoryReset);
}

char *dbus_system_export_db(char *json)
{
    SYSTEMMANAGER_DBUSSEND_2(ExportDB);
}

char *dbus_system_import_db(char *json)
{
    SYSTEMMANAGER_DBUSSEND_2(ImportDB);
}

char *dbus_system_export_log(char *json)
{
    SYSTEMMANAGER_DBUSSEND_2(ExportLog);
}

char *dbus_system_upgrade(char *json)
{
    SYSTEMMANAGER_DBUSSEND_2(Upgrade);
}

extern "C" char *system_reboot(void)
{
    return dbus_system_reboot();
}

extern "C" char *system_factory_reset(void)
{
    return dbus_system_factory_reset();
}

extern "C" char *system_export_db(const char *path)
{
    char *ret = NULL;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "sPath", json_object_new_string(path));
    ret = dbus_system_export_db((char *)json_object_to_json_string(j_cfg));
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *system_import_db(const char *path)
{
    char *ret = NULL;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "sPath", json_object_new_string(path));
    ret = dbus_system_import_db((char *)json_object_to_json_string(j_cfg));
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *system_export_log(const char *path)
{
    char *ret = NULL;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "sPath", json_object_new_string(path));
    ret = dbus_system_export_log((char *)json_object_to_json_string(j_cfg));
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *system_upgrade(const char *path)
{
    char *ret = NULL;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "sPath", json_object_new_string(path));
    ret = dbus_system_upgrade((char *)json_object_to_json_string(j_cfg));
    json_object_put(j_cfg);

    return ret;
}