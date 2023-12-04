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

#include <stdio.h>
#include <sys/poll.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>

#include "rk_defines.h"
#include "rk_debug.h"
#include "rk_mpi_vi.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_venc.h"
#include "rk_mpi_vpss.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_rgn.h"
#include "rk_common.h"
#include "rk_comm_rgn.h"
#include "rk_comm_vi.h"
#include "rk_comm_vo.h"
#include "test_common.h"
#include "test_comm_utils.h"
#include "test_comm_argparse.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_mmz.h"

#define TEST_VENC_MAX 2
#define TEST_WITH_FD 0
#define TEST_WITH_FD_SWITCH 0

#undef DBG_MOD_ID
#define DBG_MOD_ID  RK_ID_VI

// for 356x vo
#define RK356X_VO_DEV_HD0 0
#define RK356X_VO_DEV_HD1 1
#define RK356X_VOP_LAYER_CLUSTER_0 0
#define RK356X_VOP_LAYER_CLUSTER_1 2
#define RK356X_VOP_LAYER_ESMART_0 4
#define RK356X_VOP_LAYER_ESMART_1 5
#define RK356X_VOP_LAYER_SMART_0 6
#define RK356X_VOP_LAYER_SMART_1 7

#define TEST_VI_SENSOR_NUM 6

typedef struct _rkTestVencCfg {
    RK_BOOL bOutDebugCfg;
    VENC_CHN_ATTR_S stAttr;
    RK_CHAR dstFilePath[128];
    RK_CHAR dstFileName[128];
    RK_S32 s32ChnId;
    FILE *fp;
    RK_S32 selectFd;
} TEST_VENC_CFG;

typedef struct rkVPSS_CFG_S {
    const char *dstFilePath;
    RK_S32 s32DevId;
    RK_S32 s32ChnId;
    RK_U32 u32VpssChnCnt;
    VPSS_GRP_ATTR_S stGrpVpssAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr[VPSS_MAX_CHN_NUM];
} VPSS_CFG_S;

typedef struct rkRGN_CFG_S {
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stRgnChnAttr;
} RGN_CFG_S;

typedef enum rkTestVIMODE_E {
    TEST_VI_MODE_VI_ONLY = 0,
    TEST_VI_MODE_BIND_VENC = 1,
    TEST_VI_MODE_BIND_VENC_MULTI = 2,
    TEST_VI_MODE_BIND_VPSS_BIND_VENC = 3,
    TEST_VI_MODE_BIND_VO = 4,
    TEST_VI_MODE_MUTI_VI = 5,
} TEST_VI_MODE_E;

typedef struct _rkMpiVICtx {
    RK_S32 width;
    RK_S32 height;
    RK_S32 devId;
    RK_S32 pipeId;
    RK_S32 channelId;
    RK_S32 loopCountSet;
    RK_S32 selectFd;
    RK_BOOL bFreeze;
    RK_BOOL bEnRgn;
    RK_S32 s32RgnCnt;
    RK_S32 rgnType;
    RK_BOOL bUserPicEnabled;
    RK_BOOL bGetConnecInfo;
    RK_BOOL bGetEdid;
    RK_BOOL bSetEdid;
    COMPRESS_MODE_E enCompressMode;
    VI_DEV_ATTR_S stDevAttr;
    VI_DEV_BIND_PIPE_S stBindPipe;
    VI_CHN_ATTR_S stChnAttr;
    VI_SAVE_FILE_INFO_S stDebugFile;
    VIDEO_FRAME_INFO_S stViFrame;
    VI_CHN_STATUS_S stChnStatus;
    VI_USERPIC_ATTR_S stUsrPic;
    TEST_VI_MODE_E enMode;
    const char *aEntityName;
    // for vi
    RGN_CFG_S stViRgn;
    // for venc
    TEST_VENC_CFG stVencCfg[TEST_VENC_MAX];
    VENC_STREAM_S stFrame[TEST_VENC_MAX];
    VPSS_CFG_S stVpssCfg;
    // for vo
    VO_LAYER s32VoLayer;
    VO_DEV s32VoDev;
} TEST_VI_CTX_S;

RK_U8 test_edid[2][128] = {
    {
        0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x49, 0x70, 0x88, 0x35, 0x01, 0x00, 0x00, 0x00,
        0x2d, 0x1f, 0x01, 0x03, 0x80, 0x78, 0x44, 0x78, 0x0a, 0xcf, 0x74, 0xa3, 0x57, 0x4c, 0xb0, 0x23,
        0x09, 0x48, 0x4c, 0x21, 0x08, 0x00, 0x61, 0x40, 0x01, 0x01, 0x81, 0x00, 0x95, 0x00, 0xa9, 0xc0,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0xe8, 0x00, 0x30, 0xf2, 0x70, 0x5a, 0x80, 0xb0, 0x58,
        0x8a, 0x00, 0xc4, 0x8e, 0x21, 0x00, 0x00, 0x1e, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40,
        0x58, 0x2c, 0x45, 0x00, 0xb9, 0xa8, 0x42, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x52,
        0x4b, 0x2d, 0x55, 0x48, 0x44, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd,
        0x00, 0x3b, 0x46, 0x1f, 0x8c, 0x3c, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0xa3
    },
    {
        0x02, 0x03, 0x39, 0xd2, 0x51, 0x07, 0x16, 0x14, 0x05, 0x01, 0x03, 0x12, 0x13, 0x84, 0x22, 0x1f,
        0x90, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x66, 0x03,
        0x0c, 0x00, 0x30, 0x00, 0x10, 0x67, 0xd8, 0x5d, 0xc4, 0x01, 0x78, 0xc8, 0x07, 0xe3, 0x05, 0x03,
        0x01, 0xe4, 0x0f, 0x00, 0xf0, 0x01, 0xe2, 0x00, 0xff, 0x08, 0xe8, 0x00, 0x30, 0xf2, 0x70, 0x5a,
        0x80, 0xb0, 0x58, 0x8a, 0x00, 0xc4, 0x8e, 0x21, 0x00, 0x00, 0x1e, 0x02, 0x3a, 0x80, 0x18, 0x71,
        0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0xb9, 0xa8, 0x42, 0x00, 0x00, 0x9e, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd1
    }
};

