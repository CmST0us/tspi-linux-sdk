// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_GRAPH_CONTROL_H_
#define _RK_DBUS_GRAPH_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <memory>

#include "dbus_dispatcher.h"
#include "ai_uvc_graph.h"

#define RT_APP_UVC                     "app_uvc"
#define RT_APP_NN                      "app_nn"
#define RT_APP_AI_FEATURE              "app_feature"
#define RT_ACTION_CONFIG_CAMERA        "updateCameraParams"
#define RT_ACTION_CONFIG_ENCODER       "updateEncoderParams"
#define RT_ACTION_RETRIVE_FEATURE      "retriveAIFeature"

namespace rockchip {
namespace aiserver {

class RTGraphListener {
 public:
    virtual ~RTGraphListener() {}
    virtual int32_t start(const std::string &appName) = 0;
    virtual int32_t stop(const std::string &appName) = 0;
    virtual int32_t observeGraphOutput(const std::string &appName, const int32_t &enable) = 0;

    virtual int32_t setEPTZ(const AI_UVC_EPTZ_MODE &mode, const int32_t &enabled) = 0;
    virtual int32_t setZoom(const double &val) = 0;
    virtual int32_t setFaceAE(const int32_t &enabled) = 0;
    virtual int32_t setFaceLine(const int32_t &enabled) = 0;
    virtual int32_t enableAIAlgorithm(const std::string &type) = 0;
    virtual int32_t disableAIAlgorithm(const std::string &type) = 0;
    virtual int32_t updateAIAlgorithmParams(const std::string &type) = 0;
    virtual int32_t openAIMatting() = 0;
    virtual int32_t closeAIMatting() = 0;

    virtual int32_t invoke(const std::string &appName, const std::string &actionName, void *params) = 0;
    virtual int32_t ctrlSubGraph(const char* nnName, int32_t enable) = 0;
};

class DBusGraphControl : public control::graph_adaptor,
                         public DBus::IntrospectableAdaptor,
                         public DBus::ObjectAdaptor {
 public:
    DBusGraphControl() = delete;
    DBusGraphControl(DBus::Connection &connection, RTGraphListener* listener);
    virtual ~DBusGraphControl();

 public:
    int32_t Start(const std::string &appName);
    int32_t Stop(const std::string &appName);
    int32_t SetGraphOutputObserver(const std::string &appName, const int32_t &enabled);

    // UVC
    int32_t EnableEPTZ(const int32_t &enabled);
    int32_t SetZoom(const double &val);
    int32_t EnableFaceAE(const int32_t &enabled);
    int32_t EnableFaceLine(const int32_t &enabled);

    // AI
    int32_t EnableAIAlgorithm(const std::string &type);
    int32_t DisableAIAlgorithm(const std::string &type);
    int32_t UpdateAIAlgorithmParams(const std::string &cmdName);

    int32_t OpenAIMatting(const std::string &type);
    int32_t CloseAIMatting(const std::string &type);
    // Extension
    int32_t Invoke(const std::string &appName, const std::string &actionName,
                        const int32_t &ext1, const int64_t &ext2);
    // Old API
    int32_t SetRockxStatus(const std::string &nnName);
    int32_t SetNpuCtlStatus(const std::string &cmdName);

private:
    RTGraphListener* mGraphListener;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_DBUS_GRAPH_CONTROL_H_
