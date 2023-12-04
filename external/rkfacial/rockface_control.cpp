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
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

#include <list>

#include "face_common.h"
#include "database.h"
#include "rockface_control.h"
#include "play_wav.h"
#include "load_feature.h"
#include "video_common.h"
#include "camrgb_control.h"
#include "camir_control.h"
#include "snapshot.h"
#include "db_monitor.h"
#include "rkfacial.h"
#include "display.h"
#include "image_read.h"

#define TEST_RESULT_INC(x) \
    do { \
        if (g_test.en) \
            g_test.x++; \
    } while (0)

#define DEFAULT_FACE_NUMBER 1000
#define DEFAULT_FACE_PATH "/userdata"
#define FACE_DETECT_SCORE 0.55 /* range 0 - 1.0, higher score means higher expectation */

#define FACE_MASK_SIMILARITY_SCORE 1.05 /* suggest range 1.05 ~ 1.12, lower score means need higher similarity to recognize */
#define FACE_SIMILARITY_CONVERT(f) powf(2.0, -((f)))
#define FACE_SIMILARITY_SCORE 1.0 /* suggest range 0.7 ~ 1.3, lower score means need higher similarity to recognize */
#define FACE_SIMILARITY_SCORE_REGISTER 0.5
#define FACE_SCORE_REGISTER 0.99 /* range 0 - 1.0, higher score means higher expectation */
#define FACE_REGISTER_CNT 5
#define FACE_REAL_SCORE 0.5 /* range 0 - 1.0, higher score means higher expectation */
#define LICENCE_PATH PRE_PATH "/key.lic"
#define BAK_LICENCE_PATH BAK_PATH "/key.lic"
#define FACE_DATA_PATH "/usr/lib"
#define MIN_FACE_WIDTH(w) ((w) / 5)
#define FACE_TRACK_FRAME 0
#define FACE_RETRACK_TIME 1
#define SNAP_TIME 3

#define DET_BUFFER_NUM 2
#define DET_WIDTH 360
#define DET_HEIGHT 640

#define FACE_BLUR 0.85

#define DET_INTERVAL_TIME 1

struct face_buf {
    rockface_image_t img;
    rockface_det_t face;
    bo_t bo;
    int fd;
    int id;
};

static struct face_buf g_feature;
static struct face_buf g_detect[DET_BUFFER_NUM];
static pthread_mutex_t g_det_lock = PTHREAD_MUTEX_INITIALIZER;
static std::list<struct face_buf*> g_det_free;
static std::list<struct face_buf*> g_det_ready;

static struct timeval g_last_det_tv;
static struct timeval g_last_reg_tv;

static void *g_face_data = NULL;
static int g_face_index = 0;
static int g_face_cnt = DEFAULT_FACE_NUMBER;
#ifdef FACE_MASK
static void *g_mask_data = NULL;
static int g_mask_index = 0;
#endif

static rockface_handle_t face_handle;
static int g_total_cnt;

static pthread_t g_tid;
static bool g_run;
static char last_name[NAME_LEN];
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
static bool g_feature_flag;
static pthread_t g_detect_tid;
static pthread_mutex_t g_detect_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_detect_cond = PTHREAD_COND_INITIALIZER;
static bool g_detect_flag;
static int g_rgb_track = -1;
static pthread_mutex_t g_rgb_track_mutex = PTHREAD_MUTEX_INITIALIZER;

static rockface_image_t g_ir_img;
static rockface_det_t g_ir_face;
static bo_t g_ir_bo;
static int g_ir_fd = -1;

static bo_t g_ir_det_bo;
static int g_ir_det_fd = -1;

enum ir_state {
    IR_STATE_CANCELED,
    IR_STATE_PREPARED,
};

static enum ir_state g_ir_state = IR_STATE_CANCELED;
static int g_ir_detect_fail;

static bool g_ir_save_real;
static bool g_ir_save_fake;

static bool g_register = false;
static int g_register_cnt = 0;
static bool g_delete = false;

static struct snapshot g_snap;

static pthread_mutex_t g_lib_lock = PTHREAD_MUTEX_INITIALIZER;

bool g_face_en;
int g_face_width;
int g_face_height;
static int g_ratio;

static struct test_result g_test;

#ifdef IR_TEST_DATA
static bo_t g_test_bo;
static int g_test_fd;
#endif

static int g_detect_en = 1;

static int g_identity_en = 0;
static char g_identity_path[256];

