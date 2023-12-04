// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

static int CommShm(char *name, int projid, int size, int flags)
{
    int retry_cnt = 0;
    int shmid;
    key_t key = ftok(name, projid);

    if(key == -1) {
        perror("ftok");
        return -1;
    }
retry:
    shmid = shmget(key, size, flags);
    if(shmid < 0) {
        if (retry_cnt++ < 2) {
            shmid = shmget(key, 0, 0);
            if (shmid != -1) {
                shmctl(shmid, IPC_RMID, 0);
                goto retry;
            }
        }
        perror("shmget");
        return -2;
    }

    return shmid;
}

int DestroyShm(int shmid)
{
    if(shmctl(shmid, IPC_RMID,NULL) < 0) {
        perror("shmctl");
        return -1;
    }

    return 0;
}

int CreateShm(char *name, int projid, int size)
{
    return CommShm(name, projid, size, IPC_CREAT | IPC_EXCL | 0666);
}

int GetShm(char *name, int projid, int size)
{
    return CommShm(name, projid, size, IPC_CREAT);
}

char *Shmat(int shmid)
{
    return shmat(shmid, NULL, 0);
}

void Shmdt(char *addr)
{
    shmdt(addr);
}