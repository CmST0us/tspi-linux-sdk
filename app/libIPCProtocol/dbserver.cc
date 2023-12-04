// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <glib.h>
#include <dbus-c++/dbus.h>

#include "json-c/json.h"
#include "dbserver.h"
#include "dbserver_proxy.h"
#include "dbus_connection.h"
#include "libipcpro_log_control.h"

extern int ipc_pro_log_ctl;

#define DBSERVER_DBUSSEND(FUNC) \
    char *ret = NULL; \
    DBusDbServer* dbserver_proxy_ = NULL; \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        dbserver_proxy_ = new DBusDbServer(conn, DBSERVER_PATH, DBSERVER, interface); \
        auto config = dbserver_proxy_->FUNC(json); \
        ret = g_strdup(config.c_str()); \
        delete dbserver_proxy_; \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
        if (NULL != dbserver_proxy_) { \
            delete dbserver_proxy_; \
        } \
    } \
    dbus_mutex_unlock(); \
    return ret;

char *dbserver_cmd(char *json, char *interface)
{
    DBSERVER_DBUSSEND(Cmd);
}

char *dbserver_sql(char *json, char *interface)
{
    DBSERVER_DBUSSEND(Sql);
}

extern "C" char *dbserver_system_user_add(char *username, char *password, int *authlevel, int *userlevel, int *fixed)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();
    if (username)
        json_object_object_add(data, "sUserName", json_object_new_string(username));
    if (password)
        json_object_object_add(data, "sPassword", json_object_new_string(password));
    if (authlevel)
        json_object_object_add(data, "iAuthLevel", json_object_new_int(*authlevel));
    if (userlevel)
        json_object_object_add(data, "iUserLevel", json_object_new_int(*userlevel));
    if (fixed)
        json_object_object_add(data, "iFixed", json_object_new_int(*fixed));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_USER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Insert"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_user_del_username(char *username)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();
    if (username)
        json_object_object_add(key, "sUserName", json_object_new_string(username));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_USER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_user_set(char *username, char *password, int *authlevel, int *userlevel)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();
    if (username)
        json_object_object_add(key, "sUserName", json_object_new_string(username));
    if (password)
        json_object_object_add(data, "sPassword", json_object_new_string(password));
    if (authlevel)
        json_object_object_add(data, "iAuthLevel", json_object_new_int(*authlevel));
    if (userlevel)
        json_object_object_add(data, "iUserLevel", json_object_new_int(*userlevel));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_USER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_user_get(char *username, char *password, int *authlevel, int *userlevel)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (username)
        json_object_object_add(key, "sUserName", json_object_new_string(username));
    if (password)
        json_object_object_add(key, "sPassword", json_object_new_string(password));
    if (authlevel)
        json_object_object_add(key, "iAuthLevel", json_object_new_int(*authlevel));
    if (userlevel)
        json_object_object_add(key, "iUserLevel", json_object_new_int(*userlevel));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_USER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" int dbserver_system_user_num_get(char *username, char *password, int *authlevel, int *userlevel)
{
    int num = -1;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (username)
        json_object_object_add(key, "sUserName", json_object_new_string(username));
    if (password)
        json_object_object_add(key, "sPassword", json_object_new_string(password));
    if (authlevel)
        json_object_object_add(key, "iAuthLevel", json_object_new_int(*authlevel));
    if (userlevel)
        json_object_object_add(key, "iUserLevel", json_object_new_int(*userlevel));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_USER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("count(id)"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    if (ret) {
        json_object *j_array;
        json_object *j_ret;

        j_ret = json_tokener_parse(ret);
        j_array = json_object_object_get(j_ret, "jData");
        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_data = json_object_array_get_idx(j_array, i);
            num = (int)json_object_get_int(json_object_object_get(j_data, "count(id)"));
        }
        json_object_put(j_ret);

        g_free(ret);
    }

    return num;
}

extern "C" void dbserver_scopes_add(char *scopesItem)
{
    if (!scopesItem)
        return;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();
    json_object_object_add(data, "sScopeItem", json_object_new_string(scopesItem));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_SCOPES));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Insert"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

extern "C" void dbserver_system_del_by_key_char(char *table, char *key_word, char* key_content)
{
    if (!key_word)
        return;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();
    json_object_object_add(key, key_word, json_object_new_string(key_content));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    if (ret)
        g_free(ret);
}

extern "C" void dbserver_system_del_by_key_int(char *table, char *key_word, int key_content)
{
    if (!key_word)
        return;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();
    json_object_object_add(key, key_word, json_object_new_int(key_content));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    if (ret)
        g_free(ret);
}


