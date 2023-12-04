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
#include "image_read.h"
#include "vpu_decode.h"
#include "rga_control.h"

static int mjpeg_get_resolutin(FILE *fp, int *width, int *height)
{
    int ret = -1;
    char mark[2];
    char size[2];
    char data[14];

    rewind(fp);
    while (!feof(fp)) {
        fread(mark, 1, sizeof(mark), fp);
        if (mark[0] == 0xFF && mark[1] == 0xD8)
            continue;
        if (mark[0] != 0xFF)
            break;
        if (mark[1] == 0xC0) {
            fseek(fp, 3, SEEK_CUR);
            fread(data, 1, sizeof(data), fp);
            *height = data[0] * 256 + data[1];
            *width = data[2] * 256 + data[3];
            /* rga support RK_FORMAT_YCbCr_420_SP and RK_FORMAT_YCbCr_422_SP */
            if (data[4] == 3 && (data[6] == 0x22 || data[6] == 0x21) && data[9] == 0x11 && data[12] == 0x11)
                ret = 0;
            break;
        } else {
            fread(size, 1, sizeof(size), fp);
            fseek(fp, size[0] * 256 + size[1] - 2, SEEK_CUR);
            continue;
        }
    }

    return ret;
}

static int _decode(int width, int height, void *data, size_t size, int out_fd, void* out_data, int *fmt, int *hor_stride, int *ver_stride)
{
    int ret = -1;
    struct vpu_decode dec;
    memset(&dec, 0, sizeof(dec));
    if (!vpu_decode_jpeg_init(&dec, width, height))
        ret = vpu_decode_jpeg_doing(&dec, data, size, out_fd, out_data);
    *fmt = (MPP_FMT_YUV422SP != dec.fmt ? RK_FORMAT_YCbCr_420_SP : RK_FORMAT_YCbCr_422_SP);
    *hor_stride = dec.hor_stride;
    *ver_stride = dec.ver_stride;
    vpu_decode_jpeg_done(&dec);
    return ret;
}

static int image_read_begin(const char *path, bo_t *buf_bo, int *buf_fd, int *w, int *h, int *fmt, int *hor_stride, int *ver_stride)
{
    int ret = 0;
    int width, height;
    FILE *fp;
    size_t size;
    void *data;

    memset(buf_bo, 0, sizeof(bo_t));
    *buf_fd = -1;

    fp = fopen(path, "rb");
    if (!fp)
        return -1;

    if (mjpeg_get_resolutin(fp, &width, &height) || width % 2 || height % 2) {
        ret = -1;
        goto err_close;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    data = malloc(size);
    if (!data) {
        ret = -1;
        goto err_close;
    }

    rewind(fp);
    fread(data, 1, size, fp);

    *w = width;
    *h = height;
    if (rga_control_buffer_init(buf_bo, buf_fd, MPP_ALIGN(width, 16), MPP_ALIGN(height, 16), 24)) {
        printf("%s: alloc buffer failed!\n", __func__);
        ret = -1;
        goto err_free;
    }

    ret = _decode(width, height, data, size, *buf_fd, buf_bo->ptr, fmt, hor_stride, ver_stride);

err_free:
    if (data)
        free(data);

err_close:
    if (fp)
        fclose(fp);
    return ret;
}

static int image_read_end(bo_t *buf_bo, int buf_fd)
{
    rga_control_buffer_deinit(buf_bo, buf_fd);
    return 0;
}

int image_read(const char *path, rockface_image_t *img, bo_t *rgb_bo, int *rgb_fd)
{
    int ret = -1;
    bo_t dec_bo;
    int dec_fd = -1;
    int width = 0, height = 0;
    int fmt;
    int hor_stride, ver_stride;

    if (image_read_begin(path, &dec_bo, &dec_fd, &width, &height, &fmt, &hor_stride, &ver_stride)) {
        ret = -2;
        goto exit0;
    }

    if (rga_control_buffer_init(rgb_bo, rgb_fd, hor_stride, ver_stride, 24)) {
        printf("%s: alloc buffer failed!\n", __func__);
        goto exit0;
    }

    rga_info_t src, dst;
    memset(&src, 0, sizeof(rga_info_t));
    src.fd = -1;
    src.virAddr = dec_bo.ptr;
    src.mmuFlag = 1;
    rga_set_rect(&src.rect, 0, 0, width, height, hor_stride, ver_stride, fmt);
    memset(&dst, 0, sizeof(rga_info_t));
    dst.fd = -1;
    dst.virAddr = rgb_bo->ptr;
    dst.mmuFlag = 1;
    rga_set_rect(&dst.rect, 0, 0, hor_stride, ver_stride, hor_stride, ver_stride, RK_FORMAT_RGB_888);
    if (c_RkRgaBlit(&src, &dst, NULL)) {
        printf("%s: rga fail\n", __func__);
        goto exit0;
    }

    memset(img, 0, sizeof(rockface_image_t));
    img->width = hor_stride;
    img->height = ver_stride;
    img->pixel_format = ROCKFACE_PIXEL_FORMAT_RGB888;
    img->data = (uint8_t *)rgb_bo->ptr;
    ret = 0;

exit0:
    if (width > 0 && height > 0)
        image_read_end(&dec_bo, dec_fd);
    return ret;
}

int image_read_deinit(bo_t *rgb_bo, int *rgb_fd)
{
    rga_control_buffer_deinit(rgb_bo, *rgb_fd);
}
