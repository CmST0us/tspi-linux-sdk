/* Copyright 2019 Rockchip Electronics Co. LTD
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

#ifndef INCLUDE_RT_MPI_RK_MPI_MMZ_H__
#define INCLUDE_RT_MPI_RK_MPI_MMZ_H__

#include "rk_comm_mb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define RK_MMZ_ALLOC_TYPE_IOMMU     0x00000000
#define RK_MMZ_ALLOC_TYPE_CMA       0x00000001

#define RK_MMZ_ALLOC_CACHEABLE      0x00000000
#define RK_MMZ_ALLOC_UNCACHEABLE    0x00000010

#define RK_MMZ_SYNC_READONLY        0x00000000
#define RK_MMZ_SYNC_WRITEONLY       0x00000001
#define RK_MMZ_SYNC_RW              0x00000002

RK_S32 RK_MPI_MMZ_Alloc(MB_BLK *pBlk, RK_U32 u32Length, RK_U32 u32Flags);
RK_S32 RK_MPI_MMZ_Free(MB_BLK blk);

RK_U64 RK_MPI_MMZ_Handle2PhysAddr(MB_BLK blk);
RK_VOID *RK_MPI_MMZ_Handle2VirAddr(MB_BLK blk);
RK_S32 RK_MPI_MMZ_Handle2Fd(MB_BLK blk);
RK_U64 RK_MPI_MMZ_GetSize(MB_BLK blk);
MB_BLK RK_MPI_MMZ_Fd2Handle(RK_S32 u32Fd);
MB_BLK RK_MPI_MMZ_VirAddr2Handle(RK_VOID *pVirAddr);
MB_BLK RK_MPI_MMZ_PhyAddr2Handle(RK_U64 u64phyAddr);

RK_S32 RK_MPI_MMZ_IsCacheable(MB_BLK blk);
RK_S32 RK_MPI_MMZ_FlushCacheStart(MB_BLK blk, RK_U32 u32Offset, RK_U32 u32Length, RK_U32 u32Flags);
RK_S32 RK_MPI_MMZ_FlushCacheEnd(MB_BLK blk, RK_U32 u32Offset, RK_U32 u32Length, RK_U32 u32Flags);
RK_S32 RK_MPI_MMZ_FlushCacheVaddrStart(RK_VOID *pVirAddr, RK_U32 u32Length, RK_U32 u32Flags);
RK_S32 RK_MPI_MMZ_FlushCacheVaddrEnd(RK_VOID *pVirAddr, RK_U32 u32Length, RK_U32 u32Flags);
RK_S32 RK_MPI_MMZ_FlushCachePaddrStart(RK_U64 u64phyAddr, RK_U32 u32Length, RK_U32 u32Flags);
RK_S32 RK_MPI_MMZ_FlushCachePaddrEnd(RK_U64 u64phyAddr, RK_U32 u32Length, RK_U32 u32Flags);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_MPI_MMZ_H__ */
