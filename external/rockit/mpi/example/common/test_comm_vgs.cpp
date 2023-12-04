/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <pthread.h>
#include <string.h>
#include <cerrno>
#include "test_comm_vgs.h"
#include "test_comm_imgproc.h"
#include "test_comm_sys.h"
#include "test_comm_utils.h"
#include "rk_mpi_vgs.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_mmz.h"
#include "rk_mpi_sys.h"
#include "rk_common.h"
#include "rk_debug.h"

RK_S32 TEST_VGS_Save_DstFrame(TEST_VGS_PROC_CTX_S *pstCtx) {
    RK_S32 s32Ret = RK_SUCCESS;
    MB_PIC_CAL_S stCalResult;
    PIC_BUF_ATTR_S stPicBufAttr;
    RK_U32 u32DstWidth = pstCtx->stImgOut.stVFrame.u32Width;
    RK_U32 u32DstHeight = pstCtx->stImgOut.stVFrame.u32Height;
    RK_U32 u32DstPixelFormat = pstCtx->stImgOut.stVFrame.enPixelFormat;
    RK_S32 s32DstCompressMode = pstCtx->stImgOut.stVFrame.enCompressMode;
    stPicBufAttr.u32Width = u32DstWidth;
    stPicBufAttr.u32Height = u32DstHeight;
    stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)u32DstPixelFormat;
    stPicBufAttr.enCompMode = (COMPRESS_MODE_E)s32DstCompressMode;

    s32Ret = RK_MPI_CAL_VGS_GetPicBufferSize(&stPicBufAttr, &stCalResult);
    if (pstCtx->dstFileName != RK_NULL) {
        FILE *fp = fopen(pstCtx->dstFileName, "wb+");
        if (fp == RK_NULL) {
            RK_LOGE("fopen %s failed, error: %s", pstCtx->dstFileName, strerror(errno));
            return RK_FAILURE;
        }
        fwrite(RK_MPI_MB_Handle2VirAddr(pstCtx->stImgOut.stVFrame.pMbBlk),
                1, stCalResult.u32MBSize, fp);
        fflush(fp);
        fclose(fp);
    }
    return s32Ret;
}

RK_S32 TEST_VGS_AddTask(TEST_VGS_PROC_CTX_S *pstCtx, VGS_HANDLE jobHandle) {
    RK_S32 s32Ret = RK_SUCCESS;
    VGS_TASK_ATTR_S stTask;
    stTask.stImgIn = pstCtx->stImgIn;
    stTask.stImgOut = pstCtx->stImgOut;
    switch (pstCtx->opType) {
        case VGS_OP_QUICK_RESIZE: {
          s32Ret = RK_MPI_VGS_AddScaleTask(jobHandle, &stTask, VGS_SCLCOEF_NORMAL);
        } break;
        default: {
          RK_LOGE("unknown operation type %d", pstCtx->opType);
        break;
        }
    }
    return s32Ret;
}

RK_S32 TEST_VGS_BeginJob(VGS_HANDLE *jobHandle) {
    RK_S32 ret = RK_MPI_VGS_BeginJob(jobHandle);
    return ret;
}

RK_S32 TEST_VGS_EndJob(VGS_HANDLE jobHandle) {
    RK_S32 s32Ret = RK_SUCCESS;
    s32Ret = RK_MPI_VGS_EndJob(jobHandle);
    if (s32Ret != RK_SUCCESS) {
        RK_MPI_VGS_CancelJob(jobHandle);
        return RK_FAILURE;
    }
    return s32Ret;
}