static RK_S32 create_vpss(VPSS_CFG_S *pstVpssCfg, RK_S32 s32Grp, RK_S32 s32OutChnNum) {
    RK_S32 s32Ret = RK_SUCCESS;
    VPSS_CHN VpssChn[VPSS_MAX_CHN_NUM] = { VPSS_CHN0, VPSS_CHN1, VPSS_CHN2, VPSS_CHN3 };
    VPSS_CROP_INFO_S stCropInfo;

    s32Ret = RK_MPI_VPSS_CreateGrp(s32Grp, &pstVpssCfg->stGrpVpssAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    for (RK_S32 i = 0; i < s32OutChnNum; i++) {
        s32Ret = RK_MPI_VPSS_SetChnAttr(s32Grp, VpssChn[i], &pstVpssCfg->stVpssChnAttr[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        s32Ret = RK_MPI_VPSS_EnableChn(s32Grp, VpssChn[i]);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
    }

    s32Ret = RK_MPI_VPSS_EnableBackupFrame(s32Grp);
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

static RK_S32 create_venc(TEST_VI_CTX_S *ctx, RK_U32 u32Ch) {
    VENC_RECV_PIC_PARAM_S stRecvParam;

    stRecvParam.s32RecvPicNum = ctx->loopCountSet;
    RK_MPI_VENC_CreateChn(u32Ch, &ctx->stVencCfg[u32Ch].stAttr);
    RK_MPI_VENC_StartRecvFrame(u32Ch, &stRecvParam);

    return RK_SUCCESS;
}

void init_venc_cfg(TEST_VI_CTX_S *ctx, RK_U32 u32Ch, RK_CODEC_ID_E enType) {
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.enType = enType;
    ctx->stVencCfg[u32Ch].s32ChnId = u32Ch;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.enPixelFormat = ctx->stChnAttr.enPixelFormat;
    ctx->stVencCfg[u32Ch].stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
    ctx->stVencCfg[u32Ch].stAttr.stRcAttr.stH264Cbr.u32Gop = 60;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32PicWidth = ctx->width;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32PicHeight = ctx->height;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32VirWidth = ctx->width;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32VirHeight = ctx->height;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32StreamBufCnt = 5;
    ctx->stVencCfg[u32Ch].stAttr.stVencAttr.u32BufSize = ctx->width * ctx->height * 3 / 2;
}

static RK_S32 create_vo(TEST_VI_CTX_S *ctx, RK_U32 u32Ch) {
    /* Enable VO */
    VO_PUB_ATTR_S VoPubAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    RK_S32 s32Ret = RK_SUCCESS;
    VO_CHN_ATTR_S stChnAttr;
    VO_LAYER VoLayer = ctx->s32VoLayer;
    VO_DEV VoDev = ctx->s32VoDev;

    RK_MPI_VO_DisableLayer(VoLayer);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_ESMART_0);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_ESMART_1);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_SMART_0);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_SMART_1);
    RK_MPI_VO_Disable(VoDev);

    memset(&VoPubAttr, 0, sizeof(VO_PUB_ATTR_S));
    memset(&stLayerAttr, 0, sizeof(VO_VIDEO_LAYER_ATTR_S));

    stLayerAttr.enPixFormat = RK_FMT_RGB888;
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.u32DispFrmRt = 30;
    stLayerAttr.stDispRect.u32Width = 1920;
    stLayerAttr.stDispRect.u32Height = 1080;
    stLayerAttr.stImageSize.u32Width = 1920;
    stLayerAttr.stImageSize.u32Height = 1080;

    s32Ret = RK_MPI_VO_GetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    VoPubAttr.enIntfType = VO_INTF_HDMI;
    VoPubAttr.enIntfSync = VO_OUTPUT_DEFAULT;

    s32Ret = RK_MPI_VO_SetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_Enable(VoDev);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerAttr failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_BindLayer(VoLayer, VoDev, VO_LAYER_MODE_GRAPHIC);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_BindLayer failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }


    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_EnableLayer failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }

    stChnAttr.stRect.s32X = 0;
    stChnAttr.stRect.s32Y = 0;
    stChnAttr.stRect.u32Width = stLayerAttr.stImageSize.u32Width;
    stChnAttr.stRect.u32Height = stLayerAttr.stImageSize.u32Height;
    stChnAttr.u32Priority = 0;
    stChnAttr.u32FgAlpha = 128;
    stChnAttr.u32BgAlpha = 0;

    s32Ret = RK_MPI_VO_SetChnAttr(VoLayer, u32Ch, &stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("set chn Attr failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }

    return s32Ret;
}

RK_S32 test_vi_poll_event(RK_S32 timeoutMsec, RK_S32 fd) {
    RK_S32 num_fds = 1;
    struct pollfd pollFds[num_fds];
    RK_S32 ret = 0;

    RK_ASSERT(fd > 0);
    memset(pollFds, 0, sizeof(pollFds));
    pollFds[0].fd = fd;
    pollFds[0].events = (POLLPRI | POLLIN | POLLERR | POLLNVAL | POLLHUP);

    ret = poll(pollFds, num_fds, timeoutMsec);

    if (ret > 0 && (pollFds[0].revents & (POLLERR | POLLNVAL | POLLHUP))) {
        RK_LOGE("fd:%d polled error", fd);
        return -1;
    }

    return ret;
}

static RK_S32 readFromPic(TEST_VI_CTX_S *ctx, VIDEO_FRAME_S *buffer) {
    FILE            *fp    = NULL;
    RK_S32          s32Ret = RK_SUCCESS;
    MB_BLK          srcBlk = MB_INVALID_HANDLE;
    PIC_BUF_ATTR_S  stPicBufAttr;
    MB_PIC_CAL_S    stMbPicCalResult;
    const char      *user_picture_path = "/data/test_vi_user.yuv";

    stPicBufAttr.u32Width = ctx->width;
    stPicBufAttr.u32Height = ctx->height;
    stPicBufAttr.enCompMode = COMPRESS_MODE_NONE;
    stPicBufAttr.enPixelFormat = RK_FMT_YUV420SP;
    s32Ret = RK_MPI_CAL_VGS_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
        return RK_NULL;
    }

    RK_MPI_MMZ_Alloc(&srcBlk, stMbPicCalResult.u32MBSize, RK_MMZ_ALLOC_CACHEABLE);

    fp = fopen(user_picture_path, "rb");
    if (NULL == fp) {
        RK_LOGE("open %s fail", user_picture_path);
        return RK_FAILURE;
    } else {
        fread(RK_MPI_MB_Handle2VirAddr(srcBlk), 1 , stMbPicCalResult.u32MBSize, fp);
        fclose(fp);
        RK_MPI_SYS_MmzFlushCache(srcBlk, RK_FALSE);
        RK_LOGD("open %s success", user_picture_path);
    }

    buffer->u32Width = ctx->width;
    buffer->u32Height = ctx->height;
    buffer->u32VirWidth = ctx->width;
    buffer->u32VirHeight = ctx->height;
    buffer->enPixelFormat = RK_FMT_YUV420SP;
    buffer->u32TimeRef = 0;
    buffer->u64PTS = 0;
    buffer->enCompressMode = COMPRESS_MODE_NONE;
    buffer->pMbBlk = srcBlk;

    RK_LOGD("readFromPic width = %d, height = %d size = %d pixFormat = %d",
            ctx->width, ctx->height, stMbPicCalResult.u32MBSize, RK_FMT_YUV420SP);
    return RK_SUCCESS;
}

static RK_S32 create_rgn(TEST_VI_CTX_S *ctx) {
    RK_S32 i;
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stRgnChnAttr;
    RGN_HANDLE RgnHandle = 0;
    MPP_CHN_S stMppChn;
    RGN_TYPE_E rgnType = (RGN_TYPE_E)ctx->rgnType;

    /****************************************
     step 1: create overlay regions
    ****************************************/
    for (i = 0; i < ctx->s32RgnCnt; i++) {
        ctx->stViRgn.stRgnAttr.unAttr.stOverlay.u32ClutNum = 0;
        ctx->stViRgn.stRgnAttr.unAttr.stOverlay.stSize.u32Width = 128;
        ctx->stViRgn.stRgnAttr.unAttr.stOverlay.stSize.u32Height = 128;
        RgnHandle = i;
        s32Ret = RK_MPI_RGN_Create(RgnHandle, &ctx->stViRgn.stRgnAttr);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_Create (%d) failed with %#x!", RgnHandle, s32Ret);
            RK_MPI_RGN_Destroy(RgnHandle);
            return RK_FAILURE;
        }
        RK_LOGI("The handle: %d, create success!", RgnHandle);
    }

    /*********************************************
     step 2: display overlay regions to vi
    *********************************************/
    for (i = 0; i < ctx->s32RgnCnt; i++) {
        stMppChn.enModId = RK_ID_VI;
        stMppChn.s32DevId = ctx->devId;
        stMppChn.s32ChnId = ctx->channelId;
        RgnHandle = i;

        memset(&stRgnChnAttr, 0, sizeof(stRgnChnAttr));
        stRgnChnAttr.bShow = RK_TRUE;
        stRgnChnAttr.enType = rgnType;

        BITMAP_S stBitmap;
        switch (rgnType) {
            case COVER_RGN: {
                RGN_CHN_ATTR_S stCoverChnAttr;
                stCoverChnAttr.bShow = RK_TRUE;
                stCoverChnAttr.enType = COVER_RGN;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = 128 * i;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 128 * i;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 128;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 128;
                stCoverChnAttr.unChnAttr.stCoverChn.u32Color = 0xffffff;  // white
                stCoverChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
                stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = i;

                s32Ret = RK_MPI_RGN_AttachToChn(RgnHandle, &stMppChn, &stCoverChnAttr);
                if (RK_SUCCESS != s32Ret) {
                    RK_LOGE("failed with %#x!", s32Ret);
                    goto __EXIT;
                }
                s32Ret = RK_MPI_RGN_GetDisplayAttr(RgnHandle, &stMppChn, &stCoverChnAttr);
                if (RK_SUCCESS != s32Ret) {
                    RK_LOGE("failed with %#x!", s32Ret);
                    goto __EXIT;
                }
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = 128 * i;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = 128 * i;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = 128;
                stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = 128;
                stCoverChnAttr.unChnAttr.stCoverChn.u32Color = 0x0000ff;  // blue
                stCoverChnAttr.unChnAttr.stCoverChn.enCoordinate = RGN_ABS_COOR;
                stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = i;
                // change cover channel attribute below.
                s32Ret = RK_MPI_RGN_SetDisplayAttr(RgnHandle, &stMppChn, &stCoverChnAttr);
                if (RK_SUCCESS != s32Ret) {
                    RK_LOGE("failed with %#x!", s32Ret);
                    goto __EXIT;
                }

                RK_LOGI("the cover region:%d to <%d, %d, %d, %d>",
                        RgnHandle,
                        stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X,
                        stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y,
                        stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width,
                        stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height);
            } break;
            case MOSAIC_RGN: {
                RGN_CHN_ATTR_S stMoscaiChnAttr;
                stMoscaiChnAttr.bShow = RK_TRUE;
                stMoscaiChnAttr.enType = MOSAIC_RGN;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 128 * i;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 128 * i;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.u32Width = 128;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 128;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.u32Layer = i;

                s32Ret = RK_MPI_RGN_AttachToChn(RgnHandle, &stMppChn, &stMoscaiChnAttr);
                if (RK_SUCCESS != s32Ret) {
                    RK_LOGE("failed with %#x!", s32Ret);
                    goto __EXIT;
                }
                s32Ret = RK_MPI_RGN_GetDisplayAttr(RgnHandle, &stMppChn, &stMoscaiChnAttr);
                if (RK_SUCCESS != s32Ret) {
                    RK_LOGE("failed with %#x!", s32Ret);
                    goto __EXIT;
                }

                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.s32X = 128 * i;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.s32Y = 128 * i;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.u32Width = 128;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.u32Height = 128;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.enBlkSize = MOSAIC_BLK_SIZE_8;
                stMoscaiChnAttr.unChnAttr.stMosaicChn.u32Layer = i;

                // change mosaic channel attribute below.
                s32Ret = RK_MPI_RGN_SetDisplayAttr(RgnHandle, &stMppChn, &stMoscaiChnAttr);
                if (RK_SUCCESS != s32Ret) {
                    RK_LOGE("failed with %#x!", s32Ret);
                    goto __EXIT;
                }
                RK_LOGI("the mosaic region:%d to <%d, %d, %d, %d>",
                        RgnHandle,
                        stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.s32X,
                        stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.s32Y,
                        stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.u32Width,
                        stMoscaiChnAttr.unChnAttr.stMosaicChn.stRect.u32Height);
            } break;
            default:
                break;
        }
    }

    return RK_SUCCESS;
