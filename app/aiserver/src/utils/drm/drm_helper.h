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

#ifndef AI_SREVER_UTIL_DRM_H_
#define AI_SREVER_UTIL_DRM_H_

typedef long long loff32_t;

int32_t drm_open();
int32_t drm_close(int32_t fd);
int32_t drm_ioctl(int32_t fd, int32_t req, void* arg);
int32_t drm_get_phys(int fd, uint32_t handle, uint32_t *phy, uint32_t heaps);
int32_t drm_handle_to_fd(int32_t fd, uint32_t handle, int32_t *map_fd, uint32_t flags);
int32_t drm_fd_to_handle(int32_t fd, int32_t map_fd, uint32_t *handle, uint32_t flags);
int32_t drm_alloc(int32_t  fd, uint32_t len, uint32_t align, uint32_t *handle, uint32_t flags, uint32_t heaps);
int32_t drm_free(int32_t  fd, uint32_t handle);
int32_t drm_get_info_from_name(int32_t fd, uint32_t name, uint32_t *handle, int32_t *size);
int32_t drm_get_name_from_handle(int32_t fd, uint32_t handle, int32_t *name);

void   *drm_mmap(void *addr, uint32_t length, int32_t  prot, int32_t  flags, int32_t  fd, loff32_t offset);
int32_t drm_map(int32_t fd, int32_t handle, uint32_t length, int32_t prot,
                   int32_t flags, int32_t offset, void **ptr, uint32_t heaps);

#endif  // AI_SREVER_UTIL_DRM_H_
