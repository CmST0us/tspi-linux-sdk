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

#ifdef __cplusplus
extern "C"
{
#endif

enum drm_rockchip_gem_mem_type {
    /* Physically Continuous memory and used as default. */
    ROCKCHIP_BO_CONTIG   = 1 << 0,
    /* cachable mapping. */
    ROCKCHIP_BO_CACHABLE = 1 << 1,
    /* write-combine mapping. */
    ROCKCHIP_BO_WC       = 1 << 2,
    ROCKCHIP_BO_SECURE   = 1 << 3,
    ROCKCHIP_BO_MASK     = ROCKCHIP_BO_CONTIG | ROCKCHIP_BO_CACHABLE | ROCKCHIP_BO_WC
};

int drm_open(void);
void drm_close(int fd);
int drm_alloc(int fd, size_t len, size_t align, unsigned int *handle, unsigned int flags);
int drm_free(int fd, unsigned int handle);
void *drm_map_buffer(int fd, unsigned int handle, size_t len);
void drm_unmap_buffer(void *buf, size_t len);
int drm_handle_to_fd(int fd, unsigned int handle, int *map_fd, unsigned int flags);
int drm_get_info_from_name(int fd, unsigned int name, unsigned int *handle, int *size);


#ifdef __cplusplus
}
#endif
