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
#include <linux/media-bus-format.h>
#include "rkfacial.h"

static const struct rkisp_api_ctx *ctx;
static const struct rkisp_api_buf *buf;
static bool g_run;
static pthread_t g_tid;

static bool g_ir_en;
static int g_ir_width;
static int g_ir_height;
static display_callback g_display_cb = NULL;
static pthread_mutex_t g_display_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_rotation = HAL_TRANSFORM_ROT_270;

void set_ir_rotation(int angle)
{
    if (angle == 90)
        g_rotation = HAL_TRANSFORM_ROT_90;
    else if (angle == 270)
        g_rotation = HAL_TRANSFORM_ROT_270;
}

void set_ir_display(display_callback cb)
{
    pthread_mutex_lock(&g_display_lock);
    g_display_cb = cb;
    pthread_mutex_unlock(&g_display_lock);
}

void set_ir_param(int width, int height, display_callback cb)
{
    g_ir_en = true;
    g_ir_width = width;
    g_ir_height = height;
    set_ir_display(cb);
}

static void *process(void *arg)
{
    rga_info_t src, dst;

    do {
        buf = rkisp_get_frame(ctx, 0);

        rockface_control_convert_ir(buf->buf, ctx->width, ctx->height,
                                    RK_FORMAT_YCbCr_420_SP, g_rotation);

        pthread_mutex_lock(&g_display_lock);
        if (g_display_cb)
            g_display_cb(buf->buf, buf->fd, RK_FORMAT_YCbCr_420_SP,
                    ctx->width, ctx->height, g_rotation);
        pthread_mutex_unlock(&g_display_lock);

        rkisp_put_frame(ctx, buf);
    } while (g_run);

    pthread_exit(NULL);
}

int camir_control_init(void)
{
    char name[32];

    if (!g_ir_en)
        return 0;

#ifdef CAMERA_ENGINE_RKISP
    int id = get_video_id("stream_cif_dvp");
    if (id < 0) {
        printf("%s: get video id fail!\n", __func__);
        return -1;
    }

    snprintf(name, sizeof(name), "/dev/video%d", id);
    printf("%s: %s\n", __func__, name);
    ctx = rkisp_open_device(name, 0);
#endif
#ifdef CAMERA_ENGINE_RKAIQ
    ctx = rkisp_open_device2(CAM_TYPE_RKCIF);
#endif
    if (ctx == NULL) {
        printf("%s: ctx is NULL\n", __func__);
        return -1;
    }

    rkisp_set_buf(ctx, 3, NULL, 0);
#ifdef CAMERA_ENGINE_RKISP
    rkisp_set_sensor_fmt(ctx, 1280, 720, MEDIA_BUS_FMT_YUYV8_2X8);
#endif
    rkisp_set_fmt(ctx, 1280, 720, V4L2_PIX_FMT_NV12);

    if (rkisp_start_capture(ctx))
        return -1;

    g_run = true;
    if (pthread_create(&g_tid, NULL, process, NULL)) {
        printf("pthread_create fail\n");
        return -1;
    }

    return 0;
}

void camir_control_exit(void)
{
    if (!g_ir_en)
        return;

    g_run = false;
    if (g_tid) {
        pthread_join(g_tid, NULL);
        g_tid = 0;
    }

    rkisp_stop_capture(ctx);
    rkisp_close_device(ctx);
}

bool camir_control_run(void)
{
    return g_run;
}
