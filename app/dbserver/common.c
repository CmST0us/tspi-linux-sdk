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
#include "rkdb.h"
#include "common.h"
#include "dbus_helpers.h"

static gboolean signal_datachanged(DBusConnection *conn, char *interface, char *json_str)
{
    DBusMessage *signal;
    DBusMessageIter iter;

    if (interface == NULL)
        return FALSE;

    signal = dbus_message_new_signal(DB_PATH,
                                     interface, "DataChanged");
    if (!signal)
        return FALSE;

    dbus_message_iter_init_append(signal, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &json_str);

    dbus_connection_send(conn, signal, NULL);
    dbus_message_unref(signal);

    return TRUE;
}

static char *cmd_delete(json_object *j_cfg)
{
    char *where = 0;
    char *table = 0;
    json_object *j_key = 0;

    table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    j_key = json_object_object_get(j_cfg, "key");

    json_object_object_foreach(j_key, key, val) {
        if (where) {
            char *tmp = where;
            if (json_object_get_type(val) == json_type_int)
                where = g_strdup_printf("%s and %s=%d", tmp, key, (int)json_object_get_int(val));
            else
                where = g_strdup_printf("%s and %s='%s'", tmp, key, (char *)json_object_get_string(val));
            g_free(tmp);
        } else {
            if (json_object_get_type(val) == json_type_int)
                where = g_strdup_printf("%s=%d", key, (int)json_object_get_int(val));
            else
                where = g_strdup_printf("%s='%s'", key, (char *)json_object_get_string(val));
        }
    }
    return rkdb_delete(table, where);
}

static char *cmd_drop(json_object *j_cfg)
{
    char *table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    return rkdb_drop(table);
}

static char *cmd_create(json_object *j_cfg)
{
    char *table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    char *col = (char *)json_object_get_string(json_object_object_get(j_cfg, "col"));

    return rkdb_create(table, col);
}

static char *cmd_select(json_object *j_cfg)
{
    char *col = 0;
    char *where = 0;
    char *table = 0;

    json_object *j_key;
    json_object *j_data;

    table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    j_key = json_object_object_get(j_cfg, "key");
    j_data = json_object_object_get(j_cfg, "data");

    json_object_object_foreach(j_key, key, val) {
        if (where) {
            char *tmp = where;
            if (json_object_get_type(val) == json_type_int)
                where = g_strdup_printf("%s and %s=%d", tmp, key, (int)json_object_get_int(val));
            else
                where = g_strdup_printf("%s and %s='%s'", tmp, key, (char *)json_object_get_string(val));
            g_free(tmp);
        } else {
            if (json_object_get_type(val) == json_type_int)
                where = g_strdup_printf("%s=%d", key, (int)json_object_get_int(val));
            else
                where = g_strdup_printf("%s='%s'", key, (char *)json_object_get_string(val));
        }
    }

    col = (char *)json_object_get_string(j_data);
    char *ret = rkdb_select(table, col, where, NULL, NULL);
    g_free(where);

    return ret;
}