void rockface_control_set_detect_en(int en)
{
    if (en) {
        sync();
        database_bak();
        rockface_control_database();
        system("echo ondemand > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
    } else {
        /* register feature use max freq */
        system("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
        system("echo 1512000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed");
    }
    g_detect_en = en;
}

void rockface_control_set_identity_en(int en, char *path)
{
    if (path)
        strncpy(g_identity_path, path, sizeof(g_identity_path) - 1);
    g_identity_en = en;
}

void rockface_start_test(void)
{
    if (g_test.en)
        return;
    /* wait feature thread done */
    while (!g_feature_flag) {
        usleep(10000);
        continue;
    }
    memset(&g_test, 0, sizeof(struct test_result));
    g_test.en = true;
}

static get_test_callback get_test_cb = NULL;
void register_get_test_callback(get_test_callback cb)
{
    get_test_cb = cb;
}

void rockface_output_test(void)
{
    if (g_test.en && g_test.ir_detect_total >= 100) {
        if (get_test_cb)
            get_test_cb(&g_test);
        printf("%s:\n", __func__);
        printf("\trgb_detect: %d/%d\n", g_test.rgb_detect_ok, g_test.rgb_detect_total);
        printf("\trgb_track: %d/%d\n", g_test.rgb_track_ok, g_test.rgb_track_total);
        printf("\tir_detect: %d/%d\n", g_test.ir_detect_ok, g_test.ir_detect_total);
        printf("\tir_liveness: %d/%d\n", g_test.ir_liveness_ok, g_test.ir_liveness_total);
        printf("\trgb_landmark: %d/%d\n", g_test.rgb_landmark_ok, g_test.rgb_landmark_total);
        printf("\trgb_align: %d/%d\n", g_test.rgb_align_ok, g_test.rgb_align_total);
        printf("\trgb_extract: %d/%d\n", g_test.rgb_extract_ok, g_test.rgb_extract_total);
        printf("\trgb_search: %d/%d\n", g_test.rgb_search_ok, g_test.rgb_search_total);

        memset(&g_test, 0, sizeof(struct test_result));
        g_ir_save_real = false;
        g_ir_save_fake = false;
    }
}

void save_ir_real(bool flag)
{
    g_ir_save_real = flag;
}

void save_ir_fake(bool flag)
{
    g_ir_save_fake = flag;
}

void set_face_param(int width, int height, int cnt)
{
    int tmp = width < height ? width : height;
    g_face_en = true;
    g_face_width = width < height ? width : height;
    g_face_height = width > height ? width : height;
    g_face_cnt = cnt;
    g_ratio = tmp / DET_WIDTH;
}

static void check_pre_path(const char *pre)
{
    char cmd[128];
    FILE *fp;
    size_t size = 0;
    char buffer[128];

    snprintf(cmd, sizeof(cmd), "mount | grep %s", pre);
    do {
        size = 0;
        fp = popen(cmd, "r");
        if (fp) {
            size = fread(buffer, 1, sizeof(buffer), fp);
            pclose(fp);
            if (size > 0)
                break;
        }
        sleep(1);
        printf("%s %s\n", __func__, pre);
    } while (1);
}

static void *init_thread(void *arg)
{
    char cmd[256];

    check_pre_path(PRE_PATH);
    int ret = rockface_control_init();

    check_pre_path(BAK_PATH);
    snprintf(cmd, sizeof(cmd), "cp %s %s", LICENCE_PATH, BAK_LICENCE_PATH);
    system(cmd);

    database_bak();

    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

void rockface_control_init_thread(void)
{
    pthread_t tid;
    if (pthread_create(&tid, NULL, init_thread, NULL))
        printf("%s fail!\n", __func__);
}

static int get_min_pixel(int img_width)
{
    int pixel;
    int min_pixel;
    if (get_face_config_min_pixel(&pixel))
        min_pixel = pixel * img_width / g_face_width;
    else
        min_pixel = MIN_FACE_WIDTH(img_width);
    return min_pixel;
}

static float get_face_recognition_score(void)
{
    int th;
    float score;
    if (get_face_config_face_rec_th(&th)) {
        th = th < 1 ? 1 : th;
        th = th > 100 ? 100 : th;
        score = log(100.0 / (th * 1.0)) / log(2);
    } else {
        score = FACE_SIMILARITY_SCORE;
    }
    return score;
}

static float get_face_mask_recognition_score(void)
{
    int th;
    float score;
    if (get_face_config_face_mask_th(&th)) {
        th = th < 1 ? 1 : th;
        th = th > 100 ? 100 : th;
        score = log(100.0 / (th * 1.0)) / log(2);
    } else {
        score = FACE_MASK_SIMILARITY_SCORE;
    }
    return score;
}


static float get_face_detect_score(void)
{
    int th;
    float score;
    if (get_face_config_face_det_th(&th))
        score = th * 1.0 / 100.0;
    else
        score = FACE_DETECT_SCORE;
    return score;
}

static float get_live_detect_score(void)
{
    int th;
    float score;
    if (get_face_config_live_det_th(&th))
        score = th * 1.0 / 100.0;
    else
        score = FACE_REAL_SCORE;
    return score;
}

static rockface_det_t *get_max_face(rockface_det_array_t *face_array)
{
    rockface_det_t *max_face = NULL;
    if (face_array->count == 0)
        return NULL;

    for (int i = 0; i < face_array->count; i++) {
        rockface_det_t *cur_face = &(face_array->face[i]);
        if (max_face == NULL) {
            max_face = cur_face;
            continue;
        }
        int cur_face_box_area = (cur_face->box.right - cur_face->box.left) *
                                (cur_face->box.bottom - cur_face->box.top);
        int max_face_box_area = (max_face->box.right - max_face->box.left) *
                                (max_face->box.bottom - max_face->box.top);
        if (cur_face_box_area > max_face_box_area)
            max_face = cur_face;
    }

    return max_face;
}

static bool check_face_region(rockface_rect_t *box, int img_width, int img_height)
{
    int x, y, w, h, nw, nh;

    if (!get_face_config_corner_x(&x))
        x = 0;
    if (!get_face_config_corner_y(&y))
        y = 0;
    if (!get_face_config_det_width(&w))
        w = g_face_width;
    if (!get_face_config_det_height(&h))
        h = g_face_height;
    if (!get_face_config_nor_width(&nw))
        nw = g_face_width;
    if (!get_face_config_nor_height(&nh))
        nh = g_face_height;

    x = x * g_face_width / nw;
    y = y * g_face_height / nh;
    w = w * g_face_width / nw;
    h = h * g_face_height / nh;

    if (x + w > g_face_width)
        w = g_face_width - x;
    if (y + h > g_face_height)
        h = g_face_height - y;

    if (w <= 0 || h <= 0)
        return false;

    if (img_width == DET_WIDTH) {
        x /= g_ratio;
        y /= g_ratio;
        w /= g_ratio;
        h /= g_ratio;
    }

    if (box->left <= x || box->top <= y || box->right >= x + w || box->bottom >= y + h)
        return false;

    return true;
}

static int _rockface_control_detect(rockface_image_t *image, rockface_det_t *out_face, int *track)
{
    int r = 0;
    rockface_ret_t ret;
    rockface_det_array_t face_array0;
    rockface_det_array_t face_array;

    if (track) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        if (tv.tv_sec - g_last_reg_tv.tv_sec <= DET_INTERVAL_TIME &&
                tv.tv_sec - g_last_det_tv.tv_sec <= DET_INTERVAL_TIME)
            return -1;
        gettimeofday(&g_last_det_tv, NULL);
    }

    memset(&face_array0, 0, sizeof(rockface_det_array_t));
    memset(&face_array, 0, sizeof(rockface_det_array_t));
    memset(out_face, 0, sizeof(rockface_det_t));

    TEST_RESULT_INC(rgb_detect_total);
    ret = rockface_detect(face_handle, image, &face_array0);
    if (ret != ROCKFACE_RET_SUCCESS) {
        if (!track)
            printf("rockface_detect fail!\n");
        return -1;
    }

    if (track) {
        TEST_RESULT_INC(rgb_track_total);
        ret = rockface_track(face_handle, image, FACE_TRACK_FRAME, &face_array0, &face_array);
        if (ret != ROCKFACE_RET_SUCCESS)
            return -1;
        TEST_RESULT_INC(rgb_track_ok);
    } else {
        memcpy(&face_array, &face_array0, sizeof(rockface_det_array_t));
    }

    rockface_det_t* face = get_max_face(&face_array);
    if (face == NULL) {
        if (!track)
            printf("rockface_detect fail: face is NULL!\n");
        return -1;
    }
    if (face->score < get_face_detect_score()) {
        if (!track)
            printf("rockface_detect fail: face score %f, less than %f!\n", face->score, get_face_detect_score());
        return -1;
    }
    rockface_rect_t *box = &face->box;
    if (box->left < 0 || box->top < 0 || box->right >= image->width || box->bottom >= image->height) {
        if (!track)
            printf("rockface_detect fail: box [%d %d %d %d] error, image: %dx%d\n",
                   box->left, box->top, box->right, box->bottom, image->width, image->height);
        return -1;
    }

    if (track) {
        if (face->box.right - face->box.left <= get_min_pixel(image->width))
            return -1;
        if (!check_face_region(&face->box, image->width, image->height))
            return -1;
    }

    TEST_RESULT_INC(rgb_detect_ok);
    memcpy(out_face, face, sizeof(rockface_det_t));

    if (track) {
        pthread_mutex_lock(&g_rgb_track_mutex);
        if (g_delete || g_register || !strlen(last_name))
            *track = -1;
        else if (*track == face->id)
            r = -2;
        pthread_mutex_unlock(&g_rgb_track_mutex);
    }

    return r;
}

static int rockface_control_detect(rockface_image_t *image, rockface_det_t *face)
{
    int ret;
    static struct timeval t0;
    struct timeval t1;
    bool en;

    memset(face, 0, sizeof(rockface_det_t));

    if (!t0.tv_sec && !t0.tv_usec)
        gettimeofday(&t0, NULL);
    gettimeofday(&t1, NULL);
    pthread_mutex_lock(&g_rgb_track_mutex);
    if (g_rgb_track >= 0 && t1.tv_sec - t0.tv_sec > FACE_RETRACK_TIME) {
        g_rgb_track = -1;
        gettimeofday(&t0, NULL);
    }
    pthread_mutex_unlock(&g_rgb_track_mutex);
    en = (g_test.en || g_ir_save_real || g_ir_save_fake) ? true : false;
    ret = _rockface_control_detect(image, face, en ? NULL : &g_rgb_track);
    if (face->score > get_face_detect_score()) {
        int left, top, right, bottom;
        int width, height;
        display_get_resolution(&width, &height);
        if (!width || !height) {
            width = g_face_width;
            height = g_face_height;
        }
        left = face->box.left * g_ratio * width / g_face_width;
        top = face->box.top * g_ratio * height / g_face_height;
        right = face->box.right * g_ratio * width / g_face_width;
        bottom = face->box.bottom * g_ratio * height / g_face_height;
        if (rkfacial_paint_box_cb)
            rkfacial_paint_box_cb(left, top, right, bottom);
        camrgb_control_expo_weights(left, top, right, bottom);
    } else {
        if (rkfacial_paint_box_cb)
            rkfacial_paint_box_cb(0, 0, 0, 0);
        camrgb_control_expo_weights_default();
    }

    return ret;
}

static int rockface_control_init_library(void *data, int num, size_t size, size_t off, int mask)
{
    rockface_ret_t ret;

    ret = rockface_face_library_init2(face_handle, mask ? ROCKFACE_RECOG_MASK : ROCKFACE_RECOG_NORMAL, data, num, size, off);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: int library error %d!\n", __func__, ret);
        return -1;
    }

    return 0;
}

static void rockface_control_release_library(void)
{
    rockface_face_library_release(face_handle);
}

static int rockface_control_get_feature(rockface_image_t *in_image,
                                        rockface_feature_t *out_feature,
                                        rockface_feature_float_t *mask_feature,
                                        rockface_det_t *in_face,
                                        bool reg,
                                        float *mask_score)
{
    rockface_ret_t ret;

    memset(out_feature, 0, sizeof(rockface_feature_t));
    memset(mask_feature, 0, sizeof(rockface_feature_float_t));

    rockface_landmark_t landmark;
    TEST_RESULT_INC(rgb_landmark_total);
    ret = rockface_landmark5(face_handle, in_image, &(in_face->box), &landmark);
    if (ret != ROCKFACE_RET_SUCCESS || landmark.score < 0.3) {
        if (reg)
            printf("rockface_landmark5 fail!\n");
        return -1;
    }
    TEST_RESULT_INC(rgb_landmark_ok);

    rockface_landmark_t landmark106;
    rockface_angle_t angle;
    ret = rockface_landmark106(face_handle, in_image, &(in_face->box),  &landmark, &landmark106, &angle);
    if (ret != ROCKFACE_RET_SUCCESS || angle.pitch > 30.0 || angle.pitch < -30.0 ||
            angle.yaw > 30.0 || angle.yaw < -30.0 || angle.roll > 30.0 || angle.roll < -30.0)
        return -1;

#ifdef FACE_MASK
    if (reg) {
        *mask_score = 0.0;
    } else {
        ret = rockface_mask_classifier(face_handle, in_image, &(in_face->box), mask_score);
        if (ret != ROCKFACE_RET_SUCCESS) {
            printf("rockface_mask_classifier error");
            return -1;
        }
    }
#else
    *mask_score = 0.0;
#endif

    if (reg || *mask_score < 0.5) {
        rockface_image_t out_img;
        memset(&out_img, 0, sizeof(rockface_image_t));
        TEST_RESULT_INC(rgb_align_total);
        ret = rockface_align(face_handle, in_image, &(in_face->box), &landmark, &out_img);
        if (ret != ROCKFACE_RET_SUCCESS) {
            if (reg)
                printf("rockface_align fail!\n");
            return -1;
        }
        TEST_RESULT_INC(rgb_align_ok);

        TEST_RESULT_INC(rgb_extract_total);
        ret = rockface_feature_extract(face_handle, &out_img, out_feature);
        rockface_image_release(&out_img);
        if (ret != ROCKFACE_RET_SUCCESS) {
            if (reg)
                printf("rockface_feature_extract fail!\n");
            return -1;
        }
        TEST_RESULT_INC(rgb_extract_ok);
    }

#ifdef FACE_MASK
    if (reg || *mask_score >= 0.5) {
        ret = rockface_mask_feature_extract(face_handle, in_image, &in_face->box, reg ? 0 : 1, mask_feature);
        if (ret != ROCKFACE_RET_SUCCESS) {
            if (reg)
                printf("rockface_mask_feature_extract fail!\n");
            return -1;
        }
    }
#endif

    return 0;
}

int rockface_control_get_path_feature(const char *path, void *feature, void *mask_feature, float *mask_score)
{
    int ret = -1;
    rockface_feature_t *out_feature = (rockface_feature_t*)feature;
    rockface_feature_float_t *out_mask = (rockface_feature_float_t*)mask_feature;
    rockface_image_t in_img;
    rockface_det_t face;
    int cnt = 10;
    int read;
    bo_t rgb_bo;
    int rgb_fd;

    while (access(path, F_OK) && --cnt)
        usleep(100000);

    /* try hardware decode */
    read = image_read(path, &in_img, &rgb_bo, &rgb_fd);
    if (read) {
        if (read != -2)
            image_read_deinit(&rgb_bo, &rgb_fd);

        /* use software decode */
        if (rockface_image_read(path, &in_img, 1))
            return -1;
    }
    if (!_rockface_control_detect(&in_img, &face, NULL))
        ret = rockface_control_get_feature(&in_img, out_feature, out_mask, &face, true, mask_score);
    if (!read)
        image_read_deinit(&rgb_bo, &rgb_fd);
    else
        rockface_image_release(&in_img);
    return ret;
}

void rockface_set_user_info(struct user_info *info, enum user_state state,
                            rockface_det_t *ir_face, rockface_det_t *rgb_face)
{
    memset(info, 0, sizeof(struct user_info));
    info->state = state;
    if (ir_face)
        memcpy(&info->ir_face, ir_face, sizeof(rockface_det_t));
    if (rgb_face)
        memcpy(&info->rgb_face, rgb_face, sizeof(rockface_det_t));
}

static bool rockface_control_search(rockface_image_t *image, void *data, int *index, int cnt,
                              size_t size, size_t offset, rockface_det_t *face, int reg,
                              struct face_data **face_data, struct mask_data **mask_data, float *similarity)
{
    rockface_ret_t ret;
    rockface_search_result_t result;
    rockface_feature_t feature;
    rockface_feature_float_t mask;
    float mask_score;

    if (rockface_control_get_feature(image, &feature, &mask, face, false, &mask_score) == 0) {
        //printf("g_total_cnt = %d\n", ++g_total_cnt);
        if (g_identity_en) {
            rockface_feature_t f;
            rockface_feature_float_t m;
            float s;
            if (!rockface_control_get_path_feature(g_identity_path, &f, &m, &s)) {
                float simi;
                bool pass = false;
                if (mask_score < 0.5) {
                    rockface_feature_compare((rockface_feature_t *)&feature, (rockface_feature_t *)&f, &simi);
                    if (simi < get_face_recognition_score())
                        pass = true;
                } else {
                    rockface_feature_compare((rockface_feature_t *)&mask, (rockface_feature_t *)&m, &simi);
                    if (simi < get_face_mask_recognition_score())
                        pass = true;
                }
                if (pass) {
                    play_wav_signal(PLEASE_GO_THROUGH_WAV);
                    if (rkfacial_paint_info_cb) {
                        struct user_info info;
                        rockface_set_user_info(&info, USER_STATE_REAL_IDENTITY, &g_ir_face, &g_feature.face);
                        strncpy(info.sIdentityPath, g_identity_path, sizeof(info.sIdentityPath) - 1);
                        rkfacial_paint_info_cb(&info, true);
                    }
                }
            }
            return false; /* identity enable always return false */
        }
        pthread_mutex_lock(&g_lib_lock);
        TEST_RESULT_INC(rgb_search_total);
        ret = rockface_feature_search(face_handle,
                mask_score < 0.5 ? &feature : (rockface_feature_t *)&mask,
                mask_score < 0.5 ? get_face_recognition_score() : get_face_mask_recognition_score(), &result);
        if (ret == ROCKFACE_RET_SUCCESS) {
            TEST_RESULT_INC(rgb_search_ok);
            *similarity = result.similarity;
            if (mask_score < 0.5)
                *face_data = (struct face_data *)result.face_data;
            else
                *mask_data = (struct mask_data *)result.face_data;
            pthread_mutex_unlock(&g_lib_lock);
            if (g_register && ++g_register_cnt > FACE_REGISTER_CNT) {
                g_register = false;
                g_register_cnt = 0;
                play_wav_signal(REGISTER_ALREADY_WAV);
            }
            return true;
        }
        pthread_mutex_unlock(&g_lib_lock);
        if (g_register && *index < cnt && face->score > FACE_SCORE_REGISTER && reg && strlen(g_white_list)) {
            char name[NAME_LEN];
            int id = database_get_user_name_id();
            if (id < 0) {
                printf("%s: get id fail!\n", __func__);
                return false;
            }
            snprintf(name, sizeof(name), "%s/%s_%d.jpg", g_white_list, USER_NAME, id);
#ifdef USE_WEB_SERVER
            strncpy(g_snap.name, name, sizeof(g_snap.name));
            if (!snapshot_run(&g_snap, image, NULL, RK_FORMAT_RGB_888, 0, 0))
                printf("save %s success\n", name);
#endif

            if (mask_score < 0.5)
                rockface_control_add_ui(id, name, &feature, NULL);
            else
                rockface_control_add_ui(id, name, NULL, &mask);

            g_register = false;
            g_register_cnt = 0;
            play_wav_signal(REGISTER_SUCCESS_WAV);
            return false;
        }
#ifdef USE_WEB_SERVER
        memset(g_snap.name, 0, sizeof(g_snap.name));
        if (!snapshot_run(&g_snap, image, face, RK_FORMAT_RGB_888, SNAP_TIME, 'S'))
            db_monitor_snapshot_record_set(g_snap.name);
#endif
    }

    return false;
}

void rockface_control_set_delete(void)
{
    g_register = false;
    g_register_cnt = 0;
    g_delete = true;
}

void rockface_control_set_register(void)
{
    g_delete = false;
    if (g_register_cnt == 0)
        g_register = true;
}

static void rockface_control_detect_wait(void)
{
    pthread_mutex_lock(&g_detect_mutex);
    if (g_detect_flag)
        pthread_cond_wait(&g_detect_cond, &g_detect_mutex);
    g_detect_flag = true;
    pthread_mutex_unlock(&g_detect_mutex);
}

static void rockface_control_detect_signal(void)
{
    pthread_mutex_lock(&g_detect_mutex);
    g_detect_flag = false;
    pthread_cond_signal(&g_detect_cond);
    pthread_mutex_unlock(&g_detect_mutex);
}

static int rockface_control_wait(void)
{
    int ret;
    pthread_mutex_lock(&g_mutex);
    if (g_feature_flag) {
#define TIMEOUT_US 100000
#define ONE_MIN_US 1000000
        struct timeval now;
        struct timespec out;
        gettimeofday(&now, NULL);
        if (now.tv_usec + TIMEOUT_US >= ONE_MIN_US) {
            out.tv_sec = now.tv_sec + 1;
            out.tv_nsec = (now.tv_usec + TIMEOUT_US - ONE_MIN_US) * 1000;
        } else {
            out.tv_sec = now.tv_sec;
            out.tv_nsec = (now.tv_usec + TIMEOUT_US) * 1000;
        }
        ret = pthread_cond_timedwait(&g_cond, &g_mutex, &out);
    }
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

static void rockface_control_signal(void)
{
    pthread_mutex_lock(&g_mutex);
    g_feature_flag = false;
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_mutex);
}

int rockface_control_convert_detect(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation, int id)
{
    rga_info_t src, dst;
    struct face_buf *buf;

    if (!g_run || !g_detect_en)
        return -1;

    pthread_mutex_lock(&g_det_lock);
    if (g_det_free.empty()) {
        pthread_mutex_unlock(&g_det_lock);
        return -1;
    } else {
        buf = g_det_free.front();
        g_det_free.pop_front();
        pthread_mutex_unlock(&g_det_lock);
    }

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = rotation;
    rga_set_rect(&src.rect, 0, 0, width, height, width, height, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = buf->bo.ptr;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, DET_WIDTH, DET_HEIGHT,
                 DET_WIDTH, DET_HEIGHT, RK_FORMAT_RGB_888);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        goto exit;
    }
    memset(&buf->img, 0, sizeof(rockface_image_t));
    buf->img.width = DET_WIDTH;
    buf->img.height = DET_HEIGHT;
    buf->img.pixel_format = ROCKFACE_PIXEL_FORMAT_RGB888;
    buf->img.data = (uint8_t *)buf->bo.ptr;
    buf->id = id;

    pthread_mutex_lock(&g_det_lock);
    g_det_ready.push_back(buf);
    pthread_mutex_unlock(&g_det_lock);
    rockface_control_detect_signal();

    return 0;

exit:
    pthread_mutex_lock(&g_det_lock);
    g_det_free.push_back(buf);
    pthread_mutex_unlock(&g_det_lock);
    return -1;
}