__EXIT:
    for (i = 0; i < ctx->s32RgnCnt; i++) {
        stMppChn.enModId = RK_ID_VI;
        stMppChn.s32DevId = ctx->devId;
        stMppChn.s32ChnId = ctx->channelId;
        RgnHandle = i;
        s32Ret = RK_MPI_RGN_DetachFromChn(RgnHandle, &stMppChn);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_DetachFrmChn (%d) failed with %#x!", RgnHandle, s32Ret);
            return RK_FAILURE;
        }
    }
    return RK_FAILURE;
}

static RK_S32 destory_rgn(TEST_VI_CTX_S *ctx) {
    RK_S32 i;
    RK_S32 s32Ret = RK_SUCCESS;
    RGN_HANDLE RgnHandle = 0;

    for (RK_S32 i = 0; i < ctx->s32RgnCnt; i++) {
        RgnHandle = i;
        s32Ret = RK_MPI_RGN_Destroy(RgnHandle);
        if (RK_SUCCESS != s32Ret) {
            RK_LOGE("RK_MPI_RGN_Destroy (%d) failed with %#x!", RgnHandle, s32Ret);
        }
        RK_LOGI("Destory handle:%d success", RgnHandle);
    }

    RK_LOGD("vi RK_MPI_RGN_Destroy OK");
    return RK_SUCCESS;
}

static RK_S32 test_vi_hdmi_rx_infomation(TEST_VI_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;

    /* test get connect info before enable chn(need after RK_MPI_VI_SetChnAttr)*/
    if (ctx->bGetConnecInfo) {
        VI_CONNECT_INFO_S stConnectInfo;
        s32Ret = RK_MPI_VI_GetChnConnectInfo(ctx->pipeId, ctx->channelId, &stConnectInfo);
        RK_LOGE("RK_MPI_VI_GetChnConnectInfo %x, w:%d,h:%d,fmt:0x%x,connect:%d", s32Ret,
                 stConnectInfo.u32Width, stConnectInfo.u32Height,
                 stConnectInfo.enPixFmt, stConnectInfo.enConnect);
    }
    if (ctx->bGetEdid) {
        VI_EDID_S stEdid;
        memset(&stEdid, 0, sizeof(VI_EDID_S));
        stEdid.u32Blocks = 3;
        stEdid.pu8Edid = (RK_U8 *)calloc(128 * stEdid.u32Blocks, 1);
        s32Ret = RK_MPI_VI_GetChnEdid(ctx->pipeId, ctx->channelId, &stEdid);
        RK_LOGE("RK_MPI_VI_GetChnEdid %x, stEdid.u32Blocks=%d "
                 "edid[0]:0x%x,edid[1]:0x%x,edid[2]:0x%x,edid[3]:0x%x,edid[126]:0x%x edid[127]:0x%x",
                 s32Ret, stEdid.u32Blocks,
                 stEdid.pu8Edid[0], stEdid.pu8Edid[1],
                 stEdid.pu8Edid[2], stEdid.pu8Edid[3],
                 stEdid.pu8Edid[126], stEdid.pu8Edid[127]);
        free(stEdid.pu8Edid);
    }
    if (ctx->bSetEdid) {
        VI_EDID_S stEdid;
        memset(&stEdid, 0, sizeof(VI_EDID_S));
        stEdid.u32Blocks = 2;
        stEdid.pu8Edid = (RK_U8 *)test_edid;
        s32Ret = RK_MPI_VI_SetChnEdid(ctx->pipeId, ctx->channelId, &stEdid);
        RK_LOGE("RK_MPI_VI_SetChnEdid %x, edid[0]:0x%x,edid[1]:0x%x,edid[2]:0x%x,edid[3]:0x%x,"
                "edid[126]:0x%x edid[127]:0x%x",
                 s32Ret,
                 stEdid.pu8Edid[0], stEdid.pu8Edid[1],
                 stEdid.pu8Edid[2], stEdid.pu8Edid[3],
                 stEdid.pu8Edid[126], stEdid.pu8Edid[127]);
    }

    return s32Ret;
}


static RK_S32 test_vi_init(TEST_VI_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;

    // 0. get dev config status
    s32Ret = RK_MPI_VI_GetDevAttr(ctx->devId, &ctx->stDevAttr);
    if (s32Ret == RK_ERR_VI_NOT_CONFIG) {
        // 0-1.config dev
        s32Ret = RK_MPI_VI_SetDevAttr(ctx->devId, &ctx->stDevAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevAttr %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_SetDevAttr already");
    }
    // 1.get  dev enable status
    s32Ret = RK_MPI_VI_GetDevIsEnable(ctx->devId);
    if (s32Ret != RK_SUCCESS) {
        // 1-2.enable dev
        s32Ret = RK_MPI_VI_EnableDev(ctx->devId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_EnableDev %x", s32Ret);
            goto __FAILED;
        }
        // 1-3.bind dev/pipe
        ctx->stBindPipe.u32Num = ctx->pipeId;
        ctx->stBindPipe.PipeId[0] = ctx->pipeId;
        s32Ret = RK_MPI_VI_SetDevBindPipe(ctx->devId, &ctx->stBindPipe);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevBindPipe %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_EnableDev already");
    }
    // 2.config channel
    ctx->stChnAttr.stSize.u32Width = ctx->width;
    ctx->stChnAttr.stSize.u32Height = ctx->height;
    ctx->stChnAttr.enCompressMode = ctx->enCompressMode;
    s32Ret = RK_MPI_VI_SetChnAttr(ctx->pipeId, ctx->channelId, &ctx->stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_SetChnAttr %x", s32Ret);
        goto __FAILED;
    }

    /* test user picture */
    if (ctx->bUserPicEnabled) {
        ctx->stUsrPic.enUsrPicMode = VI_USERPIC_MODE_BGC;
        // ctx->stUsrPic.enUsrPicMode = VI_USERPIC_MODE_PIC;

        if (ctx->stUsrPic.enUsrPicMode == VI_USERPIC_MODE_PIC) {
            s32Ret = readFromPic(ctx, &ctx->stUsrPic.unUsrPic.stUsrPicFrm.stVFrame);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("RK_MPI_VI_SetUserPic fail:%x", s32Ret);
                goto __FAILED;
            }
        } else if (ctx->stUsrPic.enUsrPicMode == VI_USERPIC_MODE_BGC) {
            /* set background color */
            ctx->stUsrPic.unUsrPic.stUsrPicBg.u32BgColor = RGB(0, 0, 128);
        }

        s32Ret = RK_MPI_VI_SetUserPic(ctx->pipeId, ctx->channelId, &ctx->stUsrPic);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetUserPic fail:%x", s32Ret);
            goto __FAILED;
        }

        s32Ret = RK_MPI_VI_EnableUserPic(ctx->pipeId, ctx->channelId);
    }

    test_vi_hdmi_rx_infomation(ctx);

    ctx->stViRgn.stRgnAttr.enType = (RGN_TYPE_E)ctx->rgnType;
    ctx->stViRgn.stRgnChnAttr.bShow = RK_TRUE;

    // open fd before enable chn will be better
