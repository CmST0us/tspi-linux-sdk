/*
 * Copyright 2018 Rockchip Electronics Co. LTD
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

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/types.h>
#include "rk_debug.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_vpss.h"
#include "rk_mpi_venc.h"
#include "test_comm_argparse.h"
#include "test_comm_utils.h"

#define MAX_STREAM_CNT               8
#define MAX_TIME_OUT_MS              20

#ifndef VDEC_INT64_MIN
#define VDEC_INT64_MIN               (-0x7fffffffffffffffLL-1)
#endif

#ifndef VDEC_INT64_MAX
#define VDEC_INT64_MAX               INT64_C(9223372036854775807)
#endif

#define VDEC_CH_CNT 2
#define VO_CH_CNT 4
#define HDMI_W 1920
#define HDMI_H 1080
#define MIPI_W 1080
#define MIPI_H 1920

typedef struct _rkMpiVDECCtx {
    const char *srcFileUri;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_U32 u32SrcWidth1;
    RK_U32 u32SrcHeight1;
    RK_S32 s32LoopCount;
    const char *aEntityName;
    PIXEL_FORMAT_E enPixelFormat;
    RK_U32 u32ReadSize;
    RK_CODEC_ID_E enCodecId;
    RK_BOOL threadExit;
    RK_U32 u32ChnIndex;
    RK_S32 s32ChnFd;
} TEST_ALL_CTX_S;

static RK_S32 check_options(const TEST_ALL_CTX_S *ctx) {
    if (ctx->srcFileUri == RK_NULL) {
        goto __FAILED;
    }

    if (ctx->enCodecId <= RK_VIDEO_ID_Unused ||
        ctx->u32SrcWidth <= 0 ||
        ctx->u32SrcHeight <= 0) {
        goto __FAILED;
    }

    return RK_SUCCESS;

__FAILED:
    return RK_FAILURE;
}

RK_S32 mpi_create_vdec(TEST_ALL_CTX_S *ctx, RK_S32 s32Ch, VIDEO_MODE_E enMode) {
    RK_S32 s32Ret = RK_SUCCESS;
    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;
    MB_POOL_CONFIG_S stMbPoolCfg;
    VDEC_PIC_BUF_ATTR_S stVdecPicBufAttr;
    MB_PIC_CAL_S stMbPicCalResult;
    VDEC_MOD_PARAM_S stModParam;

    memset(&stAttr, 0, sizeof(VDEC_CHN_ATTR_S));
    memset(&stVdecParam, 0, sizeof(VDEC_CHN_PARAM_S));
    memset(&stModParam, 0, sizeof(VDEC_MOD_PARAM_S));

    stAttr.enMode = enMode;
    stAttr.enType = ctx->enCodecId;
    stAttr.u32PicWidth = ctx->u32SrcWidth;
    stAttr.u32PicHeight = ctx->u32SrcHeight;
    stAttr.u32FrameBufCnt = 8;
    stAttr.u32StreamBufCnt = MAX_STREAM_CNT;

    s32Ret = RK_MPI_VDEC_CreateChn(ctx->u32ChnIndex, &stAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create %d vdec failed! ", ctx->u32ChnIndex);
        return s32Ret;
    }

    if (ctx->enCodecId == RK_VIDEO_ID_MJPEG) {
        stVdecParam.stVdecPictureParam.enPixelFormat = RK_FMT_YUV420SP;
    } else {
        stVdecParam.stVdecVideoParam.enCompressMode = COMPRESS_MODE_NONE;
    }

    // it is only effective to disable MV when decoding sequence output
    stVdecParam.stVdecVideoParam.enOutputOrder = VIDEO_OUTPUT_ORDER_DISP;

    s32Ret = RK_MPI_VDEC_SetChnParam(ctx->u32ChnIndex, &stVdecParam);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("set chn %d param failed %x! ", ctx->u32ChnIndex, s32Ret);
        return s32Ret;
    }

    ctx->s32ChnFd = RK_MPI_VDEC_GetFd(ctx->u32ChnIndex);
    if (ctx->s32ChnFd <= 0) {
        RK_LOGE("get fd chn %d failed %d", ctx->u32ChnIndex, ctx->s32ChnFd);
        return s32Ret;
    }    

    s32Ret = RK_MPI_VDEC_StartRecvStream(ctx->u32ChnIndex);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("start recv chn %d failed %x! ", ctx->u32ChnIndex, s32Ret);
        return s32Ret;
    }

    return RK_SUCCESS;
}

RK_S32 mpi_destory_vdec(TEST_ALL_CTX_S *ctx, RK_S32 s32Ch) {
    RK_MPI_VDEC_StopRecvStream(s32Ch);

    if (ctx->s32ChnFd > 0) {
        RK_MPI_VDEC_CloseFd(s32Ch);
    }

    RK_MPI_VDEC_DestroyChn(s32Ch);
    
    return RK_SUCCESS;
}

RK_S32 mpi_create_stream_mode(TEST_ALL_CTX_S *ctx, RK_S32 s32Ch) {
    VIDEO_MODE_E enMode;

    if (ctx->enCodecId == RK_VIDEO_ID_MJPEG || ctx->enCodecId == RK_VIDEO_ID_JPEG) {
        ctx->u32ReadSize = ctx->u32SrcWidth * ctx->u32SrcHeight;
        enMode = VIDEO_MODE_FRAME;
    } else {
        enMode = VIDEO_MODE_STREAM;
    }

    return mpi_create_vdec(ctx, s32Ch, enMode);
}

static RK_S32 mpi_vdec_free(void *opaque) {
    if (opaque)
        free(opaque);
    return 0;
}

static void* mpi_send_stream(void *pArgs) {
    TEST_ALL_CTX_S *ctx = reinterpret_cast<TEST_ALL_CTX_S *>(pArgs);
    RK_S32 s32Size = 0;
    RK_S32 s32Ret = 0;
    RK_U8 *data = RK_NULL;
    FILE *fp = RK_NULL;
    MB_BLK buffer = RK_NULL;
    MB_EXT_CONFIG_S pstMbExtConfig;
    VDEC_CHN_STATUS_S staus;
    VDEC_CHN_ATTR_S stAttr;
    VDEC_CHN_PARAM_S stVdecParam;
    VDEC_STREAM_S stStream;
    RK_S32 s32PacketCount = 0;
    RK_S32 s32ReachEOS = 0;

    memset(&stStream, 0, sizeof(VDEC_STREAM_S));

    fp = fopen(ctx->srcFileUri, "r");
    if (fp == RK_NULL) {
        RK_LOGE("open file %s failed", ctx->srcFileUri);
        return RK_NULL;
    }

    while (!ctx->threadExit) {
        data = reinterpret_cast<RK_U8 *>(calloc(ctx->u32ReadSize, sizeof(RK_U8)));
        memset(data, 0, ctx->u32ReadSize);
        s32Size = fread(data, 1, ctx->u32ReadSize, fp);
        if (s32Size <= 0) {
               s32ReachEOS = 1; 
        }

        memset(&pstMbExtConfig, 0, sizeof(MB_EXT_CONFIG_S));
        pstMbExtConfig.pFreeCB = mpi_vdec_free;
        pstMbExtConfig.pOpaque = data;
        pstMbExtConfig.pu8VirAddr = data;
        pstMbExtConfig.u64Size = s32Size;

        RK_MPI_SYS_CreateMB(&buffer, &pstMbExtConfig);

        stStream.u64PTS = 0;
        stStream.pMbBlk = buffer;
        stStream.u32Len = s32Size;
        stStream.bEndOfStream = s32ReachEOS ? RK_TRUE : RK_FALSE;
        stStream.bEndOfFrame = s32ReachEOS ? RK_TRUE : RK_FALSE;
        stStream.bBypassMbBlk = RK_TRUE;
__RETRY:
        s32Ret = RK_MPI_VDEC_SendStream(ctx->u32ChnIndex, &stStream, MAX_TIME_OUT_MS);
        if (s32Ret < 0) {
            if (ctx->threadExit) {
                mpi_vdec_free(data);
                RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
                break;
            }
            usleep(1000llu);
            goto  __RETRY;
        } else {
            s32PacketCount++;
            RK_MPI_MB_ReleaseMB(stStream.pMbBlk);
            //RK_LOGI("send chn %d packet %d", ctx->u32ChnIndex, s32PacketCount);
        }
        if (s32ReachEOS) {
            RK_LOGI("chn %d input reach EOS", ctx->u32ChnIndex);
            break;
        }
    }

    if (fp)
        fclose(fp);

    RK_LOGI("%s out\n", __FUNCTION__);
    return RK_NULL;
}

static RK_S32 create_vo(TEST_ALL_CTX_S *ctx) {
    /* Enable VO */
    VO_PUB_ATTR_S VoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    RK_S32 s32Ret = RK_SUCCESS;
    VO_CHN_ATTR_S stChnAttr;

    memset(&VoPubAttr, 0, sizeof(VO_PUB_ATTR_S));
    memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));

    stLayerAttr.enPixFormat = RK_FMT_RGB888;
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.u32DispFrmRt = 30;
    stLayerAttr.stDispRect.u32Width = HDMI_W;
    stLayerAttr.stDispRect.u32Height = HDMI_H;
    stLayerAttr.stImageSize.u32Width = HDMI_W;
    stLayerAttr.stImageSize.u32Height = HDMI_H;

    s32Ret = RK_MPI_VO_GetPubAttr(0, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    VoPubAttr.enIntfType = VO_INTF_HDMI;
    VoPubAttr.enIntfSync = VO_OUTPUT_DEFAULT;

    s32Ret = RK_MPI_VO_SetPubAttr(0, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_Enable(0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_GetPubAttr(3, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    VoPubAttr.enIntfType = VO_INTF_MIPI;
    VoPubAttr.enIntfSync = VO_OUTPUT_DEFAULT;

    s32Ret = RK_MPI_VO_SetPubAttr(3, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_Enable(3);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_BindLayer(0, 0, VO_LAYER_MODE_VIDEO);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_BindLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_BindLayer(1, 3, VO_LAYER_MODE_GRAPHIC);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_BindLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_SetLayerAttr(0, &stLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerAttr failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    RK_MPI_VO_SetLayerPriority(0, 7);

    stLayerAttr.stDispRect.u32Width = MIPI_W;
    stLayerAttr.stDispRect.u32Height = MIPI_H;
    stLayerAttr.stImageSize.u32Width = MIPI_W;
    stLayerAttr.stImageSize.u32Height = MIPI_H;
    s32Ret = RK_MPI_VO_SetLayerAttr(1, &stLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerAttr failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    RK_MPI_VO_SetLayerPriority(1, 7);
    RK_MPI_VO_SetLayerSpliceMode(1, VO_SPLICE_MODE_RGA);

    s32Ret = RK_MPI_VO_EnableLayer(0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_EnableLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_EnableLayer(1);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_EnableLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    for (int i = 0; i < VO_CH_CNT; i++) {
        stChnAttr.stRect.u32Width = HDMI_W / (VO_CH_CNT / 2);
        stChnAttr.stRect.u32Height = HDMI_H / (VO_CH_CNT / 2);
        stChnAttr.stRect.s32X = (i % (VO_CH_CNT / 2)) * stChnAttr.stRect.u32Width;
        stChnAttr.stRect.s32Y = (i / (VO_CH_CNT / 2)) * stChnAttr.stRect.u32Height;
        stChnAttr.u32Priority = i;
        stChnAttr.u32FgAlpha = 128;
        stChnAttr.u32BgAlpha = 0;

        s32Ret = RK_MPI_VO_SetChnAttr(0, i, &stChnAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("set chn Attr failed,s32Ret:%x\n", s32Ret);
            return RK_FAILURE;
        }

        stChnAttr.stRect.u32Width = MIPI_W / (VO_CH_CNT / 2);
        stChnAttr.stRect.u32Height = MIPI_H / (VO_CH_CNT / 2);
        stChnAttr.stRect.s32X = (i % (VO_CH_CNT / 2)) * stChnAttr.stRect.u32Width;
        stChnAttr.stRect.s32Y = (i / (VO_CH_CNT / 2)) * stChnAttr.stRect.u32Height;
        s32Ret = RK_MPI_VO_SetChnAttr(1, i, &stChnAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("set chn Attr failed,s32Ret:%x\n", s32Ret);
            return RK_FAILURE;
        }
    }

    return s32Ret;
}

static RK_S32 destory_vo(void) {

    RK_S32 s32Ret = RK_SUCCESS;

    s32Ret = RK_MPI_VO_DisableLayer(0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_DisableLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }
    s32Ret = RK_MPI_VO_DisableLayer(1);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_DisableLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_UnBindLayer(0, 0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_UnBindLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_UnBindLayer(1, 3);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_UnBindLayer failed,s32Ret:%x\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_Disable(0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }   
    s32Ret = RK_MPI_VO_Disable(3);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    return s32Ret;
}

static RK_S32 create_vi(TEST_ALL_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    VI_DEV_ATTR_S stDevAttr;
    VI_DEV_BIND_PIPE_S stBindPipe;
    VI_CHN_ATTR_S stChnAttr;
    // 0. get dev config status
    s32Ret = RK_MPI_VI_GetDevAttr(0, &stDevAttr);
    if (s32Ret == RK_ERR_VI_NOT_CONFIG) {
        // 0-1.config dev
        s32Ret = RK_MPI_VI_SetDevAttr(0, &stDevAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevAttr %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_SetDevAttr already");
    }
    // 1.get  dev enable status
    s32Ret = RK_MPI_VI_GetDevIsEnable(0);
    if (s32Ret != RK_SUCCESS) {
        // 1-2.enable dev
        s32Ret = RK_MPI_VI_EnableDev(0);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_EnableDev %x", s32Ret);
            goto __FAILED;
        }
        // 1-3.bind dev/pipe
        stBindPipe.u32Num = 0;
        stBindPipe.PipeId[0] = 0;
        s32Ret = RK_MPI_VI_SetDevBindPipe(0, &stBindPipe);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevBindPipe %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_EnableDev already");
    }
    // 2.config channel
    memset(&stChnAttr, 0, sizeof(VI_CHN_ATTR_S));
    stChnAttr.stIspOpt.u32BufCount = 6;
    stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_MMAP;
    stChnAttr.stIspOpt.enCaptureType = VI_V4L2_CAPTURE_TYPE_VIDEO_CAPTURE;
    stChnAttr.u32Depth = 0;
    stChnAttr.enPixelFormat = ctx->enPixelFormat;
    stChnAttr.stFrameRate.s32SrcFrameRate = -1;
    stChnAttr.stFrameRate.s32DstFrameRate = -1;
    stChnAttr.stSize.u32Width = ctx->u32SrcWidth1;
    stChnAttr.stSize.u32Height = ctx->u32SrcHeight1;
    stChnAttr.stSize.u32Width = ctx->u32SrcWidth1;
    stChnAttr.stSize.u32Height = ctx->u32SrcHeight1;
    stChnAttr.enCompressMode = COMPRESS_MODE_NONE;
    strcpy(stChnAttr.stIspOpt.aEntityName, ctx->aEntityName);
    s32Ret = RK_MPI_VI_SetChnAttr(0, 0, &stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_SetChnAttr %x", s32Ret);
        goto __FAILED;
    }
    // 3.enable channel
    s32Ret = RK_MPI_VI_EnableChn(0, 0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_EnableChn %x", s32Ret);
        goto __FAILED;
    }
__FAILED:
    return s32Ret;
}

static RK_S32 destroy_vi() {
    RK_S32 s32Ret = RK_FAILURE;
    s32Ret = RK_MPI_VI_DisableChn(0, 0);
    RK_LOGE("RK_MPI_VI_DisableChn pipe=%d ret:%x", 0, s32Ret);

    s32Ret = RK_MPI_VI_DisableDev(0);
    RK_LOGE("RK_MPI_VI_DisableDev device=%d ret:%x", 0, s32Ret);
    return s32Ret;
}

static RK_S32 create_venc(TEST_ALL_CTX_S *ctx) {
    VENC_RECV_PIC_PARAM_S stRecvParam;
    VENC_CHN_ATTR_S stAttr;
    memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));
    stAttr.stVencAttr.enType = RK_VIDEO_ID_AVC;
    stAttr.stVencAttr.enPixelFormat = RK_FMT_YUV420SP;
    stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    stAttr.stRcAttr.stH264Cbr.u32Gop = 60;
    stAttr.stVencAttr.u32PicWidth = ctx->u32SrcWidth;
    stAttr.stVencAttr.u32PicHeight = ctx->u32SrcHeight;
    stAttr.stVencAttr.u32VirWidth = ctx->u32SrcWidth;
    stAttr.stVencAttr.u32VirHeight = ctx->u32SrcHeight;
    stAttr.stVencAttr.u32StreamBufCnt = 5;
    stAttr.stVencAttr.u32BufSize = ctx->u32SrcWidth * ctx->u32SrcHeight * 3 / 2;
    stRecvParam.s32RecvPicNum = -1;
    RK_MPI_VENC_CreateChn(0, &stAttr);
    RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);
    return RK_SUCCESS;
}

static RK_S32 destory_venc(void) {
    VENC_RECV_PIC_PARAM_S stRecvParam;
    VENC_CHN_ATTR_S stAttr;
    RK_S32 s32Ret = RK_SUCCESS;
    s32Ret = RK_MPI_VENC_StopRecvFrame(0);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    RK_LOGE("destroy enc chn:%d", 0);
    s32Ret = RK_MPI_VENC_DestroyChn(0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VDEC_DestroyChn fail %x", s32Ret);
    }
    return RK_SUCCESS;
}

        

static RK_S32 create_vpss(RK_S32 s32Grp, RK_S32 s32OutChnNum, TEST_ALL_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CHN VpssChn[VPSS_MAX_CHN_NUM] = { VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3 };
    VPSS_GRP_ATTR_S stGrpVpssAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
     /* vpss */
    memset(&stGrpVpssAttr, 0, sizeof(VPSS_GRP_ATTR_S));
    stGrpVpssAttr.u32MaxW = 4096;
    stGrpVpssAttr.u32MaxH = 4096;
    stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;
    stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;
    stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;
    stGrpVpssAttr.enCompressMode = COMPRESS_MODE_NONE;

    s32Ret = RK_MPI_VPSS_CreateGrp(s32Grp, &stGrpVpssAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    for (RK_S32 i = 0; i < s32OutChnNum; i++) {
        memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));
        stVpssChnAttr.enChnMode = VPSS_CHN_MODE_AUTO;
        stVpssChnAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
        stVpssChnAttr.enPixelFormat = RK_FMT_YUV420SP;
        stVpssChnAttr.stFrameRate.s32SrcFrameRate = -1;
        stVpssChnAttr.stFrameRate.s32DstFrameRate = -1;
        stVpssChnAttr.u32Width = ctx->u32SrcWidth / (i + 1);
        stVpssChnAttr.u32Height = ctx->u32SrcHeight / (i + 1);
        stVpssChnAttr.enCompressMode = COMPRESS_MODE_NONE;
        s32Ret = RK_MPI_VPSS_SetChnAttr(s32Grp, VpssChn[i], &stVpssChnAttr);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VPSS_EnableChn(s32Grp, VpssChn[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VPSS_SetVProcDev(s32Grp, VIDEO_PROC_DEV_RGA);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VPSS_StartGrp(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return  RK_SUCCESS;
}

static RK_S32 destory_vpss(RK_S32 s32Grp, RK_S32 s32OutChnNum) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CHN VpssChn[VPSS_MAX_CHN_NUM] = { VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3 };

    s32Ret = RK_MPI_VPSS_StopGrp(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    for (RK_S32 i = 0; i < s32OutChnNum; i++) {
        s32Ret = RK_MPI_VPSS_DisableChn(s32Grp, VpssChn[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VPSS_DisableBackupFrame(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VPSS_DestroyGrp(s32Grp);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    return  RK_SUCCESS;
}

RK_S32 unit_test_mpi_all(TEST_ALL_CTX_S *ctx) {
    MPP_CHN_S stSrcChn, stDestChn;
    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 u32Ch = 0;
    TEST_ALL_CTX_S vdecCtx[VDEC_CH_CNT];
    pthread_t vdecThread[VDEC_CH_CNT];

    for (u32Ch = 0; u32Ch < VDEC_CH_CNT; u32Ch++) {
        ctx->u32ChnIndex = u32Ch;
        memcpy(&(vdecCtx[u32Ch]), ctx, sizeof(TEST_ALL_CTX_S));

        // Does not support JPEG stream framing, read the size of one picture at a time
        // and send it to the decoder.
        mpi_create_stream_mode(&vdecCtx[u32Ch], u32Ch);
        if (u32Ch < VDEC_CH_CNT - 1)
            pthread_create(&vdecThread[u32Ch], 0, mpi_send_stream, reinterpret_cast<void *>(&vdecCtx[u32Ch]));
    }

    //create vo layer 0, layer 1, chn 0 -- 3
    s32Ret = create_vo(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create vo ch failed");
        return -1;
    }
    
    //create vi 0, 0
    s32Ret = create_vi(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create vi ch failed");
        return -1;
    }

    //create vpss grp 0, chn 0 -- 1
    s32Ret = create_vpss(0, 2, ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create vpss ch failed");
        return -1;
    }

    //create venc 0, 0
    s32Ret = create_venc(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create venc ch failed");
        return -1;
    }

    // bind vdec[0] to vo [0, 0] [1, 0]
    // bind vdec[1] to vo [0, 1] [1, 1]
    for (int i = 0; i < VDEC_CH_CNT; i++) {
        stSrcChn.enModId    = RK_ID_VDEC;
        stSrcChn.s32DevId   = 0;
        stSrcChn.s32ChnId   = i;
        stDestChn.enModId   = RK_ID_VO;
        stDestChn.s32DevId  = 0;
        stDestChn.s32ChnId  = i;
        s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vi band vo fail:%x", s32Ret);
            return -1;
        }

        stDestChn.enModId   = RK_ID_VO;
        stDestChn.s32DevId  = 1;
        stDestChn.s32ChnId  = i;
        s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vi band vo fail:%x", s32Ret);
            return -1;
        }

        // enable vo
        s32Ret = RK_MPI_VO_EnableChn(0, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", s32Ret);
            return -1;
        }
        s32Ret = RK_MPI_VO_EnableChn(1, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", s32Ret);
            return -1;
        }
    }

    //bind vi[0] to vo [0, 2] [1, 2]
    stSrcChn.enModId    = RK_ID_VI;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = VDEC_CH_CNT;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi band vo fail:%x", s32Ret);
        return -1;
    }

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 1;
    stDestChn.s32ChnId  = VDEC_CH_CNT;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi band vo fail:%x", s32Ret);
        return -1;
    }

    //bind vi [0, 0] to vpss grp 0
    stDestChn.enModId   = RK_ID_VPSS;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = 0;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi band vpss fail:%x", s32Ret);
        return -1;
    }

    //bind vpss ch 0 to venc 0
    stSrcChn.enModId    = RK_ID_VPSS;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    stDestChn.enModId   = RK_ID_VENC;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = 0;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 band venc fail:%x", s32Ret);
        return -1;
    }

    //bind venc 0 to vdec 1
    stSrcChn.enModId    = RK_ID_VENC;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    stDestChn.enModId   = RK_ID_VDEC;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = VDEC_CH_CNT - 1;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 band venc fail:%x", s32Ret);
        return -1;
    }

    //bind vpss ch 1 to vo [0, 3] [1, 3]
    stSrcChn.enModId    = RK_ID_VPSS;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 1;
    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = VDEC_CH_CNT + 1;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 band vo fail:%x", s32Ret);
        return -1;
    }

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 1;
    stDestChn.s32ChnId  = VDEC_CH_CNT + 1;
    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 band vo fail:%x", s32Ret);
        return -1;
    }

    // enable vo
    s32Ret = RK_MPI_VO_EnableChn(0, VDEC_CH_CNT);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Enalbe vo %d chn failed, s32Ret = %d\n", VDEC_CH_CNT, s32Ret);
        return -1;
    }
    s32Ret = RK_MPI_VO_EnableChn(0, VDEC_CH_CNT + 1);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", VDEC_CH_CNT + 1, s32Ret);
        return -1;
    }
    s32Ret = RK_MPI_VO_EnableChn(1, VDEC_CH_CNT);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Enalbe vo %d chn failed, s32Ret = %d\n", VDEC_CH_CNT, s32Ret);
        return -1;
    }
    s32Ret = RK_MPI_VO_EnableChn(1, VDEC_CH_CNT + 1);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", VDEC_CH_CNT + 1, s32Ret);
        return -1;
    }

    //destory
    for (u32Ch = 0; u32Ch < VDEC_CH_CNT - 1; u32Ch++) {
        pthread_join(vdecThread[u32Ch], RK_NULL);
    }

    for (int i = 0; i < VDEC_CH_CNT; i++) {
        stSrcChn.enModId    = RK_ID_VDEC;
        stSrcChn.s32DevId   = 0;
        stSrcChn.s32ChnId   = i;
        stDestChn.enModId   = RK_ID_VO;
        stDestChn.s32DevId  = 0;
        stDestChn.s32ChnId  = i;
        s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vi unband vo fail:%x", s32Ret);
            return -1;
        }

        stDestChn.enModId   = RK_ID_VO;
        stDestChn.s32DevId  = 1;
        stDestChn.s32ChnId  = i;
        s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vi band vo fail:%x", s32Ret);
            return -1;
        }

        // disable vo
        s32Ret = RK_MPI_VO_DisableChn(0, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", s32Ret);
            return -1;
        }
        s32Ret = RK_MPI_VO_DisableChn(1, i);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", s32Ret);
            return -1;
        }
        mpi_destory_vdec(&vdecCtx[u32Ch], u32Ch);
    }

    //unbind vi[0] to vo [0, 2] [1, 2]
    stSrcChn.enModId    = RK_ID_VI;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = VDEC_CH_CNT;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi band vo fail:%x", s32Ret);
        return -1;
    }

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 1;
    stDestChn.s32ChnId  = VDEC_CH_CNT;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi unband vo fail:%x", s32Ret);
        return -1;
    }

    //bind vi [0, 0] to vpss grp 0
    stDestChn.enModId   = RK_ID_VPSS;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = 0;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi unband vpss fail:%x", s32Ret);
        return -1;
    }

    //bind vpss ch 0 to venc 0
    stSrcChn.enModId    = RK_ID_VPSS;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    stDestChn.enModId   = RK_ID_VENC;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = 0;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 unband venc fail:%x", s32Ret);
        return -1;
    }

    //unbind venc 0 to vdec 1
    stSrcChn.enModId    = RK_ID_VENC;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 0;
    stDestChn.enModId   = RK_ID_VDEC;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = VDEC_CH_CNT - 1;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 unband venc fail:%x", s32Ret);
        return -1;
    }

    //unbind vpss ch 1 to vo [0, 3] [1, 3]
    stSrcChn.enModId    = RK_ID_VPSS;
    stSrcChn.s32DevId   = 0;
    stSrcChn.s32ChnId   = 1;
    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 0;
    stDestChn.s32ChnId  = VDEC_CH_CNT + 1;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 unband vo fail:%x", s32Ret);
        return -1;
    }

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = 1;
    stDestChn.s32ChnId  = VDEC_CH_CNT + 1;
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vpss ch 0 unband vo fail:%x", s32Ret);
        return -1;
    }

    // enable vo
    s32Ret = RK_MPI_VO_DisableChn(0, VDEC_CH_CNT);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Disable vo %d chn failed, s32Ret = %d\n", VDEC_CH_CNT, s32Ret);
        return -1;
    }
    s32Ret = RK_MPI_VO_DisableChn(0, VDEC_CH_CNT + 1);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Disable vo chn failed, s32Ret = %d\n", VDEC_CH_CNT + 1, s32Ret);
        return -1;
    }
    s32Ret = RK_MPI_VO_DisableChn(1, VDEC_CH_CNT);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Disable vo %d chn failed, s32Ret = %d\n", VDEC_CH_CNT, s32Ret);
        return -1;
    }
    s32Ret = RK_MPI_VO_DisableChn(1, VDEC_CH_CNT + 1);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Disable vo chn failed, s32Ret = %d\n", VDEC_CH_CNT + 1, s32Ret);
        return -1;
    }

    destroy_vi();
    destory_vpss(0, 2);
    destory_venc();
    destory_vo();
    return RK_SUCCESS;
}

