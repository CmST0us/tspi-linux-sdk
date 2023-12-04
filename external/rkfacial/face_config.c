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
#include "face_config.h"

struct face_config g_face_config;

#define GET_FACE_CONFIG_FUNC(val) \
    bool get_face_config_##val(int *arg) \
    { \
        if (!g_face_config.en) \
            return false; \
        *arg = g_face_config.val; \
        return true; \
    }

GET_FACE_CONFIG_FUNC(volume)
GET_FACE_CONFIG_FUNC(live_det_en)
GET_FACE_CONFIG_FUNC(live_det_th)
GET_FACE_CONFIG_FUNC(face_det_th)
GET_FACE_CONFIG_FUNC(face_rec_th)
GET_FACE_CONFIG_FUNC(face_mask_th)
GET_FACE_CONFIG_FUNC(min_pixel)
GET_FACE_CONFIG_FUNC(corner_x)
GET_FACE_CONFIG_FUNC(corner_y)
GET_FACE_CONFIG_FUNC(det_width)
GET_FACE_CONFIG_FUNC(det_height)
GET_FACE_CONFIG_FUNC(nor_width)
GET_FACE_CONFIG_FUNC(nor_height)
