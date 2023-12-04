/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include "drm.h"
#include "uvc_log.h"

#define DRM_DEVICE "/dev/dri/card0"

int drm_open(void)
{
    int fd;
    fd = open(DRM_DEVICE, O_RDWR);
    if (fd < 0)
    {
        LOG_ERROR("open %s failed!\n", DRM_DEVICE);
        return -1;
    }
    return fd;
}

void drm_close(int fd)
{
    if (fd >= 0)
        close(fd);
}

static int drm_ioctl(int fd, int req, void *arg)
{
    int ret;

    do
    {
        ret = ioctl(fd, req, arg);
    }
    while (ret == -1 && (errno == EINTR || errno == EAGAIN));

    return ret;
}

int drm_alloc(int fd, size_t len, size_t align, unsigned int *handle, unsigned int flags)
{
    int ret;
    struct drm_mode_create_dumb dmcb;

    memset(&dmcb, 0, sizeof(struct drm_mode_create_dumb));
    dmcb.bpp = 8;
    dmcb.width = (len + align - 1) & (~(align - 1));
    dmcb.height = 1;
    dmcb.flags = flags;

    if (handle == NULL)
        return -EINVAL;

    ret = drm_ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &dmcb);
    if (ret < 0)
        return ret;

    *handle = dmcb.handle;

    return ret;
}

int drm_free(int fd, unsigned int handle)
{
    struct drm_mode_destroy_dumb data =
    {
        .handle = handle,
    };
    return drm_ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &data);
}

void *drm_map_buffer(int fd, unsigned int handle, size_t len)
{
    struct drm_mode_map_dumb dmmd;
    void *buf = NULL;
    int ret;

    memset(&dmmd, 0, sizeof(dmmd));
    dmmd.handle = handle;

    ret = drm_ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &dmmd);
    if (ret)
    {
        LOG_ERROR("map_dumb failed: %s\n", strerror(ret));
        return NULL;
    }

    buf = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, dmmd.offset);
    if (buf == MAP_FAILED)
    {
        LOG_ERROR("mmap failed: %s\n", strerror(errno));
        return NULL;
    }

    return buf;
}

void drm_unmap_buffer(void *buf, size_t len)
{
    if (buf)
        munmap(buf, len);
}

int drm_handle_to_fd(int fd, unsigned int handle, int *map_fd, unsigned int flags)
{
    int ret;
    struct drm_prime_handle dph;

    memset(&dph, 0, sizeof(struct drm_prime_handle));
    dph.handle = handle;
    dph.fd = -1;
    dph.flags = flags;

    if (map_fd == NULL)
        return -EINVAL;

    ret = drm_ioctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &dph);
    if (ret < 0)
        return ret;

    *map_fd = dph.fd;

    if (*map_fd < 0)
    {
        LOG_ERROR("map ioctl returned negative fd\n");
        return -EINVAL;
    }

    return ret;
}


int drm_get_info_from_name(
    int   fd,
    unsigned int   name,
    unsigned int  *handle,
    int  *size)
{
    int  ret = 0;
    struct drm_gem_open req;

    req.name = name;
    ret = drm_ioctl(fd, DRM_IOCTL_GEM_OPEN, &req);
    if (ret < 0)
    {
        return ret;
    }

    *handle = req.handle;
    *size   = (int)req.size;

    return ret;
}

