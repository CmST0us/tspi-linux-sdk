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

#ifndef INCLUDE_RT_MPI_MPI_AENC_H_
#define INCLUDE_RT_MPI_MPI_AENC_H_

#include "rk_common.h"
#include "rk_comm_aenc.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

RK_S32 RK_MPI_AENC_CreateChn(AENC_CHN AeChn, const AENC_CHN_ATTR_S *pstAttr);
RK_S32 RK_MPI_AENC_DestroyChn(AENC_CHN AeChn);
RK_S32 RK_MPI_AENC_SendFrame(AENC_CHN AeChn, const AUDIO_FRAME_S *pstFrm,
                                const AEC_FRAME_S *pstAecFrm, RK_S32 s32MilliSec);
RK_S32 RK_MPI_AENC_GetStream(AENC_CHN AeChn, AUDIO_STREAM_S *pstStream, RK_S32 s32MilliSec);
RK_S32 RK_MPI_AENC_ReleaseStream(AENC_CHN AeChn, const AUDIO_STREAM_S *pstStream);
RK_S32 RK_MPI_AENC_SaveFile(AENC_CHN AeChn, const AUDIO_SAVE_FILE_INFO_S *pstSaveFileInfo);
RK_S32 RK_MPI_AENC_QueryFileStatus(AENC_CHN AeChn, AUDIO_FILE_STATUS_S* pstFileStatus);
RK_S32 RK_MPI_AENC_RegisterEncoder(RK_S32 *ps32Handle, const AENC_ENCODER_S *pstEncoder);
RK_S32 RK_MPI_AENC_UnRegisterEncoder(RK_S32 s32Handle);
RK_S32 RK_MPI_AENC_GetFd(AENC_CHN AeChn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  /* End of INCLUDE_RT_MPI_MPI_AENC_H_ */
