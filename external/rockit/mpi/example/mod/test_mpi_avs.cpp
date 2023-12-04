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
#undef DBG_MOD_ID
#define DBG_MOD_ID       RK_ID_AVS

#include <stdio.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <random>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_mmz.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_vpss.h"
#include "rk_mpi_avs.h"
#include "rk_mpi_vo.h"

#include "test_common.h"
#include "test_comm_argparse.h"
#include "test_comm_vo.h"
#include "test_comm_sys.h"
#include "test_comm_utils.h"

/* for RK3588 */
#define RK3588_VO_DEV_HDMI         0
#define RK3588_VO_DEV_MIPI         3

#define MAX_FILE_NAME_LEN          64
#define MAX_FILE_PATH_LEN          256

#define AVS_GET_CHN_FRAME_TIMEOUT_MS 200
#define VO_RGA 0
#define ENABLE_COM_POOL 0

static RK_BOOL bExit = RK_FALSE;

typedef struct rkVI_CFG_S {
    const RK_CHAR *dstFilePath;
    VI_DEV              s32DevId;
    VI_CHN              s32ChnId;
    VI_PIPE             s32PipeId;
    RK_U32              u32ViChnCnt;
    RK_U32              u32ViPipeCnt;
    RK_S32              s32SelectFd;
    RK_BOOL             bFreeze;
    VI_DEV_ATTR_S       stViDevAttr;
    VI_CHN_ATTR_S       stViChnAttr;
    VI_CHN_STATUS_S     stViChnStatus;
    VI_DEV_BIND_PIPE_S  stBindPipe;
    VI_SAVE_FILE_INFO_S stDebugFile;
    VIDEO_FRAME_INFO_S  stViFrame;
} VI_CFG_S;

typedef struct rkAVS_CFG_S {
    AVS_GRP         s32GrpId;
    AVS_CHN         s32ChnId;
    AVS_PIPE        s32PipeId;
    RK_U32          u32AvsPipeCnt;
    RK_U32          u32AvsChnCnt;
    RK_U32          u32InW;
    RK_U32          u32InH;
    RK_U32          u32OutW;
    RK_U32          u32OutH;
    AVS_MOD_PARAM_S stAvsModParam;
    AVS_GRP_ATTR_S  stAvsGrpAttr;
    AVS_OUTPUT_ATTR_S stAvsOutAttr;
    AVS_CHN_ATTR_S  stAvsChnAttr[AVS_MAX_CHN_NUM];
} AVS_CFG_S;

typedef struct _rkVO_CFG_S {
    VO_DEV                s32DevId;
    VO_CHN                s32ChnId;
    VO_LAYER              s32LayerId;
    VO_VIDEO_LAYER_ATTR_S stVoLayerAttr;
    VO_CHN_ATTR_S         stVoChnAttr;
    VO_CSC_S              stVoCscAttr;
} VO_CFG_S;

typedef enum rkTEST_MODE_E {
    TEST_MODE_AVS_BLEND       = 0,
    TEST_MODE_AVS_NOBLEND     = 1,
    TEST_MODE_VI_AVS_VO       = 2
} TEST_MODE_E;

typedef enum rkPARAMS_SOURCES_E {
    PARAMS_SOURCES_MESH       = 0,
    PARAMS_SOURCES_CALIB      = 1
} PARAMS_SOURCES_E;

typedef struct _rkTEST_AVS_CTX_S {
    char             srcFilePath[MAX_FILE_PATH_LEN];
    char             dstFilePath[MAX_FILE_PATH_LEN];
    FILE            *srcFp[AVS_PIPE_NUM];
    FILE            *dstSaveFp;

    AVS_CFG_S        avsContext;
    TEST_MODE_E      enTestMode;
    PARAMS_SOURCES_E enParamsSources;
    COMPRESS_MODE_E  enCompressMode;
    RK_S32           s32FrameSync;
    RK_S32           s32LoopCount;
    RK_U32           enVoDevType;    /*RK3588: 0: HDMI, 3: MIPI*/

    MB_CONFIG_S      stMbConfig;
} TEST_AVS_CTX_S;

static RK_S32 destroy_vo(VO_CFG_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VO_LAYER VoLayer = ctx->s32LayerId;
    VO_DEV VoDev = ctx->s32DevId;
    VO_CHN VoChn = ctx->s32ChnId;

    RK_MPI_VO_DisableLayer(VoLayer);
    RK_MPI_VO_DisableChn(VoDev, VoChn);
    RK_MPI_VO_Disable(VoDev);

    return s32Ret;
}

static RK_S32 create_vo(VO_CFG_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    RK_U32 u32DispBufLen;
    VO_PUB_ATTR_S VoPubAttr;
    VO_LAYER VoLayer = ctx->s32LayerId;
    VO_DEV VoDev = ctx->s32DevId;
    VO_CHN VoChn = ctx->s32ChnId;

    memset(&VoPubAttr, 0, sizeof(VO_PUB_ATTR_S));

    s32Ret = RK_MPI_VO_GetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    if (RK3588_VO_DEV_HDMI == VoDev) {
        VoPubAttr.enIntfType = VO_INTF_HDMI;
        VoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    } else if (RK3588_VO_DEV_MIPI == VoDev) {
        VoPubAttr.enIntfType = VO_INTF_MIPI;
        VoPubAttr.enIntfSync = VO_OUTPUT_DEFAULT;
    }

    s32Ret = RK_MPI_VO_SetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_Enable(VoDev);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_GetLayerDispBufLen(VoLayer, &u32DispBufLen);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Get display buf len failed: %d!", s32Ret);
        return s32Ret;
    }

    u32DispBufLen = 3;
    s32Ret = RK_MPI_VO_SetLayerDispBufLen(VoLayer, u32DispBufLen);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Set display buf len %d failed: %d!", u32DispBufLen, s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_BindLayer(VoLayer, VoDev, VO_LAYER_MODE_GRAPHIC);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_BindLayer failed: %#x", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, &ctx->stVoLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerAttr failed: %#x", s32Ret);
        return RK_FAILURE;
    }

#if VO_RGA
    s32Ret = RK_MPI_VO_SetLayerSpliceMode(VoLayer, VO_SPLICE_MODE_RGA);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerSpliceMode failed: %#x", s32Ret);
        return RK_FAILURE;
    }
#endif

    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_EnableLayer failed: %#x", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_SetChnAttr(VoLayer, VoChn, &ctx->stVoChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetChnAttr failed: %#x", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_SetLayerCSC(VoLayer, &ctx->stVoCscAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerCSC failed: %#x", s32Ret);
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_EnableChn(VoLayer, VoChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_EnableChn failed: %#x", s32Ret);
        return RK_FAILURE;
    }

    return s32Ret;
}