extern "C" char *dbserver_storage_set(char *table, char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_storage_disk_path_get_by_name(char *name)
{
    if (!name)
        return NULL;

    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sName", json_object_new_string(name));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_DISK_PATH));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    printf("%s: 1: ret: %s\n", __FUNCTION__, ret);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_storage_disk_path_set_by_name(char *name, char *mountpath)
{
    if (!mountpath || !name)
        return NULL;

    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sName", json_object_new_string(name));

    json_object_object_add(data, "sPath", json_object_new_string(mountpath));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_DISK_PATH));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    printf("%s: 1: ret: %s\n", __FUNCTION__, ret);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_storage_get(char *table)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_get_storage_disk_path(char *mountpath)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (mountpath)
        json_object_object_add(key, "sMOUNTPATH", json_object_new_string(mountpath));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_DISK_PATH));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_get_storage_media_folder(void)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_MEDIA_FOLDER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_update_storage_media_folder_duty(int camid, int type, int duty, int maxnum)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "iCamId", json_object_new_int(camid));
    json_object_object_add(key, "iType", json_object_new_int(type));

    json_object_object_add(data, "iDuty", json_object_new_int(duty));
    json_object_object_add(data, "iMaxNum", json_object_new_int(maxnum));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_MEDIA_FOLDER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_get_storage_config(void)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "id", json_object_new_int(0));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_CONFIG));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_update_storage_config_mountpath(char *path)
{
    char *ret = NULL;
    if (path) {
        char *json_config;
        json_object *j_cfg = json_object_new_object();
        json_object *key = json_object_new_object();
        json_object *data = json_object_new_object();

        json_object_object_add(key, "id", json_object_new_int(0));

        json_object_object_add(data, "sMountPath", json_object_new_string(path));

        json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_CONFIG));
        json_object_object_add(j_cfg, "key", key);
        json_object_object_add(j_cfg, "data", data);
        json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

        json_config = (char *)json_object_to_json_string(j_cfg);

        ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
        json_object_put(j_cfg);
    }

    return ret;
}

extern "C" char *dbserver_update_storage_config_freesize(int freesize)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "id", json_object_new_int(0));

    json_object_object_add(data, "iFreeSize", json_object_new_int(freesize));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_CONFIG));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_get_storage_plan_snap(int id)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "id", json_object_new_int(id));
    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_PLAN_SNAP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_set_storage_plan_snap(char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_STORAGE_PLAN_SNAP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_STORAGE_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

/*
 * type_id:
 * 0: timing snapshot
 * 1: event snapshot
 */
extern "C" void dbserver_storage_snap_plan_parse(int type_id, int *ebaled, int *quality, int *interval, int *num)
{
    char* str = dbserver_get_storage_plan_snap(type_id);
    if (!str)
        return;
    json_object *storage_info = json_tokener_parse(str);
    if (storage_info == NULL)
        return;
    g_free(str);
    json_object *snap_plan_data = json_object_object_get(storage_info, "jData");
    json_object *snap_plan = json_object_array_get_idx(snap_plan_data, 0);
    json_object *iEnabled = json_object_object_get(snap_plan, "iEnabled");
    json_object *iImageQuality = json_object_object_get(snap_plan, "iImageQuality");
    json_object *iShotInterval = json_object_object_get(snap_plan, "iShotInterval");
    if (num) {
        json_object *iShotNumber = json_object_object_get(snap_plan, "iShotNumber");
        *num = (int)json_object_get_int(iShotNumber);
    }
    *ebaled = (int)json_object_get_int(iEnabled);
    *quality = (int)json_object_get_int(iImageQuality);
    *interval = (int)json_object_get_int(iShotInterval);

    json_object_put(storage_info);
}

extern "C" void dbserver_storage_video_plan_parse(int *enabled) {
    char *str = dbserver_storage_get((char *)TABLE_STORAGE_ADVANCE_PARA);
    if (!str)
        return;
    json_object *storage_info = json_tokener_parse(str);
    if (storage_info == NULL)
        return;
    g_free(str);
    json_object *video_plan_data = json_object_object_get(storage_info, "jData");
    json_object *video_plan = json_object_array_get_idx(video_plan_data, 0);

    json_object *iEnabled = json_object_object_get(video_plan, "iEnabled");
    *enabled = (int)json_object_get_int(iEnabled);

    json_object_put(storage_info);
}

extern "C" char *dbserver_network_ipv4_set(char *interface, char *Method, char *Address, char *Netmask, char *Gateway)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sInterface", json_object_new_string(interface));
    if (Method)
        json_object_object_add(data, "sV4Method", json_object_new_string(Method));
    if (Address)
        json_object_object_add(data, "sV4Address", json_object_new_string(Address));
    if (Netmask)
        json_object_object_add(data, "sV4Netmask", json_object_new_string(Netmask));
    if (Gateway)
        json_object_object_add(data, "sV4Gateway", json_object_new_string(Gateway));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_IP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_dns_set(char *interface, char *dns1, char *dns2)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sInterface", json_object_new_string(interface));
    if (dns1)
        json_object_object_add(data, "sDNS1", json_object_new_string(dns1));

    if (dns2)
        json_object_object_add(data, "sDNS2", json_object_new_string(dns2));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_IP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_nicspeed_set(char *interface, char *nicspeed)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sInterface", json_object_new_string(interface));
    if (nicspeed)
        json_object_object_add(data, "sNicSpeed", json_object_new_string(nicspeed));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_IP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_ip_get(char *interface)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    if (interface)
        json_object_object_add(key, "sInterface", json_object_new_string(interface));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_IP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_service_delete(char *service)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "sService", json_object_new_string(service));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_SERVICE));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_service_connect_set(char *service, char *password, int *favorite, int *autoconnect)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sService", json_object_new_string(service));
    if (password)
        json_object_object_add(data, "sPassword", json_object_new_string(password));
    if (favorite)
        json_object_object_add(data, "iFavorite", json_object_new_int(*favorite));
    if (autoconnect)
        json_object_object_add(data, "iAutoconnect", json_object_new_int(*autoconnect));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_SERVICE));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_service_get(char *service)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    if (service)
        json_object_object_add(key, "sService", json_object_new_string(service));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_SERVICE));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_power_get(char *type)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    if (type)
        json_object_object_add(key, "sType", json_object_new_string(type));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_POWER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_network_power_set(char *type, int power)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "sType", json_object_new_string(type));
    json_object_object_add(data, "iPower", json_object_new_int(power));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NETWORK_POWER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_wifi_power_set(int power)
{
    return dbserver_network_power_set((char *)"wifi", power);
}

