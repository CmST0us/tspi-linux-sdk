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

#include "db_monitor.h"
#include "video_common.h"
#include <stdio.h>

#ifdef USE_WEB_SERVER
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>
#include <pthread.h>
#include <assert.h>

#include <json-c/json.h>
#include <glib.h>

#include <IPCProtocol.h>
#include <dbserver.h>
#include <storage_manager.h>
#include <dbus_signal.h>

#include "rockface_control.h"
#include "display.h"

#define DBSERVER  "rockchip.dbserver"
#define DBSERVER_PATH      "/"
#define DBSERVER_EVENT_INTERFACE DBSERVER ".event"

#include <list>

struct json_data {
    int id;
    char *path;
    bool add;
};

static std::list<int> g_fail_id;
static std::list<struct json_data*> g_json;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static pthread_t g_th;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static bool g_flag;

extern int g_face_width;
extern int g_face_height;

static void db_monitor_wait(void)
{
    pthread_mutex_lock(&g_mutex);
    if (g_flag)
        pthread_cond_wait(&g_cond, &g_mutex);
    g_flag = true;
    pthread_mutex_unlock(&g_mutex);
}

static void db_monitor_signal(void)
{
    pthread_mutex_lock(&g_mutex);
    g_flag = false;
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
}

static void db_monitor_check(void);
static void *db_monitor_thread(void *arg)
{
    int ret;
    int id = -1;
    struct json_data *data = NULL;

    while (!is_command_success("pidof dbserver")) {
        printf("check dbserver failed!\n");
        sleep(1);
    }
    while (!is_command_success("pidof netserver")) {
        printf("check netserver failed!\n");
        sleep(1);
    }
    while (!is_command_success("pidof storage_manager")) {
        printf("check storage_manager failed!\n");
        sleep(1);
    }
    db_monitor_check();

    while (1) {
        id = -1;
        if (data) {
            if (data->path)
                free(data->path);
            free(data);
            data = NULL;
        }
        pthread_mutex_lock(&g_lock);
        if (g_json.empty() && g_fail_id.empty()) {
            pthread_mutex_unlock(&g_lock);
            db_monitor_wait();
        } else if (!g_json.empty()){
            data = g_json.front();
            g_json.pop_front();
            pthread_mutex_unlock(&g_lock);
        } else if (!g_fail_id.empty()) {
            id = g_fail_id.front();
            g_fail_id.pop_front();
            pthread_mutex_unlock(&g_lock);
        }
        if (id != -1) {
            dbserver_face_load_complete(id, -1);
            printf("%s: Update fail: %d\n", __func__, id);
            continue;
        }
        if (!data)
            continue;

        if (data->add) {
            ret = rockface_control_add_web(data->id, data->path);
            if (ret == -1 )
                dbserver_face_load_complete(data->id, -1);
            else if (ret == 2)
                dbserver_face_load_complete(data->id, 2);
            else
                dbserver_face_load_complete(data->id, 1);
            printf("Update: id = %d, path = %s\n", data->id, data->path);
        } else {
            rockface_control_delete(data->id, data->path, false, true);
            printf("Delete: id = %d\n", data->id);
        }

        usleep(10000);
    }
    pthread_exit(NULL);
}