static RK_S32 create_vi(VI_CFG_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;

    /* 0. get dev config status */
    s32Ret = RK_MPI_VI_GetDevAttr(ctx->s32DevId, &ctx->stViDevAttr);
    if (s32Ret == RK_ERR_VI_NOT_CONFIG) {
        /* 0-1.config dev */
        s32Ret = RK_MPI_VI_SetDevAttr(ctx->s32DevId, &ctx->stViDevAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevAttr %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_SetDevAttr already");
    }
    /* 1.get  dev enable status */
    s32Ret = RK_MPI_VI_GetDevIsEnable(ctx->s32DevId);
    if (s32Ret != RK_SUCCESS) {
        /* 1-2.enable dev */
        s32Ret = RK_MPI_VI_EnableDev(ctx->s32DevId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_EnableDev %x", s32Ret);
            goto __FAILED;
        }
        /* 1-3.bind dev/pipe */
        ctx->stBindPipe.u32Num = ctx->s32PipeId;
        ctx->stBindPipe.PipeId[0] = ctx->s32PipeId;
        s32Ret = RK_MPI_VI_SetDevBindPipe(ctx->s32DevId, &ctx->stBindPipe);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevBindPipe %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_EnableDev already");
    }
    /* 2.config channel */
    s32Ret = RK_MPI_VI_SetChnAttr(ctx->s32PipeId, ctx->s32ChnId, &ctx->stViChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_SetChnAttr %x", s32Ret);
        goto __FAILED;
    }
    /* 3.enable channel */
    s32Ret = RK_MPI_VI_EnableChn(ctx->s32PipeId, ctx->s32ChnId);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_EnableChn %x", s32Ret);
        goto __FAILED;
    }
    /* 4.save debug file */
    if (ctx->stDebugFile.bCfg) {
        s32Ret = RK_MPI_VI_ChnSaveFile(ctx->s32PipeId, ctx->s32ChnId, &ctx->stDebugFile);
        RK_LOGD("RK_MPI_VI_ChnSaveFile %x", s32Ret);
    }

__FAILED:
    return s32Ret;
}

static RK_S32 destroy_vi(VI_CFG_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    s32Ret = RK_MPI_VI_DisableChn(ctx->s32PipeId, ctx->s32ChnId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_VI_DisableChn failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = RK_MPI_VI_DisableDev(ctx->s32DevId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_VI_DisableDev failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

static RK_S32 create_avs(AVS_CFG_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_AVS_SetModParam(&ctx->stAvsModParam);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("AVS set mod params failed with %#x!", s32Ret);
        goto __FAILED;
    }

    s32Ret = RK_MPI_AVS_CreateGrp(ctx->s32GrpId, &ctx->stAvsGrpAttr);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("AVS creat grp failed with %#x!", s32Ret);
        goto __FAILED;
    }

    s32Ret = RK_MPI_AVS_SetChnAttr(ctx->s32GrpId, ctx->s32ChnId, &ctx->stAvsChnAttr[0]);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("AVS set chn attr failed with %#x!", s32Ret);
        goto __FAILED;
    }

    s32Ret = RK_MPI_AVS_EnableChn(ctx->s32GrpId, ctx->s32ChnId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("AVS enable chn failed with %#x!", s32Ret);
        goto __FAILED;
    }

    s32Ret = RK_MPI_AVS_StartGrp(ctx->s32GrpId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("AVS start grp failed with %#x!", s32Ret);
        goto __FAILED;
    }

__FAILED:
    return s32Ret;
}

static RK_S32 destroy_avs(AVS_CFG_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;

    s32Ret = RK_MPI_AVS_DisableChn(ctx->s32GrpId, ctx->s32ChnId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_AVS_DisableChn failed with %#x!\n", s32Ret);
        goto __FAILED;
    }

    s32Ret = RK_MPI_AVS_StopGrp(ctx->s32GrpId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_AVS_StopGrp failed with %#x!\n", s32Ret);
        goto __FAILED;
    }

    s32Ret = RK_MPI_AVS_DestroyGrp(ctx->s32GrpId);
    if (RK_SUCCESS != s32Ret) {
        RK_LOGE("RK_MPI_AVS_DestroyGrp failed with %#x!\n", s32Ret);
        goto __FAILED;
    }

__FAILED:
    return s32Ret;
}

