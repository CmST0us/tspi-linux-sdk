#ifndef DBUS_SUSPEND_API
#define DBUS_SUSPEND_API

#ifdef __c_plus_plus
extern "C" {
#endif
#define SUSPEND_API __attribute__((visibility("default")))

typedef int (*CALLBACK)(void *data);

SUSPEND_API int request_system_suspend(void);

SUSPEND_API int register_suspend_listener(char *bus_name, CALLBACK suspend_callback, CALLBACK resume_callback);

#ifdef __c_plus_plus
}
#endif
#endif
