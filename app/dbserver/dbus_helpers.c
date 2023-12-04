/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <glib.h>

#include "dbus_helpers.h"

#define TIMEOUT         120000

struct dbus_callback {
    dbus_method_return_func_t cb;
    void *user_data;
};

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

int dbus_helpers_method_call(DBusConnection *connection,
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

