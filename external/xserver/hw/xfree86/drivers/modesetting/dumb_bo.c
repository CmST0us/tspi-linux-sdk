/*
 * Copyright © 2007 Red Hat, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Dave Airlie <airlied@redhat.com>
 *
 */

#ifdef HAVE_DIX_CONFIG_H
#include "dix-config.h"
#endif

#include "dumb_bo.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xf86drm.h>

struct dumb_bo *
dumb_bo_create(int fd,
               const unsigned width, const unsigned height, const unsigned bpp)
{
    struct drm_mode_create_dumb arg;
    struct dumb_bo *bo;
    int ret;

    bo = calloc(1, sizeof(*bo));
    if (!bo)
        return NULL;

    memset(&arg, 0, sizeof(arg));
    arg.width = width;
    arg.height = height;
    arg.bpp = bpp;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &arg);
    if (ret)
        goto err_free;

    bo->handle = arg.handle;
    bo->size = arg.size;
    bo->pitch = arg.pitch;

    return bo;
 err_free:
    free(bo);
    return NULL;
}

int
dumb_bo_map(int fd, struct dumb_bo *bo)
{
    struct drm_mode_map_dumb arg;
    int ret;
    void *map;

    if (bo->ptr) {
        return 0;
    }

    memset(&arg, 0, sizeof(arg));
    arg.handle = bo->handle;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &arg);
    if (ret)
        return ret;

    map = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, arg.offset);
    if (map == MAP_FAILED)
        return -errno;

    bo->ptr = map;
    return 0;
}

int
dumb_bo_get_fd(int fd, struct dumb_bo *bo, uint32_t flags)
{
    struct drm_prime_handle args;
    int ret;

    memset(&args, 0, sizeof(args));
    args.fd = -1;
    args.handle = bo->handle;
    args.flags = flags;

    ret = drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &args);
    if (ret)
        return ret;

    return args.fd;
}

/* From glamor_get_flink_name() */
int
dumb_bo_get_name(int fd, struct dumb_bo *bo)
{
    struct drm_gem_flink flink;

    flink.handle = bo->handle;
    if (ioctl(fd, DRM_IOCTL_GEM_FLINK, &flink) < 0) {
        /*
         * Assume non-GEM kernels have names identical to the handle
         */
        if (errno == ENODEV)
            return bo->handle;

        return -1;
    }
    return flink.name;
}

int
dumb_bo_destroy(int fd, struct dumb_bo *bo)
{
    struct drm_mode_destroy_dumb arg;
    int ret;

    if (bo->ptr) {
        munmap(bo->ptr, bo->size);
        bo->ptr = NULL;
    }

    memset(&arg, 0, sizeof(arg));
    arg.handle = bo->handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &arg);
    if (ret)
        return -errno;

    free(bo);
    return 0;
}

struct dumb_bo *
dumb_get_bo_from_handle(int fd, int handle, int pitch, int size)
{
    struct dumb_bo *bo;

    bo = calloc(1, sizeof(*bo));
    if (!bo)
        return NULL;

    bo->handle = handle;
    bo->pitch = pitch;
    bo->size = size;
    return bo;
}

struct dumb_bo *
dumb_get_bo_from_fd(int fd, int dmafd, int pitch, int size)
{
    unsigned int handle;
    int ret;

    ret = drmPrimeFDToHandle(fd, dmafd, &handle);
    if (ret)
        return NULL;

    return dumb_get_bo_from_handle(fd, handle, pitch, size);
}
