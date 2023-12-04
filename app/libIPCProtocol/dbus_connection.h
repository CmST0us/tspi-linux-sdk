// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __DBUS_CONNECTION_H
#define __DBUS_CONNECTION_H

class DXXAPI DBusConn
{
public:
  DBusConn(DBus::Connection &conn)
  : _conn(conn)
  {
  }
  ~DBusConn() {}

  DBus::Connection &conn()
  {
     return _conn;
  }
private:
  DBus::Connection	_conn;
};

DBus::Connection &get_dbus_conn(void);
void dbus_mutex_lock(void);
void dbus_mutex_unlock(void);

#endif