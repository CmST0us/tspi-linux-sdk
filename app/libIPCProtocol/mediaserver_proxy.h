// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __MEDIASERVER_PROXY_H__
#define __MEDIASERVER_PROXY_H__

#include <memory>

#include <dbus-c++/dbus.h>

#define MEDIASERVER                     "rockchip.mediaserver.control"

#define MEDIASERVER_CAMERA_PATH         "/rockchip/mediaserver/control/camera"
#define MEDIASERVER_CAMERA_INTERFACE    "rockchip.mediaserver.control.camera"
#define MEDIASERVER_ENCODER_PATH        "/rockchip/mediaserver/control/encoder"
#define MEDIASERVER_ENCODER_INTERFACE   "rockchip.mediaserver.control.encoder"
#define MEDIASERVER_AUDIO_PATH          "/rockchip/mediaserver/control/audio"
#define MEDIASERVER_AUDIO_INTERFACE     "rockchip.mediaserver.control.audio"
#define MEDIASERVER_FEATURE_PATH        "/rockchip/mediaserver/control/feature"
#define MEDIASERVER_FEATURE_INTERFACE   "rockchip.mediaserver.control.feature"
#define MEDIASERVER_ADVANCED_ENCODER_PATH       "/rockchip/mediaserver/control/advancedencoder"
#define MEDIASERVER_ADVANCED_ENCODER_INTERFACE  "rockchip.mediaserver.control.advancedencoder"
#define MEDIASERVER_FACE_RECOGNIZE_PATH         "/rockchip/mediaserver/control/facerecognize"
#define MEDIASERVER_FACE_RECOGNIZE_INTERFACE    "rockchip.mediaserver.control.facerecognize"

class MediaControlProxy
: public ::DBus::InterfaceProxy
{
public:

    MediaControlProxy(const char * interface)
    : ::DBus::InterfaceProxy(interface) {}

public:

    int32_t SetParam(const char * member, const int32_t& id)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << id;
        call.member(member);
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }

    int32_t SetParam(const char * member, const int32_t& id, const int32_t& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << id;
        wi << param;
        call.member(member);
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }

    int32_t SetParam(const char * member, const int32_t& id, const std::string& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << id;
        wi << param;
        call.member(member);
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }

    int32_t SetParam(const char * member, const int32_t& id, const std::vector<int32_t>& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << id;
        wi << param;
        call.member(member);
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }

    int32_t SetParam(const char * member, const int32_t& id,
                       const std::map< std::string, std::string >& param)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << id;
        wi << param;
        call.member(member);
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        int32_t argout;
        ri >> argout;
        return argout;
    }
};

class MediaControl : public MediaControlProxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy  {
public:
    MediaControl(DBus::Connection & conn, const char *path, const char *interface)
            : MediaControlProxy(interface)
            , DBus::ObjectProxy(0, conn, path, MEDIASERVER) {}
    virtual ~MediaControl() {}
private:
};

#endif

