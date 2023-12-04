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

#ifndef INCLUDE_RT_MPI_RK_MPI_MB_H__
#define INCLUDE_RT_MPI_RK_MPI_MB_H__

#include "rk_comm_mb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

MB_POOL  RK_MPI_MB_CreatePool(MB_POOL_CONFIG_S *pstMbPoolCfg);
RK_S32   RK_MPI_MB_DestroyPool(MB_POOL pool);

MB_BLK   RK_MPI_MB_GetMB(MB_POOL pool, RK_U64 u64Size, RK_BOOL block);
RK_S32   RK_MPI_MB_ReleaseMB(MB_BLK mb);

RK_U64   RK_MPI_MB_Handle2PhysAddr(MB_BLK mb);
RK_VOID *RK_MPI_MB_Handle2VirAddr(MB_BLK mb);
RK_S32   RK_MPI_MB_Handle2Fd(MB_BLK mb);
MB_POOL  RK_MPI_MB_Handle2PoolId(MB_BLK mb);
RK_S32   RK_MPI_MB_Handle2UniqueId(MB_BLK mb);
RK_U64   RK_MPI_MB_GetSize(MB_BLK mb);
RK_U64   RK_MPI_MB_GetLength(MB_BLK mb);
RK_U32   RK_MPI_MB_GetOffset(MB_BLK mb);
RK_S32   RK_MPI_MB_SetOffset(MB_BLK mb, RK_U32 u32Offset);

MB_BLK   RK_MPI_MB_VirAddr2Handle(RK_VOID *pstVirAddr);
RK_S32   RK_MPI_MB_UniqueId2Fd(RK_S32 s32UniqueId);

RK_S32   RK_MPI_MB_InquireUserCnt(MB_BLK mb);

RK_S32   RK_MPI_MB_SetModPoolConfig(MB_UID_E enMbUid, const MB_CONFIG_S *pstMbConfig);
RK_S32   RK_MPI_MB_GetModPoolConfig(MB_UID_E enMbUid, MB_CONFIG_S *pstMbConfig);

RK_S32   RK_MPI_MB_Init();
RK_S32   RK_MPI_MB_Exit();
RK_S32   RK_MPI_MB_SetConfig(const MB_CONFIG_S *pstMbConfig);
RK_S32   RK_MPI_MB_GetConfig(MB_CONFIG_S *pstMbConfig);

RK_S32   RK_MPI_MB_SetBufferStride(MB_BLK mb, RK_U32 u32HorStride, RK_U32 u32VerStride);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_MPI_MB_H__ */
