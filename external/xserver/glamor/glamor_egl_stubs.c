/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file glamor_egl_stubs.c
 *
 * Stubbed out glamor_egl.c functions for servers other than Xorg.
 */

#include "dix-config.h"

#include "glamor.h"

#ifdef GLAMOR_HAS_GBM_MAP
#include "gbm.h"
#endif

void
glamor_egl_screen_init(ScreenPtr screen, struct glamor_context *glamor_ctx)
{
}

int
glamor_egl_fd_name_from_pixmap(ScreenPtr screen,
                               PixmapPtr pixmap,
                               CARD16 *stride, CARD32 *size)
{
    return -1;
}


int
glamor_egl_fds_from_pixmap(ScreenPtr screen, PixmapPtr pixmap, int *fds,
                           uint32_t *offsets, uint32_t *strides,
                           uint64_t *modifier)
{
    return 0;
}

int
glamor_egl_fd_from_pixmap(ScreenPtr screen, PixmapPtr pixmap,
                          CARD16 *stride, CARD32 *size)
{
    return -1;
}

struct gbm_bo *
glamor_gbm_bo_from_pixmap(ScreenPtr screen, PixmapPtr pixmap)
{
    return NULL;
}

#ifdef GLAMOR_HAS_GBM_MAP
__attribute__((weak)) void *
gbm_bo_map(struct gbm_bo *bo,
           uint32_t x, uint32_t y, uint32_t width, uint32_t height,
           uint32_t flags, uint32_t *stride, void **mp_data)
{
    return NULL;
}
#endif
