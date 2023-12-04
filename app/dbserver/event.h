#ifndef __EVENT_H__
#define __EVENT_H__

void event_init(void);
int event_dbus_register(DBusConnection *dbus_conn);

#endif