int rockface_control_convert_feature(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation, int id)
{
    rga_info_t src, dst;
    if (!g_feature_flag || g_feature.id)
        return -1;
    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = rotation;
    rga_set_rect(&src.rect, 0, 0, width, height, width, height, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = g_feature.bo.ptr;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, height, width, height, width, RK_FORMAT_RGB_888);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return -1;
    }
    memset(&g_feature.img, 0, sizeof(g_feature.img));
    g_feature.img.width = height;
    g_feature.img.height = width;
    g_feature.img.pixel_format = ROCKFACE_PIXEL_FORMAT_RGB888;
    g_feature.img.data = (uint8_t *)g_feature.bo.ptr;
    g_feature.id = id;

    return 0;
}

static bool rockface_control_liveness_ir(void)
{
    rockface_ret_t ret;
    rockface_liveness_t result;

    TEST_RESULT_INC(ir_liveness_total);
    ret = rockface_liveness_detect(face_handle, &g_ir_img, &g_ir_face.box, &result);
    if (ret != ROCKFACE_RET_SUCCESS)
        return false;

    if (result.real_score < get_live_detect_score())
        return false;

    TEST_RESULT_INC(ir_liveness_ok);
    return true;
}

static bool rockface_control_detect_ir(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation)
{
    rockface_ret_t ret;
    rockface_det_array_t face_array;

    int src_w = width, src_h = height;
    int dst_w = DET_WIDTH, dst_h = DET_HEIGHT;
    rga_info_t src, dst;
    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = rotation;
    rga_set_rect(&src.rect, 0, 0, src_w, src_h, src_w, src_h, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = g_ir_det_bo.ptr;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, RK_FORMAT_RGB_888);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return false;
    }

    rockface_image_t ir_det_img;
    memset(&ir_det_img, 0, sizeof(rockface_image_t));
    ir_det_img.width = DET_WIDTH;
    ir_det_img.height = DET_HEIGHT;
    ir_det_img.pixel_format = ROCKFACE_PIXEL_FORMAT_RGB888;
    ir_det_img.data = (uint8_t *)g_ir_det_bo.ptr;
    rockface_output_test();
    TEST_RESULT_INC(ir_detect_total);
    ret = rockface_detect(face_handle, &ir_det_img, &face_array);
    if (ret != ROCKFACE_RET_SUCCESS)
        return false;

    rockface_det_t* face = get_max_face(&face_array);
    if (face == NULL || face->score < get_face_detect_score() ||
        face->box.right - face->box.left <= get_min_pixel(ir_det_img.width))
        return false;

    if (!check_face_region(&face->box, ir_det_img.width, ir_det_img.height))
        return false;

    face->box.left *= g_ratio;
    face->box.top *= g_ratio;
    face->box.right *= g_ratio;
    face->box.bottom *= g_ratio;
    memcpy(&g_ir_face, face, sizeof(rockface_det_t));
    TEST_RESULT_INC(ir_detect_ok);

    return true;
}