extern "C" char *dbserver_ethernet_power_set(int power)
{
    return dbserver_network_power_set((char *)"ethernet", power);
}

extern "C" char *dbserver_wifi_power_get(void)
{
    return dbserver_network_power_get((char *)"wifi");
}

extern "C" char *dbserver_ethernet_power_get(void)
{
    return dbserver_network_power_get((char *)"ethernet");
}

extern "C" char *dbserver_ntp_set(char *servers, char *timezone, char *timezonefile, char *timezonefiledst, int *autodst, int *automode, int *time)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "id", json_object_new_int(0));
    if (servers)
        json_object_object_add(data, "sNtpServers", json_object_new_string(servers));
    if (timezone)
        json_object_object_add(data, "sTimeZone", json_object_new_string(timezone));
    if (timezonefile)
        json_object_object_add(data, "sTimeZoneFile", json_object_new_string(timezonefile));
    if (timezonefiledst)
        json_object_object_add(data, "sTimeZoneFileDst", json_object_new_string(timezonefiledst));
    if (autodst)
        json_object_object_add(data, "iAutoDst", json_object_new_int(*autodst));
    if (automode)
        json_object_object_add(data, "iAutoMode", json_object_new_int(*automode));
    if (time)
        json_object_object_add(data, "iRefreshTime", json_object_new_int(*time));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NTP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_ntp_get(void)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_NTP));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_zone_get(void)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_ZONE));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_port_set(char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_PORT));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_port_get(void)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_PORT));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_NET_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_media_set(char *table, char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_media_set_by_token(char *table, char *json, char *token)
{
    if ((strcmp(table, TABLE_PROFILE)) && (strcmp(table, TABLE_VIDEO_SOURCE))
        && (strcmp(table, TABLE_VIDEO_SOURCE_CONFIGURATION)))
        return NULL;
    if (!json)
        return NULL;

    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "sToken", json_object_new_string(token));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" void dbserver_media_del_by_token(char *table, char *token)
{
    if ((strcmp(table, TABLE_PROFILE)) && (strcmp(table, TABLE_VIDEO_SOURCE))
        && (strcmp(table, TABLE_VIDEO_SOURCE_CONFIGURATION)))
        return;

    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "sToken", json_object_new_string(token));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    if (ret)
        g_free(ret);
}

extern "C" char *dbserver_media_get(char *table)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_media_get_by_id(char *table, int id)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "id", json_object_new_int(id));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

char *dbserver_osd_get(void)
{
    char *ret = NULL;
    json_object *j_cfg = json_object_new_array();
    for (int i = 0; i < MAX_OSD_NUM; i++) {
        char *str = dbserver_media_get_by_id((char *)TABLE_OSD, i);
        json_object *j_ret = json_tokener_parse(str);
        json_object *j_data = json_object_object_get(j_ret, "jData");
        json_object *j_data_0 = json_object_array_get_idx(j_data, 0);
        json_object_array_add(j_cfg, json_object_get(j_data_0));
        if (str)
            g_free(str);
        json_object_put(j_ret);
    }

    ret = (char *)json_object_get_string(j_cfg);
    json_object_put(j_cfg);
    return ret;
}

extern "C" char *dbserver_audio_set(char *json)
{
    return dbserver_media_set((char *)TABLE_AUDIO, json, 0);
}

extern "C" char *dbserver_audio_get(void)
{
    return dbserver_media_get((char *)TABLE_AUDIO);
}

