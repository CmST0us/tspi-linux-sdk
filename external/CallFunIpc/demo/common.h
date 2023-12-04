// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DBUS_NAME                "rockchip.CallFunIpcDemo"
#define DBUS_PATH                "/"
#define DBUS_IF                  DBUS_NAME ".demo"
#define SHARE_PATH               "."

struct Examples_s {
    int r;
    int g;
    int b;
    int ret;
};

#ifdef __cplusplus
}
#endif

#endif