RK_S32 TEST_VGS_LoadSrcFrame(TEST_VGS_PROC_CTX_S *pstCtx) {
    RK_S32 s32Ret = RK_SUCCESS;
    PIC_BUF_ATTR_S stPicBufAttr;
    VIDEO_FRAME_INFO_S *stVideoFrame = &(pstCtx->stImgIn);
    RK_U8 *pstVideoFrame;
    RK_U32 u32VirSrcWidth = 0;
    RK_U32 u32VirSrcHeight = 0;
    RK_U32 u32SrcWidth = pstCtx->stImgIn.stVFrame.u32Width;
    RK_U32 u32SrcHeight = pstCtx->stImgIn.stVFrame.u32Height;
    PIXEL_FORMAT_E u32SrcPixelFormat = pstCtx->stImgIn.stVFrame.enPixelFormat;
    COMPRESS_MODE_E u32SrcCompressMode = pstCtx->stImgIn.stVFrame.enCompressMode;

    stPicBufAttr.u32Width = u32SrcWidth;
    stPicBufAttr.u32Height = u32SrcHeight;
    stPicBufAttr.enPixelFormat = u32SrcPixelFormat;
    stPicBufAttr.enCompMode = u32SrcCompressMode;
    s32Ret = TEST_SYS_CreateVideoFrame(&stPicBufAttr, stVideoFrame);
    u32VirSrcWidth = stVideoFrame->stVFrame.u32VirWidth;
    u32VirSrcHeight = stVideoFrame->stVFrame.u32VirHeight;
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    pstVideoFrame = (RK_U8 *)RK_MPI_MB_Handle2VirAddr(stVideoFrame->stVFrame.pMbBlk);
    s32Ret = TEST_COMM_FillImage(
                pstVideoFrame,
                u32SrcWidth, u32SrcHeight,
                RK_MPI_CAL_COMM_GetHorStride(u32VirSrcWidth, u32SrcPixelFormat),
                u32VirSrcHeight,
                u32SrcPixelFormat, 1);

    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    RK_MPI_SYS_MmzFlushCache(stVideoFrame->stVFrame.pMbBlk, RK_FALSE);

__FAILED:
    if (s32Ret != RK_SUCCESS) {
        RK_MPI_MB_ReleaseMB(stVideoFrame->stVFrame.pMbBlk);
    }
    return s32Ret;
}

RK_S32 TEST_VGS_CreateDstFrame(TEST_VGS_PROC_CTX_S *pstCtx) {
    RK_S32 s32Ret = RK_SUCCESS;
    PIC_BUF_ATTR_S stPicBufAttr;
    VIDEO_FRAME_INFO_S *stVideoFrame = &(pstCtx->stImgOut);

    RK_U32 u32DstWidth = pstCtx->stImgOut.stVFrame.u32Width;
    RK_U32 u32DstHeight = pstCtx->stImgOut.stVFrame.u32Height;
    RK_U32 u32DstPixelFormat = pstCtx->stImgOut.stVFrame.enPixelFormat;
    RK_S32 s32DstCompressMode = pstCtx->stImgOut.stVFrame.enCompressMode;

    stPicBufAttr.u32Width = u32DstWidth;
    stPicBufAttr.u32Height = u32DstHeight;
    stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)u32DstPixelFormat;
    stPicBufAttr.enCompMode = (COMPRESS_MODE_E)s32DstCompressMode;

    s32Ret = TEST_SYS_CreateVideoFrame(&stPicBufAttr, stVideoFrame);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

__FAILED:
    if (s32Ret != RK_SUCCESS) {
        RK_MPI_MB_ReleaseMB(stVideoFrame->stVFrame.pMbBlk);
    }
    return s32Ret;
}

RK_S32 TEST_VGS_ProcessJob(TEST_VGS_PROC_CTX_S *pstCtx, VIDEO_FRAME_INFO_S *pstFrames) {
    RK_S32 s32Ret = RK_SUCCESS;
    RK_S64 s64StartTime = 0ll;
    RK_S64 s64EndTime = 0ll;
    VGS_HANDLE jobHandle;
    s32Ret = TEST_VGS_BeginJob(&jobHandle);
    RK_S32 s32TaskNum = (pstCtx->s32TaskNum == 0) ? 1 : pstCtx->s32TaskNum;

    for (RK_S32 taskIdx = 0; taskIdx < s32TaskNum; taskIdx++) {
        s32Ret = TEST_VGS_AddTask(pstCtx, jobHandle);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
    }

    s32Ret = TEST_VGS_EndJob(jobHandle);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

__FAILED:
    if (s32Ret != RK_SUCCESS) {
        RK_MPI_MB_ReleaseMB(pstCtx->stImgIn.stVFrame.pMbBlk);
        RK_MPI_MB_ReleaseMB(pstCtx->stImgOut.stVFrame.pMbBlk);
    }
    return s32Ret;
}

