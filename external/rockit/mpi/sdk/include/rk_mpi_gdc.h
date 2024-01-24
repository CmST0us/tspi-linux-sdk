/* GPL-2.0 WITH Linux-syscall-note OR Apache 2.0 */
/* Copyright (c) 2021 Fuzhou Rockchip Electronics Co., Ltd */

#ifndef INCLUDE_RT_MPI_GDC_H_
#define INCLUDE_RT_MPI_GDC_H_

#include "rk_common.h"
#include "rk_comm_video.h"
#include "rk_comm_gdc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /*__cplusplus*/

RK_S32 RK_MPI_GDC_BeginJob(GDC_HANDLE *phHandle);
RK_S32 RK_MPI_GDC_EndJob(GDC_HANDLE hHandle);
RK_S32 RK_MPI_GDC_CancelJob(GDC_HANDLE hHandle);
RK_S32 RK_MPI_GDC_StopJob(GDC_HANDLE hHandle);
RK_S32 RK_MPI_GDC_SetConfig(GDC_HANDLE hHandle, const FISHEYE_JOB_CONFIG_S *pstJobConfig);
RK_S32 RK_MPI_GDC_AddCorrectionTask(GDC_HANDLE hHandle,
                                        const GDC_TASK_ATTR_S *pstTask,
                                        const FISHEYE_ATTR_S *pstFisheyeAttr);
RK_S32 RK_MPI_GDC_AddCorrectionExTask(GDC_HANDLE hHandle,
                                        const GDC_TASK_ATTR_S *pstTask,
                                        const FISHEYE_ATTR_EX_S *pstFishEyeAttrrEx,
                                        RK_BOOL bCheckMode);
RK_S32 RK_MPI_GDC_AddPMFTask(GDC_HANDLE hHandle,
                                        const GDC_TASK_ATTR_S *pstTask,
                                        const GDC_PMF_ATTR_S *pstGdcPmfAttrr);
RK_S32 RK_MPI_GDC_FisheyePosQueryDst2Src(const GDC_FISHEYE_POINT_QUERY_ATTR_S *pstFisheyePointQueryAttr,
                                        const VIDEO_FRAME_INFO_S *pstVideoInfo,
                                        const POINT_S *pstDstPoint,
                                        POINT_S *pstSrcPoint);
RK_S32 RK_MPI_GDC_FisheyePosQueryDst2SrcArray(const GDC_FISHEYE_POINT_QUERY_ATTR_S *pstFisheyePointQueryAttr,
                                                const VIDEO_FRAME_INFO_S *pstVideoInfo,
                                                const RK_U32 u32PointNum,
                                                const POINT_S *pastDstPoint,
                                                POINT_S *pastSrcPoint);
RK_S32 RK_MPI_GDC_FisheyePosQueryDst2Pano(const GDC_FISHEYE_POINT_QUERY_ATTR_S *pstFisheyePointQueryAttr,
                                            const VIDEO_FRAME_INFO_S *pstVideoInfo,
                                            const RK_U32 u32PanoRegionIndex,
                                            const POINT_S *pstDstPoint,
                                            POINT_S *pstPanoPoint);
RK_S32 RK_MPI_GDC_FisheyePosQueryDst2PanoArray(const GDC_FISHEYE_POINT_QUERY_ATTR_S *pstFisheyePointQueryAttr,
                                                const VIDEO_FRAME_INFO_S *pstVideoInfo,
                                                const RK_U32 u32PanoRegionIndex,
                                                const RK_U32 u32PointNum,
                                                const POINT_S *pastDstPoint,
                                                POINT_S *pastPanoPoint);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*__cplusplus*/

#endif /*end of __MPI_GDC_H__*/