static RK_S32 TEST_AVS_ComCreateFrame(TEST_AVS_CTX_S *pstCtx, VIDEO_FRAME_INFO_S **pstVideoFrames) {
    RK_S32 s32Ret = RK_SUCCESS;
    PIC_BUF_ATTR_S stPicBufAttr;
    RK_CHAR cWritePath[MAX_FILE_PATH_LEN] = {0};
    RK_S32 i = 0;

    stPicBufAttr.u32Width = pstCtx->avsContext.u32InW;
    stPicBufAttr.u32Height = pstCtx->avsContext.u32InH;
    stPicBufAttr.enCompMode = pstCtx->enCompressMode;
    stPicBufAttr.enPixelFormat = RK_FMT_YUV420SP;
    for (; i < pstCtx->avsContext.u32AvsPipeCnt; i++) {
        s32Ret = TEST_SYS_CreateVideoFrame(&stPicBufAttr, pstVideoFrames[i]);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        if (pstCtx->srcFilePath != RK_NULL) {
            if (pstCtx->enCompressMode == COMPRESS_MODE_NONE) {
                snprintf(cWritePath, sizeof(cWritePath),
                        "%simage_data/camera%d_%dx%d_nv12.yuv",
                        pstCtx->srcFilePath, i, pstCtx->avsContext.u32InW, pstCtx->avsContext.u32InH);
            } else if (pstCtx->enCompressMode == COMPRESS_AFBC_16x16) {
                snprintf(cWritePath, sizeof(cWritePath),
                        "%safbc_image_data/camera%d_%dx%d_nv12_afbc.yuv",
                        pstCtx->srcFilePath, i, pstCtx->avsContext.u32InW, pstCtx->avsContext.u32InH);
            }
            RK_LOGD("fread %s!", cWritePath);
            s32Ret = TEST_COMM_FileReadOneFrame(cWritePath, pstVideoFrames[i]);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        }
        RK_MPI_SYS_MmzFlushCache(pstVideoFrames[i]->stVFrame.pMbBlk, RK_FALSE);
    }

    return s32Ret;

__FAILED:
    for (; i >= 0; i--) {
        RK_MPI_MB_ReleaseMB(pstVideoFrames[i]->stVFrame.pMbBlk);
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_ComSendFrame(TEST_AVS_CTX_S *pstCtx, VIDEO_FRAME_INFO_S **pstVideoFrames) {
    RK_S32 s32Ret = RK_SUCCESS;
    RK_S32 i = 0;

    for (; i < pstCtx->avsContext.u32AvsPipeCnt; i++) {
        s32Ret = RK_MPI_AVS_SendPipeFrame(pstCtx->avsContext.s32GrpId, i, pstVideoFrames[i], -1);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
    }

    return s32Ret;

__FAILED:
    for (; i >= 0; i--) {
        RK_MPI_MB_ReleaseMB(pstVideoFrames[i]->stVFrame.pMbBlk);
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_ComGetChnFrame(TEST_AVS_CTX_S *pstCtx, VIDEO_FRAME_INFO_S **pstVideoFrames) {
    RK_S32 s32Ret = RK_SUCCESS;

    for (RK_S32 i = 0; i < pstCtx->avsContext.u32AvsChnCnt; i++) {
        s32Ret = RK_MPI_AVS_GetChnFrame(pstCtx->avsContext.s32GrpId, i,
                                        pstVideoFrames[i], AVS_GET_CHN_FRAME_TIMEOUT_MS);
        if (s32Ret != RK_SUCCESS) {
            return RK_SUCCESS;
        }
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_ComChnSetZoom(TEST_AVS_CTX_S *pstCtx) {
    RK_S32 s32Ret = RK_SUCCESS;
    AVS_CHN_ATTR_S stChnAttr;
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<RK_U32> multiple(1, 10);

    s32Ret = RK_MPI_AVS_GetChnAttr(pstCtx->avsContext.s32GrpId, 0, &stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_AVS_GetChnAttr failed with %#x!", s32Ret);
        return s32Ret;
    }

    stChnAttr.u32Width = pstCtx->avsContext.u32OutW / 10 * multiple(rd);
    stChnAttr.u32Height = pstCtx->avsContext.u32OutH / 10 * multiple(rd);
    RK_LOGI("set random u32Width: %d, u32Height: %d", stChnAttr.u32Width, stChnAttr.u32Height);

    s32Ret = RK_MPI_AVS_SetChnAttr(pstCtx->avsContext.s32GrpId, 0, &stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_AVS_SetChnAttr failed with %#x!");
        return s32Ret;
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_8_Equirectangular(TEST_AVS_CTX_S *ctx) {
    RK_S32     s32Ret = RK_FAILURE;
    AVS_CFG_S *pstAvsCtx = reinterpret_cast<AVS_CFG_S *>(&(ctx->avsContext));
    const RK_CHAR *cTestDataPath = "/data/avs/8x_equirectangular/";
    RK_CHAR cWritePath[128] = {0};
    VIDEO_FRAME_INFO_S **stPipeFrameInfos;
    VIDEO_FRAME_INFO_S **stChnFrameInfos;

    snprintf(ctx->srcFilePath, sizeof(ctx->srcFilePath),
             "%s%s", cTestDataPath, "input_image/");
    snprintf(ctx->dstFilePath, sizeof(ctx->dstFilePath),
             "%s%s", cTestDataPath, "output_res/");

    /* avs config init */
    pstAvsCtx->s32GrpId      = 0;
    pstAvsCtx->s32ChnId      = 0;
    pstAvsCtx->u32AvsPipeCnt = 8;
    pstAvsCtx->u32AvsChnCnt  = 1;
    pstAvsCtx->u32InW        = 2560;
    pstAvsCtx->u32InH        = 1520;
    pstAvsCtx->u32OutW       = 8192;
    pstAvsCtx->u32OutH       = 3840;

    snprintf(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath,
             sizeof(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath),
             "%s%s", cTestDataPath, "avs_calib/calib_file.pto");
    snprintf(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aMeshAlphaPath,
             sizeof(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath),
             "%s%s", cTestDataPath, "avs_mesh/");

    pstAvsCtx->stAvsGrpAttr.stLUT.enAccuracy                 = AVS_LUT_ACCURACY_HIGH;
    pstAvsCtx->stAvsModParam.u32WorkingSetSize               = 67 * 1024;
    pstAvsCtx->stAvsModParam.enMBSource                      = MB_SOURCE_PRIVATE;
    pstAvsCtx->stAvsGrpAttr.enMode                           = AVS_MODE_BLEND;
    pstAvsCtx->stAvsGrpAttr.u32PipeNum                       = pstAvsCtx->u32AvsPipeCnt;
    pstAvsCtx->stAvsGrpAttr.stGainAttr.enMode                = AVS_GAIN_MODE_AUTO;

    pstAvsCtx->stAvsGrpAttr.stOutAttr.enPrjMode              = AVS_PROJECTION_EQUIRECTANGULAR;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stFOV.u32FOVX          = 36000;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stFOV.u32FOVY          = 8500;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stCenter.s32X          = 4096;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stCenter.s32Y          = 1800;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Roll  = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Pitch = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Yaw   = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Roll     = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Pitch    = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Yaw      = 0;

    pstAvsCtx->stAvsGrpAttr.bSyncPipe                     = (RK_BOOL)ctx->s32FrameSync;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32SrcFrameRate   = -1;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32DstFrameRate   = -1;

    pstAvsCtx->stAvsChnAttr[0].enCompressMode              = ctx->enCompressMode;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].u32Depth                    = 3;
    pstAvsCtx->stAvsChnAttr[0].u32Width                    = pstAvsCtx->u32OutW;
    pstAvsCtx->stAvsChnAttr[0].u32Height                   = pstAvsCtx->u32OutH;
    pstAvsCtx->stAvsChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;

#if ENABLE_COM_POOL
    pstAvsCtx->stAvsModParam.enMBSource = MB_SOURCE_COMMON;
    PIC_BUF_ATTR_S stBufAttr;
    MB_PIC_CAL_S stPicCalResult;
    memset(&stBufAttr, 0, sizeof(PIC_BUF_ATTR_S));
    memset(&stPicCalResult, 0, sizeof(MB_PIC_CAL_S));

    stBufAttr.u32Width = pstAvsCtx->stAvsChnAttr[0].u32Width;
    stBufAttr.u32Height = pstAvsCtx->stAvsChnAttr[0].u32Width;
    stBufAttr.enPixelFormat = RK_FMT_YUV420SP;
    stBufAttr.enCompMode = pstAvsCtx->stAvsChnAttr[0].enCompressMode;
    s32Ret = RK_MPI_CAL_VGS_GetPicBufferSize(&stBufAttr, &stPicCalResult);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    ctx->stMbConfig.astCommPool[0].u64MBSize = stPicCalResult.u32MBSize;
    ctx->stMbConfig.astCommPool[0].u32MBCnt = 8;
    ctx->stMbConfig.astCommPool[0].enRemapMode = MB_REMAP_MODE_CACHED;
    ctx->stMbConfig.astCommPool[0].enAllocType = MB_ALLOC_TYPE_DMA;
    ctx->stMbConfig.astCommPool[0].enDmaType = MB_DMA_TYPE_NONE;

    s32Ret = RK_MPI_MB_SetConfig(&ctx->stMbConfig);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_MB_Init();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
#endif

    /* avs create */
    s32Ret = create_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create avs [%d, %d, %d] failed %x",
                pstAvsCtx->s32GrpId,
                pstAvsCtx->s32PipeId,
                pstAvsCtx->s32ChnId,
                s32Ret);
        return s32Ret;
    }


    stPipeFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_PIPE_NUM));
    stChnFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_MAX_CHN_NUM));
    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        stPipeFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stPipeFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }
    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++) {
        stChnFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stChnFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }

    s32Ret = TEST_AVS_ComCreateFrame(ctx, stPipeFrameInfos);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    for (RK_S32 loopNum = 0; loopNum < ctx->s32LoopCount; loopNum++) {
        s32Ret = TEST_AVS_ComSendFrame(ctx, stPipeFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        s32Ret = TEST_AVS_ComGetChnFrame(ctx, stChnFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        for (RK_S32 i = 0; i < ctx->avsContext.u32AvsChnCnt; i++) {
            if (!stChnFrameInfos[i]->stVFrame.pMbBlk) {
                continue;
            }
            if (ctx->dstFilePath) {
                snprintf(cWritePath, sizeof(cWritePath), "%schn_out_%dx%d_%d_%d_%s.bin",
                            ctx->dstFilePath, stChnFrameInfos[i]->stVFrame.u32VirWidth,
                            stChnFrameInfos[i]->stVFrame.u32VirHeight, ctx->avsContext.s32GrpId, i,
                            ctx->enCompressMode ? "nv12_afbc": "nv12");

                s32Ret = TEST_COMM_FileWriteOneFrame(cWritePath, stChnFrameInfos[i]);
                if (s32Ret != RK_SUCCESS) {
                    goto __FAILED;
                }
            }
            s32Ret = RK_MPI_AVS_ReleaseChnFrame(pstAvsCtx->s32GrpId, i, stChnFrameInfos[i]);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("release failed grp[%d] chn[%d] frame %p",
                        pstAvsCtx->s32GrpId, i,
                        stChnFrameInfos[i]->stVFrame.pMbBlk);
                    goto __FAILED;
            }
        }
    }

__FAILED:
    for (RK_S32 i = 0; i < ctx->avsContext.u32AvsPipeCnt; i++) {
        RK_MPI_MB_ReleaseMB(stPipeFrameInfos[i]->stVFrame.pMbBlk);
    }

    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        RK_SAFE_FREE(stPipeFrameInfos[i]);
    }

    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++)
        RK_SAFE_FREE(stChnFrameInfos[i]);

    RK_SAFE_FREE(stPipeFrameInfos);
    RK_SAFE_FREE(stChnFrameInfos);

    /* avs destroy */
    s32Ret = destroy_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

