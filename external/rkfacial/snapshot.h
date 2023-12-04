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
#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "vpu_encode.h"
#include "rga_control.h"
#include "video_common.h"
#include <rockface/rockface.h>

struct snapshot {
    struct vpu_encode enc;
    bo_t nv12_bo;
    int nv12_fd;
    bo_t enc_bo;
    int enc_fd;
    int size;
    char name[NAME_LEN];
    struct timeval t0;
    struct timeval t1;
};

int snapshot_init(struct snapshot *s, int w, int h);
void snapshot_exit(struct snapshot *s);
int snapshot_run(struct snapshot *s, rockface_image_t *image, rockface_det_t *face,
                 RgaSURF_FORMAT fmt, long int sec, char mark);
void face_convert(rockface_det_t face, int *x, int *y, int *w, int *h, int width, int height);

#ifdef __cplusplus
}
#endif

#endif
