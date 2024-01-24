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
 */

#ifndef SRC_TESTS_RT_MPI_COMMON_TEST_COMM_TMD_H_
#define SRC_TESTS_RT_MPI_COMMON_TEST_COMM_TMD_H_
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rk_debug.h"
#include "rk_comm_vdec.h"

typedef struct _rkStreamInfo {
    VDEC_CHN VdecChn;
    void *pFFmCtx;
    void *pExtraData;
    RK_U32 u32ExtraDataSize;
    RK_U32 u32StreamIndex;
    RK_BOOL bFindKeyFrame;
    RK_CODEC_ID_E enCodecId;
    VIDEO_MODE_E enMode;
    RK_U32 u32PicWidth;
    RK_U32 u32PicHeight;
} STREAM_INFO_S;

typedef RK_S32 (*RK_FREE_CB)(void *);

typedef struct _rkStreamData_S {
    RK_U8              *pu8VirAddr;
    RK_U64              u64Size;
    RK_FREE_CB          pFreeCB;
    RK_VOID            *pOpaque;
    RK_BOOL             bEndOfStream;
    RK_BOOL             bEndOfFrame;
    RK_U64              u64PTS;
} STREAM_DATA_S;

RK_S32 TEST_COMM_CodecIDTmediaToRK(RK_S32 s32Id);
RK_S32 TEST_COMM_TmdParserOpen(const char *uri, STREAM_INFO_S *pstStreamInfo);
RK_S32 TEST_COMM_TmdParserRead(STREAM_INFO_S *pstStreamInfo, STREAM_DATA_S *pstStreamData);
RK_S32 TEST_COMM_TmdParserClose(STREAM_INFO_S *pstStreamInfo);

#endif  // SRC_TESTS_RT_MPI_COMMON_TEST_COMM_TMD_H_