#if TEST_WITH_FD
     ctx->selectFd = RK_MPI_VI_GetChnFd(ctx->pipeId, ctx->channelId);
     RK_LOGE("ctx->pipeId=%d, ctx->channelId=%d, ctx->selectFd:%d ", ctx->pipeId, ctx->channelId, ctx->selectFd);
#endif
    // 3.enable channel
    s32Ret = RK_MPI_VI_EnableChn(ctx->pipeId, ctx->channelId);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_EnableChn %x", s32Ret);
        goto __FAILED;
    }

    if (ctx->bEnRgn) {
        s32Ret = create_rgn(ctx);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Error:create rgn failed!");
            goto __FAILED;
        }
    }

    // 4.save debug file
    if (ctx->stDebugFile.bCfg) {
        s32Ret = RK_MPI_VI_ChnSaveFile(ctx->pipeId, ctx->channelId, &ctx->stDebugFile);
        RK_LOGD("RK_MPI_VI_ChnSaveFile %x", s32Ret);
    }

__FAILED:
    return s32Ret;
}

static RK_S32 test_vi_bind_vo_loop(TEST_VI_CTX_S *ctx) {
    MPP_CHN_S stSrcChn, stDestChn;
    RK_S32 loopCount = 0;
    void *pData = RK_NULL;
    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 i;

    s32Ret = test_vi_init(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi %d:%d init failed:%x", ctx->devId, ctx->channelId, s32Ret);
        goto __FAILED;
    }

    // vo  init and create
    ctx->s32VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
    ctx->s32VoDev = RK356X_VO_DEV_HD0;
    s32Ret = create_vo(ctx, 0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create vo ch:%d failed", ctx->channelId);
        goto __FAILED;
    }
    // bind vi to vo
    stSrcChn.enModId    = RK_ID_VI;
    stSrcChn.s32DevId   = ctx->devId;
    stSrcChn.s32ChnId   = ctx->channelId;

    stDestChn.enModId   = RK_ID_VO;
    stDestChn.s32DevId  = ctx->s32VoLayer;
    stDestChn.s32ChnId  = 0;

    s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi band vo fail:%x", s32Ret);
        goto __FAILED;
    }

    // enable vo
    s32Ret = RK_MPI_VO_EnableChn(ctx->s32VoLayer, 0);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Enalbe vo chn failed, s32Ret = %d\n", s32Ret);
        goto __FAILED;
    }

    while (loopCount < ctx->loopCountSet) {
        loopCount++;
        //RK_LOGE("loopCount:%d", loopCount);
        // can not get the vo frameout count . so here regard as 33ms one frame.
        usleep(33*1000);
    }

__FAILED:
    s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_SYS_UnBind fail %x", s32Ret);
    }
    // disable vo
    RK_MPI_VO_DisableLayer(ctx->s32VoLayer);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_ESMART_0);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_ESMART_1);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_SMART_0);
    RK_MPI_VO_DisableLayer(RK356X_VOP_LAYER_SMART_1);
    RK_MPI_VO_DisableChn(ctx->s32VoLayer, 0);
    RK_MPI_VO_Disable(ctx->s32VoDev);

    // 5. disable one chn
    s32Ret = RK_MPI_VI_DisableChn(ctx->pipeId, ctx->channelId);
    RK_LOGE("RK_MPI_VI_DisableChn %x", s32Ret);

    RK_MPI_VO_DisableChn(ctx->s32VoLayer, 0);

    // 6.disable dev(will diabled all chn)
    s32Ret = RK_MPI_VI_DisableDev(ctx->devId);
    RK_LOGE("RK_MPI_VI_DisableDev %x", s32Ret);

    return s32Ret;
}

static RK_S32 test_vi_bind_vpss_venc_loop(TEST_VI_CTX_S *ctx) {
    MPP_CHN_S stViChn, stVencChn[TEST_VENC_MAX], stVpssChn;
    RK_S32 loopCount = 0;
    void *pData = RK_NULL;
    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 i;
    RK_U32 u32DstCount = 1;

    s32Ret = test_vi_init(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi %d:%d init failed:%x", ctx->devId, ctx->channelId, s32Ret);
        goto __FAILED;
    }

    /* vpss */
    ctx->stVpssCfg.u32VpssChnCnt = 1;
    ctx->stVpssCfg.stGrpVpssAttr.u32MaxW = 4096;
    ctx->stVpssCfg.stGrpVpssAttr.u32MaxH = 4096;
    ctx->stVpssCfg.stGrpVpssAttr.enPixelFormat = RK_FMT_YUV420SP;
    ctx->stVpssCfg.stGrpVpssAttr.stFrameRate.s32SrcFrameRate = -1;
    ctx->stVpssCfg.stGrpVpssAttr.stFrameRate.s32DstFrameRate = -1;
    ctx->stVpssCfg.stGrpVpssAttr.enCompressMode = COMPRESS_MODE_NONE;
    for (i = 0; i < VPSS_MAX_CHN_NUM; i ++) {
        ctx->stVpssCfg.stVpssChnAttr[i].enChnMode = VPSS_CHN_MODE_PASSTHROUGH;
        ctx->stVpssCfg.stVpssChnAttr[i].enDynamicRange = DYNAMIC_RANGE_SDR8;
        ctx->stVpssCfg.stVpssChnAttr[i].enPixelFormat = RK_FMT_YUV420SP;
        ctx->stVpssCfg.stVpssChnAttr[i].stFrameRate.s32SrcFrameRate = -1;
        ctx->stVpssCfg.stVpssChnAttr[i].stFrameRate.s32DstFrameRate = -1;
        ctx->stVpssCfg.stVpssChnAttr[i].u32Width = ctx->width;
        ctx->stVpssCfg.stVpssChnAttr[i].u32Height = ctx->height;
        ctx->stVpssCfg.stVpssChnAttr[i].enCompressMode = COMPRESS_MODE_NONE;
    }

    // init vpss
    s32Ret = create_vpss(&ctx->stVpssCfg, 0, ctx->stVpssCfg.u32VpssChnCnt);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("creat 0 grp vpss failed!");
        goto __FAILED;
    }
    // bind vi to vpss
    stViChn.enModId    = RK_ID_VI;
    stViChn.s32DevId   = ctx->devId;
    stViChn.s32ChnId   = ctx->channelId;
    stVpssChn.enModId = RK_ID_VPSS;
    stVpssChn.s32DevId = 0;
    stVpssChn.s32ChnId = 0;

    RK_LOGD("vi to vpss ch %d vpss group %d", stVpssChn.s32ChnId , stVpssChn.s32DevId);
    s32Ret = RK_MPI_SYS_Bind(&stViChn, &stVpssChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi and vpss bind error ");
        goto __FAILED;
    }

    /* venc */
    for (i = 0; i < u32DstCount; i++) {
        // venc  init and create
        init_venc_cfg(ctx, i, RK_VIDEO_ID_AVC);
        s32Ret = create_venc(ctx, ctx->stVencCfg[i].s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("create %d ch venc failed", ctx->stVencCfg[i].s32ChnId);
            return s32Ret;
        }
        // bind vpss to venc
        stVencChn[i].enModId   = RK_ID_VENC;
        stVencChn[i].s32DevId  = i;
        stVencChn[i].s32ChnId  = ctx->stVencCfg[i].s32ChnId;

        s32Ret = RK_MPI_SYS_Bind(&stVpssChn, &stVencChn[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("create %d ch venc failed", ctx->stVencCfg[i].s32ChnId);
            goto __FAILED;
        }
        ctx->stFrame[i].pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));
#if TEST_WITH_FD
        ctx->stVencCfg[i].selectFd = RK_MPI_VENC_GetFd(ctx->stVencCfg[i].s32ChnId);
        RK_LOGE("venc chn:%d, ctx->selectFd:%d ", ctx->stVencCfg[i].s32ChnId, ctx->stVencCfg[i].selectFd);