static void get_face_config(json_object *j_obj)
{
    if (!j_obj)
        return;
    g_face_config.en = true;
    g_face_config.volume = json_object_get_int(json_object_object_get(j_obj, "iPromptVolume"));
    const char *live_det = json_object_get_string(json_object_object_get(j_obj, "sLiveDetect"));
    if (live_det) {
        if (!strncmp(live_det, "open", strlen("open")))
            g_face_config.live_det_en = 1;
        else
            g_face_config.live_det_en = 0;
    }
    g_face_config.live_det_th = json_object_get_int(json_object_object_get(j_obj, "iLiveDetectThreshold"));
    g_face_config.face_det_th = json_object_get_int(json_object_object_get(j_obj, "iFaceDetectionThreshold"));
    g_face_config.face_rec_th = json_object_get_int(json_object_object_get(j_obj, "iFaceRecognitionThreshold"));
    g_face_config.face_mask_th = 50; // TODO
    g_face_config.min_pixel = json_object_get_int(json_object_object_get(j_obj, "iFaceMinPixel"));
    g_face_config.corner_x = json_object_get_int(json_object_object_get(j_obj, "iLeftCornerX"));
    g_face_config.corner_y = json_object_get_int(json_object_object_get(j_obj, "iLeftCornerY"));
    g_face_config.det_width = json_object_get_int(json_object_object_get(j_obj, "iDetectWidth"));
    g_face_config.det_height = json_object_get_int(json_object_object_get(j_obj, "iDetectHeight"));
    g_face_config.nor_width = json_object_get_int(json_object_object_get(j_obj, "iNormalizedWidth"));
    g_face_config.nor_height = json_object_get_int(json_object_object_get(j_obj, "iNormalizedHeight"));
    printf("face_config: en %d\n", g_face_config.en);
    printf("             volume %d\n", g_face_config.volume);
    printf("             live_det_en %d\n", g_face_config.live_det_en);
    printf("             live_det_th %d\n", g_face_config.live_det_th);
    printf("             face_det_th %d\n", g_face_config.face_det_th);
    printf("             face_rec_th %d\n", g_face_config.face_rec_th);
    printf("             min_pixel %d\n", g_face_config.min_pixel);
    printf("             corner_x %d\n", g_face_config.corner_x);
    printf("             corner_y %d\n", g_face_config.corner_y);
    printf("             det_width %d\n", g_face_config.det_width);
    printf("             det_height %d\n", g_face_config.det_height);
    printf("             nor_width %d\n", g_face_config.nor_width);
    printf("             nor_height %d\n", g_face_config.nor_height);
    if (get_face_config_region_cb) {
        int x, y, w, h;
        int width, height;
        display_get_resolution(&width, &height);
        if (!width || !height) {
            width = g_face_width;
            height = g_face_height;
        }
        x = g_face_config.corner_x * width / g_face_config.nor_width;
        y = g_face_config.corner_y * height / g_face_config.nor_height;
        w = g_face_config.det_width * width / g_face_config.nor_width;
        h = g_face_config.det_height * height / g_face_config.nor_height;
        get_face_config_region_cb(x, y, w, h);
    }
}

void db_monitor_run(void *json_str)
{
    if (!json_str) {
        printf("%s error!\n", __func__);
        return;
    }
    json_object *j_cfg = json_tokener_parse((const char*)json_str);
    json_object *j_table = json_object_object_get(j_cfg, "table");

    const char *table = json_object_get_string(j_table);
    if (strstr(table, "FaceConfig")) {
        json_object *j_data = json_object_object_get(j_cfg, "data");
        get_face_config(j_data);
        json_object_put(j_cfg);
        return;
    } else if (!strstr(table, "FaceList")) {
        json_object_put(j_cfg);
        return;
    }

    json_object *j_key = json_object_object_get(j_cfg, "key");
    json_object *j_id = json_object_object_get(j_key, "id");
    json_object *j_data = json_object_object_get(j_cfg, "data");
    json_object *j_cmd = json_object_object_get(j_cfg, "cmd");

    int id = json_object_get_int(j_id);
    const char *cmd = json_object_get_string(j_cmd);

    if (cmd && strstr(cmd, "Update")) {
        json_object *j_note = json_object_object_get(j_data, "sNote");
        const char *note = json_object_get_string(j_note);
        if (note && !strstr(note, "undone")) {
            json_object *j_path = json_object_object_get(j_data, "sPicturePath");
            const char *path = json_object_get_string(j_path);
            if (path) {
                struct json_data *data = (struct json_data*)calloc(sizeof(struct json_data), 1);
                if (data) {
                    data->add = true;
                    data->id = id;
                    data->path = strdup(path);
                    if (data->path) {
                        pthread_mutex_lock(&g_lock);
                        g_json.push_back(data);
                        pthread_mutex_unlock(&g_lock);
                    } else {
                        free(data);
                        pthread_mutex_lock(&g_lock);
                        g_fail_id.push_back(id);
                        pthread_mutex_unlock(&g_lock);
                    }
                } else {
                    pthread_mutex_lock(&g_lock);
                    g_fail_id.push_back(id);
                    pthread_mutex_unlock(&g_lock);
                }
                db_monitor_signal();
            }
        }
    }
    if (cmd && strstr(cmd, "Delete")) {
        struct json_data *data = (struct json_data*)calloc(sizeof(struct json_data), 1);
        if (data) {
            data->id = id;
            data->add = false;
            json_object *j_path = json_object_object_get(j_key, "sPicturePath");
            const char *path = json_object_get_string(j_path);
            if (!j_id && !j_path) {
                printf("Delete all!\n");
                rockface_control_delete_all();
                free(data);
            } else if (path) {
                data->path = strdup(path);
                if (data->path) {
                    pthread_mutex_lock(&g_lock);
                    g_json.push_back(data);
                    pthread_mutex_unlock(&g_lock);
                    db_monitor_signal();
                } else {
                    printf("strdup %d path fail!\n", id);
                    free(data);
                }
            } else {
                printf("get %d path fail!\n", id);
                free(data);
            }
        } else {
            printf("%s: Delete fail: %d\n", __func__, id);
        }
    }

    json_object_put(j_cfg);
}