#if ENABLE_COM_POOL
    s32Ret = RK_MPI_MB_Exit();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
#endif

    return s32Ret;
}

static RK_S32 TEST_AVS_6_Rectilinear(TEST_AVS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    AVS_CFG_S *pstAvsCtx = reinterpret_cast<AVS_CFG_S *>(&(ctx->avsContext));
    const RK_CHAR *cTestDataPath = "/data/avs/6x_rectlinear/";
    RK_CHAR  cWritePath[128] = {0};
    VIDEO_FRAME_INFO_S **stPipeFrameInfos;
    VIDEO_FRAME_INFO_S **stChnFrameInfos;

    snprintf(ctx->srcFilePath, sizeof(ctx->srcFilePath),
             "%s%s", cTestDataPath, "input_image/");
    snprintf(ctx->dstFilePath, sizeof(ctx->dstFilePath),
             "%s%s", cTestDataPath, "output_res/");

    /* avs config init */
    pstAvsCtx->s32GrpId      = 0;
    pstAvsCtx->s32ChnId      = 0;
    pstAvsCtx->u32AvsPipeCnt = 6;
    pstAvsCtx->u32AvsChnCnt  = 1;
    pstAvsCtx->u32InW        = 2560;
    pstAvsCtx->u32InH        = 1520;
    pstAvsCtx->u32OutW       = 8192;
    pstAvsCtx->u32OutH       = 2700;

    snprintf(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath,
             sizeof(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath),
             "%s%s", cTestDataPath, "avs_calib/calib_file.pto");
    snprintf(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aMeshAlphaPath,
            sizeof(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath),
            "%s%s", cTestDataPath, "avs_mesh/");

    pstAvsCtx->stAvsGrpAttr.stLUT.enAccuracy                 = AVS_LUT_ACCURACY_HIGH;
    pstAvsCtx->stAvsModParam.u32WorkingSetSize               = 67 * 1024;
    pstAvsCtx->stAvsModParam.enMBSource                      = MB_SOURCE_PRIVATE;
    pstAvsCtx->stAvsGrpAttr.enMode                           = AVS_MODE_BLEND;
    pstAvsCtx->stAvsGrpAttr.u32PipeNum                       = pstAvsCtx->u32AvsPipeCnt;
    pstAvsCtx->stAvsGrpAttr.stGainAttr.enMode                = AVS_GAIN_MODE_AUTO;

    pstAvsCtx->stAvsGrpAttr.stOutAttr.enPrjMode              = AVS_PROJECTION_RECTILINEAR;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stCenter.s32X          = 4220;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stCenter.s32Y          = 2124;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stFOV.u32FOVX          = 28000;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stFOV.u32FOVY          = 9500;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Roll  = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Pitch = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Yaw   = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Roll     = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Pitch    = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Yaw      = 0;

    pstAvsCtx->stAvsGrpAttr.bSyncPipe                     = (RK_BOOL)ctx->s32FrameSync;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32SrcFrameRate   = -1;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32DstFrameRate   = -1;

    pstAvsCtx->stAvsChnAttr[0].enCompressMode              = ctx->enCompressMode;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].u32Depth                    = 3;
    pstAvsCtx->stAvsChnAttr[0].u32Width                    = pstAvsCtx->u32OutW;
    pstAvsCtx->stAvsChnAttr[0].u32Height                   = pstAvsCtx->u32OutH;
    pstAvsCtx->stAvsChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;

    /* avs create */
    s32Ret = create_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create avs [%d, %d, %d] failed %x",
                pstAvsCtx->s32GrpId,
                pstAvsCtx->s32PipeId,
                pstAvsCtx->s32ChnId,
                s32Ret);
        return s32Ret;
    }

    stPipeFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_PIPE_NUM));
    stChnFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_MAX_CHN_NUM));
    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        stPipeFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stPipeFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }
    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++) {
        stChnFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stChnFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }

    s32Ret = TEST_AVS_ComCreateFrame(ctx, stPipeFrameInfos);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    for (RK_S32 loopNum = 0; loopNum < ctx->s32LoopCount; loopNum++) {
        s32Ret = TEST_AVS_ComSendFrame(ctx, stPipeFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        s32Ret = TEST_AVS_ComGetChnFrame(ctx, stChnFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        for (RK_S32 i = 0; i < ctx->avsContext.u32AvsChnCnt; i++) {
            if (!stChnFrameInfos[i]->stVFrame.pMbBlk) {
                continue;
            }
            if (ctx->dstFilePath) {
                snprintf(cWritePath, sizeof(cWritePath), "%schn_out_%dx%d_%d_%d_%s.bin",
                            ctx->dstFilePath, stChnFrameInfos[i]->stVFrame.u32VirWidth,
                            stChnFrameInfos[i]->stVFrame.u32VirHeight, ctx->avsContext.s32GrpId, i,
                            ctx->enCompressMode ? "nv12_afbc": "nv12");

                s32Ret = TEST_COMM_FileWriteOneFrame(cWritePath, stChnFrameInfos[i]);
                if (s32Ret != RK_SUCCESS) {
                    goto __FAILED;
                }
            }
            s32Ret = RK_MPI_AVS_ReleaseChnFrame(pstAvsCtx->s32GrpId, i, stChnFrameInfos[i]);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("release failed grp[%d] chn[%d] frame %p",
                        pstAvsCtx->s32GrpId, i,
                        stChnFrameInfos[i]->stVFrame.pMbBlk);
                    goto __FAILED;
            }
        }
    }

__FAILED:
    for (RK_S32 i = 0; i < ctx->avsContext.u32AvsPipeCnt; i++) {
        RK_MPI_MB_ReleaseMB(stPipeFrameInfos[i]->stVFrame.pMbBlk);
    }

    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        RK_SAFE_FREE(stPipeFrameInfos[i]);
    }

    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++)
        RK_SAFE_FREE(stChnFrameInfos[i]);

    RK_SAFE_FREE(stPipeFrameInfos);
    RK_SAFE_FREE(stChnFrameInfos);

    /* avs destroy */
    s32Ret = destroy_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_4_Equirectangular_Trans(TEST_AVS_CTX_S *ctx) {
    RK_LOGE("Test data is not prepared yet!");
    return RK_SUCCESS;
}

static RK_S32 TEST_AVS_2_Cylindrical(TEST_AVS_CTX_S *ctx) {
    RK_LOGE("Test data is not prepared yet!");
    return RK_SUCCESS;
}