#endif
    }

    while (loopCount < ctx->loopCountSet) {
        for (i = 0; i < u32DstCount; i++) {
#if TEST_WITH_FD
            test_vi_poll_event(-1, ctx->stVencCfg[i].selectFd);
#endif

            // freeze test
            RK_MPI_VI_SetChnFreeze(ctx->pipeId, ctx->channelId, ctx->bFreeze);

            s32Ret = RK_MPI_VENC_GetStream(ctx->stVencCfg[i].s32ChnId, &ctx->stFrame[i], -1);
            if (s32Ret == RK_SUCCESS) {
                if (ctx->stVencCfg[i].bOutDebugCfg) {
                    pData = RK_MPI_MB_Handle2VirAddr(ctx->stFrame[i].pstPack->pMbBlk);
                    fwrite(pData, 1, ctx->stFrame[i].pstPack->u32Len, ctx->stVencCfg[i].fp);
                    fflush(ctx->stVencCfg[i].fp);
                }
                RK_LOGD("chn:%d, loopCount:%d enc->seq:%d wd:%d\n", i, loopCount,
                         ctx->stFrame[i].u32Seq, ctx->stFrame[i].pstPack->u32Len);
                s32Ret = RK_MPI_VENC_ReleaseStream(ctx->stVencCfg[i].s32ChnId, &ctx->stFrame[i]);
                if (s32Ret != RK_SUCCESS) {
                    RK_LOGE("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
                }
                loopCount++;
            } else {
                RK_LOGE("RK_MPI_VI_GetChnFrame fail %x", s32Ret);
            }
        }
        usleep(10*1000);
    }

__FAILED:
    // unbind vi->vpss
    s32Ret = RK_MPI_SYS_UnBind(&stViChn, &stVpssChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_SYS_UnBind fail %x", s32Ret);
    }
    // unbind vpss->venc
    for (i = 0; i < u32DstCount; i++) {
        s32Ret = RK_MPI_SYS_UnBind(&stVpssChn, &stVencChn[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_SYS_UnBind fail %x", s32Ret);
        }
    }
    // destory vpss
    s32Ret = destory_vpss(0, ctx->stVpssCfg.u32VpssChnCnt);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("destory vpss error");
        return s32Ret;
    }
    // 5. disable one chn
    s32Ret = RK_MPI_VI_DisableChn(ctx->pipeId, ctx->channelId);
    RK_LOGE("RK_MPI_VI_DisableChn %x", s32Ret);

    for (i = 0; i < u32DstCount; i++) {
        s32Ret = RK_MPI_VENC_StopRecvFrame(ctx->stVencCfg[i].s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        RK_LOGE("destroy enc chn:%d", ctx->stVencCfg[i].s32ChnId);
        s32Ret = RK_MPI_VENC_DestroyChn(ctx->stVencCfg[i].s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VDEC_DestroyChn fail %x", s32Ret);
        }
    }

    // 6.disable dev(will diabled all chn)
    s32Ret = RK_MPI_VI_DisableDev(ctx->devId);
    RK_LOGE("RK_MPI_VI_DisableDev %x", s32Ret);
    for (i = 0; i < u32DstCount; i++) {
      if (ctx->stFrame[i].pstPack)
          free(ctx->stFrame[i].pstPack);
      if (ctx->stVencCfg[i].fp)
          fclose(ctx->stVencCfg[i].fp);
    }
    return s32Ret;
}

static RK_S32 test_vi_bind_venc_loop(TEST_VI_CTX_S *ctx) {
    MPP_CHN_S stSrcChn, stDestChn[TEST_VENC_MAX];
    RK_S32 loopCount = 0;
    void *pData = RK_NULL;
    RK_S32 s32Ret = RK_FAILURE;
    RK_U32 i;
    RK_U32 u32DstCount = ((ctx->enMode == TEST_VI_MODE_BIND_VENC_MULTI) ? 2 : 1);

    s32Ret = test_vi_init(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi %d:%d init failed:%x", ctx->devId, ctx->channelId, s32Ret);
        goto __FAILED;
    }

    /* venc */
    for (i = 0; i < u32DstCount; i++) {
        // venc  init and create
        init_venc_cfg(ctx, i, RK_VIDEO_ID_AVC);
        s32Ret = create_venc(ctx, ctx->stVencCfg[i].s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("create %d ch venc failed", ctx->stVencCfg[i].s32ChnId);
            return s32Ret;
        }
         // bind vi to venc
        stSrcChn.enModId    = RK_ID_VI;
        stSrcChn.s32DevId   = ctx->devId;
        stSrcChn.s32ChnId   = ctx->channelId;

        stDestChn[i].enModId   = RK_ID_VENC;
        stDestChn[i].s32DevId  = i;
        stDestChn[i].s32ChnId  = ctx->stVencCfg[i].s32ChnId;

        s32Ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("create %d ch venc failed", ctx->stVencCfg[i].s32ChnId);
            goto __FAILED;
        }
        ctx->stFrame[i].pstPack = reinterpret_cast<VENC_PACK_S *>(malloc(sizeof(VENC_PACK_S)));
#if TEST_WITH_FD
        ctx->stVencCfg[i].selectFd = RK_MPI_VENC_GetFd(ctx->stVencCfg[i].s32ChnId);
        RK_LOGE("venc chn:%d, ctx->selectFd:%d ", ctx->stVencCfg[i].s32ChnId, ctx->stVencCfg[i].selectFd);
#endif
    }

    while (loopCount < ctx->loopCountSet) {
        for (i = 0; i < u32DstCount; i++) {
#if TEST_WITH_FD
            test_vi_poll_event(-1, ctx->stVencCfg[i].selectFd);
#endif
            // freeze test
            RK_MPI_VI_SetChnFreeze(ctx->pipeId, ctx->channelId, ctx->bFreeze);

            s32Ret = RK_MPI_VENC_GetStream(ctx->stVencCfg[i].s32ChnId, &ctx->stFrame[i], -1);
            if (s32Ret == RK_SUCCESS) {
                if (ctx->stVencCfg[i].bOutDebugCfg) {
                    pData = RK_MPI_MB_Handle2VirAddr(ctx->stFrame[i].pstPack->pMbBlk);
                    fwrite(pData, 1, ctx->stFrame[i].pstPack->u32Len, ctx->stVencCfg[i].fp);
                    fflush(ctx->stVencCfg[i].fp);
                }
                RK_U64 nowUs = TEST_COMM_GetNowUs();

                RK_LOGD("chn:%d, loopCount:%d enc->seq:%d wd:%d pts=%lld delay=%lldus\n", i, loopCount,
                         ctx->stFrame[i].u32Seq, ctx->stFrame[i].pstPack->u32Len,
                         ctx->stFrame[i].pstPack->u64PTS,
                         nowUs - ctx->stFrame[i].pstPack->u64PTS);
                s32Ret = RK_MPI_VENC_ReleaseStream(ctx->stVencCfg[i].s32ChnId, &ctx->stFrame[i]);
                if (s32Ret != RK_SUCCESS) {
                    RK_LOGE("RK_MPI_VENC_ReleaseStream fail %x", s32Ret);
                }
                loopCount++;
            } else {
                RK_LOGE("RK_MPI_VI_GetChnFrame fail %x", s32Ret);
            }
        }
        usleep(10*1000);
    }

__FAILED:
    for (i = 0; i < u32DstCount; i++) {
        s32Ret = RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_SYS_UnBind fail %x", s32Ret);
        }
    }
    // 5. disable one chn
    s32Ret = RK_MPI_VI_DisableChn(ctx->pipeId, ctx->channelId);
    RK_LOGE("RK_MPI_VI_DisableChn %x", s32Ret);

    for (i = 0; i < u32DstCount; i++) {
        s32Ret = RK_MPI_VENC_StopRecvFrame(ctx->stVencCfg[i].s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            return s32Ret;
        }
        RK_LOGE("destroy enc chn:%d", ctx->stVencCfg[i].s32ChnId);
        s32Ret = RK_MPI_VENC_DestroyChn(ctx->stVencCfg[i].s32ChnId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VDEC_DestroyChn fail %x", s32Ret);
        }
    }

    // 6.disable dev(will diabled all chn)
    s32Ret = RK_MPI_VI_DisableDev(ctx->devId);
    RK_LOGE("RK_MPI_VI_DisableDev %x", s32Ret);
    for (i = 0; i < u32DstCount; i++) {
      if (ctx->stFrame[i].pstPack)
          free(ctx->stFrame[i].pstPack);
      if (ctx->stVencCfg[i].fp)
          fclose(ctx->stVencCfg[i].fp);
    }
    return s32Ret;
}