extern "C" char *dbserver_video_set(char* json, char* stream_type)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    if (g_str_equal(stream_type, "mainStream"))
        json_object_object_add(key, "id", json_object_new_int(0));
    if (g_str_equal(stream_type, "subStream"))
        json_object_object_add(key, "id", json_object_new_int(1));
    if (g_str_equal(stream_type, "thirdStream"))
        json_object_object_add(key, "id", json_object_new_int(2));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_VIDEO));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_video_get(void)
{
    return dbserver_media_get((char *)TABLE_VIDEO);
}

extern "C" char *dbserver_video_region_clip_set(char* json, int id)
{
    return dbserver_media_set((char *)TABLE_VIDEO_REGION_CLIP, json, id);
}

extern "C" char *dbserver_video_region_clip_get(void)
{
    return dbserver_media_get((char *)TABLE_VIDEO_REGION_CLIP);
}

extern "C" char *dbserver_stream_url_set(char *json, int id)
{
    return dbserver_media_set((char *)TABLE_STREAM_URL, json, id);
}

extern "C" char *dbserver_stream_url_get(void)
{
    return dbserver_media_get((char *)TABLE_STREAM_URL);
}

extern "C" char *dbserver_media_get_by_key_char(char *table, char *key_word, char *val)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, key_word, json_object_new_string(val));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_media_profile_get(char *token)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (token)
        json_object_object_add(key, "sToken", json_object_new_string(token));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_PROFILE));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_video_source_cfg_get(char *token)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (token)
        json_object_object_add(key, "sToken", json_object_new_string(token));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_VIDEO_SOURCE_CONFIGURATION));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_video_source_get(char *token)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (token)
        json_object_object_add(key, "sVideoSourceToken", json_object_new_string(token));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_VIDEO_SOURCE));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_video_enc_cfg_get(char *token)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    if (token)
        json_object_object_add(key, "sVideoEncoderConfigurationToken", json_object_new_string(token));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_VIDEO));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_MEDIA_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_set(char *table, char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_get(char *table)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_get_by_key_char(char *table, char *key_word, char *val)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, key_word, json_object_new_string(val));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_para_get_by_name(char *para_name)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "name", json_object_new_string(para_name));
    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_PARA));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("para"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_system_para_set_by_name(char *para_name, char *para)
{
    if (!para)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    json_object_object_add(key, "name", json_object_new_string(para_name));
    json_object_object_add(data, "para", json_object_new_string(para));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_PARA));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" void dbserver_set_static_cap_option(struct StaticLocation location, char *options)
{
    char *ret = NULL;
    char *para_str = dbserver_system_para_get_by_name(location.cap_name);
    json_object *para_cap = json_tokener_parse(para_str);
    json_object *para_cap_jdata = json_object_object_get(para_cap, "jData");
    json_object *para_cap_0 = json_object_array_get_idx(para_cap_jdata, 0);
    json_object *para_cap_content = json_object_object_get(para_cap_0, "para");
    const char* old_str = json_object_get_string(para_cap_content);
    json_object *old_cap = json_tokener_parse(old_str);

    json_object *static_cap = json_object_object_get(old_cap, "static");
    if (!static_cap) {
        static_cap = json_object_new_object();
        json_object_object_add(old_cap, "static", static_cap);
    }

    // delete old data
    if (json_object_object_get(static_cap, location.target_key) != NULL)
    {
        json_object_object_del(static_cap, location.target_key);
    }

    // if options is not NULL, add new options
    if (NULL != options) {
        json_object *new_cap = json_object_new_object();
        json_object_object_add(new_cap, "type", json_object_new_string("options"));
        json_object *option_cfg = json_tokener_parse(options);
        json_object_object_add(new_cap, "options", option_cfg);
        json_object_object_add(static_cap, location.target_key, new_cap);
    }

    ret = dbserver_system_para_set_by_name(location.cap_name, (char *)json_object_to_json_string(old_cap));

    json_object_put(para_cap);
    json_object_put(old_cap);

    if (ret) {
        g_free(ret);
    }
}

