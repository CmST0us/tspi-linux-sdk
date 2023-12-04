// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __SYSTEM_MANAGER_PROXY_H__
#define __SYSTEM_MANAGER_PROXY_H__

#include <dbus-c++/dbus.h>
#include <cassert>

class system_manager_proxy
    : public ::DBus::InterfaceProxy
{
public:

    system_manager_proxy(const char *interface)
        : ::DBus::InterfaceProxy(interface) {}

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    void Reboot(void)
    {
        ::DBus::CallMessage call;
        call.member("Reboot");
        ::DBus::Message ret = invoke_method (call);
    }

    void FactoryReset(void)
    {
        ::DBus::CallMessage call;
        call.member("FactoryReset");
        ::DBus::Message ret = invoke_method (call);
    }

    std::string ExportDB(const std::string& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("ExportDB");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::string ImportDB(const std::string& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("ImportDB");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::string ExportLog(const std::string& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("ExportLog");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }

    std::string Upgrade(const std::string& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("Upgrade");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::string argout;
        ri >> argout;
        return argout;
    }
};

class DBusSystemManager : public system_manager_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy
{
public:
    DBusSystemManager(DBus::Connection &connection, const char *adaptor_path,
                      const char *adaptor_name, const char *interface)
    : DBus::ObjectProxy(0, connection, adaptor_path, adaptor_name), system_manager_proxy::system_manager_proxy(interface) {}
    ~DBusSystemManager() {}
};
#endif
