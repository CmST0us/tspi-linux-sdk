#ifndef __SYSTEM_H__
#define __SYSTEM_H__

void system_init(void);
int system_dbus_register(DBusConnection *dbus_conn);

#endif