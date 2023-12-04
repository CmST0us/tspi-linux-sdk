// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTEM_MANAGER "rockchip.system"
#define SYSTEM_MANAGER_PATH "/"
#define SYSTEM_MANAGER_INTERFACE SYSTEM_MANAGER ".server"

char *system_reboot(void);
char *system_factory_reset(void);
char *system_export_db(const char *path);
char *system_import_db(const char *path);
char *system_export_log(const char *path);
char *system_upgrade(const char *path);

#ifdef __cplusplus
}
#endif
#endif