static void mpi_all_test_show_options(const TEST_ALL_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("video file name        : %s\n", ctx->srcFileUri);
    RK_PRINT("vdec width            : %d\n", ctx->u32SrcWidth);
    RK_PRINT("vdec height           : %d\n", ctx->u32SrcHeight);
    RK_PRINT("vdec codecId          : %d\n", ctx->enCodecId);
    RK_PRINT("loop count             : %d\n", ctx->s32LoopCount);
    RK_PRINT("aEntityName           : %s\n", ctx->aEntityName);
    RK_PRINT("enPixelFormat         : %d\n", ctx->enPixelFormat);
    RK_PRINT("vi width            : %d\n", ctx->u32SrcWidth1);
    RK_PRINT("vi height           : %d\n", ctx->u32SrcHeight1);
    return;
}

static const char *const usages[] = {
    "./rk_mpi_all_test -i /data/xxx.h264 -w 1920 -h 1080 -W 1920 -H 1080 -n /dev/vide20 -f 65543",
    NULL,
};

int main(int argc, const char **argv) {
    TEST_ALL_CTX_S ctx;
    memset(&ctx, 0, sizeof(TEST_ALL_CTX_S));
    ctx.s32LoopCount = 1;
    ctx.u32ReadSize = 1024;
    ctx.u32SrcWidth = 1920;
    ctx.u32SrcHeight = 1080;
    ctx.u32SrcWidth1 = 1920;
    ctx.u32SrcHeight1 = 1080;
    ctx.enCodecId = RK_VIDEO_ID_AVC;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input", &(ctx.srcFileUri),
                   "video file name. <required>", NULL, 0, 0),
        OPT_INTEGER('C', "codec", &(ctx.enCodecId),
                   "video stream codec(8:h264, 9:mjpeg, 12:h265,...) <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('w', "vdec_width", &(ctx.u32SrcWidth),
                    "video source width <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('h', "vdec_height", &(ctx.u32SrcHeight),
                    "video source height <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('l', "loop_count", &(ctx.s32LoopCount),
                    "loop running count. default(1)", NULL, 0, 0),
        OPT_INTEGER('W', "vi_width", &(ctx.u32SrcWidth1),
                    "video source width <required on StreamMode>", NULL, 0, 0),
        OPT_INTEGER('H', "vi_height", &(ctx.u32SrcHeight1),
                    "video source height <required on StreamMode>", NULL, 0, 0),
        OPT_STRING('n', "name", &(ctx.aEntityName),
                   "set the entityName (required, default null;\n\t"
                   "rv1126 sensor:rkispp_m_bypass rkispp_scale0 rkispp_scale1 rkispp_scale2;\n\t"
                   "rv1126 hdmiin/bt1120/sensor:/dev/videox such as /dev/video19 /dev/video20;\n\t"
                   "rk356x hdmiin/bt1120/sensor:/dev/videox such as /dev/video0 /dev/video1", NULL, 0, 0),     
        OPT_INTEGER('f', "format", &(ctx.enPixelFormat),
                  "set the format(default 0; 0:RK_FMT_YUV420SP 10:RK_FMT_YUV422_UYVY"
                   "131080:RK_FMT_RGB_BAYER_SBGGR_12BPP.....)", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_all_test_show_options(&ctx);

    if (check_options(&ctx)) {
        RK_LOGE("illegal input parameters");
        argparse_usage(&argparse);
        goto __FAILED;
    }

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        goto __FAILED;
    }

    while (ctx.s32LoopCount > 0) {
        if (unit_test_mpi_all(&ctx) < 0) {
            goto __FAILED;
        }
        ctx.s32LoopCount--;
    }

    if (RK_MPI_SYS_Exit() != RK_SUCCESS) {
        goto __FAILED;
    }

    RK_LOGE("test running success!");
    return RK_SUCCESS;
__FAILED:
    RK_LOGE("test running failed! %d count running done not yet.", ctx.s32LoopCount);
    pause();
    return RK_FAILURE;
}