static RK_S32 TEST_AVS_4_NoBlend_Qr(TEST_AVS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    AVS_CFG_S *pstAvsCtx = reinterpret_cast<AVS_CFG_S *>(&(ctx->avsContext));
    const RK_CHAR *cTestDataPath = "/data/avs/4x_qr/";
    RK_CHAR    cWritePath[128] = {0};
    VIDEO_FRAME_INFO_S **stPipeFrameInfos;
    VIDEO_FRAME_INFO_S **stChnFrameInfos;

    snprintf(ctx->srcFilePath, sizeof(ctx->srcFilePath), "%s%s",
             cTestDataPath, "input_image/");
    snprintf(ctx->dstFilePath, sizeof(ctx->dstFilePath), "%s%s",
             cTestDataPath, "output_res/");

    /* avs config init */
    pstAvsCtx->s32GrpId      = 0;
    pstAvsCtx->s32ChnId      = 0;
    pstAvsCtx->u32AvsPipeCnt = 4;
    pstAvsCtx->u32AvsChnCnt  = 1;
    pstAvsCtx->u32InW        = 2560;
    pstAvsCtx->u32InH        = 1520;

    pstAvsCtx->stAvsModParam.u32WorkingSetSize             = 67 * 1024;
    pstAvsCtx->stAvsModParam.enMBSource                    = MB_SOURCE_PRIVATE;
    pstAvsCtx->stAvsGrpAttr.enMode                         = AVS_MODE_NOBLEND_QR;
    pstAvsCtx->stAvsGrpAttr.u32PipeNum                     = pstAvsCtx->u32AvsPipeCnt;
    pstAvsCtx->stAvsGrpAttr.bSyncPipe                      = (RK_BOOL)ctx->s32FrameSync;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    pstAvsCtx->stAvsChnAttr[0].enCompressMode              = ctx->enCompressMode;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].u32Depth                    = 3;
    pstAvsCtx->stAvsChnAttr[0].u32Width                    = 0;
    pstAvsCtx->stAvsChnAttr[0].u32Height                   = 0;
    pstAvsCtx->stAvsChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;

    /* avs create */
    s32Ret = create_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create avs [%d, %d, %d] failed %x",
                pstAvsCtx->s32GrpId,
                pstAvsCtx->s32PipeId,
                pstAvsCtx->s32ChnId,
                s32Ret);
        return s32Ret;
    }

    stPipeFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_PIPE_NUM));
    stChnFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_MAX_CHN_NUM));
    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        stPipeFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stPipeFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }
    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++) {
        stChnFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stChnFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }

    s32Ret = TEST_AVS_ComCreateFrame(ctx, stPipeFrameInfos);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    for (RK_S32 loopNum = 0; loopNum < ctx->s32LoopCount; loopNum++) {
        s32Ret = TEST_AVS_ComSendFrame(ctx, stPipeFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        s32Ret = TEST_AVS_ComGetChnFrame(ctx, stChnFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        for (RK_S32 i = 0; i < ctx->avsContext.u32AvsChnCnt; i++) {
            if (!stChnFrameInfos[i]->stVFrame.pMbBlk) {
                continue;
            }
            if (ctx->dstFilePath) {
                snprintf(cWritePath, sizeof(cWritePath), "%schn_out_%dx%d_%d_%d_%s.bin",
                            ctx->dstFilePath, stChnFrameInfos[i]->stVFrame.u32VirWidth,
                            stChnFrameInfos[i]->stVFrame.u32VirHeight, ctx->avsContext.s32GrpId, i,
                            ctx->enCompressMode ? "nv12_afbc": "nv12");

                s32Ret = TEST_COMM_FileWriteOneFrame(cWritePath, stChnFrameInfos[i]);
                if (s32Ret != RK_SUCCESS) {
                    goto __FAILED;
                }
            }
            s32Ret = RK_MPI_AVS_ReleaseChnFrame(pstAvsCtx->s32GrpId, i, stChnFrameInfos[i]);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("release failed grp[%d] chn[%d] frame %p",
                        pstAvsCtx->s32GrpId, i,
                        stChnFrameInfos[i]->stVFrame.pMbBlk);
                    goto __FAILED;
            }
        }
    }

__FAILED:
    for (RK_S32 i = 0; i < ctx->avsContext.u32AvsPipeCnt; i++) {
        RK_MPI_MB_ReleaseMB(stPipeFrameInfos[i]->stVFrame.pMbBlk);
    }

    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        RK_SAFE_FREE(stPipeFrameInfos[i]);
    }

    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++)
        RK_SAFE_FREE(stChnFrameInfos[i]);

    RK_SAFE_FREE(stPipeFrameInfos);
    RK_SAFE_FREE(stChnFrameInfos);

    /* avs destroy */
    s32Ret = destroy_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_6_NoBlend_Hor(TEST_AVS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    AVS_CFG_S *pstAvsCtx = reinterpret_cast<AVS_CFG_S *>(&(ctx->avsContext));
    const RK_CHAR *cTestDataPath = "/data/avs/6x_hor_ver/";
    RK_CHAR    cWritePath[128] = {0};
    VIDEO_FRAME_INFO_S **stPipeFrameInfos;
    VIDEO_FRAME_INFO_S **stChnFrameInfos;

    snprintf(ctx->srcFilePath, sizeof(ctx->srcFilePath), "%s%s",
             cTestDataPath, "input_image/");
    snprintf(ctx->dstFilePath, sizeof(ctx->dstFilePath), "%s%s",
             cTestDataPath, "output_res/");

    /* avs config init */
    pstAvsCtx->s32GrpId      = 0;
    pstAvsCtx->s32ChnId      = 0;
    pstAvsCtx->u32AvsPipeCnt = 6;
    pstAvsCtx->u32AvsChnCnt  = 1;
    pstAvsCtx->u32InW        = 2560;
    pstAvsCtx->u32InH        = 1520;

    pstAvsCtx->stAvsModParam.u32WorkingSetSize             = 67 * 1024;
    pstAvsCtx->stAvsModParam.enMBSource                    = MB_SOURCE_PRIVATE;
    pstAvsCtx->stAvsGrpAttr.enMode                         = AVS_MODE_NOBLEND_HOR;
    pstAvsCtx->stAvsGrpAttr.u32PipeNum                     = pstAvsCtx->u32AvsPipeCnt;
    pstAvsCtx->stAvsGrpAttr.bSyncPipe                      = (RK_BOOL)ctx->s32FrameSync;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    pstAvsCtx->stAvsChnAttr[0].enCompressMode              = ctx->enCompressMode;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].u32Depth                    = 3;
    pstAvsCtx->stAvsChnAttr[0].u32Width                    = 0;
    pstAvsCtx->stAvsChnAttr[0].u32Height                   = 0;
    pstAvsCtx->stAvsChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;

    /* avs create */
    s32Ret = create_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create avs [%d, %d, %d] failed %x",
                pstAvsCtx->s32GrpId,
                pstAvsCtx->s32PipeId,
                pstAvsCtx->s32ChnId,
                s32Ret);
        return s32Ret;
    }

    stPipeFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_PIPE_NUM));
    stChnFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_MAX_CHN_NUM));
    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        stPipeFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stPipeFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }
    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++) {
        stChnFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stChnFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }

    s32Ret = TEST_AVS_ComCreateFrame(ctx, stPipeFrameInfos);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    for (RK_S32 loopNum = 0; loopNum < ctx->s32LoopCount; loopNum++) {
        s32Ret = TEST_AVS_ComSendFrame(ctx, stPipeFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        s32Ret = TEST_AVS_ComGetChnFrame(ctx, stChnFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        for (RK_S32 i = 0; i < ctx->avsContext.u32AvsChnCnt; i++) {
            if (!stChnFrameInfos[i]->stVFrame.pMbBlk) {
                continue;
            }
            if (ctx->dstFilePath) {
                snprintf(cWritePath, sizeof(cWritePath), "%schn_out_%dx%d_%d_%d_%s.bin",
                            ctx->dstFilePath, stChnFrameInfos[i]->stVFrame.u32VirWidth,
                            stChnFrameInfos[i]->stVFrame.u32VirHeight, ctx->avsContext.s32GrpId, i,
                            ctx->enCompressMode ? "nv12_afbc": "nv12");

                s32Ret = TEST_COMM_FileWriteOneFrame(cWritePath, stChnFrameInfos[i]);
                if (s32Ret != RK_SUCCESS) {
                    goto __FAILED;
                }
            }
            s32Ret = RK_MPI_AVS_ReleaseChnFrame(pstAvsCtx->s32GrpId, i, stChnFrameInfos[i]);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("release failed grp[%d] chn[%d] frame %p",
                        pstAvsCtx->s32GrpId, i,
                        stChnFrameInfos[i]->stVFrame.pMbBlk);
                    goto __FAILED;
            }
        }
    }

__FAILED:
    for (RK_S32 i = 0; i < ctx->avsContext.u32AvsPipeCnt; i++) {
        RK_MPI_MB_ReleaseMB(stPipeFrameInfos[i]->stVFrame.pMbBlk);
    }

    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        RK_SAFE_FREE(stPipeFrameInfos[i]);
    }

    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++)
        RK_SAFE_FREE(stChnFrameInfos[i]);

    RK_SAFE_FREE(stPipeFrameInfos);
    RK_SAFE_FREE(stChnFrameInfos);

    /* avs destroy */
    s32Ret = destroy_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

