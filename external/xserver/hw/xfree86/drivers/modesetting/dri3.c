/*
 *  Copyright (c) 2020, Fuzhou Rockchip Electronics Co., Ltd
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

#ifdef HAVE_DIX_CONFIG_H
#include "dix-config.h"
#endif

#include <unistd.h>
#include <fcntl.h>
#include <drm_fourcc.h>

#include "xf86.h"

#include "dri3.h"
#include "driver.h"

/* Based on glamor/glamor_egl.c#glamor_dri3_open_client */
static int
ms_exa_dri3_open_client(ClientPtr client,
                        ScreenPtr screen,
                        RRProviderPtr provider,
                        int *fdp)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    int fd;
    drm_magic_t magic;

    fd = open(ms->drmmode.dri3_device_name, O_RDWR|O_CLOEXEC);
    if (fd < 0)
        return BadAlloc;

    /* Before FD passing in the X protocol with DRI3 (and increased
     * security of rendering with per-process address spaces on the
     * GPU), the kernel had to come up with a way to have the server
     * decide which clients got to access the GPU, which was done by
     * each client getting a unique (magic) number from the kernel,
     * passing it to the server, and the server then telling the
     * kernel which clients were authenticated for using the device.
     *
     * Now that we have FD passing, the server can just set up the
     * authentication on its own and hand the prepared FD off to the
     * client.
     */
    if (drmGetMagic(fd, &magic) < 0) {
        if (errno == EACCES) {
            /* Assume that we're on a render node, and the fd is
             * already as authenticated as it should be.
             */
            *fdp = fd;
            return Success;
        } else {
            close(fd);
            return BadMatch;
        }
    }

    if (drmAuthMagic(ms->drmmode.fd, magic) < 0) {
        close(fd);
        return BadMatch;
    }

    *fdp = fd;
    return Success;
}

static PixmapPtr
ms_exa_pixmap_from_fds(ScreenPtr screen,
                       CARD8 num_fds, const int *fds,
                       CARD16 width, CARD16 height,
                       const CARD32 *strides, const CARD32 *offsets,
                       CARD8 depth, CARD8 bpp,
                       uint64_t modifier)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
	PixmapPtr pixmap;
	struct dumb_bo *bo;

    if (num_fds != 1 || offsets[0] || modifier != DRM_FORMAT_MOD_INVALID)
        return NULL;

    pixmap = drmmode_create_pixmap_header(screen, width, height,
                                          depth, bpp, strides[0], NULL);
    if (!pixmap)
        return NULL;

    bo = dumb_get_bo_from_fd(ms->drmmode.fd, fds[0],
                             strides[0], strides[0] * height);
    if (!bo)
        goto err_free_pixmap;

    if (!ms_exa_set_pixmap_bo(scrn, pixmap, bo, TRUE))
        goto err_free_bo;

    return pixmap;
err_free_bo:
    dumb_bo_destroy(ms->drmmode.fd, bo);
err_free_pixmap:
    screen->DestroyPixmap(pixmap);
    return NULL;
}

static int
ms_exa_egl_fd_from_pixmap(ScreenPtr screen, PixmapPtr pixmap,
                          CARD16 *stride, CARD32 *size)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
	struct dumb_bo *bo;
	int fd;

    bo = ms_exa_bo_from_pixmap(screen, pixmap);
    if (!bo)
        return -1;

    fd = dumb_bo_get_fd(ms->drmmode.fd, bo, 0);
    *stride = bo->pitch;
	*size = bo->size;

    return fd;
}

static int
ms_exa_egl_fds_from_pixmap(ScreenPtr screen, PixmapPtr pixmap, int *fds,
                           uint32_t *strides, uint32_t *offsets,
                           uint64_t *modifier)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
	struct dumb_bo *bo;

    bo = ms_exa_bo_from_pixmap(screen, pixmap);
    if (!bo)
        return 0;

    fds[0] = dumb_bo_get_fd(ms->drmmode.fd, bo, 0);
    strides[0] = bo->pitch;
    offsets[0] = 0;
    *modifier = DRM_FORMAT_MOD_INVALID;

    return 1;
}

static Bool
ms_exa_get_formats(ScreenPtr screen,
				   CARD32 *num_formats, CARD32 **formats)
{
	/* TODO: Return formats */
    *num_formats = 0;
    return TRUE;
}

static Bool
ms_exa_get_modifiers(ScreenPtr screen, uint32_t format,
                     uint32_t *num_modifiers, uint64_t **modifiers)
{
    *num_modifiers = 0;
    return TRUE;
}

static Bool
ms_exa_get_drawable_modifiers(DrawablePtr draw, uint32_t format,
                              uint32_t *num_modifiers, uint64_t **modifiers)
{
    *num_modifiers = 0;
    return TRUE;
}

static const dri3_screen_info_rec ms_exa_dri3_info = {
    .version = 2,
    .open_client = ms_exa_dri3_open_client,
    .pixmap_from_fds = ms_exa_pixmap_from_fds,
    .fd_from_pixmap = ms_exa_egl_fd_from_pixmap,
    .fds_from_pixmap = ms_exa_egl_fds_from_pixmap,
    .get_formats = ms_exa_get_formats,
    .get_modifiers = ms_exa_get_modifiers,
    .get_drawable_modifiers = ms_exa_get_drawable_modifiers,
};

Bool
ms_exa_dri3_init(ScreenPtr screen)
{
    ScrnInfoPtr scrn = xf86ScreenToScrn(screen);
    modesettingPtr ms = modesettingPTR(scrn);
    ms->drmmode.dri3_device_name = drmGetDeviceNameFromFd2(ms->drmmode.fd);

	return dri3_screen_init(screen, &ms_exa_dri3_info);
}