// old_cap -> dynamic_cap -> dynamic_content -> dynamic_content_valcontent -> option_cap
extern "C" void dbserver_set_dynamic_cap_option(struct DynamicLocation location, char *options)
{
    char *ret = NULL;
    char *para_str = dbserver_system_para_get_by_name(location.cap_name);
    json_object *para_cap = json_tokener_parse(para_str);
    json_object *para_cap_jdata = json_object_object_get(para_cap, "jData");
    json_object *para_cap_0 = json_object_array_get_idx(para_cap_jdata, 0);
    json_object *para_cap_content = json_object_object_get(para_cap_0, "para");
    const char* old_str = json_object_get_string(para_cap_content);
    json_object *old_cap = json_tokener_parse(old_str);

    json_object *dynamic_content = NULL;
    json_object *dynamic_content_valcontent = NULL;

    json_object *dynamic_cap = json_object_object_get(old_cap, "dynamic");
    if (!dynamic_cap)
    {
        if (NULL == options) {
            json_object_put(para_cap);
            return;
        }
        dynamic_cap = json_object_new_object();
        json_object_object_add(old_cap, "dynamic", dynamic_cap);
    }

    if (json_object_object_get(dynamic_cap, location.dynamic_key) != NULL)
    {
        dynamic_content = json_object_object_get(dynamic_cap, location.dynamic_key);
    } else if (NULL != options)
    {
        dynamic_content = json_object_new_object();
        json_object_object_add(dynamic_cap, location.dynamic_key, dynamic_content);
    }

    if (json_object_object_get(dynamic_content, location.dynamic_val) != NULL)
    {
        dynamic_content_valcontent = json_object_object_get(dynamic_content, location.dynamic_val);
        if (json_object_object_get(dynamic_content_valcontent, location.target_key) != NULL)
        {
            json_object_object_del(dynamic_content_valcontent, location.target_key);
        }
    } else if (NULL != options)
    {
        dynamic_content_valcontent = json_object_new_object();
        json_object_object_add(dynamic_content, location.dynamic_val, dynamic_content_valcontent);
    }

    if (NULL != options) {
        json_object *option_cap = json_object_new_object();
        json_object_object_add(option_cap, "type", json_object_new_string("options"));
        json_object *option_cfg = json_tokener_parse(options);
        json_object_object_add(option_cap, "options", option_cfg);
        json_object_object_add(dynamic_content_valcontent, location.target_key, option_cap);
    }

    ret = dbserver_system_para_set_by_name(location.cap_name, (char *)json_object_get_string(old_cap));

    json_object_put(para_cap);
    json_object_put(old_cap);

    if (ret) {
        g_free(ret);
    }
}

extern "C" void dbserver_set_static_cap_range(struct StaticLocation location, struct RangeJsonPara range)
{
    char *ret = NULL;
    char *para_str = dbserver_system_para_get_by_name(location.cap_name);
    json_object *para_cap = json_tokener_parse(para_str);
    json_object *para_cap_jdata = json_object_object_get(para_cap, "jData");
    json_object *para_cap_0 = json_object_array_get_idx(para_cap_jdata, 0);
    json_object *para_cap_content = json_object_object_get(para_cap_0, "para");
    const char* old_str = json_object_get_string(para_cap_content);
    json_object *old_cap = json_tokener_parse(old_str);

    json_object *static_cap = json_object_object_get(old_cap, "static");
    if (!static_cap) {
        static_cap = json_object_new_object();
        json_object_object_add(old_cap, "static", static_cap);
    }

    // delete old data
    if (json_object_object_get(static_cap, location.target_key) != NULL)
    {
        json_object_object_del(static_cap, location.target_key);
    }

    // if options is not NULL, add new options
    if (range.step >= 0) {
        json_object *new_cap = json_object_new_object();
        json_object_object_add(new_cap, "type", json_object_new_string("range"));
        json_object *range_cfg = json_object_new_object();
        json_object_object_add(range_cfg, "min", json_object_new_int(range.min));
        json_object_object_add(range_cfg, "max", json_object_new_int(range.max));
        if (range.step != 0)
            json_object_object_add(range_cfg, "step", json_object_new_int(range.step));
        json_object_object_add(new_cap, "range", range_cfg);
        json_object_object_add(static_cap, location.target_key, new_cap);
    }

    ret = dbserver_system_para_set_by_name(location.cap_name, (char *)json_object_to_json_string(old_cap));

    json_object_put(para_cap);
    json_object_put(old_cap);

    if (ret) {
        g_free(ret);
    }
}