static RK_S32 TEST_AVS_6_NoBlend_Ver(TEST_AVS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    AVS_CFG_S *pstAvsCtx = reinterpret_cast<AVS_CFG_S *>(&(ctx->avsContext));
    const RK_CHAR *cTestDataPath = "/data/avs/6x_hor_ver/";
    RK_CHAR    cWritePath[MAX_FILE_PATH_LEN] = {0};
    VIDEO_FRAME_INFO_S **stPipeFrameInfos;
    VIDEO_FRAME_INFO_S **stChnFrameInfos;

    snprintf(ctx->srcFilePath, sizeof(ctx->srcFilePath), "%s%s",
             cTestDataPath, "input_image/");
    snprintf(ctx->dstFilePath, sizeof(ctx->dstFilePath), "%s%s",
             cTestDataPath, "output_res/");

    /* avs config init */
    pstAvsCtx->s32GrpId      = 0;
    pstAvsCtx->s32ChnId      = 0;
    pstAvsCtx->u32AvsPipeCnt = 6;
    pstAvsCtx->u32AvsChnCnt  = 1;
    pstAvsCtx->u32InW        = 2560;
    pstAvsCtx->u32InH        = 1520;

    pstAvsCtx->stAvsModParam.u32WorkingSetSize             = 67 * 1024;
    pstAvsCtx->stAvsModParam.enMBSource                    = MB_SOURCE_PRIVATE;
    pstAvsCtx->stAvsGrpAttr.enMode                         = AVS_MODE_NOBLEND_VER;
    pstAvsCtx->stAvsGrpAttr.u32PipeNum                     = pstAvsCtx->u32AvsPipeCnt;
    pstAvsCtx->stAvsGrpAttr.bSyncPipe                      = (RK_BOOL)ctx->s32FrameSync;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32DstFrameRate    = -1;

    pstAvsCtx->stAvsChnAttr[0].enCompressMode              = ctx->enCompressMode;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].u32Depth                    = 3;
    pstAvsCtx->stAvsChnAttr[0].u32Width                    = 0;
    pstAvsCtx->stAvsChnAttr[0].u32Height                   = 0;
    pstAvsCtx->stAvsChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;

    /* avs create */
    s32Ret = create_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create avs [%d, %d, %d] failed %x",
                pstAvsCtx->s32GrpId,
                pstAvsCtx->s32PipeId,
                pstAvsCtx->s32ChnId,
                s32Ret);
        return s32Ret;
    }

    stPipeFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_PIPE_NUM));
    stChnFrameInfos = reinterpret_cast<VIDEO_FRAME_INFO_S **>(
                        malloc(sizeof(VIDEO_FRAME_INFO_S *) * AVS_MAX_CHN_NUM));
    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        stPipeFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stPipeFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }
    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++) {
        stChnFrameInfos[i] = reinterpret_cast<VIDEO_FRAME_INFO_S *>(malloc(sizeof(VIDEO_FRAME_INFO_S)));
        memset(stChnFrameInfos[i], 0, sizeof(VIDEO_FRAME_INFO_S));
    }

    s32Ret = TEST_AVS_ComCreateFrame(ctx, stPipeFrameInfos);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    for (RK_S32 loopNum = 0; loopNum < ctx->s32LoopCount; loopNum++) {
        s32Ret = TEST_AVS_ComSendFrame(ctx, stPipeFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        s32Ret = TEST_AVS_ComGetChnFrame(ctx, stChnFrameInfos);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }

        for (RK_S32 i = 0; i < ctx->avsContext.u32AvsChnCnt; i++) {
            if (!stChnFrameInfos[i]->stVFrame.pMbBlk) {
                continue;
            }
            if (ctx->dstFilePath) {
                snprintf(cWritePath, sizeof(cWritePath), "%schn_out_%dx%d_%d_%d_%s.bin",
                            ctx->dstFilePath, stChnFrameInfos[i]->stVFrame.u32VirWidth,
                            stChnFrameInfos[i]->stVFrame.u32VirHeight, ctx->avsContext.s32GrpId, i,
                            ctx->enCompressMode ? "nv12_afbc": "nv12");

                s32Ret = TEST_COMM_FileWriteOneFrame(cWritePath, stChnFrameInfos[i]);
                if (s32Ret != RK_SUCCESS) {
                    goto __FAILED;
                }
            }
            s32Ret = RK_MPI_AVS_ReleaseChnFrame(pstAvsCtx->s32GrpId, i, stChnFrameInfos[i]);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("release failed grp[%d] chn[%d] frame %p",
                        pstAvsCtx->s32GrpId, i,
                        stChnFrameInfos[i]->stVFrame.pMbBlk);
                    goto __FAILED;
            }
        }
    }

__FAILED:
    for (RK_S32 i = 0; i < ctx->avsContext.u32AvsPipeCnt; i++) {
        RK_MPI_MB_ReleaseMB(stPipeFrameInfos[i]->stVFrame.pMbBlk);
    }

    for (RK_S32 i = 0; i < AVS_PIPE_NUM; i++) {
        RK_SAFE_FREE(stPipeFrameInfos[i]);
    }

    for (RK_S32 i = 0; i < AVS_MAX_CHN_NUM; i++)
        RK_SAFE_FREE(stChnFrameInfos[i]);

    RK_SAFE_FREE(stPipeFrameInfos);
    RK_SAFE_FREE(stChnFrameInfos);

    /* avs destroy */
    s32Ret = destroy_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return s32Ret;
}

static RK_VOID sigterm_handler(int sig) {
    RK_LOGE("signal %d\n", sig);
    bExit = RK_TRUE;
}

