#ifndef __NETWORK_H__
#define __NETWORK_H__

void network_init(void);
int network_dbus_register(DBusConnection *dbus_conn);

#endif