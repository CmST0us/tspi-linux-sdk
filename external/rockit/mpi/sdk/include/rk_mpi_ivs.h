/*
 * Copyright 2022 Rockchip Electronics Co. LTD
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

#ifndef INCLUDE_RT_MPI_RK_MPI_IVS_H_
#define INCLUDE_RT_MPI_RK_MPI_IVS_H_

#include "rk_common.h"
#include "rk_comm_video.h"
#include "rk_comm_ivs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

RK_S32 RK_MPI_IVS_Init(RK_VOID);

RK_S32 RK_MPI_IVS_Exit(RK_VOID);

RK_S32 RK_MPI_IVS_CreateChn(IVS_CHN IvsChn, IVS_CHN_ATTR_S *pstAttr);

RK_S32 RK_MPI_IVS_DestroyChn(IVS_CHN IvsChn);

RK_S32 RK_MPI_IVS_SetChnAttr(IVS_CHN IvsChn, IVS_CHN_ATTR_S *pstAttr);

RK_S32 RK_MPI_IVS_GetChnAttr(IVS_CHN IvsChn, IVS_CHN_ATTR_S *pstAttr);

RK_S32 RK_MPI_IVS_SendFrame(IVS_CHN VdChn, const VIDEO_FRAME_INFO_S *pstFrame, RK_S32 s32MilliSec);

RK_S32 RK_MPI_IVS_GetResults(IVS_CHN VdChn, IVS_RESULT_INFO_S *pstResults, RK_S32 s32MilliSec);
RK_S32 RK_MPI_IVS_ReleaseResults(IVS_CHN IvsChn, IVS_RESULT_INFO_S *pstResults);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  // INCLUDE_RT_MPI_RK_MPI_IVS_H_
