/*
 * Copyright 2019 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <cstdio>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "drm.h"
#include "drm_mode.h"
#include "drm_helper.h"
#include "logger/log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "drm_helper"

static const char *DRM_DEV_NAME = "/dev/dri/card0";

int32_t drm_open() {
    int32_t fd = open(DRM_DEV_NAME, O_RDWR);
    if (fd < 0) {
        LOG_ERROR("fail to open drm device(%s)\n", DRM_DEV_NAME);
    }

    return fd;
}

int32_t drm_close(int32_t fd) {
    int32_t ret = close(fd);
    if (ret < 0) {
        return -errno;
    }

    return ret;
}

int32_t drm_get_phys(int32_t fd, uint32_t  handle, uint32_t  *phy, uint32_t  heaps) {
    /* no physical address */
    if (heaps != ROCKCHIP_BO_SECURE || heaps != ROCKCHIP_BO_CONTIG) {
        *phy = 0;
        return 0;
    }

    struct drm_rockchip_gem_phys phys_arg;
    phys_arg.handle = handle;
    int32_t ret = drm_ioctl(fd, DRM_IOCTL_ROCKCHIP_GEM_GET_PHYS, &phys_arg);
    if (ret < 0) {
        LOG_ERROR("fail to get phys(fd = %d), error: %s\n", fd, strerror(errno));
        return ret;
    } else {
        *phy = phys_arg.phy_addr;
    }
    return ret;
}

int32_t drm_ioctl(int32_t fd, int32_t req, void* arg) {
    int32_t ret = ioctl(fd, req, arg);
    if (ret < 0) {
        LOG_ERROR("fail to drm_ioctl(fd = %d, req =%d), error: %s\n", fd, req, strerror(errno));
        return -errno;
    }
    return ret;
}

int32_t drm_handle_to_fd(int32_t fd, uint32_t  handle, int32_t *map_fd, uint32_t flags) {
    int32_t ret;
    struct drm_prime_handle dph;
    memset(&dph, 0, sizeof(struct drm_prime_handle));
    dph.handle = handle;
    dph.fd = -1;
    dph.flags = flags;

    if (map_fd == NULL)
        return -EINVAL;

    ret = drm_ioctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &dph);
    if (ret < 0) {
        return ret;
    }

    *map_fd = dph.fd;

    if (*map_fd < 0) {
        LOG_ERROR("fail to handle_to_fd(fd=%d)\n", fd);
        return -EINVAL;
    }

    return ret;
}

int32_t drm_fd_to_handle(
        int32_t fd,
        int32_t map_fd,
        uint32_t  *handle,
        uint32_t  flags) {
    int32_t ret;
    struct drm_prime_handle dph;

    dph.fd = map_fd;
    dph.flags = flags;

    ret = drm_ioctl(fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &dph);
    if (ret < 0) {
        return ret;
    }

    *handle = dph.handle;
    return ret;
}

int32_t drm_get_info_from_name(
        int32_t   fd,
        uint32_t   name,
        uint32_t  *handle,
        int32_t  *size) {
    int32_t  ret = 0;
    struct drm_gem_open req;

    req.name = name;
    ret = drm_ioctl(fd, DRM_IOCTL_GEM_OPEN, &req);
    if (ret < 0) {
        return ret;
    }

    *handle = req.handle;
    *size   = (int32_t)req.size;

    return ret;
}

int32_t drm_get_name_from_handle(int32_t fd, uint32_t handle, int32_t *name) {
    struct drm_gem_flink req;
    int32_t ret = 0;

    req.handle = handle,
    ret = drm_ioctl(fd, DRM_IOCTL_GEM_FLINK, &req);

    *name = req.name;
    return ret;
}

int32_t drm_alloc(
        int32_t fd,
        uint32_t  len,
        uint32_t  align,
        uint32_t  *handle,
        uint32_t  flags,
        uint32_t  heaps) {
    (void)flags;
    int32_t ret;
    struct drm_mode_create_dumb dmcb;

    memset(&dmcb, 0, sizeof(struct drm_mode_create_dumb));
    dmcb.bpp = 8;
    dmcb.width = (len + align - 1) & (~(align - 1));
    dmcb.height = 1;
    dmcb.flags = heaps;

    if (handle == NULL) {
        LOG_ERROR("illegal parameter, handler is null\n");
        return -EINVAL;
    }

    ret = drm_ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &dmcb);
    if (ret < 0) {
        return ret;
    }

    *handle = dmcb.handle;
    return ret;
}

int32_t drm_free(int32_t fd, uint32_t  handle) {
    struct drm_mode_destroy_dumb data = {
        .handle = handle,
    };
    return drm_ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &data);
}

#if 1
#include <errno.h> /* for EINVAL */

inline void *drm_mmap(void *addr, uint32_t  length, int32_t prot, int32_t flags,
                             int32_t fd, loff32_t offset) {
    /* offset must be aligned to 4096 (not necessarily the page size) */
    if (offset & 4095) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    return mmap64(addr, length, prot, flags, fd, offset);
}

#define drm_munmap(addr, length) \
              munmap(addr, length)

#else

/* assume large file support exists */
#  define drm_mmap(addr, length, prot, flags, fd, offset) \
              mmap(addr, length, prot, flags, fd, offset)

#  define drm_munmap(addr, length) \
              munmap(addr, length)

#endif

int32_t drm_map(int32_t fd, int32_t handle, uint32_t  length, int32_t prot,
                   int32_t flags, int32_t offset, void **ptr, uint32_t  heaps) {
    int32_t ret;
    static uint32_t  pagesize_mask = 0;
    (void)offset;

    if (fd <= 0)
        return -EINVAL;
    if (ptr == NULL)
        return -EINVAL;

    if (!pagesize_mask)
        pagesize_mask = sysconf(_SC_PAGESIZE) - 1;

    length = (length + pagesize_mask) & ~pagesize_mask;

    struct drm_mode_map_dumb dmmd;

    memset(&dmmd, 0, sizeof(dmmd));
    dmmd.handle = (uint32_t )handle;

    ret = drm_ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &dmmd);
    if (ret) {
        LOG_ERROR("map_dumb failed: %s\n", strerror(ret));
        return ret;
    }

    *ptr = drm_mmap(NULL, length, prot, flags, fd, dmmd.offset);
    if (*ptr == MAP_FAILED) {
        if (heaps == ROCKCHIP_BO_SECURE) {
            LOG_ERROR("fail to drm_mmap(fd = %d), without physical address\n", fd);
            ret = 0;
        } else {
            LOG_ERROR("fail to drm_mmap(fd = %d), error: %s\n", fd, strerror(errno));
            ret = -errno;
        }
    }

    return ret;
}
