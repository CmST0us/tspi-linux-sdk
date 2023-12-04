#ifndef __DBUS_H
#define __DBUS_H

#ifdef __cplusplus
extern "C" {
#endif

struct UserData {
    pthread_mutex_t mutex;
    DBusConnection *connection;
    char *json_str;
};

typedef int (*dbus_method_return_func_t)(DBusMessageIter *iter,
                                         const char *error, void *user_data);

typedef void (*dbus_append_func_t)(DBusMessageIter *iter,
                                   void *user_data);

int callfunipc_dbus_method_call(DBusConnection *connection,
                     const char *service, const char *path, const char *interface,
                     const char *method, dbus_method_return_func_t cb,
                     void * user_data, dbus_append_func_t append_fn,
                     void *append_data);

void callfunipc_dbus_deconnection(struct UserData* userdata);
int callfunipc_dbus_async(struct UserData* userdata);
struct UserData* callfunipc_dbus_connection(void);
int callfunipc_populate_get(DBusMessageIter *iter, const char *error, void *user_data);
int callfunipc_populate_set(DBusMessageIter *iter, const char *error, void *user_data);
void callfunipc_append_path(DBusMessageIter *iter, void *user_data);

void callfunipc_disable_loop(void);

#ifdef __cplusplus
}
#endif

#endif