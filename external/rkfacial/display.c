/*
 * Copyright (C) 2020 Rockchip Electronics Co., Ltd.
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
#include <pthread.h>

#include <rga/RgaApi.h>
#include "display.h"
#include "rkdrm_display.h"
#include "rkfacial.h"

#define BUF_COUNT 3
#define USE_NV12

struct display {
    int fmt;
    int width;
    int height;
    int plane_type;
    struct drm_dev dev;
    struct drm_buf buf[BUF_COUNT];
    int buf_cnt;
    int rga_fmt;

    /* face rect */
    int x;
    int y;
    int w;
    int h;

    YUV_Color color;
};

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
struct display g_disp;

static int drm_display_init(struct display *disp)
{
    int ret;
    if (drmInit(&disp->dev)) {
        fprintf(stderr, "drmInit: Failed\n");
        return -1;
    }

    for (int i = 0; i < disp->buf_cnt; i++) {
        ret = drmGetBuffer(disp->dev.drm_fd, disp->width, disp->height, disp->fmt, &disp->buf[i]);
        if (ret) {
            fprintf(stderr, "Alloc drm buffer failed, %d\n", i);
            return -1;
        }
    }

    return 0;
}

int display_init(int width, int height)
{
    int ret;
#ifdef USE_NV12
    g_disp.fmt = DRM_FORMAT_NV12;
    g_disp.rga_fmt = RK_FORMAT_YCbCr_420_SP;
#endif
#ifdef USE_RGB888
    g_disp.fmt = DRM_FORMAT_BGR888;
    g_disp.rga_fmt = RK_FORMAT_RGB_888;
#endif
    g_disp.width = width;
    g_disp.height = height;
    g_disp.plane_type = DRM_PLANE_TYPE_OVERLAY;
    g_disp.buf_cnt = BUF_COUNT;
    g_disp.color = set_yuv_color(COLOR_R);
    ret = drm_display_init(&g_disp);
    if (ret)
        return ret;

    return 0;
}


static void drm_display_exit(struct display *disp)
{
    drmDeinit(&disp->dev);
    for (int i = 0; i < disp->buf_cnt; i++)
        drmPutBuffer(disp->dev.drm_fd, &disp->buf[i]);
}

void display_exit(void)
{
    drm_display_exit(&g_disp);
}

void drm_commit(struct display *disp, int num, void *ptr, int fd, int fmt, int w, int h, int rotation)
{
    int ret;
    rga_info_t src, dst;
    char *map = disp->buf[num].map;
    int dst_w = disp->width;
    int dst_h = disp->height;
    int dst_fmt = disp->rga_fmt;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = ptr;
    src.mmuFlag = 1;
    src.rotation = rotation;
    rga_set_rect(&src.rect, 0, 0, w, h, w, h, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = map;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, dst_w, dst_h, dst_w, dst_h, dst_fmt);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return;
    }

    pthread_mutex_lock(&g_lock);
    YUV_Rect rect = {g_disp.x, g_disp.y, g_disp.w, g_disp.h};
    YUV_Color color = g_disp.color;
    pthread_mutex_unlock(&g_lock);
    if (rect.x || rect.y || rect.width || rect.height)
        yuv420_draw_rectangle(map, dst_w, dst_h, rect, color);
    ret = drmCommit(&disp->buf[num], disp->width, disp->height, 0, 0, &disp->dev, disp->plane_type);
    if (ret) {
        fprintf(stderr, "display commit error, ret = %d\n", ret);
    }
}

void display_commit(void *ptr, int fd, int fmt, int w, int h, int rotation)
{
    static int num = 0;

    drm_commit(&g_disp, num, ptr, fd, fmt, w, h, rotation);
    num = (num + 1) % BUF_COUNT;
}

void display_switch(enum display_video_type type)
{
    set_rgb_display(NULL);
    set_ir_display(NULL);
    set_usb_display(NULL);
    if (type == DISPLAY_VIDEO_RGB)
        set_rgb_display(display_commit);
    else if (type == DISPLAY_VIDEO_IR)
        set_ir_display(display_commit);
    else if (type == DISPLAY_VIDEO_USB)
        set_usb_display(display_commit);
}

void display_get_resolution(int *width, int *height)
{
    *width = g_disp.width;
    *height = g_disp.height;
}

void display_paint_box(int left, int top, int right, int bottom)
{
    pthread_mutex_lock(&g_lock);
    g_disp.x = left;
    g_disp.y = top;
    g_disp.w = right - left;
    g_disp.h = bottom - top;
    pthread_mutex_unlock(&g_lock);
}

void display_set_color(YUV_Color color)
{
    pthread_mutex_lock(&g_lock);
    g_disp.color = color;
    pthread_mutex_unlock(&g_lock);
}