// old_cap -> dynamic_cap -> dynamic_content -> dynamic_content_valcontent -> new_cap
extern "C" void dbserver_set_dynamic_cap_range(struct DynamicLocation location, struct RangeJsonPara range)
{
    char *ret = NULL;
    char *para_str = dbserver_system_para_get_by_name(location.cap_name);
    json_object *para_cap = json_tokener_parse(para_str);
    json_object *para_cap_jdata = json_object_object_get(para_cap, "jData");
    json_object *para_cap_0 = json_object_array_get_idx(para_cap_jdata, 0);
    json_object *para_cap_content = json_object_object_get(para_cap_0, "para");
    const char* old_str = json_object_get_string(para_cap_content);
    json_object *old_cap = json_tokener_parse(old_str);

    json_object *dynamic_content = NULL;
    json_object *dynamic_content_valcontent = NULL;

    json_object *dynamic_cap = json_object_object_get(old_cap, "dynamic");
    if (!dynamic_cap)
    {
        if (range.step < 0) {
            json_object_put(para_cap);
            return;
        }
        dynamic_cap = json_object_new_object();
        json_object_object_add(old_cap, "dynamic", dynamic_cap);
    }

    if (json_object_object_get(dynamic_cap, location.dynamic_key) != NULL)
    {
        dynamic_content = json_object_object_get(dynamic_cap, location.dynamic_key);
    } else if (range.step >= 0)
    {
        dynamic_content = json_object_new_object();
        json_object_object_add(dynamic_cap, location.dynamic_key, dynamic_content);
    }

    if (json_object_object_get(dynamic_content, location.dynamic_val) != NULL)
    {
        dynamic_content_valcontent = json_object_object_get(dynamic_content, location.dynamic_val);
        if (json_object_object_get(dynamic_content_valcontent, location.target_key) != NULL)
        {
            json_object_object_del(dynamic_content_valcontent, location.target_key);
        }
    } else if (range.step >= 0)
    {
        dynamic_content_valcontent = json_object_new_object();
        json_object_object_add(dynamic_content, location.dynamic_val, dynamic_content_valcontent);
    }

    if (range.step >= 0) {
        json_object *new_cap = json_object_new_object();
        json_object_object_add(new_cap, "type", json_object_new_string("range"));
        json_object *range_cfg = json_object_new_object();
        json_object_object_add(range_cfg, "min", json_object_new_int(range.min));
        json_object_object_add(range_cfg, "max", json_object_new_int(range.max));
        if (range.step != 0)
            json_object_object_add(range_cfg, "step", json_object_new_int(range.step));
        json_object_object_add(new_cap, "range", range_cfg);
        json_object_object_add(dynamic_content_valcontent, location.target_key, new_cap);
    }

    ret = dbserver_system_para_set_by_name(location.cap_name, (char *)json_object_get_string(old_cap));

    json_object_put(para_cap);
    json_object_put(old_cap);

    if (ret) {
        g_free(ret);
    }
}

extern "C" void dbserver_system_user_delete(int id)
{
    char *ret = NULL;
    char *json_config;
    json_object *key = json_object_new_object();
    json_object *j_cfg = json_object_new_object();
    json_object_object_add(key, "id", json_object_new_int(id));
    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_SYSTEM_USER));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_cmd(json_config, (char *)DBSERVER_SYSTEM_INTERFACE);

    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

extern "C" char *dbserver_event_set(char *table, char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_event_set_by_char_key(char *table, char *json, char* key_name, char* key_content)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, key_name, json_object_new_string(key_content));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_event_get(char *table)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_event_get_by_id(char *table, int id)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, "id", json_object_new_int(id));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_event_get_by_key_int(char *table, char *key_word, int val)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, key_word, json_object_new_int(val));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_event_get_by_key_char(char *table, char *key_word, char *val)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, key_word, json_object_new_string(val));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" void dbserver_event_delete_by_key_int(char *table, char *key_word, int val)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, key_word, json_object_new_int(val));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    if (ret)
        g_free(ret);
}

extern "C" void dbserver_event_delete_by_key_char(char *table, char *key_word, char *val)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(key, key_word, json_object_new_string(val));
    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

    if (ret)
        g_free(ret);
}

extern "C" void dbserver_face_list_add(int id, char *path, char *name, char *type)
{
    char *ret = NULL;
    char *json_config;
    char current_time[30];
    time_t raw_time;
    struct tm *time_info;
    json_object *j_cfg = json_object_new_object();

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(current_time, 30, "%FT%T", time_info);

    json_object_object_add(j_cfg, "sRegistrationTime", json_object_new_string(current_time));
    json_object_object_add(j_cfg, "sPicturePath", json_object_new_string(path));
    json_object_object_add(j_cfg, "sName", json_object_new_string(name));
    json_object_object_add(j_cfg, "sType", json_object_new_string(type));
    json_object_object_add(j_cfg, "sNote", json_object_new_string("undone"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_event_set((char *)TABLE_FACE_LIST, json_config, id);

    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

extern "C" void dbserver_face_load_complete(int id, int flag)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "iLoadCompleted", json_object_new_int(flag));
    json_object_object_add(j_cfg, "sNote", json_object_new_string("undone"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_event_set((char *)TABLE_FACE_LIST, json_config, id);

    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

extern "C" void dbserver_face_load_complete_by_path(char* path, int flag, int face_db_id)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "iLoadCompleted", json_object_new_int(flag));
    json_object_object_add(j_cfg, "iFaceDBId", json_object_new_int(face_db_id));
    json_object_object_add(j_cfg, "sNote", json_object_new_string("undone"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_event_set_by_char_key((char *)TABLE_FACE_LIST, json_config, (char *)"sPicturePath", path);

    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

extern "C" char *dbserver_face_list_delete(int id)
{
    char *str = dbserver_event_get_by_id((char *)TABLE_FACE_LIST, id);
    if (!str)
        return NULL;
    json_object *face_info = json_tokener_parse(str);
    g_free(str);
    if (face_info == NULL)
        return NULL;
    json_object *face_data = json_object_object_get(face_info, "jData");
    json_object *face = json_object_array_get_idx(face_data, 0);
    if (!face) {
        printf("%s face NULL\n", __func__);
        json_object_put(face_info);
        return NULL;
    }
    json_object *pic_path = json_object_object_get(face, "sPicturePath");
    if (!pic_path) {
        printf("%s pic_path NULL\n", __func__);
        json_object_put(face_info);
        return NULL;
    }
    const char *path = json_object_get_string(pic_path);
    if (!path) {
        printf("%s path NULL\n", __func__);
        json_object_put(face_info);
        return NULL;
    }
    json_object *face_id = json_object_object_get(face, "iFaceDBId");

    char *ret = NULL;
    char *json_config;
    json_object *key = json_object_new_object();
    json_object *j_cfg = json_object_new_object();
    json_object_object_add(key, "id", json_object_new_int(id));
    json_object_object_add(key, "sPicturePath", json_object_new_string(path));
    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_FACE_LIST));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);

    char *delete_string = (char *)json_object_get_string(face_info);

    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
    return delete_string;
}

extern "C" void dbserver_face_reset(char *table)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Delete"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);

		if (ret)
				g_free(ret);
}

extern "C" void dbserver_snapshot_record_set(char *path)
{
    char *ret = NULL;
    char *json_config;
    char current_time[30];
    time_t raw_time;
    struct tm *time_info;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(current_time, 30, "%FT%T", time_info);
    json_object_object_add(data, "sTime", json_object_new_string(current_time));
    json_object_object_add(data, "sPicturePath", json_object_new_string(path));
    json_object_object_add(data, "sStatus", json_object_new_string("Processed"));
    json_object_object_add(data, "sNote", json_object_new_string("Snapshot"));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_FACE_SNAPSHOT_RECORD));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Insert"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

