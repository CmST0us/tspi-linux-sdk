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

#ifndef SRC_TESTS_RT_MPI_COMMON_TEST_COMM_VGS_H_
#define SRC_TESTS_RT_MPI_COMMON_TEST_COMM_VGS_H_

#include "rk_common.h"
#include "rk_comm_vgs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum TEST_VGS_OP_TYPE_E {
    VGS_OP_QUICK_COPY = 0,
    VGS_OP_QUICK_RESIZE,
    VGS_OP_QUICK_FILL,
    VGS_OP_ROTATION,
    VGS_OP_MIRROR,
    VGS_OP_COLOR_KEY
} TEST_VGS_OP_TYPE_E;

typedef struct _TEST_VGS_PROC_CTX {
    const char *srcFileName;
    const char *dstFileName;
    VIDEO_FRAME_INFO_S stImgIn;
    VIDEO_FRAME_INFO_S stImgOut;
    TEST_VGS_OP_TYPE_E stOpt;
    RK_S32 fillData;
    ROTATION_E rotateAngle;
    TEST_VGS_OP_TYPE_E opType;
    RK_S32 s32JobNum;
    RK_S32 s32TaskNum;
    RK_S32 s32ProcessTimes;
    RK_S64 s32ProcessTotalTime;
    RK_DOUBLE s32fps;
} TEST_VGS_PROC_CTX_S;

RK_S32 TEST_VGS_BeginJob(VGS_HANDLE *jobHandle);
RK_S32 TEST_VGS_AddTask(TEST_VGS_PROC_CTX_S *pstCtx, VGS_HANDLE jobHandle);
RK_S32 TEST_VGS_EndJob(VGS_HANDLE jobHandle);

RK_S32 TEST_VGS_LoadSrcFrame(TEST_VGS_PROC_CTX_S *pstCtx);
RK_S32 TEST_VGS_CreateDstFrame(TEST_VGS_PROC_CTX_S *pstCtx);
RK_S32 TEST_VGS_CreateDstCMAFrame(TEST_VGS_PROC_CTX_S *pstCtx);
RK_S32 TEST_VGS_ProcessJob(TEST_VGS_PROC_CTX_S *pstCtx, VIDEO_FRAME_INFO_S *pstFrames);
RK_S32 TEST_VGS_MultiTest(TEST_VGS_PROC_CTX_S *ctx);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // SRC_TESTS_RT_MPI_COMMON_TEST_COMM_VGS_H_
