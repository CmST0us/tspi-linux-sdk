/**
 * Copyright (C) 2018 Fuzhou Rockchip Electronics Co., Ltd
 * author: Chad.ma <Chad.ma@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "roots.h"
#include "usbboot.h"

extern size_t strlcpy(char *dst, const char *src, size_t dsize);
extern size_t strlcat(char *dst, const char *src, size_t dsize);

bool is_boot_from_udisk(void)
{
    bool bUDisk = false;
    char param[1024];
    int fd, ret;
    char *s = NULL;
    printf("read cmdline\n");
    memset(param, 0, 1024);

    fd = open("/proc/cmdline", O_RDONLY);
    ret = read(fd, (char*)param, 1024);

    s = strstr(param, "usbfwupdate");
    if (s != NULL) {
        bUDisk = true;
        printf(">>> Boot from U-Disk\n");
    } else {
        bUDisk = false;
        printf(">>> Boot from non-U-Disk\n");
    }

    close(fd);
    return bUDisk;
}

void ensure_udisk_mounted(bool *bMounted)
{
    int i;
    bool bSucc = false;
    for (i = 0; i < 3; i++) {
        if (0 == ensure_path_mounted(EX_UDISK_ROOT)) {
            *bMounted = true;
            bSucc = true;
            break;
        } else {
            printf("delay 1 sec to try /mnt/udisk\n");
            sleep(1);
        }
    }

    if (!bSucc) {   // try another mount point
        for (i = 0; i < 3; i++) {
            if (0 == ensure_path_mounted(EX_UDISK_ROOT2)) {
                *bMounted = true;
                bSucc = true;
                break;
            } else {
                printf("delay 1 sec to try /mnt/usb_storage\n");
                sleep(1);
            }
        }
    }

    if (bSucc)
        *bMounted = false;
}

#define MaxLine 1024
static int get_cfg_Item(char *pFileName /*in*/, char *pKey /*in*/,
                        char * pValue/*in out*/, int * pValueLen /*out*/)
{
    int     ret = 0;
    FILE    *fp = NULL;
    char    *pTmp = NULL, *pEnd = NULL, *pBegin = NULL;

    char lineBuf[MaxLine];

    fp = fopen(pFileName, "r");
    if (fp == NULL) {
        ret = -1;
        return ret;
    }

    while (!feof(fp)) {
        memset(lineBuf, 0, sizeof(lineBuf));
        fgets(lineBuf, MaxLine, fp);
        printf("lineBuf: %s ", lineBuf);

        pTmp = strchr(lineBuf, '=');
        if (pTmp == NULL)
            continue;

        pTmp = strstr(lineBuf, pKey);
        if (pTmp == NULL)
            continue;

        pTmp = pTmp + strlen(pKey);
        pTmp = strchr(pTmp, '=');
        if (pTmp == NULL)
            continue;

        pTmp = pTmp + 1;

        while (1) {
            if (*pTmp == ' ') {
                pTmp ++ ;
            } else {
                pBegin = pTmp;
                if (*pBegin == '\n') {
                    goto End;
                }
                break;
            }
        }

        while (1) {
            if ((*pTmp == ' ' || *pTmp == '\n'))
                break;
            else
                pTmp ++;
        }
        pEnd = pTmp;

        *pValueLen = pEnd - pBegin;
        memcpy(pValue, pBegin, pEnd - pBegin);
    }

End:
    if (fp == NULL)
        fclose(fp);

    return 0;
}

bool is_udisk_update(void)
{
    int  ret = 0;
    bool bUdiskMounted = false;
    char configFile[64] = {0};
    int vlen = 0;
    char str_val[10] = {0};
    char *str_key = "fw_update";

    printf("%s in\n", __func__);
    ensure_udisk_mounted(&bUdiskMounted);
    if (!bUdiskMounted) {
        printf("Error! U-Disk not mounted\n");
        return false;
    }

    strlcpy(configFile, EX_UDISK_ROOT, sizeof(configFile));
    strlcat(configFile, "/sd_boot_config.config", sizeof(configFile));
    printf("configFile = %s \n", configFile);
    ret = get_cfg_Item(configFile, str_key, str_val, &vlen);

    if (ret != 0) {
        printf("func get_cfg_Item err:%d \n", ret);
        return false;
    }

    printf("\n %s:%s \n", str_key, str_val);

    if (strcmp(str_val, "1") != 0) {
        return false;
    }

    printf("firmware update will from UDisk.\n");
    printf("%s out\n", __func__);
    return true;
}