extern "C" void dbserver_control_record_set(int face_id, char *path, char *status, char *similarity)
{
    char *ret = NULL;
    char *json_config;
    char current_time[30];
    time_t raw_time;
    struct tm *time_info;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_object_new_object();

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(current_time, 30, "%FT%T", time_info);
    json_object_object_add(data, "sTime", json_object_new_string(current_time));
    json_object_object_add(data, "iFaceId", json_object_new_int(face_id));
    json_object_object_add(data, "sSnapshotPath", json_object_new_string(path));
    json_object_object_add(data, "sStatus", json_object_new_string(status));
    json_object_object_add(data, "sSimilarity", json_object_new_string(similarity));

    json_object_object_add(j_cfg, "table", json_object_new_string(TABLE_FACE_CONTROL_RECORD));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Insert"));
    json_config = (char *)json_object_to_json_string(j_cfg);
    ret = dbserver_cmd(json_config, (char *)DBSERVER_EVENT_INTERFACE);
    json_object_put(j_cfg);
    if (ret)
        g_free(ret);
}

/*
 * type_id:
 * 0: for motion detect
 * 1: for intrusion detect
 * 2: for storage video plan
 * 3: for storage screenshot. remark:must be last for add other type of schedule
 */
extern "C" void dbserver_event_schedules_parse(struct week *schedule_week, int type_id)
{
    char *str = dbserver_event_get_by_id((char *)TABLE_EVENT_SCHEDULES, type_id);
    do {
        if (!str)
            break;
        json_object *schedules_info = json_tokener_parse(str);
        if (schedules_info == NULL)
            break;
        do {
            do {
                json_object *schedule_data = json_object_object_get(schedules_info, "jData");
                json_object *schedule = json_object_array_get_idx(schedule_data, 0);
                json_object *week_s = json_object_object_get(schedule, "sSchedulesJson");

                if (!week_s) {
                    printf("%s: week_s is null\n");
                    break;
                }

                json_object *week_js = json_tokener_parse(json_object_get_string(week_s));
                // printf("week_js is %s\n", json_object_to_json_string(week_js));
                if (week_js == NULL)
                    break;
                json_object *day_js = NULL;
                json_object *period_js = NULL;
                for (int i = 0 ; i < 7 ; i++) {
                    day_js = json_object_array_get_idx(week_js, i);
                    int day_period_size = json_object_array_length(day_js);
                    // printf("day_js is %s\n", json_object_to_json_string(day_js));
                    // printf("day_period_size is %d\n", day_period_size);
                    for (int j = 0; j < day_period_size; j++) {
                        period_js = json_object_array_get_idx(day_js, j);
                        json_object *start = json_object_object_get(period_js, "start");
                        json_object *end = json_object_object_get(period_js, "end");
                        json_object *type = json_object_object_get(period_js, "type");
                        schedule_week->week_day[i].day_period[j].start_minute = round(json_object_get_double(start) * 1440.0);
                        schedule_week->week_day[i].day_period[j].end_minute = round(json_object_get_double(end) * 1440.0);
                        if(type != NULL)
                            strcpy(schedule_week->week_day[i].day_period[j].type, json_object_get_string(type));
                    }
                }
                json_object_put(week_js);
            } while(0);
            json_object_put(schedules_info);
        } while (0);
        g_free(str);
    } while (0);

}

