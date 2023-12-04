// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>

#include "logger/log.h"
#include "dbus_graph_control.h"

namespace rockchip {
namespace aiserver {

DBusGraphControl::DBusGraphControl(DBus::Connection &connection, RTGraphListener* listener)
                 :DBus::ObjectAdaptor(connection, MEDIA_CONTROL_PATH_GRAPH){
    mGraphListener = listener;
}

DBusGraphControl::~DBusGraphControl() {}

int32_t DBusGraphControl::Start(const std::string &appName) {
    if (NULL != mGraphListener) {
        return mGraphListener->start(appName);
    }

    return -1;
}

int32_t DBusGraphControl::Stop(const std::string &appName) {
    if (NULL != mGraphListener) {
        return mGraphListener->stop(appName);
    }

    return -1;
}

int32_t DBusGraphControl::SetGraphOutputObserver(const std::string &appName, const int32_t &enabled) {
    if (NULL != mGraphListener) {
        return mGraphListener->observeGraphOutput(appName, enabled);
    }

    return -1;
}

int32_t DBusGraphControl::EnableEPTZ(const int32_t &enabled) {
    if (NULL != mGraphListener) {
        return mGraphListener->setEPTZ(AI_UVC_EPTZ_AUTO, enabled);
    }

    return -1;
}

int32_t DBusGraphControl::SetZoom(const double &val) {
    if (NULL != mGraphListener) {
        return mGraphListener->setZoom(val);
    }

    return -1;
}
int32_t DBusGraphControl::EnableFaceAE(const int32_t &enabled){
    if (NULL != mGraphListener) {
        return mGraphListener->setFaceAE(enabled);
    }

    return -1;

}
int32_t DBusGraphControl::EnableFaceLine(const int32_t &enabled){
    if (NULL != mGraphListener) {
        return mGraphListener->setFaceLine(enabled);
    }

    return -1;

}

int32_t DBusGraphControl::EnableAIAlgorithm(const std::string &type) {
    if (NULL != mGraphListener) {
        return mGraphListener->enableAIAlgorithm(type);
    }

    return -1;
}

int32_t DBusGraphControl::DisableAIAlgorithm(const std::string &type) {
    if (NULL != mGraphListener) {
        return mGraphListener->disableAIAlgorithm(type);
    }

    return -1;
}

int32_t DBusGraphControl::OpenAIMatting(const std::string &type) {
    if (NULL != mGraphListener) {
        return mGraphListener->openAIMatting();
    }

    return -1;
}

int32_t DBusGraphControl::CloseAIMatting(const std::string &type) {
    if (NULL != mGraphListener) {
        return mGraphListener->closeAIMatting();
    }

    return -1;
}

int32_t DBusGraphControl::Invoke(const std::string &appName, const std::string &actionName,
                    const int32_t &ext1, const int64_t &ext2) {
    if (NULL != mGraphListener) {
        int64_t exts[] = {ext1, ext2};
        return mGraphListener->invoke(appName, actionName, exts);
    }

    return -1;
}

int32_t DBusGraphControl::SetRockxStatus(const std::string &nnName) {
    LOG_INFO("%s: Dbus GraphCtrl received: %s\n", __FUNCTION__, nnName.c_str());
    char nnTypeName[64];
    memset(nnTypeName, 0, 64);
    const char *s = nullptr;
    if (!(s = strstr(nnName.c_str(), ":"))) {
        LOG_INFO("string trans rect format error, string=%s\n", nnName.c_str());
        return -1;
    }
    strncpy(nnTypeName, nnName.c_str(), s - nnName.c_str());
    int32_t enable = atoi(s + 1);

    if (NULL != mGraphListener) {
        mGraphListener->ctrlSubGraph(nnTypeName, enable);
    }
    return 0;
}

int32_t DBusGraphControl::SetNpuCtlStatus(const std::string &cmdName) {
    LOG_INFO("%s: Dbus GraphCtrl received: %s\n", __FUNCTION__, cmdName.c_str());
    const char *ss = nullptr;
    if (!(ss = strstr(cmdName.c_str(), ":"))) {
        LOG_INFO("unkown npu control format, rawcmd:%s\n", cmdName.c_str());
        return -1;
    }

    if (NULL != mGraphListener) {
        return mGraphListener->invoke(RT_APP_AI_FEATURE, RT_ACTION_RETRIVE_FEATURE, (void *)cmdName.c_str());
    }

    return 0;
}

int32_t DBusGraphControl::UpdateAIAlgorithmParams(const std::string &cmdName) {
    LOG_INFO("%s: Dbus GraphCtrl received: %s\n", __FUNCTION__, cmdName.c_str());
    const char *ss = nullptr;
    if (!(ss = strstr(cmdName.c_str(), ":"))) {
        LOG_INFO("unkown npu control format, rawcmd:%s\n", cmdName.c_str());
        return -1;
    }

    if (NULL != mGraphListener) {
        return mGraphListener->updateAIAlgorithmParams(cmdName.c_str());
    }

    return 0;
}

} // namespace aiserver
} // namespace rockchip
