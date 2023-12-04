// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __STORAGE_MANAGER_H__
#define __STORAGE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGEMANAGER  "rockchip.StorageManager"
#define STORAGEMANAGER_PATH      "/"

#define SM_SIGNAL_UPDATEDISKSTATUS    "UpdateDisksStatus"
#define SM_SIGNAL_UPDATEMEDIAPATH     "UpdateMediaPath"
#define SM_SIGNAL_FREESIZENOTICE      "FreeSizeNotice"

#define STORAGEMANAGER_INTERFACE  STORAGEMANAGER ".file"

char *storage_manager_get_disks_status(char *mountpath);
char *storage_manager_get_filelist_id(int rootid, int pathid, int order, char *limit);
char *storage_manager_get_filelist_path(char *path, int *StartTime, int *EndTime, int order, char *limit);
char *storage_manager_get_media_path(void);
char *storage_manager_diskformat(char *mountpath, char *type);
char *storage_manager_media_storage_stop_notify(char *server_name);

#ifdef __cplusplus
}
#endif

#endif