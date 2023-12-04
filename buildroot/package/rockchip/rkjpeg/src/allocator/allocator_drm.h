/*
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
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
 */

#ifndef __ALLOCATOR_DRM_H__
#define __ALLOCATOR_DRM_H__

#include "mpp_err.h"
#include "vpu_type.h"
#include <sys/types.h>
#include <stdio.h>

typedef enum {
    MPP_BUFFER_TYPE_NORMAL,
    MPP_BUFFER_TYPE_ION,
    MPP_BUFFER_TYPE_EXT_DMA,
    MPP_BUFFER_TYPE_DRM,
    MPP_BUFFER_TYPE_BUTT,
} MppBufferType;
/*
VPU_MEM_INFO
RK_U32	phy_addr;
RK_U32	*vir_addr;
RK_U32	size;
RK_U32	*offset;
*/
typedef struct MppBufferInfo_t {
    MppBufferType   type;
    size_t          size;
    void            *vir_addr;
    void            *hnd;
    int             phy_addr;
    int             index;
	int				offset;
} MppBufferInfo;

typedef MPP_RET (*OsAllocatorFunc)(void *ctx, MppBufferInfo *info);

typedef struct os_allocator_t {
    MPP_RET (*open)(void **ctx, size_t alignment);
    MPP_RET (*close)(void *ctx);

    OsAllocatorFunc alloc;
    OsAllocatorFunc free;
    OsAllocatorFunc import;
    OsAllocatorFunc release;
    OsAllocatorFunc mmap;
} os_allocator;

extern os_allocator allocator_drm;


#endif
