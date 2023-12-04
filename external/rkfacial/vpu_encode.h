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
#ifndef __VPU_ENCODE_H__
#define __VPU_ENCODE_H__

#include <memory.h>
#include <stdint.h>
#include <stdio.h>

#include <rockchip/rk_mpi.h>

struct vpu_encode {
	MppCtx mpp_ctx;
	MppApi* mpi;
	int width;
	int height;
	RK_U8* enc_out_data;
	RK_U32 enc_out_length;
	MppPacket packet;
};

#ifdef __cplusplus
extern "C" {
#endif

int vpu_encode_jpeg_init(struct vpu_encode* encode,
        int width,
        int height,
        int quant,
        MppFrameFormat format);
int vpu_encode_jpeg_doing(struct vpu_encode* encode,
        void* srcbuf,
        int src_fd,
        size_t src_size,
        void *dst_buf,
        int dst_fd,
        size_t dst_size);
void vpu_encode_jpeg_done(struct vpu_encode* encode);

#ifdef __cplusplus
}
#endif

#endif