void* TEST_VGS_SingleProc(void *pArgs) {
    RK_S32 s32Ret = RK_SUCCESS;
    TEST_VGS_PROC_CTX_S *pstCtx = reinterpret_cast<TEST_VGS_PROC_CTX_S *>(pArgs);
    VIDEO_FRAME_INFO_S pstFrames;

    s32Ret = TEST_VGS_ProcessJob(pstCtx, &pstFrames);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    TEST_VGS_Save_DstFrame(pstCtx);

__FAILED:
    RK_MPI_MB_ReleaseMB(pstCtx->stImgIn.stVFrame.pMbBlk);
    RK_MPI_MB_ReleaseMB(pstCtx->stImgOut.stVFrame.pMbBlk);
    return RK_NULL;
}

void* TEST_VGS_MultiProc(void *pArgs) {
    RK_S32 s32Ret = RK_SUCCESS;
    TEST_VGS_PROC_CTX_S *pstCtx = reinterpret_cast<TEST_VGS_PROC_CTX_S *>(pArgs);
    VIDEO_FRAME_INFO_S pstFrames;

    TEST_VGS_LoadSrcFrame(pstCtx);
    TEST_VGS_CreateDstFrame(pstCtx);
    for (RK_S32 i = 0; i < pstCtx->s32ProcessTimes; i++) {
        s32Ret = TEST_VGS_ProcessJob(pstCtx, &pstFrames);
    }
    RK_MPI_MB_ReleaseMB(pstCtx->stImgIn.stVFrame.pMbBlk);
    RK_MPI_MB_ReleaseMB(pstCtx->stImgOut.stVFrame.pMbBlk);
    return RK_NULL;
}

RK_S32 TEST_VGS_MultiTest(TEST_VGS_PROC_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    pthread_t tids[VGS_MAX_JOB_NUM];
    TEST_VGS_PROC_CTX_S tmpCtx[VGS_MAX_JOB_NUM];
    RK_S64 s64StartTime = 0ll;
    RK_S64 s64EndTime = 0ll;
    s64StartTime = TEST_COMM_GetNowUs();

    for (RK_S32 jobIndex = 0; jobIndex < ctx->s32JobNum; jobIndex++) {
        memcpy(&(tmpCtx[jobIndex]), ctx, sizeof(TEST_VGS_PROC_CTX_S));
        pthread_create(&tids[jobIndex], 0, TEST_VGS_MultiProc, reinterpret_cast<void *>(&tmpCtx[jobIndex]));
    }


    for (RK_S32 jobIndex = 0; jobIndex < ctx->s32JobNum; jobIndex++) {
        void *retval;
        s32Ret = pthread_join(tids[jobIndex], &retval);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vgs multi test error test id:%d", tids[jobIndex]);
            s32Ret = RK_FAILURE;
        }
    }

    s64EndTime = TEST_COMM_GetNowUs();
    RK_S32 s32TaskNum = (ctx->s32TaskNum == 0) ? 1 : ctx->s32TaskNum;
    RK_S32 runningTimes = ctx->s32JobNum * s32TaskNum * ctx->s32ProcessTimes;
    RK_DOUBLE s32fps = (RK_DOUBLE)runningTimes * 1000000 / (s64EndTime - s64StartTime);
    RK_LOGD("run count:%d, run totalTime:%lld, run fps:%f",
            runningTimes, (s64EndTime - s64StartTime), s32fps);

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
