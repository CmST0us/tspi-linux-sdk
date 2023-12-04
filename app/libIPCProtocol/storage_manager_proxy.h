// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __STORAGE_MANAGER_PROXY_H__
#define __STORAGE_MANAGER_PROXY_H__

#include <dbus-c++/dbus.h>
#include <cassert>

class storage_manager_proxy
    : public ::DBus::InterfaceProxy
{
public:

    storage_manager_proxy(const char *interface)
        : ::DBus::InterfaceProxy(interface) {}

    /* properties exported by this interface */
public:

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    std::string GetFileList(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("GetFileList");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;

        return argout;
    }

    std::string DiskFormat(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("DiskFormat");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;

        return argout;
    }

    std::string GetMediaPath(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("GetMediaPath");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;

        return argout;
    }

    std::string GetDisksStatus(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("GetDisksStatus");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;

        return argout;
    }

    std::string MediaStorageStopNotify(const std::string& param)
    {
        std::string argout;

        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << param;
        call.member("MediaStorageStopNotify");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ri >> argout;

        return argout;
    }
};

class DBusStorageManager : public storage_manager_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy
{
public:
    DBusStorageManager(DBus::Connection &connection, const char *adaptor_path,
                       const char *adaptor_name, const char *interface)
    : DBus::ObjectProxy(0, connection, adaptor_path, adaptor_name), storage_manager_proxy::storage_manager_proxy(interface) {}
    ~DBusStorageManager() {}
};
#endif
