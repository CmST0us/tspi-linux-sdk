/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "video_common.h"
#include "face_common.h"

#define MAX_VIDEO_ID 20

char g_snapshot[NAME_LEN];
char g_white_list[NAME_LEN];
char g_black_list[NAME_LEN];

int get_video_id(char *name)
{
    FILE *fp = NULL;
    char buf[1024];
    int i;
    char cmd[128];
    bool exist = false;

    for (i = 0; i < MAX_VIDEO_ID; i++) {
        snprintf(cmd, sizeof(cmd), "/sys/class/video4linux/video%d/name", i);
        if (access(cmd, F_OK))
            continue;
        snprintf(cmd, sizeof(cmd), "cat /sys/class/video4linux/video%d/name", i);
        fp = popen(cmd, "r");
        if (fp) {
            if (fgets(buf, sizeof(buf), fp)) {
                if (strstr(buf, name))
                    exist = true;
            }
            pclose(fp);
        }
        if (exist)
            break;
    }
    return (i == MAX_VIDEO_ID ? -1 : i);
}

int check_path_dir(const char *name)
{
    char dir[NAME_LEN];
    const char *ptr = name;
    const char *tmp = name;

    while (*ptr) {
        ptr += 1;
        tmp = strchr(ptr, '/');
        if (!tmp)
            tmp = strchr(ptr, '\0');
        if (!tmp)
            break;
        ptr = tmp;
        memset(dir, 0, sizeof(dir));
        memcpy(dir, name, ptr - name);
        if (access(dir, F_OK)) {
            if (mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
                printf("%s %s fail!\n", __func__, dir);
                return -1;
            }
        }
    }
    return 0;
}

void save_file(void *buf, size_t size, const char *dir, const char *ext)
{
    struct timeval t;
    char name[256];
    FILE *fp;

    check_path_dir(dir);
    gettimeofday(&t, NULL);
    snprintf(name, sizeof(name), "%s/%ld.%06ld_%s", dir, t.tv_sec, t.tv_usec, ext);
    fp = fopen(name, "wb");
    if (fp) {
        fwrite(buf, 1, size, fp);
        fclose(fp);
        printf("save %s ok.\n", name);
    } else {
        printf("save %s fail.\n", name);
    }
}

bool is_command_success(const char *cmd)
{
    char buffer[128];
    FILE *fp;
    int num;
    bool result = false;

    fp = popen(cmd, "r");
    if (fp != NULL) {
        num = fread(buffer, 1, sizeof(buffer), fp);
        if (num > 0)
            result = true;
        pclose(fp);
    }

    return result;
}
