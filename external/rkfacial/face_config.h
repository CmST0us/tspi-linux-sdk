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
#ifndef __FACE_CONFIG_H__
#define __FACE_CONFIG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct face_config {
    bool en;
    int volume;
    int live_det_en;
    int live_det_th;
    int face_det_th;
    int face_rec_th;
    int face_mask_th;
    int min_pixel;
    int corner_x;
    int corner_y;
    int det_width;
    int det_height;
    int nor_width;
    int nor_height;
};

extern struct face_config g_face_config;

bool get_face_config_nor_height(int *height);
bool get_face_config_nor_width(int *width);
bool get_face_config_det_height(int *height);
bool get_face_config_det_width(int *width);
bool get_face_config_corner_y(int *y);
bool get_face_config_corner_x(int *x);
bool get_face_config_min_pixel(int *pixel);
bool get_face_config_face_rec_th(int *th);
bool get_face_config_face_mask_th(int *th);
bool get_face_config_face_det_th(int *th);
bool get_face_config_live_det_th(int *th);
bool get_face_config_live_det_en(int *en);
bool get_face_config_volume(int *volume);

#ifdef __cplusplus
}
#endif
#endif
