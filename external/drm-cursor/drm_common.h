/*
 *  Copyright (c) 2021, Jeffy Chen <jeffy.chen@rock-chips.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef __DRM_COMMON_H_
#define __DRM_COMMON_H_

#include <drm_fourcc.h>

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

#define LIBDRM_CURSOR_VERSION "1.4.1~20230414"

#define drm_private __attribute__((visibility("hidden")))

#ifndef DRM_FORMAT_MOD_VENDOR_ARM
#define DRM_FORMAT_MOD_VENDOR_ARM 0x08
#endif

#ifndef DRM_FORMAT_MOD_ARM_AFBC
#define DRM_FORMAT_MOD_ARM_AFBC(__afbc_mode) fourcc_mod_code(ARM, __afbc_mode)
#endif

#ifndef AFBC_FORMAT_MOD_BLOCK_SIZE_16x16
#define AFBC_FORMAT_MOD_BLOCK_SIZE_16x16 (1ULL)
#endif

#ifndef AFBC_FORMAT_MOD_SPARSE
#define AFBC_FORMAT_MOD_SPARSE (((__u64)1) << 6)
#endif

#define DRM_AFBC_MODIFIER \
  (DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_SPARSE) | \
   DRM_FORMAT_MOD_ARM_AFBC(AFBC_FORMAT_MOD_BLOCK_SIZE_16x16))

#define DRM_LOG(tag, ...) { \
  struct timeval tv; gettimeofday(&tv, NULL); \
  fprintf(g_log_fp ? g_log_fp : stderr, "[%05ld.%03ld] " tag ": %s(%d) ", \
          tv.tv_sec % 100000, tv.tv_usec / 1000, __func__, __LINE__); \
  fprintf(g_log_fp ? g_log_fp : stderr, __VA_ARGS__); \
  fflush(g_log_fp ? g_log_fp : stderr); }

#define DRM_DEBUG(...) \
  if (g_drm_debug) DRM_LOG("DRM_DEBUG", __VA_ARGS__)

#define DRM_INFO(...) DRM_LOG("DRM_INFO", __VA_ARGS__)

#define DRM_ERROR(...) DRM_LOG("DRM_ERROR", __VA_ARGS__)

drm_private extern int g_drm_debug;
drm_private extern FILE *g_log_fp;

#endif
