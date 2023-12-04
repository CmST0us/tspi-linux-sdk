// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dbus_dispatcher.h"

#include <signal.h>
#include <stdio.h>

namespace rockchip {
namespace aiserver {

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "dbus_dispatcher.cpp"

DBus::BusDispatcher DbusDispatcher::dispatcher_;

DbusDispatcher::DbusDispatcher(int timeout_ms) : timeout_ms_(timeout_ms) {
  DBus::_init_threading();

  DBus::default_dispatcher = &dispatcher_;
  new DBus::DefaultTimeout(timeout_ms_, false, &dispatcher_);
}

} // namespace aiserver
} // names
