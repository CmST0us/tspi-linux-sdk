// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __DBSERVER_PROXY_H__
#define __DBSERVER_PROXY_H__

#include <dbus-c++/dbus.h>
#include <cassert>

class dbserver_proxy
    : public ::DBus::InterfaceProxy
{
public:

    dbserver_proxy(const char *interface)
        : ::DBus::InterfaceProxy(interface) {}
    ~dbserver_proxy() {}

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    std::string Cmd(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("Cmd");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;
        return argout;
    }

    std::string Sql(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("Sql");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;
        return argout;
    }
};

class DBusDbServer : public dbserver_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy
{
public:
    DBusDbServer(DBus::Connection &connection, const char *adaptor_path,
                 const char *adaptor_name, const char *interface)
    : DBus::ObjectProxy(0, connection, adaptor_path, adaptor_name), dbserver_proxy::dbserver_proxy(interface) {}
    ~DBusDbServer() {}
};

#endif
