#ifndef __DBSERVER_MEDIA_H__
#define __DBSERVER_MEDIA_H__

void media_init(void);
int media_dbus_register(DBusConnection *dbus_conn);
#endif