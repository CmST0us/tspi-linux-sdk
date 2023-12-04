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
#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

void database_bak(void);
int database_init(void);
void database_exit(void);
void database_reset(void);
int database_insert(void *data, size_t size, const char *name, size_t n_size, int id, bool sync_flag, void *mask, size_t mask_size);
int database_record_count(void);
int database_get_data(void *dst, const int cnt, size_t d_size, size_t d_off,
                      size_t i_size, size_t i_off, int mask);
bool database_is_name_exist(const char *name);
bool database_is_id_exist(int id, char *name, size_t size);
int database_get_user_name_id(void);
void database_delete(int id, bool sync_flag);

#ifdef __cplusplus
}
#endif

#endif
