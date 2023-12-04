// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

int DestroyShm(int shmid);
int CreateShm(char *name, int projid, int size);
int GetShm(char *name, int projid, int size);
char *Shmat(int shmid);
void Shmdt(char *addr);

#ifdef __cplusplus
}
#endif

#endif