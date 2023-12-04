// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_DBUS_ENCODER_CONTROL_H_
#define _RK_DBUS_ENCODER_CONTROL_H_

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dbus_dispatcher.h"

namespace rockchip {
namespace aiserver {

class FlowUnit;
class DBusEncoderControl : public control::encoder_adaptor,
                           public DBus::IntrospectableAdaptor,
                           public DBus::ObjectAdaptor {
public:
  DBusEncoderControl() = delete;
  DBusEncoderControl(DBus::Connection &connection);
  virtual ~DBusEncoderControl();

  int32_t SetBitRate(const int32_t &id, const int32_t &param);
  int32_t SetFrameRate(const int32_t &id, const int32_t &param);
  int32_t SetForceIdrFrame(const int32_t &id);
  int32_t SetQP(const int32_t &id, const int32_t &param);
  int32_t SetOSDData(const int32_t &id, const std::string &param);
};

} // namespace aiserver
} // namespace rockchip

#endif // _RK_MPP_ENCODER_H_
