/* GPL-2.0 WITH Linux-syscall-note OR Apache 2.0 */
/* Copyright (c) 2022 Fuzhou Rockchip Electronics Co., Ltd */

#ifndef INCLUDE_RT_MPI_RK_MPI_PVS_H__
#define INCLUDE_RT_MPI_RK_MPI_PVS_H__

#include "rk_common.h"
#include "rk_comm_video.h"
#include "rk_comm_pvs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

RK_S32 RK_MPI_PVS_EnableDev(PVS_DEV PvsDevId);
RK_S32 RK_MPI_PVS_DisableDev(PVS_DEV PvsDevId);

RK_S32 RK_MPI_PVS_EnableChn(PVS_DEV PvsDevId, PVS_CHN PvsChnId);
RK_S32 RK_MPI_PVS_DisableChn(PVS_DEV PvsDevId, PVS_CHN PvsChnId);

RK_S32 RK_MPI_PVS_SetDevAttr(PVS_DEV PvsDevId, const PVS_DEV_ATTR_S *pstDevAttr);
RK_S32 RK_MPI_PVS_GetDevAttr(PVS_DEV PvsDevId, PVS_DEV_ATTR_S *pstDevAttr);

RK_S32 RK_MPI_PVS_SetChnAttr(PVS_DEV PvsDevId, PVS_CHN PvsChnId, PVS_CHN_ATTR_S *pstChnAttr);
RK_S32 RK_MPI_PVS_GetChnAttr(PVS_DEV PvsDevId, PVS_CHN PvsChnId, PVS_CHN_ATTR_S *pstChnAttr);

RK_S32 RK_MPI_PVS_SetChnParam(PVS_DEV PvsDevId, PVS_CHN PvsChnId, PVS_CHN_PARAM_S *pstChnParam);
RK_S32 RK_MPI_PVS_GetChnParam(PVS_DEV PvsDevId, PVS_CHN PvsChnId, PVS_CHN_PARAM_S *pstChnParam);

RK_S32 RK_MPI_PVS_SendFrame(PVS_DEV PvsDevId, PVS_CHN PvsChnId, const VIDEO_FRAME_INFO_S *pstFrameInfo);
RK_S32 RK_MPI_PVS_GetFrame(PVS_DEV PvsDevId, VIDEO_FRAME_INFO_S *pstFrameInfo, RK_S32 s32MilliSec);
RK_S32 RK_MPI_PVS_ReleaseFrame(const VIDEO_FRAME_INFO_S *pstFrameInfo);

RK_S32 RK_MPI_PVS_SetVProcDev(PVS_DEV PvsDevId, VIDEO_PROC_DEV_TYPE_E enVProcDev);
RK_S32 RK_MPI_PVS_GetVProcDev(PVS_DEV PvsDevId, VIDEO_PROC_DEV_TYPE_E *enVProcDev);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_MPI_PVS_H__ */
