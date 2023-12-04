// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __DBUS_SIGNAL_H
#define __DBUS_SIGNAL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*dbus_signal_func_t)(void *user_data);
void dbus_monitor_signal_registered(char *interface, char *signal, dbus_signal_func_t cb);
void dbus_monitor_signal_unregistered(char *interface, char *signal, dbus_signal_func_t cb);
void disable_loop(void);

#ifdef __cplusplus
}
#endif

#endif