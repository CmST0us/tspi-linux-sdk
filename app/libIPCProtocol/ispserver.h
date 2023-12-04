// Copyright 2021 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __ISPSERVER_H__
#define __ISPSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ISPSERVER_BUS_NAME "rockchip.ispserver"
#define ISPSERVER_PATH "/"
#define ISPSERVER_INTERFACE ISPSERVER_BUS_NAME ".server"

#define NS_SIGNAL_ISPSTATUSCHANGED    "IspStatusChanged"

char *ispserver_expo_info_get();
void ispserver_turnoff_signal_send();

#ifdef __cplusplus
}
#endif

#endif