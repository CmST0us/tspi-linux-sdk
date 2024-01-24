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

#ifndef __DRM_EGL_H_
#define __DRM_EGL_H_

#include <stdint.h>

#include "drm_common.h"

drm_private void *egl_init_ctx(int fd, int num_surfaces, int format, uint64_t modifier);
drm_private void egl_free_ctx(void *data);
drm_private uint32_t egl_convert_fb(int fd, void *data, uint32_t handle, int w, int h, int scaled_w, int scaled_h, int x, int y);

#endif