static RK_S32 test_vi_avs_vo_loop(TEST_AVS_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    VI_CFG_S  *pstViCtx;
    AVS_CFG_S *pstAvsCtx = reinterpret_cast<AVS_CFG_S *>(&(ctx->avsContext));
    VO_CFG_S  *pstVoCtx;
    MPP_CHN_S stViChn[AVS_PIPE_NUM],
              stAvsChn[AVS_PIPE_NUM],
              stVoChn;
    RK_S32 loopCount = 0;
    RK_S32 i = 0;
    RK_U32 cameraNum = 6;

    pstViCtx = reinterpret_cast<VI_CFG_S *>(malloc(sizeof(VI_CFG_S) * AVS_PIPE_NUM));
    pstVoCtx  = reinterpret_cast<VO_CFG_S *>(malloc(sizeof(VO_CFG_S)));
    memset(pstViCtx, 0, sizeof(VI_CFG_S));
    memset(pstVoCtx, 0, sizeof(VO_CFG_S));

    for (i = 0; i < cameraNum; i++) {
    /* vi config init */
        pstViCtx[i].s32DevId = i;
        pstViCtx[i].s32PipeId = pstViCtx[i].s32DevId;
        if (ctx->enCompressMode == COMPRESS_MODE_NONE) {
            pstViCtx[i].s32ChnId = 0;      // main path
        } else if (ctx->enCompressMode == COMPRESS_AFBC_16x16) {
            pstViCtx[i].s32ChnId = 2;      // fbc path
        }
        if (cameraNum == 2) {
            pstViCtx[i].stViChnAttr.stSize.u32Width = 2688;
            pstViCtx[i].stViChnAttr.stSize.u32Height = 1520;
        } else if (cameraNum == 4 || cameraNum == 6) {
            pstViCtx[i].stViChnAttr.stSize.u32Width = 2560;
            pstViCtx[i].stViChnAttr.stSize.u32Height = 1520;
        }
        pstViCtx[i].stViChnAttr.stIspOpt.enMemoryType       = VI_V4L2_MEMORY_TYPE_DMABUF;
        pstViCtx[i].stViChnAttr.stIspOpt.u32BufCount        = 10;
        pstViCtx[i].stViChnAttr.u32Depth                    = 2;
        pstViCtx[i].stViChnAttr.enPixelFormat               = RK_FMT_YUV420SP;
        pstViCtx[i].stViChnAttr.enCompressMode              = ctx->enCompressMode;
        pstViCtx[i].stViChnAttr.stFrameRate.s32SrcFrameRate = -1;
        pstViCtx[i].stViChnAttr.stFrameRate.s32DstFrameRate = -1;

    /* vi create */
        s32Ret = create_vi(&pstViCtx[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vi [%d, %d] init failed: %x",
                    pstViCtx[i].s32DevId, pstViCtx[i].s32ChnId,
                    s32Ret);
            goto __FAILED;
        }
    }

    /* avs config init */
    pstAvsCtx->s32GrpId = 0;
    pstAvsCtx->s32PipeId = 0;
    pstAvsCtx->s32ChnId = 0;
    pstAvsCtx->u32AvsPipeCnt = cameraNum;
    pstAvsCtx->u32AvsChnCnt = 1;

    pstAvsCtx->stAvsGrpAttr.enMode = AVS_MODE_BLEND;
    pstAvsCtx->u32OutW = 8192;
    pstAvsCtx->u32OutH = 2700;

    if (PARAMS_SOURCES_MESH == ctx->enParamsSources) {
        snprintf(pstAvsCtx->stAvsGrpAttr.stLUT.aFilePath, sizeof("/usr/share/avs_mesh/"),
                "/usr/share/avs_mesh/");
    } else if (PARAMS_SOURCES_CALIB == ctx->enParamsSources) {
        snprintf(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aCalibFilePath,
                 sizeof("/usr/share/avs_calib/calib_file_pos.pto"),
                 "/usr/share/avs_calib/calib_file_pos.pto");
        snprintf(pstAvsCtx->stAvsGrpAttr.stOutAttr.stCalib.aMeshAlphaPath, sizeof("/usr/share/avs_calib/"),
                 "/usr/share/avs_calib/");
    }

    pstAvsCtx->stAvsGrpAttr.stLUT.enAccuracy                 = AVS_LUT_ACCURACY_HIGH;

    pstAvsCtx->stAvsModParam.u32WorkingSetSize               = 67 * 1024;
    pstAvsCtx->stAvsGrpAttr.u32PipeNum                       = pstAvsCtx->u32AvsPipeCnt;
    pstAvsCtx->stAvsGrpAttr.stGainAttr.enMode                = AVS_GAIN_MODE_AUTO;

    pstAvsCtx->stAvsGrpAttr.stOutAttr.enPrjMode              = AVS_PROJECTION_EQUIRECTANGULAR;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stCenter.s32X          = 4196;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stCenter.s32Y          = 2080;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stFOV.u32FOVX          = 28000;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stFOV.u32FOVY          = 9500;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Roll  = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Pitch = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stORIRotation.s32Yaw   = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Roll     = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Pitch    = 0;
    pstAvsCtx->stAvsGrpAttr.stOutAttr.stRotation.s32Yaw      = 0;

    pstAvsCtx->stAvsGrpAttr.bSyncPipe                     = (RK_BOOL)ctx->s32FrameSync;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32SrcFrameRate   = -1;
    pstAvsCtx->stAvsGrpAttr.stFrameRate.s32DstFrameRate   = -1;

    pstAvsCtx->stAvsChnAttr[0].enCompressMode              = ctx->enCompressMode;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    pstAvsCtx->stAvsChnAttr[0].u32Depth                    = 0;
    pstAvsCtx->stAvsChnAttr[0].u32Width                    = pstAvsCtx->u32OutW;
    pstAvsCtx->stAvsChnAttr[0].u32Height                   = pstAvsCtx->u32OutH;
    pstAvsCtx->stAvsChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;

    /* avs create */
    s32Ret = create_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create avs [%d, %d, %d] error %x",
                pstAvsCtx->s32GrpId,
                pstAvsCtx->s32PipeId,
                pstAvsCtx->s32ChnId,
                s32Ret);
        return s32Ret;
    }

    /* vo config init */
    pstVoCtx->s32LayerId = RK356X_VOP_LAYER_CLUSTER0;
    pstVoCtx->s32DevId = ctx->enVoDevType;
    if (RK3588_VO_DEV_HDMI == pstVoCtx->s32DevId) {
        pstVoCtx->stVoLayerAttr.stDispRect.u32Width = 1920;
        pstVoCtx->stVoLayerAttr.stDispRect.u32Height = 1080;
    } else if (RK3588_VO_DEV_MIPI == pstVoCtx->s32DevId) {
        pstVoCtx->stVoLayerAttr.stDispRect.u32Width = 1080;
        pstVoCtx->stVoLayerAttr.stDispRect.u32Height = 1920;
    }
    pstVoCtx->stVoLayerAttr.enPixFormat = RK_FMT_RGB888;
    pstVoCtx->stVoLayerAttr.bBypassFrame = RK_FALSE;

    pstVoCtx->s32ChnId = 0;
    pstVoCtx->stVoCscAttr.enCscMatrix = VO_CSC_MATRIX_IDENTITY;
    pstVoCtx->stVoCscAttr.u32Contrast = 50;
    pstVoCtx->stVoCscAttr.u32Hue = 50;
    pstVoCtx->stVoCscAttr.u32Luma = 50;
    pstVoCtx->stVoCscAttr.u32Satuature = 50;

    pstVoCtx->stVoLayerAttr.stDispRect.s32X = 0;
    pstVoCtx->stVoLayerAttr.stDispRect.s32Y = 0;
    pstVoCtx->stVoLayerAttr.stImageSize.u32Width =
        pstVoCtx->stVoLayerAttr.stDispRect.u32Width;
    pstVoCtx->stVoLayerAttr.stImageSize.u32Height =
        pstVoCtx->stVoLayerAttr.stDispRect.u32Height;

    pstVoCtx->stVoLayerAttr.u32DispFrmRt = 30;
    pstVoCtx->stVoChnAttr.stRect.s32X = 0;
    pstVoCtx->stVoChnAttr.stRect.s32Y = 0;
    pstVoCtx->stVoChnAttr.stRect.u32Width =
        pstVoCtx->stVoLayerAttr.stImageSize.u32Width;
    pstVoCtx->stVoChnAttr.stRect.u32Height =
        pstVoCtx->stVoLayerAttr.stImageSize.u32Height;
    pstVoCtx->stVoChnAttr.bDeflicker = RK_FALSE;
    pstVoCtx->stVoChnAttr.u32Priority = 1;
    pstVoCtx->stVoChnAttr.u32FgAlpha = 128;
    pstVoCtx->stVoChnAttr.u32BgAlpha = 0;

    /* vo creat */
    s32Ret = create_vo(pstVoCtx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vo [%d, %d, %d] create failed: %x",
                pstVoCtx->s32DevId,
                pstVoCtx->s32LayerId,
                pstVoCtx->s32ChnId,
                s32Ret);
        goto __FAILED;
    }

    /* bind vi -> avs */
    for (i = 0; i < cameraNum; i++) {
        stViChn[i].enModId    = RK_ID_VI;
        stViChn[i].s32DevId   = pstViCtx[i].s32DevId;
        stViChn[i].s32ChnId   = pstViCtx[i].s32ChnId;

        stAvsChn[i].enModId   = RK_ID_AVS;
        stAvsChn[i].s32DevId  = pstAvsCtx->s32GrpId;
        stAvsChn[i].s32ChnId  = i;

        RK_LOGI("vi [%d, %d] -> avs [%d, %d]",
                stViChn[i].s32DevId , stViChn[i].s32ChnId,
                stAvsChn[i].s32DevId , stAvsChn[i].s32ChnId);
        s32Ret = RK_MPI_SYS_Bind(&stViChn[i], &stAvsChn[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("bind error %x: vi [%d, %d] -> avs [%d, %d]",
                    s32Ret,
                    stViChn[i].s32DevId , stViChn[i].s32ChnId,
                    stAvsChn[i].s32DevId , stAvsChn[i].s32ChnId);
            goto __FAILED;
        }
    }

    /* bind avs -> vo */
    stAvsChn[0].enModId  = RK_ID_AVS;
    stAvsChn[0].s32DevId = 0;
    stAvsChn[0].s32ChnId = 0;
    stVoChn.enModId   = RK_ID_VO;
    stVoChn.s32DevId  = 0;
    stVoChn.s32ChnId  = 0;

    RK_LOGI("avs [%d, %d] -> vo [%d, %d]",
            stAvsChn[0].s32DevId , stAvsChn[0].s32ChnId,
            stVoChn.s32DevId , stVoChn.s32ChnId);
    s32Ret = RK_MPI_SYS_Bind(&stAvsChn[0], &stVoChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("bind error %x: avs [%d, %d] -> vo [%d, %d]",
                s32Ret,
                stAvsChn[0].s32DevId, stAvsChn[0].s32ChnId,
                stVoChn.s32DevId, stVoChn.s32ChnId);
        goto __FAILED;
    }

    /* thread: do somethings */
    signal(SIGINT, sigterm_handler);
    if (ctx->s32LoopCount < 0) {
        while (!bExit) {
            sleep(60);
        }
    } else {
        while (!bExit && loopCount < ctx->s32LoopCount) {
            sleep(5);
            loopCount++;
        }
    }

    /* unbind avs -> vo */
    s32Ret = RK_MPI_SYS_UnBind(&stAvsChn[0], &stVoChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("unbind error %x: avs [%d, %d] -> vo [%d, %d]",
                s32Ret,
                stAvsChn[0].s32DevId, stAvsChn[0].s32ChnId,
                stVoChn.s32DevId, stVoChn.s32ChnId);
        goto __FAILED;
    }

    /* unbind vi -> avs */
    for (RK_S32 i = 0; i < cameraNum; i++) {
        s32Ret = RK_MPI_SYS_UnBind(&stViChn[i], &stAvsChn[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("unbind error %x: vi [%d, %d] -> avs [%d, %d]",
                    s32Ret,
                    stViChn[i].s32DevId, stViChn[i].s32ChnId,
                    stAvsChn[i].s32DevId, stAvsChn[i].s32ChnId);
            goto __FAILED;
        }
    }

    /* destroy vo */
    s32Ret = destroy_vo(pstVoCtx);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    /* destroy avs */
    s32Ret = destroy_avs(pstAvsCtx);
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }

    /* destroy vi*/
    for (i = 0; i < cameraNum; i++) {
        s32Ret = destroy_vi(&pstViCtx[i]);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
    }

__FAILED:
    RK_SAFE_FREE(pstViCtx);
    RK_SAFE_FREE(pstVoCtx);

    return s32Ret;
}

