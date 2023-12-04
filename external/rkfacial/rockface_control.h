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
#ifndef __ROCKFACE_CONTROL_H__
#define __ROCKFACE_CONTROL_H__

#include <rockface/rockface.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "rga_control.h"

int rockface_control_init(void);
void rockface_control_init_thread(void);
void rockface_control_exit(void);
int rockface_control_get_path_feature(const char *path, void *feature, void *mask_feature, float *mask_score);
int rockface_control_convert_detect(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation, int id);
int rockface_control_convert_feature(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation, int id);
void rockface_control_set_delete(void);
void rockface_control_set_register(void);
int rockface_control_convert_ir(void *ptr, int width, int height, RgaSURF_FORMAT fmt, int rotation);
void rockface_control_delete_all(void);
int rockface_control_delete(int id, const char *pname, bool notify, bool del);
int rockface_control_add_ui(int id, const char *name, void *feature, void *mask_feature);
int rockface_control_add_web(int id, const char *name);
int rockface_control_add_local(const char *name);
void rockface_control_database(void);
void rockface_control_set_detect_en(int en);
void rockface_control_set_identity_en(int en, char *path);

#ifdef __cplusplus
}
#endif

#endif
