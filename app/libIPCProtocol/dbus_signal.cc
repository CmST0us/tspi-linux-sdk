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

#include <gdbus.h>

#include "json-c/json.h"
#include "dbus_signal.h"
#include "IPCProtocol.h"

static DBusConnection *dbusconn = 0;
int need_loop = 1;
GMainLoop *main_loop = NULL;

struct DbusSignal {
    char *interface;
    char *signal;
    dbus_signal_func_t cb;
    struct DbusSignal *next;
};

static struct DbusSignal *dbus_signal_list = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

static DBusHandlerResult dbus_monitor_changed(
    DBusConnection *connection,
    DBusMessage *message, void *user_data)
{
    DBusMessageIter iter;
    DBusHandlerResult handled;

    handled = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

    pthread_mutex_lock(&mutex);
    struct DbusSignal *tmp = dbus_signal_list;

    while(tmp) {
       if (dbus_message_is_signal(message, tmp->interface,
                               tmp->signal)) {
            char *json_str;
            handled = DBUS_HANDLER_RESULT_HANDLED;

            dbus_message_iter_init(message, &iter);
            dbus_message_iter_get_basic(&iter, &json_str);
            if (tmp->cb)
                tmp->cb(json_str);
        }

        tmp = tmp->next;
    }
    pthread_mutex_unlock(&mutex);

    return handled;
}

static struct DbusSignal *check_dbus_signel(char *interface, char *signal, dbus_signal_func_t cb)
{
    struct DbusSignal *tmp = dbus_signal_list;

    while(tmp) {
        if ((cb == tmp->cb) && g_str_equal(interface, tmp->interface) && (g_str_equal(signal, tmp->signal)))
            break;
        tmp = tmp->next;
    }

    return tmp;
}

static void add_dbus_sigal(struct DbusSignal *dbus_signal)
{
    struct DbusSignal *tmp = dbus_signal_list;
    if (tmp == NULL) {
        dbus_signal_list = dbus_signal;
    } else {
        while(tmp->next) tmp = tmp->next;
        tmp->next = dbus_signal;
    }
}

void dbus_monitor_signal_registered(char *interface, char *signal, dbus_signal_func_t cb)
{
    DBusError err;
    char *tmp;

    if (need_loop && main_loop == NULL) {
        pthread_t tid;
        pthread_create(&tid, NULL, loop_thread, NULL);
    }

    dbus_error_init(&err);
    if (dbusconn == NULL) {
        pthread_mutex_init(&mutex, NULL);
        dbusconn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, &err);
        dbus_connection_add_filter(dbusconn, dbus_monitor_changed, NULL, NULL);
    }
    if (check_dbus_signel(interface, signal, cb) == NULL) {
        struct DbusSignal *dbus_signal = (struct DbusSignal *)malloc(sizeof(struct DbusSignal));
        dbus_signal->interface = g_strdup(interface);
        dbus_signal->signal = g_strdup(signal);
        dbus_signal->cb = cb;
        dbus_signal->next = NULL;
        add_dbus_sigal(dbus_signal);
        tmp = g_strdup_printf("type='signal',interface='%s'", interface);
        dbus_bus_add_match(dbusconn, tmp, &err);
        g_free(tmp);
    }
}

void dbus_monitor_signal_unregistered(char *interface, char *signal, dbus_signal_func_t cb)
{
    struct DbusSignal *tmp = dbus_signal_list;

    if (tmp == NULL)
        return;

    pthread_mutex_lock(&mutex);
    if ((cb == tmp->cb) && g_str_equal(interface, tmp->interface) && (g_str_equal(signal, tmp->signal))) {
        g_free(tmp->interface);
        g_free(tmp->signal);
        dbus_signal_list = tmp->next;
        free(tmp);
    } else {
        struct DbusSignal *tmp_pre = tmp;
        tmp = tmp->next;
        while(tmp) {
            if ((cb == tmp->cb) && g_str_equal(interface, tmp->interface) && (g_str_equal(signal, tmp->signal))) {
                g_free(tmp->interface);
                g_free(tmp->signal);
                tmp_pre = tmp->next;
                free(tmp);
                break;
            }
            tmp_pre = tmp;
            tmp = tmp->next;
        }
    }
    pthread_mutex_unlock(&mutex);
}

void disable_loop(void)
{
    need_loop = 0;
}
