/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
 * This is api of audio filter
 */

#ifndef INCLUDE_RT_MPI_MPI_AF_H_
#define INCLUDE_RT_MPI_MPI_AF_H_

#include <vector>
#include <string>

#include "rk_type.h"
#include "rk_common.h"
#include "rk_comm_af.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

RK_S32 RK_MPI_AF_Create(AF_CHN AfChn, const AF_ATTR_S *pstAttr);
RK_S32 RK_MPI_AF_Destroy(AF_CHN AfChn);
RK_S32 RK_MPI_AF_SendFrame(AF_CHN AfChn, AUDIO_FRAME_INFO_S *frame, RK_S32 s32MilliSec);
RK_S32 RK_MPI_AF_GetFrame(AF_CHN AfChn, AUDIO_FRAME_INFO_S *frame, RK_S32 s32MilliSec);
RK_S32 RK_MPI_AF_ReleaseFrame(AF_CHN AfChn, AUDIO_FRAME_INFO_S *frame);
RK_S32 RK_MPI_AF_SetChnAttr(AF_CHN AfChn, const AF_ATTR_S *pstAttr);
RK_S32 RK_MPI_AF_GetChnAttr(AF_CHN AfChn, AF_ATTR_S *pstAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // INCLUDE_RT_MPI_MPI_AF_H_