static RK_S32 test_vi_get_release_frame_loop(TEST_VI_CTX_S *ctx) {
    RK_S32 s32Ret;
    RK_S32 loopCount = 0;
    RK_S32 waitTime = 33;

    /* test use getframe&release_frame */
    s32Ret = test_vi_init(ctx);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("vi %d:%d init failed:%x", ctx->devId, ctx->channelId, s32Ret);
        goto __FAILED;
    }

    while (loopCount < ctx->loopCountSet) {
#if TEST_WITH_FD_SWITCH
        if (loopCount % 10 == 0 && ctx->selectFd != -1) {  // test close/reopen the fd
            RK_MPI_VI_CloseChnFd(ctx->pipeId, ctx->channelId);
            RK_LOGE("close ctx->pipeId=%d, ctx->channelId=%d, ctx->selectFd:%d",
                     ctx->pipeId, ctx->channelId, ctx->selectFd);
            ctx->selectFd = -1;
        } else {
            if (ctx->selectFd < 0) {
                ctx->selectFd = RK_MPI_VI_GetChnFd(ctx->pipeId, ctx->channelId);
                RK_LOGE("regetfd ctx->pipeId=%d, ctx->channelId=%d, ctx->selectFd:%d",
                         ctx->pipeId, ctx->channelId, ctx->selectFd);
                // do not use non-block polling for the first time after switching fd
                test_vi_poll_event(33, ctx->selectFd);
            } else {
                test_vi_poll_event(-1, ctx->selectFd);
            }
        }
#elif TEST_WITH_FD
        test_vi_poll_event(-1, ctx->selectFd);
#endif

        /* test user picture */
        if (loopCount > 5 && ctx->bUserPicEnabled) {
            ctx->bUserPicEnabled = RK_FALSE;
            RK_MPI_VI_DisableUserPic(ctx->pipeId, ctx->channelId);
            if (ctx->stUsrPic.enUsrPicMode == VI_USERPIC_MODE_PIC &&
                ctx->stUsrPic.unUsrPic.stUsrPicFrm.stVFrame.pMbBlk) {
                RK_MPI_MMZ_Free(ctx->stUsrPic.unUsrPic.stUsrPicFrm.stVFrame.pMbBlk);
                ctx->stUsrPic.unUsrPic.stUsrPicFrm.stVFrame.pMbBlk = RK_NULL;
            }
        }

        // freeze test
        RK_MPI_VI_SetChnFreeze(ctx->pipeId, ctx->channelId, ctx->bFreeze);

        // 5.get the frame
        s32Ret = RK_MPI_VI_GetChnFrame(ctx->pipeId, ctx->channelId, &ctx->stViFrame, waitTime);
        if (s32Ret == RK_SUCCESS) {
            RK_U64 nowUs = TEST_COMM_GetNowUs();
            void *data = RK_MPI_MB_Handle2VirAddr(ctx->stViFrame.stVFrame.pMbBlk);

            RK_LOGD("RK_MPI_VI_GetChnFrame ok:data %p loop:%d seq:%d pts:%lld ms len=%d", data, loopCount,
                     ctx->stViFrame.stVFrame.u32TimeRef, ctx->stViFrame.stVFrame.u64PTS/1000,
                     RK_MPI_MB_GetLength(ctx->stViFrame.stVFrame.pMbBlk));
            // 6.get the channel status
            s32Ret = RK_MPI_VI_QueryChnStatus(ctx->pipeId, ctx->channelId, &ctx->stChnStatus);
            RK_LOGD("RK_MPI_VI_QueryChnStatus ret %x, w:%d,h:%d,enable:%d," \
                    "current frame id:%d,input lost:%d,output lost:%d," \
                    "framerate:%d,vbfail:%d delay=%lldus",
                     s32Ret,
                     ctx->stChnStatus.stSize.u32Width,
                     ctx->stChnStatus.stSize.u32Height,
                     ctx->stChnStatus.bEnable,
                     ctx->stChnStatus.u32CurFrameID,
                     ctx->stChnStatus.u32InputLostFrame,
                     ctx->stChnStatus.u32OutputLostFrame,
                     ctx->stChnStatus.u32FrameRate,
                     ctx->stChnStatus.u32VbFail,
                     nowUs - ctx->stViFrame.stVFrame.u64PTS);

            // 7.release the frame
            s32Ret = RK_MPI_VI_ReleaseChnFrame(ctx->pipeId, ctx->channelId, &ctx->stViFrame);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("RK_MPI_VI_ReleaseChnFrame fail %x", s32Ret);
            }
            loopCount++;
        } else {
            RK_LOGE("RK_MPI_VI_GetChnFrame timeout %x", s32Ret);
        }

        usleep(10*1000);
    }

__FAILED:
    // if enable rgn,deinit it and destory it.
    if (ctx->bEnRgn) {
        destory_rgn(ctx);
    }

    // 9. disable one chn
    s32Ret = RK_MPI_VI_DisableChn(ctx->pipeId, ctx->channelId);
    // 10.disable dev(will diabled all chn)
    s32Ret = RK_MPI_VI_DisableDev(ctx->devId);

    return s32Ret;
}

static RK_S32 create_vi(TEST_VI_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;

    // 0. get dev config status
    s32Ret = RK_MPI_VI_GetDevAttr(ctx->devId, &ctx->stDevAttr);
    if (s32Ret == RK_ERR_VI_NOT_CONFIG) {
        // 0-1.config dev
        s32Ret = RK_MPI_VI_SetDevAttr(ctx->devId, &ctx->stDevAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevAttr %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_SetDevAttr already");
    }
    // 1.get  dev enable status
    s32Ret = RK_MPI_VI_GetDevIsEnable(ctx->devId);
    if (s32Ret != RK_SUCCESS) {
        // 1-2.enable dev
        s32Ret = RK_MPI_VI_EnableDev(ctx->devId);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_EnableDev %x", s32Ret);
            goto __FAILED;
        }
        // 1-3.bind dev/pipe
        ctx->stBindPipe.u32Num = ctx->pipeId;
        ctx->stBindPipe.PipeId[0] = ctx->pipeId;
        s32Ret = RK_MPI_VI_SetDevBindPipe(ctx->devId, &ctx->stBindPipe);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VI_SetDevBindPipe %x", s32Ret);
            goto __FAILED;
        }
    } else {
        RK_LOGE("RK_MPI_VI_EnableDev already");
    }
    // 2.config channel
    s32Ret = RK_MPI_VI_SetChnAttr(ctx->pipeId, ctx->channelId, &ctx->stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_SetChnAttr %x", s32Ret);
        goto __FAILED;
    }
    // 3.enable channel
    RK_LOGD("RK_MPI_VI_EnableChn %x %d %d", ctx->devId, ctx->pipeId, ctx->channelId);
    s32Ret = RK_MPI_VI_EnableChn(ctx->pipeId, ctx->channelId);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VI_EnableChn %x", s32Ret);
        goto __FAILED;
    }
    // 4.save debug file
    if (ctx->stDebugFile.bCfg) {
        s32Ret = RK_MPI_VI_ChnSaveFile(ctx->pipeId, ctx->channelId, &ctx->stDebugFile);
        RK_LOGD("RK_MPI_VI_ChnSaveFile %x", s32Ret);
    }

__FAILED:
    return s32Ret;
}

static RK_S32 destroy_vi(TEST_VI_CTX_S *ctx) {
    RK_S32 s32Ret = RK_FAILURE;
    s32Ret = RK_MPI_VI_DisableChn(ctx->pipeId, ctx->channelId);
    RK_LOGE("RK_MPI_VI_DisableChn pipe=%d ret:%x", ctx->pipeId, s32Ret);

    s32Ret = RK_MPI_VI_DisableDev(ctx->devId);
    RK_LOGE("RK_MPI_VI_DisableDev pipe=%d ret:%x", ctx->pipeId, s32Ret);

    RK_SAFE_FREE(ctx);
    return s32Ret;
}