static void save_ir(const char *path)
{
    char ext[128];
    snprintf(ext, sizeof(ext), "(%f)[%d,%d,%d,%d]", g_ir_face.score,
            g_ir_face.box.left, g_ir_face.box.top,
            g_ir_face.box.right, g_ir_face.box.bottom);
    save_file(g_ir_bo.ptr, g_ir_img.width * g_ir_img.height, path, ext);
}

int rockface_control_convert_ir(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation)
{
    int ret = -1;
    rga_info_t src, dst;

    if (!g_run || !g_detect_en)
        return ret;

    if (g_ir_state != IR_STATE_PREPARED)
        return ret;

    if (!rockface_control_detect_ir(ptr, width, height, fmt, rotation))
        goto exit;

    memset(&g_ir_img, 0, sizeof(rockface_image_t));
    g_ir_img.pixel_format = ROCKFACE_PIXEL_FORMAT_GRAY8;
    g_ir_img.data = (uint8_t *)g_ir_bo.ptr;
    switch (rotation) {
    case 0:
        g_ir_img.width = width;
        g_ir_img.height = height;
        break;
    case HAL_TRANSFORM_ROT_90:
    case HAL_TRANSFORM_ROT_270:
        g_ir_img.width = height;
        g_ir_img.height = width;
        break;
    default:
        printf("%s: unsupport rotation!\n", __func__);
        goto exit;
    }

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = rotation;
    rga_set_rect(&src.rect, 0, 0, width, height, width, height, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = g_ir_bo.ptr;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, g_ir_img.width, g_ir_img.height,
                 g_ir_img.width, g_ir_img.height, fmt);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        goto exit;
    }

    if (g_ir_save_real && g_ir_save_fake) {
        save_ir(IR_PATH);
        return 0;
    }

    if (rockface_control_liveness_ir()) {
        rockface_control_signal();

        if (g_ir_save_real)
            save_ir(IR_REAL_PATH);
    } else {
        if (rkfacial_paint_info_cb) {
            struct user_info info;
            rockface_set_user_info(&info, USER_STATE_FAKE, &g_ir_face, &g_feature.face);
            rkfacial_paint_info_cb(&info, false);
        }

        if (g_ir_save_fake)
            save_ir(IR_FAKE_PATH);
    }

    g_ir_state = IR_STATE_CANCELED;
    return 0;

