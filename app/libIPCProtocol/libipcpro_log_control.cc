// Copyright 2021 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libipcpro_log_control.h"

int ipc_pro_log_ctl = 1;

int IPCProtocol_log_en_get() {
    return ipc_pro_log_ctl;
}

void IPCProtocol_log_en_set(bool log_en) {
    ipc_pro_log_ctl = log_en ? 1 : 0;
}
