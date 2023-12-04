// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_AI_SERVER_H_
#define _RK_AI_SERVER_H_

#include "ai_scene_director.h"
#include "dbus_server.h"

namespace rockchip {
namespace aiserver {

class AIServer {
 public:
    AIServer();
    virtual ~AIServer();

 public:
    void setupTaskGraph();
    void waitUntilDone();
    void interrupt();

 private:
    std::unique_ptr<DBusServer>      mDbusServer;
    std::unique_ptr<AISceneDirector> mAIDirector;
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_AI_SERVER_H_
