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

#ifndef __UVC_ENCODE_H__
#define __UVC_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "mpi_enc.h"

struct uvc_encode
{
    int width;
    int height;
    int video_id;
    int fcc;
    unsigned int fps;
    MpiEncTestCmd mpi_cmd;
    MpiEncTestData *mpi_data;
    void *extra_data;
    size_t extra_size;
    long long int loss_frm;
};

int uvc_encode_init(struct uvc_encode *e, int width, int height, int fcc, int h265, unsigned int fps);
void uvc_encode_exit(struct uvc_encode *e);
void uvc_encode_inbuf_deinit(struct uvc_encode *e);
bool uvc_encode_process(struct uvc_encode *e, void *virt, struct MPP_ENC_INFO *info);

#ifdef __cplusplus
}
#endif

#endif
