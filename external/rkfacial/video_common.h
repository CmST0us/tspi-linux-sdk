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
#ifndef __VIDEO_COMMON_H__
#define __VIDEO_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "face_common.h"

#define WIDTH 1280
#define HEIGHT 720

#define IR_PATH "/userdata/ir"
#define IR_REAL_PATH "/userdata/ir_real"
#define IR_FAKE_PATH "/userdata/ir_fake"

extern char g_snapshot[NAME_LEN];
extern char g_white_list[NAME_LEN];
extern char g_black_list[NAME_LEN];

int get_video_id(char *name);
int check_path_dir(const char *name);
void save_file(void *buf, size_t size, const char *dir, const char *ext);
bool is_command_success(const char *cmd);

#ifdef __cplusplus
}
#endif
#endif