extern "C" void dbserver_event_triggers_parse(int id, int *record_ebaled)
{
    char *str = dbserver_event_get((char *)TABLE_EVENT_TRIGGERS);
    if (!str)
        return;
    json_object *event_info = json_tokener_parse(str);
    if (event_info == NULL)
        return;
    g_free(str);
    json_object *trigger_data = json_object_object_get(event_info, "jData");
    json_object *trigger = json_object_array_get_idx(trigger_data, id);

    json_object *iRecord1Enabled = json_object_object_get(trigger, "iNotificationRecord1Enabled");
    *record_ebaled = (int)json_object_get_int(iRecord1Enabled);

    json_object_put(event_info);
}

extern "C" char *dbserver_peripherals_set(char *table, char *json, int id)
{
    if (!json)
        return NULL;
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();
    json_object *data = json_tokener_parse(json);

    json_object_object_add(key, "id", json_object_new_int(id));

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", data);
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Update"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_PERIPHERALS_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_peripherals_get(char *table)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();
    json_object *key = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "key", key);
    json_object_object_add(j_cfg, "data", json_object_new_string("*"));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Select"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, (char *)DBSERVER_PERIPHERALS_INTERFACE);
    json_object_put(j_cfg);

    return ret;
}


extern "C" char *dbserver_drop_table(char *table, char *interface)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Drop"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, interface);

    json_object_put(j_cfg);

    return ret;
}

extern "C" char *dbserver_create_table(char *table, char *col, char *interface)
{
    char *ret = NULL;
    char *json_config;
    json_object *j_cfg = json_object_new_object();

    json_object_object_add(j_cfg, "table", json_object_new_string(table));
    json_object_object_add(j_cfg, "col", json_object_new_string(col));
    json_object_object_add(j_cfg, "cmd", json_object_new_string("Create"));

    json_config = (char *)json_object_to_json_string(j_cfg);

    ret = dbserver_cmd(json_config, interface);

    json_object_put(j_cfg);

    return ret;
}

// WARN: when col_para alter, sync it to dbserver/event
extern "C" int dbserver_reset_face_table(char *table)
{
    char *col_para = NULL;
    char *ret = NULL;
    if (g_str_equal(table, TABLE_FACE_LIST)) {
        col_para = (char *)"id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "sPicturePath TEXT DEFAULT ''," \
               "sRegistrationTime TEXT DEFAULT '2020-01-01T00:00:00'," \
               "iAge INT DEFAULT 20," \
               "sListType TEXT DEFAULT 'permanent'," \
               "sType TEXT DEFAULT 'whiteList'," \
               "sName TEXT DEFAULT 'User'," \
               "sGender TEXT DEFAULT 'male'," \
               "sNation TEXT DEFAULT '汉'," \
               "sCertificateType TEXT DEFAULT 'identityCard'," \
               "sCertificateNumber TEXT DEFAULT ''," \
               "sBirthday TEXT DEFAULT '2000-01-01'," \
               "sTelephoneNumber TEXT DEFAULT ''," \
               "sHometown TEXT DEFAULT ''," \
               "sAddress TEXT DEFAULT ''," \
               "iAccessCardNumber INT DEFAULT 0," \
               "iLoadCompleted INT DEFAULT 0," \
               "iFaceDBId INT DEFAULT -1," \
               "sNote TEXT DEFAULT ''";
    } else if (g_str_equal(table, TABLE_FACE_SNAPSHOT_RECORD)) {
        col_para = (char *)"id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "sTime TEXT DEFAULT '2020-01-01T00:00:00'," \
               "sPicturePath TEXT DEFAULT ''," \
               "sStatus TEXT DEFAULT 'Processed'," \
               "sNote TEXT DEFAULT 'Snapshot'";
    } else if (g_str_equal(table, TABLE_FACE_CONTROL_RECORD)) {
        col_para = (char *)"id INTEGER PRIMARY KEY AUTOINCREMENT," \
               "sTime TEXT DEFAULT '2020-01-01T00:00:00'," \
               "iFaceId INT DEFAULT 0," \
               "sSnapshotPath TEXT DEFAULT ''," \
               "sStatus TEXT DEFAULT 'open'," \
               "sSimilarity TEXT DEFAULT '75.00'";
    } else {
        return -1;
    }

    ret = dbserver_drop_table(table, (char *)DBSERVER_EVENT_INTERFACE);
    if (ret)
        g_free(ret);
    ret = dbserver_create_table(table, col_para, (char *)DBSERVER_EVENT_INTERFACE);
    if (ret)
        g_free(ret);

    return 0;
}

extern "C" void dbserver_free(char *ret_str)
{
    if (ret_str)
        g_free(ret_str);
}