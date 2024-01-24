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

#include <pthread.h>

#include "test_comm_tmd.h"

#include "rk_debug.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct test_vdec_thread_s {
     RK_BOOL bThreadStart;
     pthread_t VdecPid;
     STREAM_INFO_S stStreamInfo;
} TEST_VDEC_THREAD_S;

// get pic thread info
static TEST_VDEC_THREAD_S gGetPicThread[VDEC_MAX_CHN_NUM];
// send stream thread info
static TEST_VDEC_THREAD_S gSendStremThread[VDEC_MAX_CHN_NUM];

RK_S32 TEST_VDEC_Start(VDEC_CHN VdecChn,
                        VDEC_CHN_ATTR_S *pstVdecAttr,
                        VDEC_CHN_PARAM_S *pstVdecParam,
                        VIDEO_DISPLAY_MODE_E enDispMode) {
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VDEC_CreateChn(VdecChn, pstVdecAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed with %#x!", VdecChn, s32Ret);
        return s32Ret;
    }
    s32Ret = RK_MPI_VDEC_SetChnParam(VdecChn, pstVdecParam);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VDEC_SetChnParam failed with %#x!", s32Ret);
        return s32Ret;
    }
    s32Ret = RK_MPI_VDEC_StartRecvStream(VdecChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VDEC_StartRecvStream failed with %#x!", s32Ret);
        return s32Ret;
    }
    s32Ret = RK_MPI_VDEC_SetDisplayMode(VdecChn, enDispMode);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VDEC_SetDisplayMode failed with %#x!", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

RK_S32 TEST_VDEC_Stop(VDEC_CHN VdecChn) {
    RK_S32 s32Ret;

    s32Ret = RK_MPI_VDEC_StopRecvStream(VdecChn);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_VDEC_StopRecvStream failed with %#x!", s32Ret);
    }

    s32Ret = RK_MPI_VDEC_DestroyChn(VdecChn);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_VDEC_DestroyChn failed with %#x!", s32Ret);
    }

    return s32Ret;
}

static void* TEST_VDEC_SendStreamProc(void *pArgs) {
    RK_S32 s32Ret = RK_SUCCESS;
    RK_BOOL bReachEos = RK_FALSE;
    MB_BLK pStreamBlk = MB_INVALID_HANDLE;
    STREAM_DATA_S stStreamData;
    MB_EXT_CONFIG_S stMbExtConfig;
    VDEC_STREAM_S stStream;
    TEST_VDEC_THREAD_S *pstThreadInfo = (TEST_VDEC_THREAD_S *)pArgs;
    STREAM_INFO_S *pstStreamInfo = &pstThreadInfo->stStreamInfo;

    memset(&stMbExtConfig, 0, sizeof(MB_EXT_CONFIG_S));
    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    while (pstThreadInfo->bThreadStart) {
        memset(&stStreamData, 0, sizeof(STREAM_DATA_S));
        s32Ret = TEST_COMM_TmdParserRead(pstStreamInfo, &stStreamData);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("TEST_COMM_FFmParserRead failed with 0x%x!", s32Ret);
            break;
        }

        stMbExtConfig.pFreeCB = stStreamData.pFreeCB;
        stMbExtConfig.pOpaque = stStreamData.pOpaque;
        stMbExtConfig.pu8VirAddr = stStreamData.pu8VirAddr;
        stMbExtConfig.u64Size = stStreamData.u64Size;

        RK_MPI_SYS_CreateMB(&pStreamBlk, &stMbExtConfig);

        stStream.u64PTS = stStreamData.u64PTS;
        stStream.pMbBlk = pStreamBlk;
        stStream.u32Len = stStreamData.u64Size;
        stStream.bEndOfStream = stStreamData.bEndOfStream;
        stStream.bEndOfFrame = stStreamData.bEndOfFrame;
        stStream.bBypassMbBlk = RK_TRUE;
__RETRY:
        s32Ret = RK_MPI_VDEC_SendStream(pstStreamInfo->VdecChn, &stStream, 200);
        if (s32Ret != RK_SUCCESS) {
            if (!pstThreadInfo->bThreadStart) {
                break;
            }
            RK_LOGV("RK_MPI_VDEC_SendStream failed with 0x%x", s32Ret);
            goto  __RETRY;
        }
        RK_MPI_SYS_Free(stStream.pMbBlk);

        if (stStreamData.bEndOfStream) {
            RK_LOGE("reach eos");
            break;
        }
    }

    RK_LOGD("%s out\n", __FUNCTION__);
    return RK_NULL;
}

RK_S32 TEST_VDEC_StartSendStream(VDEC_CHN VdecChn, STREAM_INFO_S *pstStreamInfo) {
    RK_S32 s32Ret = 0;

    gSendStremThread[VdecChn].bThreadStart = RK_TRUE;
    memcpy(&gSendStremThread[VdecChn].stStreamInfo, pstStreamInfo, sizeof(STREAM_INFO_S));

    s32Ret = pthread_create(&(gSendStremThread[VdecChn].VdecPid), 0,
                            TEST_VDEC_SendStreamProc,
                            (RK_VOID *)&(gSendStremThread[VdecChn]));
    if (s32Ret < 0)
        return RK_FAILURE;

    return RK_SUCCESS;
}

RK_S32 TEST_VENC_StopSendFrame(VDEC_CHN VdecChn) {
    if (RK_TRUE == gSendStremThread[VdecChn].bThreadStart) {
        gSendStremThread[VdecChn].bThreadStart = RK_FALSE;
        pthread_join(gSendStremThread[VdecChn].VdecPid, 0);
    }

    return RK_SUCCESS;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
