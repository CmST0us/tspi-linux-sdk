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

#ifndef INCLUDE_RT_MPI_MPI_VI_H_
#define INCLUDE_RT_MPI_MPI_VI_H_

#include "rk_comm_vi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* 1 for vi device */
RK_S32 RK_MPI_VI_SetDevAttr(VI_DEV ViDev, const VI_DEV_ATTR_S *pstDevAttr);
RK_S32 RK_MPI_VI_GetDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstDevAttr);
RK_S32 RK_MPI_VI_EnableDev(VI_DEV ViDev);
RK_S32 RK_MPI_VI_DisableDev(VI_DEV ViDev);
RK_S32 RK_MPI_VI_SetDevBindPipe(VI_DEV ViDev, const VI_DEV_BIND_PIPE_S *pstDevBindPipe);
RK_S32 RK_MPI_VI_GetDevBindPipe(VI_DEV ViDev, VI_DEV_BIND_PIPE_S *pstDevBindPipe);
RK_S32 RK_MPI_VI_GetDevIsEnable(VI_DEV ViDev);

/* 2 for vi chn */
RK_S32 RK_MPI_VI_SetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, const VI_CHN_ATTR_S *pstChnAttr);
RK_S32 RK_MPI_VI_GetChnAttr(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_ATTR_S *pstChnAttr);

RK_S32 RK_MPI_VI_EnableChn(VI_PIPE ViPipe, VI_CHN ViChn);
RK_S32 RK_MPI_VI_DisableChn(VI_PIPE ViPipe, VI_CHN ViChn);
RK_S32 RK_MPI_VI_PauseChn(VI_PIPE ViPipe, VI_CHN ViChn);
RK_S32 RK_MPI_VI_ResumeChn(VI_PIPE ViPipe, VI_CHN ViChn);

RK_S32 RK_MPI_VI_GetChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, VIDEO_FRAME_INFO_S *pstFrameInfo, RK_S32 s32MilliSec);
RK_S32 RK_MPI_VI_ReleaseChnFrame(VI_PIPE ViPipe, VI_CHN ViChn, const VIDEO_FRAME_INFO_S *pstFrameInfo);
RK_S32 RK_MPI_VI_ChnSaveFile(VI_PIPE ViPipe, VI_CHN ViChn, VI_SAVE_FILE_INFO_S* pstSaveFileInfo);
RK_S32 RK_MPI_VI_QueryChnStatus(VI_PIPE ViPipe, VI_CHN ViChn, VI_CHN_STATUS_S *pstChnStatus);
RK_S32 RK_MPI_VI_GetChnFd(VI_PIPE ViPipe, VI_CHN ViChn);
RK_S32 RK_MPI_VI_CloseChnFd(VI_PIPE ViPipe, VI_CHN ViChn);

RK_S32 RK_MPI_VI_SetChnFreeze(VI_PIPE ViPipe, VI_CHN ViChn, RK_BOOL bFreeze);
RK_S32 RK_MPI_VI_GetChnFreeze(VI_PIPE ViPipe, VI_CHN ViChn, RK_BOOL *pbFreeze);

RK_S32 RK_MPI_VI_SetUserPic(VI_PIPE ViPipe, VI_CHN ViChn, const VI_USERPIC_ATTR_S *pstUsrPic);
RK_S32 RK_MPI_VI_EnableUserPic(VI_PIPE ViPipe, VI_CHN ViChn);
RK_S32 RK_MPI_VI_DisableUserPic(VI_PIPE ViPipe, VI_CHN ViChn);

RK_S32 RK_MPI_VI_GetChnConnectInfo(VI_PIPE ViPipe, VI_CHN ViChn, VI_CONNECT_INFO_S *pstConnectInfo);
RK_S32 RK_MPI_VI_SetChnEdid(VI_PIPE ViPipe, VI_CHN ViChn, const VI_EDID_S *pstEdid);
RK_S32 RK_MPI_VI_GetChnEdid(VI_PIPE ViPipe, VI_CHN ViChn, VI_EDID_S *pstEdid);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_MPI_VI_H_ */


