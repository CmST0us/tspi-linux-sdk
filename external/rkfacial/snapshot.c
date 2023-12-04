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
#include "snapshot.h"
#include <sys/time.h>

#define SNAP_ALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

int snapshot_init(struct snapshot *s, int w, int h)
{
    if (s->size)
        return 0;

    if (rga_control_buffer_init_nocache(&s->nv12_bo, &s->nv12_fd, w, h, 12))
        return -1;
    if (rga_control_buffer_init_nocache(&s->enc_bo, &s->enc_fd, w, h, 12))
        return -1;
    s->size = w * h * 3 / 2;

    return 0;
}

void snapshot_exit(struct snapshot *s)
{
    if (s->size) {
        rga_control_buffer_deinit(&s->nv12_bo, s->nv12_fd);
        rga_control_buffer_deinit(&s->enc_bo, s->enc_fd);
    }
}

void face_convert(rockface_det_t face, int *x, int *y, int *w, int *h, int width, int height)
{
    int temp;

    /* expand face region 50% */
    temp = (face.box.right - face.box.left) / 4;
    face.box.left -= temp;
    face.box.right += temp;
    temp = (face.box.bottom - face.box.top) / 4;
    face.box.top -= temp;
    face.box.bottom += temp;
    if (face.box.left <= 0)
        face.box.left = 0;
    if (face.box.right >= width)
        face.box.right = width - 1;
    if (face.box.top <= 0)
        face.box.top = 0;
    if (face.box.bottom >= height)
        face.box.bottom = height - 1;

    *w = face.box.right - face.box.left;
    *h = face.box.bottom - face.box.top;
    *w = SNAP_ALIGN(*w, 16);
    *h = SNAP_ALIGN(*h, 16);

    if (*w > width)
        *w = width;
    if (*h > height)
        *h = height;

    if (face.box.left + (*w) >= width)
        *x = face.box.right - (*w);
    else
        *x = face.box.left;

    if (face.box.top + (*h) >= height)
        *y = face.box.bottom - (*h);
    else
        *y = face.box.top;

    *x = SNAP_ALIGN(*x, 2);
    *y = SNAP_ALIGN(*y, 2);

    if (*x < 0)
        *x = 0;
    if (*y < 0)
        *y = 0;
}

int snapshot_run(struct snapshot *s, rockface_image_t *image, rockface_det_t *face,
                 RgaSURF_FORMAT fmt, long int sec, char mark)
{
    FILE *fp;
    rga_info_t src, dst;
    int w, h;
    int x, y;

    void *buffer = image->data;
    int width = image->width;
    int height = image->height;

    if (!strlen(g_snapshot))
        return -1;

    if (snapshot_init(s, width, height))
        return -1;

    if (width * height * 3 / 2 > s->size)
        return -1;

    if (sec) {
        if (!s->t0.tv_sec && !s->t0.tv_usec) {
            gettimeofday(&s->t0, NULL);
        } else {
            gettimeofday(&s->t1, NULL);
            if (s->t1.tv_sec - s->t0.tv_sec < sec)
                return -1;
            else
                gettimeofday(&s->t0, NULL);
        }
    } else {
        gettimeofday(&s->t0, NULL);
    }

    if (!strlen(s->name)) {
        if (mark)
            snprintf(s->name, sizeof(s->name), "%s/%c%ld.%06ld.jpg",
                    g_snapshot, mark, s->t0.tv_sec, s->t0.tv_usec);
        else
            snprintf(s->name, sizeof(s->name), "%s/%ld.%06ld.jpg",
                    g_snapshot, s->t0.tv_sec, s->t0.tv_usec);
    }

    if (face) {
        face_convert(*face, &x, &y, &w, &h, width, height);
        if (!w || !h)
            return -1;
    } else {
        w = width;
        h = height;
        x = 0;
        y = 0;
    }

    if (vpu_encode_jpeg_init(&s->enc, w, h, 7, MPP_FMT_YUV420SP))
        return -1;

    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = buffer;
    src.mmuFlag = 1;
    rga_set_rect(&src.rect, x, y, w, h, width, height, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = s->nv12_bo.ptr;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, w, h, w, h, RK_FORMAT_YCbCr_420_SP);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        return -1;
    }

    vpu_encode_jpeg_doing(&s->enc, s->nv12_bo.ptr, s->nv12_fd, w * h * 3 / 2,
            s->enc_bo.ptr, s->enc_fd, s->size);

    fp = fopen(s->name, "wb");
    if (fp) {
        fwrite(s->enc.enc_out_data, 1, s->enc.enc_out_length, fp);
        fclose(fp);
        printf("%s: save %s ok!\n", __func__, s->name);
    } else {
        printf("%s: open %s fail!\n", __func__, s->name);
    }

    vpu_encode_jpeg_done(&s->enc);
    return 0;
}
