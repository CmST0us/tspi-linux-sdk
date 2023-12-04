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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "face_common.h"
#include "load_feature.h"
#include "database.h"
#include "db_monitor.h"

static get_path_feature_t get_path_feature_cb = NULL;
void register_get_path_feature(get_path_feature_t cb)
{
    get_path_feature_cb = cb;
}

int count_file(const char *path, char *fmt)
{
    struct dirent *ent = NULL;
    DIR *dir;
    struct stat st;
    char name[512];
    int cnt = 0;

    dir = opendir(path);
    if (!dir) {
        printf("%s is not exist or is not a directory!\n", path);
        return cnt;
    }
    while ((ent = readdir(dir))) {
        snprintf(name, sizeof(name), "%s/%s", path, ent->d_name);
        stat(name, &st);
        if (S_ISDIR(st.st_mode)) {
            if (strcmp(".", ent->d_name) && strcmp("..", ent->d_name))
                cnt += count_file(name, fmt);
        } else if (strstr(ent->d_name, fmt)) {
            cnt++;
        }
    }
    closedir(dir);
    return cnt;
}

int load_feature(const char *path, const char *fmt, void *data, unsigned int cnt)
{
    struct dirent *ent = NULL;
    DIR *dir;
    struct stat st;
    char name[NAME_LEN];
    int index = 0;
    int id;

    dir = opendir(path);
    if (!dir) {
        printf("%s is not exist or is not a directory!\n", path);
        return index;
    }
    while ((ent = readdir(dir))) {
        snprintf(name, sizeof(name), "%s/%s", path, ent->d_name);
        stat(name, &st);
        if (S_ISDIR(st.st_mode)) {
            if (strcmp(".", ent->d_name) && strcmp("..", ent->d_name) &&
                    strcmp("snapshot", ent->d_name))
                index += load_feature(name, fmt, (struct face_data*)data + index, cnt - index);
        } else if (strstr(ent->d_name, fmt)) {
            if (index >= cnt)
                break;
            if (database_is_name_exist(name))
                continue;
            struct face_data *face_data = (struct face_data*)data + index;
            rockface_feature_float_t m;
            float mask_score;
            if (get_path_feature_cb && get_path_feature_cb(name, &face_data->feature, &m, &mask_score) == 0) {
                char tmp[NAME_LEN];
                memset(tmp, 0, sizeof(tmp));
                memcpy(tmp, ent->d_name, strrchr(ent->d_name, '.') - ent->d_name);
                index++;
                id = database_get_user_name_id();
                if (id < 0) {
                    printf("%s: get id fail!\n", __func__);
                    break;
                }
                face_data->id = id;
                database_insert(&face_data->feature, sizeof(face_data->feature),
                                name, sizeof(name), id, false, &m, sizeof(m));
                if (strstr(name, "black_list"))
                    db_monitor_face_list_add(id, name, tmp, "blackList");
                else
                    db_monitor_face_list_add(id, name, tmp, "whiteList");
            }
        }
    }
    closedir(dir);
    return index;
}