static const char *const usages[] = {
    "./rk_mpi_avs_test [-m TEST_MODE] [-c LINK_COMPRESS_MODE] [-n LOOP_COUNT] [-s STITCHING_NUM]",
    RK_NULL,
};

static void mpi_avs_test_show_options(const TEST_AVS_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("test mode           : %d\n", ctx->enTestMode);
    RK_PRINT("loop count          : %d\n", ctx->s32LoopCount);
    RK_PRINT("compress mode       : %d\n", ctx->enCompressMode);
    RK_PRINT("avs frame sync      : %d\n", ctx->s32FrameSync);
    RK_PRINT("params sources      : %d\n", ctx->enParamsSources);
    RK_PRINT("vo dev type         : %d\n", ctx->enVoDevType);
}

RK_S32 main(int argc, const char **argv) {
    RK_S32 s32Ret = RK_FAILURE;
    TEST_AVS_CTX_S   ctx;

    memset(&ctx, 0, sizeof(TEST_AVS_CTX_S));

    ctx.s32LoopCount       = 100;
    ctx.enTestMode         = TEST_MODE_AVS_BLEND;
    ctx.enParamsSources    = PARAMS_SOURCES_MESH;
    ctx.s32FrameSync       = 0;
    ctx.enCompressMode     = COMPRESS_AFBC_16x16;
    ctx.enVoDevType        = RK3588_VO_DEV_MIPI;

    RK_LOGE("avs test running enter!");

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_INTEGER('m', "test_mode", &(ctx.enTestMode),
                    "test mode(default 0) \n\t"
                    "0: avs module. 8xEquirectangular, 6xRectilinear. \n\t"
                    "1: avs module. 6xNoBlend_Hor, 6xNoBlend_Ver, 4xNoBlend_Qr. \n\t"
                    "2: vi -> avs -> vo. 6xEquirectangular. \n\t", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx.s32LoopCount),
                    "loop running count. default(100)", NULL, 0, 0),
        OPT_INTEGER('c', "link_compress_mode", &(ctx.enCompressMode),
                    "link compression mode. default(1. 0: Uncompress, 1: AFBC).", NULL, 0, 0),
        OPT_INTEGER('p', "avs_pipe_sync", &(ctx.s32FrameSync),
                    "whether enable avs pipe sync. default(0. 0: Disable)", NULL, 0, 0),
        OPT_INTEGER('\0', "params_sources", &(ctx.enParamsSources),
                    "params required for stitch are obtained by mesh or calib. (default 0. 0: mesh, 1: calib)",
                    NULL, 0, 0),
        OPT_INTEGER('\0', "connector_type", &(ctx.enVoDevType),
                     "Connctor Type. (default 3. 0: HDMI0, 3: MIPI)", NULL, 0, 0),
        OPT_END(),
    };
    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_avs_test_show_options(&ctx);

    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    switch (ctx.enTestMode) {
        case TEST_MODE_AVS_BLEND: {
            s32Ret = TEST_AVS_8_Equirectangular(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
            s32Ret = TEST_AVS_6_Rectilinear(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
            s32Ret = TEST_AVS_4_Equirectangular_Trans(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
            s32Ret = TEST_AVS_2_Cylindrical(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        } break;
        case TEST_MODE_AVS_NOBLEND: {
            s32Ret = TEST_AVS_6_NoBlend_Hor(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
            s32Ret = TEST_AVS_6_NoBlend_Ver(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
            s32Ret = TEST_AVS_4_NoBlend_Qr(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        } break;
        case TEST_MODE_VI_AVS_VO: {
            s32Ret = test_vi_avs_vo_loop(&ctx);
            if (s32Ret != RK_SUCCESS) {
                goto __FAILED;
            }
        } break;
        default:
            RK_LOGE("nosupport test mode: %d", ctx.enTestMode);
        break;
    }

    s32Ret = RK_MPI_SYS_Exit();
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    RK_LOGI("test running ok.");

    return RK_SUCCESS;
__FAILED:
    RK_LOGE("test running exit: %x", s32Ret);
    RK_MPI_SYS_Exit();

    return s32Ret;
}
