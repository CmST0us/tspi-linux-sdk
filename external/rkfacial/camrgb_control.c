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
#include <pthread.h>
#include <stdbool.h>
#include "rockface_control.h"
#include "video_common.h"

#ifdef CAMERA_ENGINE_RKISP
#include <camera_engine_rkisp/interface/rkisp_api.h>
#endif
#ifdef CAMERA_ENGINE_RKAIQ
#include <rkaiq/rkisp_api.h>
#endif
#include "rga_control.h"
#include "rkfacial.h"

static bool g_def_expo_weights = false;
bool g_expo_weights_en = false;
static unsigned char weights[81];

static const struct rkisp_api_ctx *ctx;
static const struct rkisp_api_buf *buf;
static bool g_run;
static pthread_t g_tid;

bool g_rgb_en;
int g_rgb_width;
int g_rgb_height;
static display_callback g_display_cb = NULL;
static pthread_mutex_t g_display_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_rotation = HAL_TRANSFORM_ROT_90;

void set_rgb_rotation(int angle)
{
    if (angle == 90)
        g_rotation = HAL_TRANSFORM_ROT_90;
    else if (angle == 270)
        g_rotation = HAL_TRANSFORM_ROT_270;
}

void set_rgb_display(display_callback cb)
{
    pthread_mutex_lock(&g_display_lock);
    g_display_cb = cb;
    pthread_mutex_unlock(&g_display_lock);
}

void set_rgb_param(int width, int height, display_callback cb, bool expo)
{
    g_rgb_en = true;
    g_rgb_width = width;
    g_rgb_height = height;
    set_rgb_display(cb);
    g_expo_weights_en = expo;
}

static inline void camrgb_inc_fps(void)
{
    static int fps = 0;
    static struct timeval t0;
    struct timeval t1;

    fps++;
    gettimeofday(&t1, NULL);
    if (t1.tv_sec - t0.tv_sec > 1) {
        fps = 0;
        gettimeofday(&t0, NULL);
    } else if ((t1.tv_sec - t0.tv_sec) * 1000000 + (t1.tv_usec - t0.tv_usec) > 1000000) {
        printf("ISP fps: %d\n", fps);
        fps = 0;
        gettimeofday(&t0, NULL);
    }
}

static void *process(void *arg)
{
    int id = 0;
    do {
        id++;
#if 0
        camrgb_inc_fps();
#endif
        buf = rkisp_get_frame(ctx, 0);

        if (!rockface_control_convert_detect(buf->buf, ctx->width, ctx->height, RK_FORMAT_YCbCr_420_SP, g_rotation, id))
            rockface_control_convert_feature(buf->buf, ctx->width, ctx->height, RK_FORMAT_YCbCr_420_SP, g_rotation, id);

        pthread_mutex_lock(&g_display_lock);
        if (g_display_cb)
            g_display_cb(buf->buf, buf->fd, RK_FORMAT_YCbCr_420_SP,
                    ctx->width, ctx->height, g_rotation);
        pthread_mutex_unlock(&g_display_lock);

        rkisp_put_frame(ctx, buf);
    } while (g_run);

    pthread_exit(NULL);
}

int camrgb_control_init(void)
{
    int id = -1;
    char name[32];

    if (!g_rgb_en)
        return 0;

#ifdef CAMERA_ENGINE_RKISP
    id = get_video_id("rkisp1_mainpath");
    if (id < 0) {
        printf("%s: get video id fail!\n", __func__);
        return -1;
    }
    snprintf(name, sizeof(name), "/dev/video%d", id);
    printf("%s: %s\n", __func__, name);
    ctx = rkisp_open_device(name, 1);
#endif
#ifdef CAMERA_ENGINE_RKAIQ
    ctx = rkisp_open_device2(CAM_TYPE_RKISP1);
#endif
    if (ctx == NULL) {
        printf("%s: ctx is NULL\n", __func__);
        return -1;
    }

    rkisp_set_buf(ctx, 3, NULL, 0);

    rkisp_set_fmt(ctx, g_rgb_width, g_rgb_height, V4L2_PIX_FMT_NV12);

    if (rkisp_start_capture(ctx))
        return -1;

#ifdef CAMERA_ENGINE_RKISP
    rkisp_get_expo_weights(ctx, weights, sizeof(weights));
    printf("default weights:\n");
    for (int i = 0; i < 81; i++) {
        printf("0x%02x ", weights[i]);
        if ((i + 1) % 9 == 0)
            printf("\n");
    }
    printf("\n");
#endif

    g_run = true;
    if (pthread_create(&g_tid, NULL, process, NULL)) {
        printf("pthread_create fail\n");
        return -1;
    }

    return 0;
}

void camrgb_control_exit(void)
{
    if (!g_rgb_en)
        return;

    g_run = false;
    if (g_tid) {
        pthread_join(g_tid, NULL);
        g_tid = 0;
    }

    rkisp_stop_capture(ctx);
    rkisp_close_device(ctx);
}

static void camrgb_control_expo_weights_270(int left, int top, int right, int bottom)
{
#ifdef CAMERA_ENGINE_RKISP
    if (!g_rgb_en)
        return;

    if (g_expo_weights_en) {
        unsigned char weights[81];
        int x = ctx->width - bottom;
        int y = left;
        int w = bottom - top;
        int h = right - left;
        x = x * 9 / ctx->width;
        y = y * 9 / ctx->height;
        w = w * 9 / ctx->width;
        h = h * 9 / ctx->height;
        w = w ? : 1;
        h = h ? : 1;
        memset(weights, 2, sizeof(weights));
        for (int j = 0; j < 9; j++)
            for (int i = 0; i < 9; i++)
                if (i > x && i <= x + w && j > y && j <= y + h)
                    weights[j * 9 + i] = 31;
        rkisp_set_expo_weights(ctx, weights, sizeof(weights));
        g_def_expo_weights = false;
    }
#endif
}

static void camrgb_control_expo_weights_90(int left, int top, int right, int bottom)
{
#ifdef CAMERA_ENGINE_RKISP
    if (!g_rgb_en)
        return;

    if (g_expo_weights_en) {
        unsigned char weights[81];
        int x = ctx->width - top;
        int y = ctx->height - right;
        int w = bottom - top;
        int h = right - left;
        x = x * 9 / ctx->width;
        y = y * 9 / ctx->height;
        w = w * 9 / ctx->width;
        h = h * 9 / ctx->height;
        w = w ? : 1;
        h = h ? : 1;
        memset(weights, 2, sizeof(weights));
        for (int j = 0; j < 9; j++)
            for (int i = 0; i < 9; i++)
                if (i > x && i <= x + w && j > y && j <= y + h)
                    weights[j * 9 + i] = 31;
        rkisp_set_expo_weights(ctx, weights, sizeof(weights));
        g_def_expo_weights = false;
    }
#endif
}

void camrgb_control_expo_weights(int left, int top, int right, int bottom)
{
    if (g_rotation == HAL_TRANSFORM_ROT_90)
        camrgb_control_expo_weights_90(left, top, right, bottom);
    else if (g_rotation == HAL_TRANSFORM_ROT_270)
        camrgb_control_expo_weights_270(left, top, right, bottom);
}

void camrgb_control_expo_weights_default(void)
{
#ifdef CAMERA_ENGINE_RKISP
    if (!g_rgb_en)
        return;

    if (g_expo_weights_en) {
        if (!g_def_expo_weights) {
            g_def_expo_weights = true;
            rkisp_set_expo_weights(ctx, weights, sizeof(weights));
        }
    }
#endif
}