static char *cmd_update(json_object *j_cfg)
{
    int ret;
    char *table = 0;
    json_object *j_key = 0;
    json_object *j_data = 0;
    char *j_str;
    json_object *j_ret;

    table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    j_key = json_object_object_get(j_cfg, "key");
    j_data = json_object_object_get(j_cfg, "data");

    char *where = 0;
    char *set = 0;

    json_object_object_foreach(j_key, key, val) {
        if (where) {
            char *tmp = where;
            if (json_object_get_type(val) == json_type_int)
                where = g_strdup_printf("%s and %s=%d", tmp, key, (int)json_object_get_int(val));
            else
                where = g_strdup_printf("%s and %s='%s'", tmp, key, (char *)json_object_get_string(val));
            g_free(tmp);
        } else {
            if (json_object_get_type(val) == json_type_int)
                where = g_strdup_printf("%s=%d", key, (int)json_object_get_int(val));
            else
                where = g_strdup_printf("%s='%s'", key, (char *)json_object_get_string(val));
        }
    }

    json_object_object_foreach(j_data, key1, val1) {
        if (set) {
            char *tmp = set;
            if (json_object_get_type(val1) == json_type_int)
                set = g_strdup_printf("%s,%s=%d", tmp, key1, (int)json_object_get_int(val1));
            else
                set = g_strdup_printf("%s,%s='%s'", tmp, key1, (char *)json_object_get_string(val1));
            g_free(tmp);
        } else {
            if (json_object_get_type(val1) == json_type_int)
                set = g_strdup_printf("%s=%d", key1, (int)json_object_get_int(val1));
            else
                set = g_strdup_printf("%s='%s'", key1, (char *)json_object_get_string(val1));
        }
    }
    j_str = rkdb_select(table, "*", where, NULL, "0,1");
    j_ret = json_tokener_parse(j_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");
    int num = json_object_array_length(j_array);
    json_object_put(j_ret);
    g_free(j_str);

    if (num == 0) {
        char *cols = 0;
        char *vals = 0;

        json_object_object_foreach(j_key, key, val) {
            if (cols) {
                char *tmp = cols;
                cols = g_strdup_printf("%s,%s", tmp, key);
                g_free(tmp);
            } else {
                cols = g_strdup_printf("%s", key);
            }

            if (vals) {
                char *tmp = vals;
                if (json_object_get_type(val) == json_type_int)
                    vals = g_strdup_printf("%s,%d", tmp, (int)json_object_get_int(val));
                else
                    vals = g_strdup_printf("%s,'%s'", tmp, (char *)json_object_get_string(val));
                g_free(tmp);
            } else {
                if (json_object_get_type(val) == json_type_int)
                    vals = g_strdup_printf("%d", (int)json_object_get_int(val));
                else
                    vals = g_strdup_printf("'%s'", (char *)json_object_get_string(val));
            }
        }

        json_object_object_foreach(j_data, key1, val1) {
            if (cols) {
                char *tmp = cols;
                cols = g_strdup_printf("%s,%s", tmp, key1);
                g_free(tmp);
            } else {
                cols = g_strdup_printf("%s", key1);
            }
            if (vals) {
                char *tmp = vals;
                if (json_object_get_type(val1) == json_type_int)
                    vals = g_strdup_printf("%s,%d", tmp, (int)json_object_get_int(val1));
                else
                    vals = g_strdup_printf("%s,'%s'", tmp, (char *)json_object_get_string(val1));
                g_free(tmp);
            } else {
                if (json_object_get_type(val1) == json_type_int)
                    vals = g_strdup_printf("%d", (int)json_object_get_int(val1));
                else
                    vals = g_strdup_printf("'%s'", (char *)json_object_get_string(val1));
            }
        }
        j_str = rkdb_insert(table, cols, vals);
        if (cols)
            g_free(cols);
        if (vals)
            g_free(vals);
    } else {
        j_str = rkdb_update(table, set, where);
    }

    if (where)
        g_free(where);
    if (set)
        g_free(set);

    return j_str;
}

static char *cmd_insert(json_object *j_cfg)
{
    char *cols = 0;
    char *vals = 0;
    char *ret;
    char *table = 0;
    json_object *j_key = 0;
    json_object *j_data = 0;

    table = (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
    j_key = json_object_object_get(j_cfg, "key");
    j_data = json_object_object_get(j_cfg, "data");

    json_object_object_foreach(j_key, key, val) {
        if (cols) {
            char *tmp = cols;
            cols = g_strdup_printf("%s,%s", tmp, key);
            g_free(tmp);
        } else {
            cols = g_strdup_printf("%s", key);
        }

        if (vals) {
            char *tmp = vals;
            if (json_object_get_type(val) == json_type_int)
                vals = g_strdup_printf("%s,%d", tmp, (int)json_object_get_int(val));
            else
                vals = g_strdup_printf("%s,'%s'", tmp, (char *)json_object_get_string(val));
            g_free(tmp);
        } else {
            if (json_object_get_type(val) == json_type_int)
                vals = g_strdup_printf("%d", (int)json_object_get_int(val));
            else
                vals = g_strdup_printf("'%s'", (char *)json_object_get_string(val));
        }
    }

    json_object_object_foreach(j_data, key1, val1) {
        if (cols) {
            char *tmp = cols;
            cols = g_strdup_printf("%s,%s", tmp, key1);
            g_free(tmp);
        } else {
            cols = g_strdup_printf("%s", key1);
        }
        if (vals) {
            char *tmp = vals;
            if (json_object_get_type(val1) == json_type_int)
                vals = g_strdup_printf("%s,%d", tmp, (int)json_object_get_int(val1));
            else
                vals = g_strdup_printf("%s,'%s'", tmp, (char *)json_object_get_string(val1));
            g_free(tmp);
        } else {
            if (json_object_get_type(val1) == json_type_int)
                vals = g_strdup_printf("%d", (int)json_object_get_int(val1));
            else
                vals = g_strdup_printf("'%s'", (char *)json_object_get_string(val1));
        }
    }

    ret = rkdb_insert(table, cols, vals);

    if (cols)
        g_free(cols);
    if (vals)
        g_free(vals);

    return ret;
}

DBusMessage *method_cmd(DBusConnection *conn,
                           DBusMessage *msg, void *data)
{
    int ret;
    const char *sender;
    DBusMessage *reply;
    char *json_str;
    json_object *j_cfg;
    DBusMessageIter array;
    char *str = NULL;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &json_str,
                          DBUS_TYPE_INVALID);

    j_cfg = json_tokener_parse(json_str);
    char *cmd = (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));
    int need_signal = 0;
    if (g_str_equal(cmd, "Drop")) {
        str = cmd_drop(j_cfg);
        need_signal = 1;
    } else if (g_str_equal(cmd, "Create")) {
        str = cmd_create(j_cfg);
    } else if (g_str_equal(cmd, "Select")) {
        str = cmd_select(j_cfg);
    } else if (g_str_equal(cmd, "Delete")) {
        str = cmd_delete(j_cfg);
        need_signal = 1;
    } else if (g_str_equal(cmd, "Insert")) {
        str = cmd_insert(j_cfg);
        need_signal = 1;
    } else if (g_str_equal(cmd, "Update")) {
        str = cmd_update(j_cfg);
        need_signal = 1;
    }

    if (need_signal && str) {
        json_object *j_ret = json_tokener_parse(str);
        if ((int)json_object_get_int(json_object_object_get(j_ret, "iReturn")) == 0)
            signal_datachanged(conn, data, json_str);
        json_object_put(j_ret);
    }

    json_object_put(j_cfg);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);
    g_free(str);

    return reply;
}

