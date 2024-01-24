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

#ifndef INCLUDE_RT_MPI_MPI_COMM_AF_H_
#define INCLUDE_RT_MPI_MPI_COMM_AF_H_

#include "rk_type.h"
#include "rk_comm_aio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum rkAUDIO_FILTER_TYPE_E {
    AUDIO_FILTER_RESAMPLE = 0,
    AUDIO_FILTER_3A       = 1,
    AUDIO_FILTER_BUTT
} AUDIO_FILTER_TYPE_E;

typedef struct rkAF_RESAMPLE_ATTR_S {
    RK_U32 u32InRate;
    RK_U32 u32InChn;
    AUDIO_BIT_WIDTH_E   enInBitWidth;

    RK_U32 u32OutRate;
    RK_U32 u32OutChn;
    AUDIO_BIT_WIDTH_E   enOutBitWidth;
} AF_RESAMPLE_ATTR_S;

typedef struct rkAF_3A_ATTR_S {
    char   cfgPath[MAX_AUDIO_FILE_PATH_LEN];
    RK_U32 u32SampleRate;
    RK_U32 u32Channels;
    AUDIO_BIT_WIDTH_E enBitWidth;
    RK_U32 u32ChnLayout;
    RK_U32 u32RefLayout;
    RK_U32 u32RecLayout;
} AF_3A_ATTR_S;

typedef struct rkAF_ATTR_S {
    AUDIO_FILTER_TYPE_E enType;
    RK_U32 u32InBufCount;   /* max buffer count for input cache */
    RK_U32 u32OutBufCount;  /* max buffer count for output cache */
    union {
        AF_RESAMPLE_ATTR_S stResAttr;
        AF_3A_ATTR_S       st3AAttr;
    };
} AF_ATTR_S;

/* at lease one parameter is illagal ,eg, an illegal enumeration value  */
#define RK_ERR_AF_INVALID_CHNID     RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_CHNID)
#define RK_ERR_AF_EXIST             RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_EXIST)
#define RK_ERR_AF_UNEXIST           RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)
#define RK_ERR_AF_ILLEGAL_PARAM     RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
#define RK_ERR_AF_NULL_PTR          RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
#define RK_ERR_AF_BUF_FULL          RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_FULL)
#define RK_ERR_AF_NOBUF             RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_NOBUF)
#define RK_ERR_AF_SYS_NOTREADY      RK_DEF_ERR(RK_ID_AF, RK_ERR_LEVEL_ERROR, RK_ERR_NOTREADY)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif  // INCLUDE_RT_MPI_MPI_COMM_AF_H_
