// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>
#include <gdbus.h>

#include "json-c/json.h"
#include "call_fun_ipc.h"
#include "shared_memory.h"
#include "dbus.h"

static DBusConnection *dbus_conn_s = NULL;
static struct FunMap *fun_map = NULL;
static int fun_num = 0;
char *DbusName = NULL;
char *DbusPath = NULL;
char *DbusIf = NULL;
char *SharePath = NULL;

struct Share_s {
    char *addr;
    int shmid;
    struct Share_s* next;
};

static struct Share_s* share_client_list = NULL;
static struct Share_s* share_server_list = NULL;

void client_list_add(struct Share_s* share)
{
    if (share_client_list == NULL) {
        share_client_list = share;
    } else {
        struct Share_s* tmp = share_client_list;
        while (tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = share;
    }
}

void client_list_del(int shmid)
{
    if (share_client_list) {
        if (share_client_list->shmid == shmid) {
            struct Share_s* tmp = share_client_list->next;
            free(share_client_list);
            share_client_list = tmp;
        } else {
            struct Share_s* tmp = share_client_list;
            while (tmp->next) {
                if (tmp->next->shmid == shmid) {
                   struct Share_s* tmp_next = tmp->next;
                   tmp->next = tmp_next->next;
                   free(tmp_next);
                }
            }
        }
    }
}

void server_list_add(struct Share_s* share)
{
    if (share_server_list == NULL) {
        share_server_list = share;
    } else {
        struct Share_s* tmp = share_server_list;
        while (tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = share;
    }
}

void server_list_del(int shmid)
{
    if (share_server_list) {
        if (share_server_list->shmid == shmid) {
            struct Share_s* tmp = share_server_list->next;
            free(share_server_list);
            share_server_list = tmp;
        } else {
            struct Share_s* tmp = share_server_list;
            while (tmp->next) {
                if (tmp->next->shmid == shmid) {
                   struct Share_s* tmp_next = tmp->next;
                   tmp->next = tmp_next->next;
                   free(tmp_next);
                }
            }
        }
    }
}

DBusMessage *dbus_message_new_method_return_string(DBusMessage *msg, char *str)
{
    DBusMessageIter array;
    DBusMessage *reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);

    return reply;
}

DBusMessage *method_callfun(DBusConnection *conn,
                           DBusMessage *msg, void *data)
{
    int ret = 0;
    char *json_str;
    json_object *j_cfg;

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    j_cfg = json_tokener_parse(json_str);
    //printf("%s, json_str = %s\n", __func__, json_str);
    char *FunName = (char *)json_object_get_string(json_object_object_get(j_cfg, "FunName"));

    for (int i = 0; i < fun_num; i++) {
        if (fun_map[i].fun_name && fun_map[i].fun) {
            if (!strcmp(fun_map[i].fun_name, FunName)) {
                void *addr = NULL;
                char *SharePath = (char *)json_object_get_string(json_object_object_get(j_cfg, "SharePath"));
                int ShareId = (int)json_object_get_int(json_object_object_get(j_cfg, "ShareId"));
                int ShareSize = (int)json_object_get_int(json_object_object_get(j_cfg, "ShareSize"));
                int shmid = GetShm(SharePath, ShareId, ShareSize);
                if (ShareSize > 0) {
                    struct Share_s *share_server;
                    addr = (void *)Shmat(shmid);
                    share_server = calloc(sizeof(struct Share_s), 1);
                    share_server->shmid = shmid;
                    share_server->addr = addr;
                    server_list_add(share_server);
                }
                fun_map[i].fun(addr);
                if (ShareSize > 0) {
                    Shmdt((char *)addr);
                    server_list_del(shmid);
                }
            }
        }
    }

    json_object_put(j_cfg);

    json_object *j_ret = json_object_new_object();
    json_object_object_add(j_ret, "Return", json_object_new_int(ret));
    char *str = (char *)json_object_to_json_string(j_ret);

    DBusMessage *reply = dbus_message_new_method_return_string(msg, str);
    json_object_put(j_ret);

    return reply;
}

const GDBusMethodTable methods[] = {
    {
        GDBUS_ASYNC_METHOD("callfun",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_callfun)
    },
    { },
};

const GDBusSignalTable signals[] = {
    {
        GDBUS_SIGNAL("callback",
        GDBUS_ARGS({ "json", "s" }))
    },
    { },
};

static int dbus_manager_init(DBusConnection *dbus_conn, char *dbus_if, char *dbus_path)
{
    g_dbus_register_interface(dbus_conn, dbus_path,
                              dbus_if,
                              methods,
                              signals, NULL, dbus_path, NULL);

    return 0;
}

void call_fun_ipc_server_init(struct FunMap *map, int num, char *dbus_name, char *dbus_if, char *dbus_path, int needloop)
{
    DBusError dbus_err;

    if (needloop == 0)
        callfunipc_disable_loop();
    dbus_error_init(&dbus_err);
    dbus_conn_s = g_dbus_setup_bus(DBUS_BUS_SYSTEM, dbus_name, &dbus_err);
    fun_map = map;
    fun_num = num;
    dbus_manager_init(dbus_conn_s, dbus_if, dbus_path);
}

void call_fun_ipc_server_deinit(void)
{
    struct Share_s* tmp = share_server_list;

    share_server_list = NULL;
    while (tmp) {
        struct Share_s* tmp1 = tmp;

        if (tmp->addr >= 0)
            Shmdt((char *)tmp->addr);
        tmp = tmp->next;
        free(tmp1);
    }
}

int call_fun_ipc_call(char *funname, void *data, int len, int restore)
{
    int retry_cnt = 0;
    static int projid = 0;
    int ret = 0xfefefefe;
    char *str_ret = NULL;
    int shmid;
    void *addr;
    json_object *j_cfg = json_object_new_object();

    if (len > 0) {
        struct Share_s *share_client;
        projid++;

        shmid = CreateShm((char *)SharePath, projid, len);
        if (shmid < 0)
            return ret;
        addr = (void *)Shmat(shmid);
        share_client = calloc(sizeof(struct Share_s), 1);
        share_client->shmid = shmid;
        share_client->addr = addr;
        client_list_add(share_client);
        memcpy(addr, data, len);
    }
    json_object_object_add(j_cfg, "FunName", json_object_new_string(funname));
    json_object_object_add(j_cfg, "SharePath", json_object_new_string(SharePath));
    json_object_object_add(j_cfg, "ShareId", json_object_new_int(projid));
    json_object_object_add(j_cfg, "ShareSize", json_object_new_int(len));

    struct UserData* userdata;
    userdata = callfunipc_dbus_connection();
    retry_cnt = 0;
retry:
    callfunipc_dbus_method_call(userdata->connection,
                     DbusName, DbusPath,
                     DbusIf, "callfun",
                     callfunipc_populate_get, userdata, callfunipc_append_path, (char *)json_object_to_json_string(j_cfg));
    if (callfunipc_dbus_async(userdata) == -1 && retry_cnt++ < 2)
        goto retry;

    str_ret = userdata->json_str;
    callfunipc_dbus_deconnection(userdata);

    json_object_put(j_cfg);

    if (str_ret) {
        json_object *j_ret = json_tokener_parse(str_ret);
        ret = (int)json_object_get_int(json_object_object_get(j_ret, "Return"));
        json_object_put(j_ret);
        g_free(str_ret);
    }
    if (len > 0) {
        if (restore)
            memcpy(data, addr, len);
        Shmdt((char *)addr);
        DestroyShm(shmid);
        client_list_del(shmid);
    }

    return ret;
}

void call_fun_ipc_client_init(char *dbus_name, char *dbus_if, char *dbus_path, char *share_path, int needloop)
{
    DBusError dbus_err;

    if (needloop == 0)
        callfunipc_disable_loop();
    dbus_error_init(&dbus_err);
    dbus_conn_s = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, &dbus_err);
    printf("%s dbus_conn_s = %d\n", __func__, dbus_conn_s);
    DbusName = g_strdup(dbus_name);
    DbusPath = g_strdup(dbus_path);
    DbusIf = g_strdup(dbus_if);
    SharePath = g_strdup(share_path);
    while (call_fun_ipc_call("", NULL, 0, 0) == 0xfefefefe) {
        usleep(100000);
        //printf("%s wait\n", __func__);
    }
}

void call_fun_ipc_client_deinit(void)
{
    struct Share_s* tmp = share_client_list;

    share_client_list = NULL;
    while (tmp) {
        struct Share_s* tmp1 = tmp;

        if (tmp->addr >= 0)
            Shmdt((char *)tmp->addr);
        if (tmp->shmid >= 0)
            DestroyShm(tmp->shmid);
        tmp = tmp->next;
        free(tmp1);
    }
}