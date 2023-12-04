/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
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

#include "yuv.h"

void NV12_to_YUYV(int width, int height, void* src, void* dst)
{
    int i = 0, j = 0;
    int* src_y = (int*)src;
    int* src_uv = (int*)((char*)src + width * height);
    int* line = (int*)dst;

    for (j = 0; j < height; j++) {
        if (j % 2 != 0)
            src_uv -= width >> 2;
        for (i = 0; i<width >> 2; i++) {
            *line++ = ((*src_y & 0x000000ff)) | ((*src_y & 0x0000ff00) << 8) |
                      ((*src_uv & 0x000000ff) << 8) | ((*src_uv & 0x0000ff00) << 16);
            *line++ = ((*src_y & 0x00ff0000) >> 16) | ((*src_y & 0xff000000) >> 8) |
                      ((*src_uv & 0x00ff0000) >> 8) | ((*src_uv & 0xff000000));
            src_y++;
            src_uv++;
        }
    }
}

void raw16_to_raw8(int width, int height, void* src, void* dst)
{
    unsigned int i, j;
    unsigned int *buf_src, *buf_dst;
    unsigned int cycle;
    buf_src = (unsigned int*)src;
    buf_dst = (unsigned int*)dst;
    cycle = width * height * 2 * 2 / 8;

    for (i = 0, j = 0; j < cycle; j++, i += 2)
        buf_dst[j] = ((buf_src[i] | (buf_src[i] >> 8)) & 0x0000FFFF) | \
                     ((buf_src[i+1] << 8 | (buf_src[i+1] << 16)) & 0xFFFF0000);
}
