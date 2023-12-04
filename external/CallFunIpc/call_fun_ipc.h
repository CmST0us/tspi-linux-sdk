// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __CALL_FUN_IPC_H__
#define __CALL_FUN_IPC_H__

#ifdef __cplusplus
extern "C" {
#endif

struct FunMap{
    char *fun_name;
    int (*fun)(void *);
};

void call_fun_ipc_server_init(struct FunMap *map, int num, char *dbus_name, char *dbus_if, char *dbus_path, int needloop);
void call_fun_ipc_server_deinit(void);

int call_fun_ipc_call(char *funname, void *data, int len, int restore);
void call_fun_ipc_client_init(char *dbus_name, char *dbus_if, char *dbus_path, char *share_path, int needloop);
void call_fun_ipc_client_deinit(void);

void disable_loop(void);

#ifdef __cplusplus
}
#endif

#endif