exit:
    if (g_ir_save_real && g_ir_save_fake)
        return ret;
    /* ir detect 2 times may cost 130ms, should not try over 3 times */
    if (++g_ir_detect_fail >= 2) {
        g_ir_state = IR_STATE_CANCELED;
        g_ir_detect_fail = 0;
        if (rkfacial_paint_info_cb) {
            struct user_info info;
            rockface_set_user_info(&info, USER_STATE_FAKE, NULL, &g_feature.face);
            rkfacial_paint_info_cb(&info, false);
        }
    }
    return ret;
}

static void *rockface_control_detect_thread(void *arg)
{
    rockface_ret_t ret;
    rga_info_t src, dst;
    int det;
    struct face_buf *buf = NULL;
    int live_det_en;

    while (g_run) {
        if (buf) {
            pthread_mutex_lock(&g_det_lock);
            g_det_free.push_back(buf);
            pthread_mutex_unlock(&g_det_lock);
        }
        buf = NULL;
        pthread_mutex_lock(&g_det_lock);
        if (g_det_ready.empty()) {
            pthread_mutex_unlock(&g_det_lock);
            rockface_control_detect_wait();
        } else {
            buf = g_det_ready.front();
            g_det_ready.pop_front();
            pthread_mutex_unlock(&g_det_lock);
        }
        if (!buf)
            continue;

        if (!g_run)
            break;

        det = rockface_control_detect(&buf->img, &buf->face);
        if (det) {
            if (det == -1)
                memset(last_name, 0, sizeof(last_name));
            g_feature.id = 0;
            continue;
        }

        if (!get_face_config_live_det_en(&live_det_en))
            live_det_en = true;
        if (!g_feature_flag || (live_det_en && g_ir_state != IR_STATE_CANCELED))
            continue;

        if (g_feature.id == buf->id) {
            memcpy(&g_feature.face, &buf->face, sizeof(rockface_det_t));
            g_feature.face.box.left = buf->face.box.left * g_ratio;
            g_feature.face.box.top = buf->face.box.top * g_ratio;
            g_feature.face.box.right = buf->face.box.right * g_ratio;
            g_feature.face.box.bottom = buf->face.box.bottom * g_ratio;
            g_feature.id = 0;
            pthread_mutex_lock(&g_rgb_track_mutex);
            g_rgb_track = buf->face.id;
            pthread_mutex_unlock(&g_rgb_track_mutex);
            if (live_det_en) {
                memset(&g_ir_face, 0, sizeof(rockface_det_t));
                g_ir_detect_fail = 0;
                g_ir_state = IR_STATE_PREPARED;
#ifdef IR_TEST_DATA
                if (!camir_control_run()) {
                    rockface_control_convert_ir(g_test_bo.ptr, g_face_width, g_face_height,
                                                RK_FORMAT_YCbCr_420_SP, 0);
                    g_ir_state = IR_STATE_CANCELED;
                }
#endif
            } else {
                rockface_control_signal();
            }
        } else if (g_feature.id < buf->id) {
            g_feature.id = 0;
        }
    }

    pthread_exit(NULL);
}