DBusMessage *method_sql(DBusConnection *conn,
                           DBusMessage *msg, void *data)
{
    const char *sender;
    DBusMessage *reply;
    DBusMessageIter array, dict;
    char *str;
    char *sql;

    sender = dbus_message_get_sender(msg);

    dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &sql,
                          DBUS_TYPE_INVALID);

    str = rkdb_sql(sql);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
        return NULL;

    dbus_message_iter_init_append(reply, &array);
    dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING, &str);
    g_free(str);

    return reply;
}

void creat_version_table(char *table, char *version)
{
    char *col_para = "id INTEGER PRIMARY KEY," \
                     "sVersion TEXT NOT NULL";
    g_free(rkdb_create(table, col_para));
    char *vals = g_strdup_printf("0,'%s'", version);
    g_free(rkdb_insert(table, "id,sVersion", vals));
    g_free(vals);
}

int equal_version(char *table, char *version)
{
    int ret = 0;

    char *j_str = rkdb_select(table, "sVersion", "id=0", NULL, NULL);
    json_object *j_ret = json_tokener_parse(j_str);
    json_object *j_array = json_object_object_get(j_ret, "jData");
    int num = json_object_array_length(j_array);
    if (num > 0) {
        json_object *j_obj = json_object_array_get_idx(j_array, 0);
        char *ver = (char *)json_object_get_string(json_object_object_get(j_obj, "sVersion"));
        if (g_str_equal(ver, version))
            ret = 1;
    }
    json_object_put(j_ret);
    g_free(j_str);

    return ret;
}

const GDBusMethodTable methods[] = {
    {
        GDBUS_ASYNC_METHOD("Cmd",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_cmd)
    },
    {
        GDBUS_ASYNC_METHOD("Sql",
        GDBUS_ARGS({ "json", "s" }), GDBUS_ARGS({ "json", "s" }),
        method_sql)
    },
    { },
};

const GDBusSignalTable signals[] = {
    {
        GDBUS_SIGNAL("DataChanged",
        GDBUS_ARGS({ "json", "s" }))
    },
    { },
};