// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __NETSERVER_H__
#define __NETSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NETSERVER_BUS_NAME "rockchip.netserver"
#define NETSERVER_PATH "/"
#define NETSERVER_INTERFACE NETSERVER_BUS_NAME ".server"

#define NS_SIGNAL_POWERCHANGED    "PowerChanged"

void netserver_scan_wifi(void);
char *netserver_get_service(char *type);
char *netserver_get_config(char *service);
char *netserver_get_networkip(char *interface);
void netserver_get_service_by_ssid(char *ssid, char *service);

#ifdef __cplusplus
}
#endif

#endif