static RK_S32 test_vi_muti_vi_loop(TEST_VI_CTX_S *ctx_out) {
    RK_S32 s32Ret = RK_FAILURE;
    TEST_VI_CTX_S *pstViCtx[TEST_VI_SENSOR_NUM];
    RK_S32 loopCount = 0;
    RK_S32 waitTime = 33;
    RK_S32 i = 0;

    for (i = 0; i < TEST_VI_SENSOR_NUM; i++) {
        pstViCtx[i] = reinterpret_cast<TEST_VI_CTX_S *>(malloc(sizeof(TEST_VI_CTX_S)));
        memset(pstViCtx[i], 0, sizeof(TEST_VI_CTX_S));

        /* vi config init */
        pstViCtx[i]->devId = i;
        pstViCtx[i]->pipeId = pstViCtx[i]->devId;
        pstViCtx[i]->channelId = 2;

        if (TEST_VI_SENSOR_NUM == 2) {
            pstViCtx[i]->stChnAttr.stSize.u32Width = 2688;
            pstViCtx[i]->stChnAttr.stSize.u32Height = 1520;
        } else if (TEST_VI_SENSOR_NUM == 4 || TEST_VI_SENSOR_NUM == 6) {
            pstViCtx[i]->stChnAttr.stSize.u32Width = 2560;
            pstViCtx[i]->stChnAttr.stSize.u32Height = 1520;
        }
        pstViCtx[i]->stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;

        pstViCtx[i]->stChnAttr.stIspOpt.u32BufCount = 10;
        pstViCtx[i]->stChnAttr.u32Depth = 2;
        pstViCtx[i]->stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
        pstViCtx[i]->stChnAttr.enCompressMode = COMPRESS_AFBC_16x16;
        pstViCtx[i]->stChnAttr.stFrameRate.s32SrcFrameRate = -1;
        pstViCtx[i]->stChnAttr.stFrameRate.s32DstFrameRate = -1;

        /* vi create */
        s32Ret = create_vi(pstViCtx[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("vi [%d, %d] init failed: %x",
                    pstViCtx[i]->devId, pstViCtx[i]->channelId,
                    s32Ret);
            goto __FAILED_VI;
        }
    }

    while (loopCount < ctx_out->loopCountSet) {
        for (i = 0; i < TEST_VI_SENSOR_NUM; i++) {
            // get the frame
            s32Ret = RK_MPI_VI_GetChnFrame(pstViCtx[i]->pipeId, pstViCtx[i]->channelId,
                                           &pstViCtx[i]->stViFrame, waitTime);
            if (s32Ret == RK_SUCCESS) {
                RK_U64 nowUs = TEST_COMM_GetNowUs();
                void *data = RK_MPI_MB_Handle2VirAddr(pstViCtx[i]->stViFrame.stVFrame.pMbBlk);
                RK_LOGD("dev %d GetChnFrame Success!", i);
                RK_LOGD("RK_MPI_VI_GetChnFrame ok:data %p loop:%d seq:%d pts:%lld ms len=%d",
                        data, loopCount, pstViCtx[i]->stViFrame.stVFrame.u32TimeRef,
                        pstViCtx[i]->stViFrame.stVFrame.u64PTS/1000,
                        RK_MPI_MB_GetLength(pstViCtx[i]->stViFrame.stVFrame.pMbBlk));
                // get the channel status
                s32Ret = RK_MPI_VI_QueryChnStatus(pstViCtx[i]->pipeId, pstViCtx[i]->channelId,
                                                  &pstViCtx[i]->stChnStatus);
                RK_LOGD("RK_MPI_VI_QueryChnStatus ret %x, w:%d,h:%d,enable:%d," \
                        "input lost:%d, output lost:%d, framerate:%d,vbfail:%d delay=%lldus",
                        s32Ret,
                        pstViCtx[i]->stChnStatus.stSize.u32Width,
                        pstViCtx[i]->stChnStatus.stSize.u32Height,
                        pstViCtx[i]->stChnStatus.bEnable,
                        pstViCtx[i]->stChnStatus.u32InputLostFrame,
                        pstViCtx[i]->stChnStatus.u32OutputLostFrame,
                        pstViCtx[i]->stChnStatus.u32FrameRate,
                        pstViCtx[i]->stChnStatus.u32VbFail,
                        nowUs - pstViCtx[i]->stViFrame.stVFrame.u64PTS);
                // release the frame
                s32Ret = RK_MPI_VI_ReleaseChnFrame(pstViCtx[i]->pipeId, pstViCtx[i]->channelId,
                                                   &pstViCtx[i]->stViFrame);
                if (s32Ret != RK_SUCCESS) {
                    RK_LOGE("RK_MPI_VI_ReleaseChnFrame fail %x", s32Ret);
                }
            } else {
                RK_LOGE("dev %d RK_MPI_VI_GetChnFrame timeout %x", i, s32Ret);
            }
            usleep(3*1000);
        }
        loopCount++;
        usleep(10*1000);
    }

__FAILED_VI:
    /* destroy vi*/
    for (RK_S32 i = 0; i < TEST_VI_SENSOR_NUM; i++) {
        s32Ret = destroy_vi(pstViCtx[i]);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("destroy vi [%d, %d] error %x",
                    pstViCtx[i]->devId, pstViCtx[i]->channelId,
                    s32Ret);
            goto __FAILED;
        }
    }
__FAILED:
    return s32Ret;
}

static void mpi_vi_test_show_options(const TEST_VI_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");

    RK_PRINT("output file open      : %d\n", ctx->stDebugFile.bCfg);
    RK_PRINT("yuv output file name  : %s/%s\n", ctx->stDebugFile.aFilePath, ctx->stDebugFile.aFileName);
    RK_PRINT("enc0 output file path : /%s/%s\n", ctx->stVencCfg[0].dstFilePath, ctx->stVencCfg[0].dstFileName);
    RK_PRINT("enc1 output file path : /%s/%s\n", ctx->stVencCfg[1].dstFilePath, ctx->stVencCfg[1].dstFileName);
    RK_PRINT("loop count            : %d\n", ctx->loopCountSet);
    RK_PRINT("enMode                : %d\n", ctx->enMode);
    RK_PRINT("dev                   : %d\n", ctx->devId);
    RK_PRINT("pipe                  : %d\n", ctx->pipeId);
    RK_PRINT("channel               : %d\n", ctx->channelId);
    RK_PRINT("width                 : %d\n", ctx->width);
    RK_PRINT("height                : %d\n", ctx->height);
    RK_PRINT("enCompressMode        : %d\n", ctx->enCompressMode);
    RK_PRINT("enMemoryType          : %d\n", ctx->stChnAttr.stIspOpt.enMemoryType);
    RK_PRINT("aEntityName           : %s\n", ctx->stChnAttr.stIspOpt.aEntityName);
    RK_PRINT("depth                 : %d\n", ctx->stChnAttr.u32Depth);
    RK_PRINT("enPixelFormat         : %d\n", ctx->stChnAttr.enPixelFormat);
    RK_PRINT("bFreeze               : %d\n", ctx->bFreeze);
    RK_PRINT("src_frame rate        : %d\n", ctx->stChnAttr.stFrameRate.s32SrcFrameRate);
    RK_PRINT("dst frame rate        : %d\n", ctx->stChnAttr.stFrameRate.s32DstFrameRate);
    RK_PRINT("out buf count         : %d\n", ctx->stChnAttr.stIspOpt.u32BufCount);
    RK_PRINT("bUserPicEnabled       : %d\n", ctx->bUserPicEnabled);
    RK_PRINT("bEnRgn                : %d\n", ctx->bEnRgn);
    RK_PRINT("rgn count             : %d\n", ctx->s32RgnCnt);
    RK_PRINT("rgn type              : %d\n", ctx->rgnType);
    RK_PRINT("bGetConnecInfo        : %d\n", ctx->bGetConnecInfo);
    RK_PRINT("bGetEdid              : %d\n", ctx->bGetEdid);
    RK_PRINT("bSetEdid              : %d\n", ctx->bSetEdid);
}

static const char *const usages[] = {
    "./rk_mpi_vi_test -w 1920 -h 1080 -t 4 -n rkispp_scale0",
    RK_NULL,
};