static void *rockface_control_feature_thread(void *arg)
{
    int index;
    struct face_data *result;
    struct mask_data *mask;
    rockface_det_t face;
    struct timeval t0, t1;
    int del_timeout = 0;
    int reg_timeout = 0;
    bool ret;
    char result_name[NAME_LEN];
    int timeout;
    float similar;
    int id;
    char has_mask;

    while (g_run) {
        pthread_mutex_lock(&g_mutex);
        g_feature_flag = true;
        pthread_mutex_unlock(&g_mutex);
        timeout = rockface_control_wait();
        if (!g_run)
            break;
        if (g_delete) {
            if (!del_timeout) {
                play_wav_signal(DELETE_START_WAV);
            }
            del_timeout++;
            if (del_timeout > 100) {
                del_timeout = 0;
                g_delete = false;
                play_wav_signal(DELETE_TIMEOUT_WAV);
            }
        } else {
            del_timeout = 0;
        }
        if (g_register && g_face_index < g_face_cnt) {
            if (!reg_timeout) {
                play_wav_signal(REGISTER_START_WAV);
            }
            reg_timeout++;
            if (reg_timeout > 100) {
                reg_timeout = 0;
                g_register = false;
                play_wav_signal(REGISTER_TIMEOUT_WAV);
            }
        } else if (g_register && g_face_index >= g_face_cnt) {
            g_register = false;
            g_register_cnt = 0;
            play_wav_signal(REGISTER_LIMIT_WAV);
        } else {
            reg_timeout = 0;
        }
        if (timeout == ETIMEDOUT)
            continue;
        memcpy(&face, &g_feature.face, sizeof(face));
        gettimeofday(&t0, NULL);
        result = NULL;
        mask = NULL;
        ret = (struct face_data*)rockface_control_search(&g_feature.img, g_face_data, &g_face_index,
                        g_face_cnt, sizeof(struct face_data), 0, &face, reg_timeout, &result, &mask,
                        &similar);
        if (result) {
            id = result->id;
            has_mask = 0;
        } else if (mask) {
            id = mask->id;
            has_mask = 1;
        } else {
            id = -1;
            has_mask = 0;
        }
        gettimeofday(&t1, NULL);
        if (g_delete && del_timeout && id >= 0) {
            rockface_control_delete(id, NULL, true, true);
            del_timeout = 0;
            g_delete = false;
            play_wav_signal(DELETE_SUCCESS_WAV);
            if (rkfacial_paint_info_cb) {
                struct user_info info;
                rockface_set_user_info(&info, USER_STATE_REAL_UNREGISTERED, &g_ir_face, &g_feature.face);
                rkfacial_paint_info_cb(&info, true);
            }
        } else if (id >= 0 && face.score > get_face_detect_score()) {
            if (database_is_id_exist(id, result_name, NAME_LEN)) {
                if (!g_register && memcmp(last_name, result_name, sizeof(last_name))) {
                    char status[64];
                    char similarity[64];
                    char mark;
                    printf("name: %s\n", result_name);
                    memset(last_name, 0, sizeof(last_name));
                    strncpy(last_name, result_name, sizeof(last_name) - 1);
                    if (strstr(result_name, "black_list")) {
                        printf("%s in black_list\n", result_name);
                        snprintf(status, sizeof(status), "close");
                        mark = 'B';
                    } else {
                        snprintf(status, sizeof(status), "open");
                        mark = 'W';
                        play_wav_signal(PLEASE_GO_THROUGH_WAV);
                    }
                    snprintf(similarity, sizeof(similarity), "%f", FACE_SIMILARITY_CONVERT(similar));
#ifdef USE_WEB_SERVER
                    memset(g_snap.name, 0, sizeof(g_snap.name));
                    if (!snapshot_run(&g_snap, &g_feature.img, &face, RK_FORMAT_RGB_888, 0, mark))
                        db_monitor_control_record_set(id, g_snap.name,
                                status, similarity);
#endif
                }
                if (rkfacial_paint_info_cb) {
                    struct user_info info;
                    enum user_state state = USER_STATE_REAL_REGISTERED_WHITE;
                    if (strstr(result_name, "black_list"))
                        state = USER_STATE_REAL_REGISTERED_BLACK;
                    rockface_set_user_info(&info, state, &g_ir_face, &g_feature.face);
                    info.has_mask = has_mask;
                    strncpy(info.sPicturePath, result_name, sizeof(info.sPicturePath) - 1);
                    db_monitor_get_user_info(&info, id);
                    strncpy(info.snap_path, g_snap.name, sizeof(info.snap_path) - 1);
                    rkfacial_paint_info_cb(&info, true);
                }
            }
        } else {
            if (!g_identity_en && rkfacial_paint_info_cb) {
                struct user_info info;
                rockface_set_user_info(&info, USER_STATE_REAL_UNREGISTERED, &g_ir_face, &g_feature.face);
                rkfacial_paint_info_cb(&info, true);
            }
            if (rkfacial_paint_face_cb) {
                int x, y, w, h;
                face_convert(g_feature.face, &x, &y, &w, &h, g_feature.img.width, g_feature.img.height);
                if (w && h)
                    rkfacial_paint_face_cb(g_feature.bo.ptr, RK_FORMAT_RGB_888, g_feature.img.width, g_feature.img.height,
                                           x, y, w, h);
            }
        }
#if 0
        if (face.score > get_face_detect_score())
            printf("box = (%d %d %d %d) score = %f\n", face.box.left, face.box.top,
                    face.box.right, face.box.bottom, face.score);
#endif
    }

    pthread_exit(NULL);
}