void db_monitor_storage_init(void)
{
    char *json_str = NULL;
    printf("%s\n", __func__);
    do {
        json_str = dbserver_get_storage_media_folder();
        if (json_str)
            break;
        usleep(1000000);
    } while (1);

    json_object *j_config = json_tokener_parse(json_str);
    json_object *j_data = json_object_object_get(j_config, "jData");
    if (j_data) {
        int num = json_object_array_length(j_data);
        for (int i = 0; i < num; i++) {
            json_object *j_obj = json_object_array_get_idx(j_data, i);
            int camid = (int)json_object_get_int(json_object_object_get(j_obj, "iCamId"));
            int type = (int)json_object_get_int(json_object_object_get(j_obj, "iType"));
            switch (type) {
            case TYPE_VIDEO:
                dbserver_update_storage_media_folder_duty(camid, type, 0, -1);
                break;
            case TYPE_PHOTO:
                dbserver_update_storage_media_folder_duty(camid, type, 0, -1);
                break;
            case TYPE_BLACK_LIST:
                dbserver_update_storage_media_folder_duty(camid, type, 10, -1);
                break;
            case TYPE_SNAPSHOT:
                dbserver_update_storage_media_folder_duty(camid, type, 40, -1);
                break;
            case TYPE_WHITE_LIST:
                dbserver_update_storage_media_folder_duty(camid, type, 50, -1);
                break;
            default:
                printf("%s: type error\n", __func__);
                break;
            }
        }
    }

    json_object_put(j_config);
    g_free(json_str);
}

static void db_monitor_get_media_path(int type, char *path, size_t size)
{
    char *json_str = NULL;
    int status;
    json_object *j_config;

    printf("%s: type = %d\n", __func__, type);
    do {
        json_str = storage_manager_get_media_path();
        if (json_str) {
            j_config = json_tokener_parse(json_str);
            json_object *j_status = json_object_object_get(j_config, "iStatus");
            status = json_object_get_int(j_status);
        } else {
            usleep(1000000);
            continue;
        }
        if (status)
            break;
        json_object_put(j_config);
        g_free(json_str);
        usleep(1000000);
    } while (1);

    json_object *j_array = json_object_object_get(j_config, "sScanPath");
    if (j_array) {
        int num = json_object_array_length(j_array);
        for (int i = 0; i < num; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);
            int t = (int)json_object_get_int(json_object_object_get(j_obj, "iType"));
            if (t == type) {
                const char *p = json_object_get_string(json_object_object_get(j_obj, "sMediaPath"));
                strncpy(path, p, size - 1);
                break;
            }
        }
    }

    json_object_put(j_config);
    g_free(json_str);
}

void db_monitor_init()
{
    if (pthread_create(&g_th, NULL, db_monitor_thread, NULL)) {
        printf("%s create thread failed!\n", __func__);
        return;
    }
}

