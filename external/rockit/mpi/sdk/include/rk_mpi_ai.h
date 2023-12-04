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
 */

#ifndef INCLUDE_RT_MPI_MPI_AI_H_
#define INCLUDE_RT_MPI_MPI_AI_H_

#include "rk_common.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

RK_S32 RK_MPI_AI_SetPubAttr(AUDIO_DEV AiDevId, const AIO_ATTR_S *pstAttr);
RK_S32 RK_MPI_AI_GetPubAttr(AUDIO_DEV AiDevId, AIO_ATTR_S *pstAttr);
RK_S32 RK_MPI_AI_Enable(AUDIO_DEV AiDevId);
RK_S32 RK_MPI_AI_Disable(AUDIO_DEV AiDevId);
RK_S32 RK_MPI_AI_EnableChn(AUDIO_DEV AiDevId, AI_CHN AiChn);
RK_S32 RK_MPI_AI_DisableChn(AUDIO_DEV AiDevId, AI_CHN AiChn);
RK_S32 RK_MPI_AI_GetFrame(AUDIO_DEV AiDevId, AI_CHN AiChn,
                                 AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm , RK_S32 s32MilliSec);
RK_S32 RK_MPI_AI_ReleaseFrame(AUDIO_DEV AiDevId, AI_CHN AiChn,
                                      const AUDIO_FRAME_S *pstFrm, const AEC_FRAME_S *pstAecFrm);
RK_S32 RK_MPI_AI_SetChnParam(AUDIO_DEV AiDevId, AI_CHN AiChn, const AI_CHN_PARAM_S *pstChnParam);
RK_S32 RK_MPI_AI_GetChnParam(AUDIO_DEV AiDevId, AI_CHN AiChn, AI_CHN_PARAM_S *pstChnParam);
RK_S32 RK_MPI_AI_EnableReSmp(AUDIO_DEV AiDevId, AI_CHN AiChn, AUDIO_SAMPLE_RATE_E enOutSampleRate);
RK_S32 RK_MPI_AI_DisableReSmp(AUDIO_DEV AiDevId, AI_CHN AiChn);
RK_S32 RK_MPI_AI_SetTrackMode(AUDIO_DEV AiDevId, AUDIO_TRACK_MODE_E enTrackMode);
RK_S32 RK_MPI_AI_GetTrackMode(AUDIO_DEV AiDevId, AUDIO_TRACK_MODE_E *penTrackMode);
RK_S32 RK_MPI_AI_ClrPubAttr(AUDIO_DEV AiDevId);
RK_S32 RK_MPI_AI_SaveFile(AUDIO_DEV AiDevId, AI_CHN AiChn,
                                const AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo);
RK_S32 RK_MPI_AI_QueryFileStatus(AUDIO_DEV AiDevId, AI_CHN AiChn,
                                        AUDIO_FILE_STATUS_S *pstFileStatus);
RK_S32 RK_MPI_AI_GetFd(AUDIO_DEV AiDevId, AI_CHN AiChn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  /* End of INCLUDE_RT_MPI_MPI_AI_H_ */