int rockface_control_init(void)
{
    int width = g_face_width;
    int height = g_face_height;
    rockface_ret_t ret;
    char cmd[256];

    if (!g_face_en)
        return 0;

    face_handle = rockface_create_handle();

    if (access(LICENCE_PATH, F_OK)) {
        check_pre_path(BAK_PATH);
        if (access(BAK_LICENCE_PATH, F_OK) == 0) {
            snprintf(cmd, sizeof(cmd), "cp %s %s", BAK_LICENCE_PATH, LICENCE_PATH);
            system(cmd);
        }
    }

    ret = rockface_set_licence(face_handle, LICENCE_PATH);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: authorization error %d!\n", __func__, ret);
        play_wav_signal(AUTHORIZE_FAIL_WAV);
    }
    ret = rockface_set_data_path(face_handle, FACE_DATA_PATH);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: set data path error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_recognizer(face_handle);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init recognizer error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_detector2(face_handle, 5);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init detector error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_landmark(face_handle, 5);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init landmark error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_landmark(face_handle, 106);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init landmark106 error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_liveness_detector(face_handle);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init liveness detector error %d!\n", __func__, ret);
        return -1;
    }

#ifdef FACE_MASK
    ret = rockface_init_mask_recognizer(face_handle);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init mask recognizer error %d!\n", __func__, ret);
        return -1;
    }

    ret = rockface_init_mask_classifier(face_handle);
    if (ret != ROCKFACE_RET_SUCCESS) {
        printf("%s: init mask classifier error %d!\n", __func__, ret);
        return -1;
    }
#endif

    if (g_face_cnt <= 0)
        g_face_cnt = DEFAULT_FACE_NUMBER;
    g_face_data = calloc(g_face_cnt, sizeof(struct face_data));
    if (!g_face_data) {
        printf("face data alloc failed!\n");
        return -1;
    }
#ifdef FACE_MASK
    g_mask_data = calloc(g_face_cnt, sizeof(struct mask_data));
    if (!g_mask_data) {
        printf("face data alloc failed!\n");
        return -1;
    }
#endif

    if (access(DATABASE_PATH, F_OK)) {
        check_pre_path(BAK_PATH);
        if (access(BAK_DATABASE_PATH, F_OK) == 0) {
            snprintf(cmd, sizeof(cmd), "cp %s %s", BAK_DATABASE_PATH, DATABASE_PATH);
            system(cmd);
        }
    }
    if (access(DATABASE_PATH, F_OK) == 0) {
        printf("load face feature from %s\n", DATABASE_PATH);
        if (database_init())
            return -1;
        g_face_index += database_get_data(g_face_data, g_face_cnt, sizeof(rockface_feature_t), 0,
                                          sizeof(int), sizeof(rockface_feature_t), 0);
#ifdef FACE_MASK
        g_mask_index += database_get_data(g_mask_data, g_face_cnt, sizeof(rockface_feature_float_t), 0,
                                          sizeof(int), sizeof(rockface_feature_float_t), 1);
#endif
        database_exit();
    }

    if (database_init())
        return -1;
#ifndef USE_WEB_SERVER
    printf("load face feature from %s\n", DEFAULT_FACE_PATH);
    g_face_index += load_feature(DEFAULT_FACE_PATH, ".jpg",
                        (struct face_data*)g_face_data + g_face_index, g_face_cnt - g_face_index);
#endif
    printf("face number is %d\n", g_face_index);
    sync();
    if (rockface_control_init_library(g_face_data, g_face_index, sizeof(struct face_data), 0, 0))
        return -1;
#ifdef FACE_MASK
    if (rockface_control_init_library(g_mask_data, g_mask_index, sizeof(struct mask_data), 0, 1))
        return -1;
#endif

    for (int i = 0; i < DET_BUFFER_NUM; i++) {
        if (rga_control_buffer_init(&g_detect[i].bo, &g_detect[i].fd, DET_WIDTH, DET_HEIGHT, 24))
            return -1;
        pthread_mutex_lock(&g_det_lock);
        g_det_free.push_back(&g_detect[i]);
        pthread_mutex_unlock(&g_det_lock);
    }

    if (rga_control_buffer_init(&g_feature.bo, &g_feature.fd, width, height, 24))
        return -1;

#ifdef IR_TEST_DATA
    if (rga_control_buffer_init(&g_test_bo, &g_test_fd, width, height, 12))
        return -1;
    FILE *fp = fopen("/oem/ir.yuv", "rb");
    if (!fp) {
        printf("open ir.yuv failed!\n");
        return -1;
    }
    fread(g_test_bo.ptr, 1, width * height * 3 / 2, fp);
    fclose(fp);
#endif

    if (rga_control_buffer_init(&g_ir_bo, &g_ir_fd, width, height, 12))
        return -1;
    if (rga_control_buffer_init(&g_ir_det_bo, &g_ir_det_fd, DET_WIDTH, DET_HEIGHT, 24))
        return -1;

    g_run = true;
    if (pthread_create(&g_detect_tid, NULL, rockface_control_detect_thread, NULL)) {
        printf("%s: pthread_create error!\n", __func__);
        g_run = false;
        return -1;
    }
    if (pthread_create(&g_tid, NULL, rockface_control_feature_thread, NULL)) {
        printf("%s: pthread_create error!\n", __func__);
        g_run = false;
        return -1;
    }

    return 0;
}

void rockface_control_exit(void)
{
    if (!g_face_en)
        return;

    g_run = false;
    rockface_control_detect_signal();
    if (g_detect_tid) {
        pthread_join(g_detect_tid, NULL);
        g_detect_tid = 0;
    }
    rockface_control_signal();
    if (g_tid) {
        pthread_join(g_tid, NULL);
        g_tid = 0;
    }

    rockface_control_release_library();
    rockface_release_handle(face_handle);

    database_exit();

    if (g_face_data) {
        free(g_face_data);
        g_face_data = NULL;
    }
#ifdef FACE_MASK
    if (g_mask_data) {
        free(g_mask_data);
        g_mask_data = NULL;
    }
#endif

    for (int i = 0; i < DET_BUFFER_NUM; i++) {
        rga_control_buffer_deinit(&g_detect[i].bo, g_detect[i].fd);
    }
    rga_control_buffer_deinit(&g_feature.bo, g_feature.fd);
#ifdef IR_TEST_DATA
    rga_control_buffer_deinit(&g_test_bo, g_test_fd);
#endif
    rga_control_buffer_deinit(&g_ir_bo, g_ir_fd);
    rga_control_buffer_deinit(&g_ir_det_bo, g_ir_det_fd);
    snapshot_exit(&g_snap);
}

