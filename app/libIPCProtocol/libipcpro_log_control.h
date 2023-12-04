// Copyright 2021 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __LIBIPCPRO_LOG_CONTROL_H__
#define __LIBIPCPRO_LOG_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

int IPCProtocol_log_en_get();
void IPCProtocol_log_en_set(bool log_en);

#ifdef __cplusplus
}
#endif

#endif