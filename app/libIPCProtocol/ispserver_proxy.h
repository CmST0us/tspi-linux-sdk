// Copyright 2021 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __ISPSERVER_PROXY_H
#define __ISPSERVER_PROXY_H

#include <dbus-c++/dbus.h>
#include <cassert>

class ispserver_proxy
    : public ::DBus::InterfaceProxy
{
public:

    ispserver_proxy(const char *interface)
        : ::DBus::InterfaceProxy(interface) {}

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    void SendTurnoffIspSignal(void)
    {
        ::DBus::CallMessage call;

        call.member("SendTurnoffIspSignal");
        ::DBus::Message ret = invoke_method (call);
    }

    std::string GetDumpExposureInfo(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        call.member("GetDumpExposureInfo");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;

        return argout;
    }
};

class DBusNetServer : public ispserver_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy
{
public:
    DBusNetServer(DBus::Connection &connection, const char *adaptor_path,
                  const char *adaptor_name, const char *interface)
    : DBus::ObjectProxy(0, connection, adaptor_path, adaptor_name), ispserver_proxy::ispserver_proxy(interface) {}
    ~DBusNetServer() {}
};
#endif //__dbusxx__ispserver_proxy_h__PROXY_MARSHAL_H