void rockface_control_database(void)
{
    pthread_mutex_lock(&g_lib_lock);
    memset(g_face_data, 0, g_face_cnt * sizeof(struct face_data));
    g_face_index = database_get_data(g_face_data, g_face_cnt,
            sizeof(rockface_feature_t), 0, sizeof(int), sizeof(rockface_feature_t), 0);
#ifdef FACE_MASK
    memset(g_mask_data, 0, g_face_cnt * sizeof(struct mask_data));
    g_mask_index = database_get_data(g_mask_data, g_face_cnt,
            sizeof(rockface_feature_float_t), 0, sizeof(int), sizeof(rockface_feature_float_t), 1);
#endif
    rockface_control_release_library();
    rockface_control_init_library(g_face_data, g_face_index,
            sizeof(struct face_data), 0, 0);
#ifdef FACE_MASK
    rockface_control_init_library(g_mask_data, g_mask_index,
            sizeof(struct mask_data), 0, 1);
#endif
    pthread_mutex_unlock(&g_lib_lock);
}

void rockface_control_delete_all(void)
{
    database_reset();
    rockface_control_database();
}

int rockface_control_delete(int id, const char *pname, bool notify, bool del)
{
    char name[NAME_LEN];

    if (!database_is_id_exist(id, name, NAME_LEN)) {
        if (pname && strlen(pname))
            unlink(pname);
        return -1;
    }

    printf("delete %d from %s\n", id, DATABASE_PATH);
    database_delete(id, true);
    if (del && strlen(name))
        unlink(name);
    if (notify)
        db_monitor_face_list_delete(id);

    rockface_control_database();

    return 0;
}

int rockface_control_add_ui(int id, const char *name, void *feature, void *mask_feature)
{
    printf("add %s, %d to %s\n", name, id, DATABASE_PATH);
    char user[] = USER_NAME;
    char type[] = "whiteList";
    database_insert(feature, feature ? sizeof(rockface_feature_t) : 0, name, NAME_LEN, id, true,
                    mask_feature, mask_feature ? sizeof(rockface_feature_float_t) : 0);
    db_monitor_face_list_add(id, (char*)name, user, type);

    rockface_control_database();

    return 0;
}

int rockface_control_add_web(int id, const char *name)
{
    rockface_control_delete(id, NULL, false, false);
    printf("add %s, %d to %s\n", name, id, DATABASE_PATH);
    gettimeofday(&g_last_reg_tv, NULL);
    rockface_feature_t f;
    rockface_feature_float_t m;
    float mask_score;
    if (!rockface_control_get_path_feature(name, &f, &m, &mask_score)) {
#if 1
        database_insert(&f, sizeof(rockface_feature_t), name, NAME_LEN, id, g_detect_en ? true : false, &m, sizeof(rockface_feature_float_t));
#else
        rockface_search_result_t result;
        rockface_ret_t ret;
        char result_name[NAME_LEN];
        pthread_mutex_lock(&g_lib_lock);
        ret = rockface_feature_search(face_handle, mask_score < 0.5 ? &f : (rockface_feature_t *)&m,
                                      FACE_SIMILARITY_SCORE_REGISTER, &result);
        pthread_mutex_unlock(&g_lib_lock);
        if (ret != ROCKFACE_RET_SUCCESS) {
            database_insert(&f, sizeof(rockface_feature_t), name, NAME_LEN, id, g_detect_en ? true : false, &m, sizeof(rockface_feature_float_t));
        } else {
            int id;
            if (mask_score < 0.5) {
                struct face_data *face_data = (struct face_data *)result.face_data;
                id = face_data->id;
            } else {
                struct mask_data *mask_data = (struct mask_data *)result.face_data;
                id = mask_data->id;
            }
            memset(result_name, 0, NAME_LEN);
            database_is_id_exist(id, result_name, NAME_LEN);
            printf("%s is similar with %s, similarity is %f\n", name, result_name, result.similarity);
            return 2;
        }
#endif
    } else {
        printf("%s %s fail!\n", __func__, name);
        return -1;
    }

    if (g_detect_en)
        rockface_control_database();

    return 0;
}

int rockface_control_add_local(const char *name)
{
    int id = database_get_user_name_id();
    if (id < 0 || id >= g_face_cnt) {
        printf("%s: get id fail!\n", __func__);
        return -3;
    }
    printf("add %s, %d to %s\n", name, id, DATABASE_PATH);
    gettimeofday(&g_last_reg_tv, NULL);
    rockface_feature_t f;
    rockface_feature_float_t m;
    float mask_score;
    if (!rockface_control_get_path_feature(name, &f, &m, &mask_score)) {
        char type[] = "whiteList";
        char tmp[NAME_LEN];
        const char *begin = strrchr(name, '/');
        const char *end = strrchr(name, '.');
        memset(tmp, 0, sizeof(tmp));
        if (begin && end && end > begin)
            memcpy(tmp, begin + 1, end - begin - 1);
        else
            strcpy(tmp, "unknown_user");
#if 1
        database_insert(&f, sizeof(rockface_feature_t), name, NAME_LEN, id, g_detect_en ? true : false, &m, sizeof(rockface_feature_float_t));
        db_monitor_face_list_add(id, (char*)name, tmp, type);
#else
        rockface_search_result_t result;
        rockface_ret_t ret;
        char result_name[NAME_LEN];
        pthread_mutex_lock(&g_lib_lock);
        ret = rockface_feature_search(face_handle, mask_score < 0.5 ? &f : (rockface_feature_t *)&m,
                                      FACE_SIMILARITY_SCORE_REGISTER, &result);
        pthread_mutex_unlock(&g_lib_lock);
        if (ret != ROCKFACE_RET_SUCCESS) {
            database_insert(&f, sizeof(rockface_feature_t), name, NAME_LEN, id, g_detect_en ? true : false, &m, sizeof(rockface_feature_float_t));
            db_monitor_face_list_add(id, (char*)name, tmp, type);
        } else {
            int id;
            if (mask_score < 0.5) {
                struct face_data *face_data = (struct face_data *)result.face_data;
                id = face_data->id;
            } else {
                struct mask_data *mask_data = (struct mask_data *)result.face_data;
                id = mask_data->id;
            }
            memset(result_name, 0, NAME_LEN);
            database_is_id_exist(id, result_name, NAME_LEN);
            printf("%s is similar with %s, similarity is %f\n", name, result_name, result.similarity);
            return -2;
        }
#endif
    } else {
        printf("%s %s fail!\n", __func__, name);
        return -1;
    }

    if (g_detect_en)
        rockface_control_database();

    return id;
}
