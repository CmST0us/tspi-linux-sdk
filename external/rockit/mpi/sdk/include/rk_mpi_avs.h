/*
 * Copyright 2020 RockcRKp Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use tRKs file except in compliance with the License.
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

#ifndef INCLUDE_RT_MPI_RK_MPI_AVS_H_
#define INCLUDE_RT_MPI_RK_MPI_AVS_H_

#include "rk_common.h"
#include "rk_comm_video.h"
#include "rk_comm_mb.h"
#include "rk_comm_avs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Group Settings */
RK_S32 RK_MPI_AVS_CreateGrp(AVS_GRP AVSGrp, const AVS_GRP_ATTR_S *pstGrpAttr);
RK_S32 RK_MPI_AVS_DestroyGrp(AVS_GRP AVSGrp);
RK_S32 RK_MPI_AVS_StartGrp(AVS_GRP AVSGrp);
RK_S32 RK_MPI_AVS_StopGrp(AVS_GRP AVSGrp);
RK_S32 RK_MPI_AVS_ResetGrp(AVS_GRP AVSGrp);
RK_S32 RK_MPI_AVS_GetGrpAttr(AVS_GRP AVSGrp, AVS_GRP_ATTR_S *pstGrpAttr);
RK_S32 RK_MPI_AVS_SetGrpAttr(AVS_GRP AVSGrp, const AVS_GRP_ATTR_S *pstGrpAttr);

/* Pipe Settings */
RK_S32 RK_MPI_AVS_SendPipeFrame(AVS_GRP AVSGrp, AVS_PIPE AVSPipe,
                                const VIDEO_FRAME_INFO_S *pstVideoFrame, RK_S32 s32MilliSec);
RK_S32 RK_MPI_AVS_GetPipeFrame(AVS_GRP AVSGrp, AVS_PIPE AVSPipe,
                               VIDEO_FRAME_INFO_S *pstVideoFrame);
RK_S32 RK_MPI_AVS_ReleasePipeFrame(AVS_GRP AVSGrp, AVS_PIPE AVSPipe,
                                   const VIDEO_FRAME_INFO_S *pstVideoFrame);

/* Channel Settings */
RK_S32 RK_MPI_AVS_SetChnAttr(AVS_GRP AVSGrp, AVS_CHN AVSChn, const AVS_CHN_ATTR_S *pstChnAttr);
RK_S32 RK_MPI_AVS_GetChnAttr(AVS_GRP AVSGrp, AVS_CHN AVSChn, AVS_CHN_ATTR_S *pstChnAttr);
RK_S32 RK_MPI_AVS_EnableChn(AVS_GRP AVSGrp, AVS_CHN AVSChn);
RK_S32 RK_MPI_AVS_DisableChn(AVS_GRP AVSGrp, AVS_CHN AVSChn);
RK_S32 RK_MPI_AVS_GetChnFrame(AVS_GRP AVSGrp, AVS_CHN AVSChn,
                              VIDEO_FRAME_INFO_S *pstVideoFrame, RK_S32 s32MilliSec);
RK_S32 RK_MPI_AVS_ReleaseChnFrame(AVS_GRP AVSGrp, AVS_CHN AVSChn,
                                  const VIDEO_FRAME_INFO_S *pstVideoFrame);
RK_S32 RK_MPI_AVS_SetModParam(const AVS_MOD_PARAM_S *pstModParam);
RK_S32 RK_MPI_AVS_GetModParam(AVS_MOD_PARAM_S *pstModParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_MPI_AVS_H_ */
