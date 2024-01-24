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

#include "test_comm_tmd.h"

#include "rk_common.h"

#define SIZE_ARRAY_ELEMS(a)          (sizeof(a) / sizeof((a)[0]))

typedef struct _rkCodecInfo {
    RK_CODEC_ID_E enRkCodecId;
    int enTmdCodecId;
    char mine[16];
} CODEC_INFO;

static CODEC_INFO gCodecMapList[] = {
    { RK_VIDEO_ID_MPEG1VIDEO,    0,  "mpeg1" },
};

RK_S32 TEST_COMM_CodecIDTmdToRK(RK_S32 s32Id) {
    RK_BOOL bFound = RK_FALSE;
    RK_U32 i = 0;
    for (i = 0; i < SIZE_ARRAY_ELEMS(gCodecMapList); i++) {
        if (s32Id == gCodecMapList[i].enTmdCodecId) {
            bFound = RK_TRUE;
            break;
        }
    }

    if (bFound)
        return gCodecMapList[i].enRkCodecId;
    else
        return RK_VIDEO_ID_Unused;
}

static RK_S32 mpi_tmedia_free(void *opaque) {
    return 0;
}

RK_S32 TEST_COMM_TmdParserOpen(const char *uri, STREAM_INFO_S *pstStreamInfo) {  
    return RK_FAILURE;
}

RK_S32 TEST_COMM_TmdParserRead(STREAM_INFO_S *pstStreamInfo, STREAM_DATA_S *pstStreamData) {
    return RK_FAILURE;
}

RK_S32 TEST_COMM_TmdParserClose(STREAM_INFO_S *pstStreamInfo) {
    return RK_SUCCESS;
}