static void db_monitor_check(void)
{
    char *json_str = NULL;
    json_object *j_config;

    db_monitor_storage_init();

    db_monitor_get_media_path(TYPE_SNAPSHOT, g_snapshot, sizeof(g_snapshot));
    db_monitor_get_media_path(TYPE_BLACK_LIST, g_black_list, sizeof(g_black_list));
    db_monitor_get_media_path(TYPE_WHITE_LIST, g_white_list, sizeof(g_white_list));
    if (check_path_dir(g_snapshot))
        printf("check %s error\n", g_snapshot);
    if (check_path_dir(g_white_list))
        printf("check %s error\n", g_white_list);
    if (check_path_dir(g_black_list))
        printf("check %s error\n", g_black_list);

    json_str = dbserver_event_get("FaceConfig");
    if (json_str) {
        j_config = json_tokener_parse(json_str);
        int ret = json_object_get_int(json_object_object_get(j_config, "iReturn"));
        if (ret == 0) {
            json_object *j_data = json_object_object_get(j_config, "jData");
            json_object *j_obj = json_object_array_get_idx(j_data, 0);
            get_face_config(j_obj);
        }
        json_object_put(j_config);
        free(json_str);
    }

    dbus_monitor_signal_registered(DBSERVER_EVENT_INTERFACE, "DataChanged", &db_monitor_run);
}

void db_monitor_face_list_add(int id, char *path, char *name, char *type)
{
    dbserver_face_list_add(id, path, name, type);
    dbserver_face_load_complete(id, 1);
}

void db_monitor_face_list_delete(int id)
{
    dbserver_face_list_delete(id);
}

void db_monitor_snapshot_record_set(char *path)
{
    dbserver_snapshot_record_set(path);
}

void db_monitor_control_record_set(int face_id, char *path, char *status, char *similarity)
{
    dbserver_control_record_set(face_id, path, status, similarity);
}


void db_monitor_get_user_info(struct user_info *info, int i)
{
    char *json_str = NULL;

    json_str = dbserver_event_get_by_id(TABLE_FACE_LIST, i);
    if (json_str) {
        json_object *j_cfg = json_tokener_parse((const char*)json_str);
        json_object *j_data = json_object_object_get(j_cfg, "jData");
        json_object *j_obj = json_object_array_get_idx(j_data, 0);
#define COPY_INT(x) info->x = json_object_get_int(json_object_object_get(j_obj, #x))
#define COPY_STRING(x) \
        do { \
            if (json_object_get_string(json_object_object_get(j_obj, #x))) \
                strncpy(info->x, json_object_get_string(json_object_object_get(j_obj, #x)), sizeof(info->x) - 1); \
        } while (0)
        COPY_INT(id);
        COPY_STRING(sPicturePath);
        COPY_STRING(sRegistrationTime);
        COPY_INT(iAge);
        COPY_STRING(sListType);
        COPY_STRING(sType);
        COPY_STRING(sName);
        COPY_STRING(sGender);
        COPY_STRING(sNation);
        COPY_STRING(sCertificateType);
        COPY_STRING(sCertificateNumber);
        COPY_STRING(sBirthday);
        COPY_STRING(sTelephoneNumber);
        COPY_STRING(sHometown);
        COPY_INT(iAccessCardNumber);
        json_object_put(j_cfg);
        free(json_str);
    } else {
        printf("%s %d failed\n", __func__, i);
    }
}
#else
void db_monitor_init()
{
    snprintf(g_snapshot, sizeof(g_snapshot), "/userdata/snapshot");
    snprintf(g_black_list, sizeof(g_black_list), "/userdata/black_list");
    snprintf(g_white_list, sizeof(g_white_list), "/userdata/white_list");
    if (check_path_dir(g_snapshot))
        printf("check %s error\n", g_snapshot);
    if (check_path_dir(g_white_list))
        printf("check %s error\n", g_white_list);
    if (check_path_dir(g_black_list))
        printf("check %s error\n", g_black_list);
}
void db_monitor_face_list_add(int id, char *path, char *name, char *type) {}
void db_monitor_face_list_delete(int id) {}
void db_monitor_snapshot_record_set(char *path) {}
void db_monitor_control_record_set(int face_id, char *path, char *status, char *similarity) {}
void db_monitor_get_user_info(struct user_info *info, int id) {}
#endif
