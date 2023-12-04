// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
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
#include "dbus.h"

#define TIMEOUT         120000
static DBusConnection *dbusconn = 0;
static int need_loop = 1;
GMainLoop *main_loop = NULL;

struct dbus_callback {
    dbus_method_return_func_t cb;
    void *user_data;
};

static pthread_mutex_t mutex;

static void dbus_method_reply(DBusPendingCall *call, void *user_data)
{
    struct dbus_callback *callback = user_data;
    int res = 0;
    DBusMessage *reply;
    DBusMessageIter iter;

    reply = dbus_pending_call_steal_reply(call);
    dbus_pending_call_unref(call);
    if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
        DBusError err;

        dbus_error_init(&err);
        dbus_set_error_from_message(&err, reply);

        callback->cb(NULL, err.message, callback->user_data);

        dbus_error_free(&err);
        goto end;
    }

    dbus_message_iter_init(reply, &iter);
    res = callback->cb(&iter, NULL, callback->user_data);

end:

    g_free(callback);
    dbus_message_unref(reply);
}

static int send_method_call(DBusConnection *connection,
                            DBusMessage *message, dbus_method_return_func_t cb,
                            void *user_data)
{
    int res = -ENXIO;
    DBusPendingCall *call;
    struct dbus_callback *callback;

    if (!dbus_connection_send_with_reply(connection, message, &call, TIMEOUT))
        goto end;

    if (!call)
        goto end;

    if (cb) {
        callback = g_new0(struct dbus_callback, 1);
        callback->cb = cb;
        callback->user_data = user_data;
        dbus_pending_call_set_notify(call, dbus_method_reply,
                                     callback, NULL);
        res = -EINPROGRESS;
    }

end:
    dbus_message_unref(message);
    return res;
}

int callfunipc_dbus_method_call(DBusConnection *connection,
                     const char *service, const char *path, const char *interface,
                     const char *method, dbus_method_return_func_t cb,
                     void *user_data, dbus_append_func_t append_func,
                     void *append_data)
{
    DBusMessage *message;
    DBusMessageIter iter;

    message = dbus_message_new_method_call(service, path, interface,
                                           method);

    if (!message)
        return -ENOMEM;

    if (append_func) {
        dbus_message_iter_init_append(message, &iter);
        append_func(&iter, append_data);
    }

    return send_method_call(connection, message, cb, user_data);
}

void callfunipc_append_path(DBusMessageIter *iter, void *user_data)
{
    const char *path = user_data;

    dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &path);
}

int callfunipc_populate_set(DBusMessageIter *iter, const char *error,
                 void *user_data)
{
    char *json;
    struct UserData* userdata = user_data;

    if (userdata)
        pthread_mutex_unlock(&userdata->mutex);

    return 0;
}

int callfunipc_populate_get(DBusMessageIter *iter, const char *error,
                 void *user_data)
{
    char *json;
    struct UserData* userdata = user_data;

    if (error) {
        printf("%s err\n", __func__);
        if (userdata)
            pthread_mutex_unlock(&userdata->mutex);

        return 0;
    }

    dbus_message_iter_get_basic(iter, &json);
    userdata->json_str = g_strdup(json);

    if (userdata)
        pthread_mutex_unlock(&userdata->mutex);

    return 0;
}

static void *loop_thread(void *arg)
{
    main_loop = g_main_loop_new(NULL, FALSE);

    //g_timeout_add(100, time_cb, NULL);
    g_main_loop_run(main_loop);

    if (main_loop)
        g_main_loop_unref(main_loop);

    main_loop = NULL;

    return 0;
}

int callfunipc_dbus_async(struct UserData* userdata)
{
    struct timespec tout;

    if (need_loop && main_loop == NULL) {
        pthread_t tid;
        pthread_create(&tid, NULL, (void*)loop_thread, NULL);
    }

    clock_gettime(CLOCK_REALTIME, &tout);
    tout.tv_sec += 1;

    if (pthread_mutex_timedlock(&userdata->mutex, &tout) != 0) {
        printf("%s again get\n", __func__);
        return -1;
    }

    pthread_mutex_unlock(&userdata->mutex);

    return 0;
}

void callfunipc_dbus_deconnection(struct UserData* userdata)
{
    dbus_connection_unref(userdata->connection);
    if (userdata)
        g_free(userdata);
}

struct UserData* callfunipc_dbus_connection(void)
{
    DBusError dbus_err;
    struct UserData* userdata;

    userdata = malloc(sizeof(struct UserData));
    memset(userdata, 0, sizeof(struct UserData));
    pthread_mutex_init(&userdata->mutex, NULL);

    dbus_error_init(&dbus_err);
    userdata->connection = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, &dbus_err);
    pthread_mutex_lock(&userdata->mutex);

    return userdata;
}

void callfunipc_disable_loop(void)
{
    need_loop = 0;
}