int main(int argc, const char **argv) {
    RK_S32 i;
    RK_S32 s32Ret = RK_FAILURE;
    TEST_VI_CTX_S *ctx;
    ctx = reinterpret_cast<TEST_VI_CTX_S *>(malloc(sizeof(TEST_VI_CTX_S)));
    memset(ctx, 0, sizeof(TEST_VI_CTX_S));

    ctx->width = 0;
    ctx->height = 0;
    ctx->devId = 0;
    ctx->pipeId = ctx->devId;
    ctx->channelId = 1;
    ctx->loopCountSet = 100;
    ctx->enMode = TEST_VI_MODE_BIND_VENC;
    ctx->stChnAttr.stIspOpt.u32BufCount = 3;
    ctx->stChnAttr.stIspOpt.enMemoryType = VI_V4L2_MEMORY_TYPE_DMABUF;
    ctx->stChnAttr.stIspOpt.enCaptureType = VI_V4L2_CAPTURE_TYPE_VIDEO_CAPTURE;
    ctx->stChnAttr.u32Depth = 0;
    ctx->aEntityName = RK_NULL;
    ctx->stChnAttr.enPixelFormat = RK_FMT_YUV420SP;
    ctx->stChnAttr.stFrameRate.s32SrcFrameRate = -1;
    ctx->stChnAttr.stFrameRate.s32DstFrameRate = -1;
    ctx->bEnRgn = RK_FALSE;
    ctx->s32RgnCnt = 1;
    ctx->rgnType = RGN_BUTT;
    RK_LOGE("test running enter!");

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_INTEGER('w', "width", &(ctx->width),
                    "set capture channel width(required, default 0)", NULL, 0, 0),
        OPT_INTEGER('h', "height", &(ctx->height),
                    "set capture channel height(required, default 0)", NULL, 0, 0),
        OPT_INTEGER('d', "dev", &(ctx->devId),
                    "set dev id(default 0)", NULL, 0, 0),
        OPT_INTEGER('p', "pipe", &(ctx->pipeId),
                    "set pipe id(default 0)", NULL, 0, 0),
        OPT_INTEGER('c', "channel", &(ctx->channelId),
                    "set channel id(default 1)", NULL, 0, 0),
        OPT_INTEGER('l', "loopcount", &(ctx->loopCountSet),
                    "set capture frame count(default 100)", NULL, 0, 0),
        OPT_INTEGER('C', "compressmode", &(ctx->enCompressMode),
                    "set capture compressmode(default 0; 0:MODE_NONE 1:AFBC_16x16)", NULL, 0, 0),
        OPT_INTEGER('o', "output", &(ctx->stDebugFile.bCfg),
                    "save output file, file at /data/test_<devid>_<pipeid>_<channelid>.bin"
                    " (default 0; 0:no save 1:save)", NULL, 0, 0),
        OPT_INTEGER('m', "mode", &(ctx->enMode),
                    "test mode(default 1; 0:vi get&release frame 1:vi bind one venc(h264) \n\t"
                    "2:vi bind two venc(h264)) 3:vi bind vpss bind venc \n\t"
                    "4:vi bind vo(only support 356x now)", NULL, 0, 0),
        OPT_INTEGER('t', "memorytype", &(ctx->stChnAttr.stIspOpt.enMemoryType),
                    "set the buf memorytype(required, default 4; 1:mmap(hdmiin/bt1120/sensor input) "
                    "2:userptr(invalid) 3:overlay(invalid) 4:dma(sensor))", NULL, 0, 0),
        OPT_STRING('n', "name", &(ctx->aEntityName),
                   "set the entityName (required, default null;\n\t"
                   "rv1126 sensor:rkispp_m_bypass rkispp_scale0 rkispp_scale1 rkispp_scale2;\n\t"
                   "rv1126 hdmiin/bt1120/sensor:/dev/videox such as /dev/video19 /dev/video20;\n\t"
                   "rk356x hdmiin/bt1120/sensor:/dev/videox such as /dev/video0 /dev/video1", NULL, 0, 0),
        OPT_INTEGER('D', "depth", &(ctx->stChnAttr.u32Depth),
                    "channel output depth, default{u32BufCount(not bind) or 0(bind venc/vpss/...)}", NULL, 0, 0),
        OPT_INTEGER('f', "format", &(ctx->stChnAttr.enPixelFormat),
                    "set the format(default 0; 0:RK_FMT_YUV420SP 10:RK_FMT_YUV422_UYVY"
                    "131080:RK_FMT_RGB_BAYER_SBGGR_12BPP)", NULL, 0, 0),
        OPT_INTEGER('\0', "freeze", &(ctx->bFreeze),
                    "freeze output(default 0; 0:disable freeze 1:enable freeze)", NULL, 0, 0),
        OPT_INTEGER('\0', "src_rate", &(ctx->stChnAttr.stFrameRate.s32SrcFrameRate),
                    "src frame rate(default -1; -1:not control; other:1-max_fps<isp_out_fps>)", NULL, 0, 0),
        OPT_INTEGER('\0', "dst_rate", &(ctx->stChnAttr.stFrameRate.s32DstFrameRate),
                    "dst frame rate(default -1; -1:not control; other:1-src_fps<src_rate>)", NULL, 0, 0),
        OPT_INTEGER('\0', "buf_count", &(ctx->stChnAttr.stIspOpt.u32BufCount),
                    "out buf count, range[1-8] default(3)", NULL, 0, 0),
        OPT_INTEGER('U', "user_pic", &(ctx->bUserPicEnabled),
                    "enable using user specify picture as vi input.", NULL, 0, 0),
        OPT_INTEGER('\0', "rgn_type", &(ctx->rgnType),
                    "rgn type. 0:overlay, 1:overlayEx,2:cover,3:coverEx,4:mosaic,5:moscaiEx", NULL, 0, 0),
        OPT_INTEGER('\0', "rgn_cnt", &(ctx->s32RgnCnt),
                    "rgn count. default(1),max:8", NULL, 0, 0),
        OPT_INTEGER('\0', "en_rgn", &(ctx->bEnRgn),
                    "enable rgn. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "get_connect_info", &(ctx->bGetConnecInfo),
                    "get connect info. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "get_edid", &(ctx->bGetEdid),
                    "get edid. default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "set_edid", &(ctx->bSetEdid),
                    "set edid. default(0)", NULL, 0, 0),

        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");
    argc = argparse_parse(&argparse, argc, argv);

    if (!ctx->width || !ctx->height) {
        argparse_usage(&argparse);
        goto __FAILED2;
    }

    if (ctx->pipeId != ctx->devId)
        ctx->pipeId = ctx->devId;

    if (ctx->stDebugFile.bCfg) {
        if (ctx->enMode == TEST_VI_MODE_BIND_VENC || ctx->enMode == TEST_VI_MODE_BIND_VPSS_BIND_VENC) {
            ctx->stVencCfg[0].bOutDebugCfg = ctx->stDebugFile.bCfg;
        } else if (ctx->enMode == TEST_VI_MODE_BIND_VENC_MULTI) {
            ctx->stVencCfg[0].bOutDebugCfg = ctx->stDebugFile.bCfg;
            ctx->stVencCfg[1].bOutDebugCfg = ctx->stDebugFile.bCfg;
        }
        memcpy(&ctx->stDebugFile.aFilePath, "/data", strlen("/data"));
        snprintf(ctx->stDebugFile.aFileName, MAX_VI_FILE_PATH_LEN,
                 "test_%d_%d_%d.bin", ctx->devId, ctx->pipeId, ctx->channelId);
    }
    for (i = 0; i < ctx->enMode; i++) {
        if (ctx->stVencCfg[i].bOutDebugCfg) {
            char name[256] = {0};
            memcpy(&ctx->stVencCfg[i].dstFilePath, "data", strlen("data"));
            snprintf(ctx->stVencCfg[i].dstFileName, sizeof(ctx->stVencCfg[i].dstFileName),
                   "venc_%d.bin", i);
            snprintf(name, sizeof(name), "/%s/%s",
                     ctx->stVencCfg[i].dstFilePath, ctx->stVencCfg[i].dstFileName);
            ctx->stVencCfg[i].fp = fopen(name, "wb");
            if (ctx->stVencCfg[i].fp == RK_NULL) {
                RK_LOGE("chn %d can't open file %s in get picture thread!\n", i, name);
                goto __FAILED;
            }
        }
    }

    RK_LOGE("test running enter ctx->aEntityName=%s!", ctx->aEntityName);
    if (ctx->aEntityName != RK_NULL)
        memcpy(ctx->stChnAttr.stIspOpt.aEntityName, ctx->aEntityName, strlen(ctx->aEntityName));

    if (ctx->pipeId != ctx->devId)
        ctx->pipeId = ctx->devId;

    mpi_vi_test_show_options(ctx);

    if (RK_MPI_SYS_Init() != RK_SUCCESS) {
        RK_LOGE("rk mpi sys init fail!");
        goto __FAILED;
    }
    switch (ctx->enMode) {
        case TEST_VI_MODE_VI_ONLY:
            if (!ctx->stChnAttr.u32Depth) {
                RK_LOGE("depth need > 0 when vi not bind any other module!");
                ctx->stChnAttr.u32Depth = ctx->stChnAttr.stIspOpt.u32BufCount;
            }
            s32Ret = test_vi_get_release_frame_loop(ctx);
        break;
        case TEST_VI_MODE_BIND_VENC:
        case TEST_VI_MODE_BIND_VENC_MULTI:
            s32Ret = test_vi_bind_venc_loop(ctx);
        break;
        case TEST_VI_MODE_BIND_VPSS_BIND_VENC:
            s32Ret = test_vi_bind_vpss_venc_loop(ctx);
        break;
        case TEST_VI_MODE_BIND_VO:
            s32Ret = test_vi_bind_vo_loop(ctx);
        break;
        case TEST_VI_MODE_MUTI_VI:
            s32Ret = test_vi_muti_vi_loop(ctx);
            break;
        default:
            RK_LOGE("no support such test mode:%d", ctx->enMode);
        break;
    }
__FAILED:
    RK_LOGE("test running exit:%d", s32Ret);
    RK_MPI_SYS_Exit();
__FAILED2:
    if (ctx) {
        free(ctx);
        ctx = RK_NULL;
    }

    return 0;
}
