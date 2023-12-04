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
#ifndef __VPU_DECODE_H__
#define __VPU_DECODE_H__

#include <memory.h>
#include <stdint.h>
#include <stdio.h>

#include <rockchip/rk_mpi.h>

#define MPP_ALIGN(x, a) (((x) + (a)-1) & ~((a)-1))

struct vpu_decode {
    int in_width;
    int in_height;
    int hor_stride;
    int ver_stride;
    RK_S32 pkt_size;
    MppCtx mpp_ctx;
    MppApi* mpi;
    MppBufferGroup memGroup;
    MppFrameFormat fmt;
};

#ifdef __cplusplus
extern "C" {
#endif

int vpu_decode_jpeg_init(struct vpu_decode* decode, int width, int height);
int vpu_decode_jpeg_doing(struct vpu_decode* decode, void* in_data, RK_S32 in_size,
                          int out_fd, void* out_data);
int vpu_decode_jpeg_done(struct vpu_decode* decode);

#ifdef __cplusplus
}
#endif

#endif
