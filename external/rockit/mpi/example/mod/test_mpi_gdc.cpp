/*
 * Copyright 2022 Rockchip Electronics Co. LTD
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
#define DBG_MOD_ID       RK_ID_GDC

#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include "rk_type.h"
#include "rk_debug.h"
#include "rk_mpi_gdc.h"
#include "rk_mpi_mb.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_cal.h"
#include "rk_mpi_vo.h"
#include "rk_comm_vo.h"

#include "test_comm_argparse.h"

// for vo
#define RK35XX_VO_DEV_HD0 0
#define RK35XX_VO_DEV_HD1 1
#define RK35XX_VO_GDC_CHN 0
#define RK35XX_VOP_LAYER_CLUSTER_0 0
#define RK35XX_VOP_LAYER_CLUSTER_1 2
#define RK35XX_VOP_LAYER_ESMART_0 4
#define RK35XX_VOP_LAYER_ESMART_1 5
#define RK35XX_VOP_LAYER_SMART_0 6
#define RK35XX_VOP_LAYER_SMART_1 7

#define RK35XX_VO_LINE_CHN 127

RK_U16 gau16PointColor[9] = {
    0x8000 | 0x44c4,  // red
    0x8000 | 0x1234,  // blue
    0x8000 | 0x7fff,  // white
    0x8000 | 0x7322,
    0x8000 | 0x5433,
    0x8000 | 0x6544,
    0x8000 | 0x7644,
    0x8000 | 0x8744,
    0x8000 | 0x9844
};

typedef enum _TestGdcDeskCeilMode {
    TEST_GDC_DESK_CEIL_MODE_1P_ADD_1     = 0,
    TEST_GDC_DESK_CEIL_MODE_2P           = 1,
    TEST_GDC_DESK_CEIL_MODE_1_ADD_3      = 2,
    TEST_GDC_DESK_CEIL_MODE_1_ADD_4      = 3,
    TEST_GDC_DESK_CEIL_MODE_1P_ADD_6     = 4,
    TEST_GDC_DESK_CEIL_MODE_1_ADD_8      = 5,
    TEST_GDC_DESK_CEIL_MODE_BUTT
} TestGdcDeskCeilMode;

typedef enum _TestGdcWallMode {
    TEST_GDC_WALL_MODE_1P                = 0,
    TEST_GDC_WALL_MODE_1P_ADD_3          = 1,
    TEST_GDC_WALL_MODE_1P_ADD_4          = 2,
    TEST_GDC_WALL_MODE_1P_ADD_8          = 3,
    TEST_GDC_WALL_MODE_BUTT
} TestGdcWallMode;

typedef struct _rkMpiGdcCtx {
    const char *srcFilePath;
    RK_S32  s32LoopCount;
    RK_S32  s32JobNum;
    RK_S32  s32TaskSum;
    RK_S32  s32TaskType;
    RK_S32  s32TaskMode;
    RK_S32  u32SrcWidth;
    RK_S32  u32SrcHeight;
    RK_S32  u32SrcVirWidth;
    RK_S32  u32SrcVirHeight;
    RK_S32  u32LineWidth;
    RK_S32  u32LineHeight;
    RK_S32  s32SrcCompressMode;
    RK_S32  s32SrcPixFormat;
    RK_S32  u32DstWidth;
    RK_S32  u32DstHeight;
    RK_S32  s32DstCompressMode;
    RK_S32  s32DstPixFormat;
    RK_S32  s32JobIdx;
    RK_U32  u32SrcSize;
    RK_U32  u32ShowPoint;
    RK_U32  u32ChangeParam;
    RK_U32  u32PointLen;
    MB_POOL inPool;
    TestGdcDeskCeilMode enDeskCeilMode;
    TestGdcWallMode enWallMode;
    FISHEYE_JOB_CONFIG_S stFisheyeJobConfig;
    GDC_TASK_ATTR_S stTask;
    FISHEYE_ATTR_S stFisheyeAttr;
    POINT_S *pastDstPoint;
    POINT_S *pastSrcPoint;
    // for vo
    VO_CHN_ATTR_S stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    VO_LAYER s32VoLayer;
    VO_DEV s32VoDev;
    VO_CHN s32VoChn;
    VO_INTF_SYNC_E stVoOutputResolution;
    RK_S32 u32VoWidth;
    RK_S32 u32VoHeight;
    RECT_S stChnRect;
} TEST_GDC_CTX_S;

typedef enum _rkGdcChangeType {
    TEST_GDC_CHANGE_TYPE_VIEW_MODE,
    TEST_GDC_CHANGE_TYPE_IN_RADIUS,
    TEST_GDC_CHANGE_TYPE_OUT_RADIUS,
    TEST_GDC_CHANGE_TYPE_PAN,
    TEST_GDC_CHANGE_TYPE_TILT,
    TEST_GDC_CHANGE_TYPE_HOR_ZOOM,
    TEST_GDC_CHANGE_TYPE_VER_ZOOM,
    TEST_GDC_CHANGE_TYPE_ALL
} TEST_GDC_CHANGE_TYPE;

static RK_S32 create_vo(TEST_GDC_CTX_S *ctx, RK_U32 u32Ch) {
    /* Enable VO */
    VO_PUB_ATTR_S VoPubAttr;
    RK_S32 s32Ret = RK_SUCCESS;
    VO_LAYER VoLayer = ctx->s32VoLayer;
    VO_DEV VoDev = ctx->s32VoDev;

    memset(&VoPubAttr, 0, sizeof(VO_PUB_ATTR_S));

    s32Ret = RK_MPI_VO_GetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    VoPubAttr.enIntfType = VO_INTF_HDMI;
    VoPubAttr.enIntfSync = ctx->stVoOutputResolution;

    s32Ret = RK_MPI_VO_SetPubAttr(VoDev, &VoPubAttr);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }
    s32Ret = RK_MPI_VO_Enable(VoDev);
    if (s32Ret != RK_SUCCESS) {
        return s32Ret;
    }

    s32Ret = RK_MPI_VO_SetLayerAttr(VoLayer, &ctx->stLayerAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_SetLayerAttr failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_BindLayer(VoLayer, VoDev, VO_LAYER_MODE_VIDEO);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_BindLayer failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }


    s32Ret = RK_MPI_VO_EnableLayer(VoLayer);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_VO_EnableLayer failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }

    s32Ret = RK_MPI_VO_SetChnAttr(VoLayer, u32Ch, &ctx->stChnAttr);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("set chn Attr failed,s32Ret:%d\n", s32Ret);
        return RK_FAILURE;
    }

    return s32Ret;
}

#if 0
RK_S32 unit_test_mpi_gdc(TEST_GDC_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    pthread_t tids[GDC_MAX_JOB_NUM];
    TEST_GDC_CTX_S tmpCtx[GDC_MAX_JOB_NUM];

    for (RK_S32 jobIndex = 0; jobIndex < ctx->s32JobNum; jobIndex++) {
        memcpy(&(tmpCtx[jobIndex]), ctx, sizeof(TEST_GDC_CTX_S));
        pthread_create(&tids[jobIndex], 0, unit_test_gdc_counts, reinterpret_cast<void *>(&tmpCtx[jobIndex]));
        ctx->s32JobIdx++;
    }

    for (RK_S32 jobIndex = 0; jobIndex < ctx->s32JobNum; jobIndex++) {
        pthread_join(tids[jobIndex], RK_NULL);
    }

    return s32Ret;
}
#endif
static RK_S32 read_with_pixel_width(RK_U8 *pBuf, RK_U32 u32Width, RK_U32 u32VirHeight,
                                     RK_U32 u32VirWidth, RK_U32 u32PixWidth, FILE *fp) {
    RK_U32 u32Row;
    RK_S32 s32ReadSize;

    for (u32Row = 0; u32Row < u32VirHeight; u32Row++) {
        s32ReadSize = fread(pBuf + u32Row * u32VirWidth * u32PixWidth, 1, u32Width * u32PixWidth, fp);
        if (s32ReadSize != u32Width * u32PixWidth) {
            RK_LOGE("read file failed expect %d vs %d\n",
                      u32Width * u32PixWidth, s32ReadSize);
            return RK_FAILURE;
        }
    }

    return RK_SUCCESS;
}

static RK_S32 read_image(RK_U8 *pVirAddr, RK_U32 u32Width, RK_U32 u32Height,
                            RK_U32 u32VirWidth, RK_U32 u32VirHeight, RK_U32 u32PixFormat, FILE *fp) {
    RK_U32 u32Row = 0;
    RK_U32 u32ReadSize = 0;
    RK_S32 s32Ret = RK_SUCCESS;

    RK_U8 *pBufy = pVirAddr;
    RK_U8 *pBufu = pBufy + u32VirWidth * u32VirHeight;
    RK_U8 *pBufv = pBufu + u32VirWidth * u32VirHeight / 4;

    switch (u32PixFormat) {
        case RK_FMT_YUV420SP:
        case RK_FMT_YUV420SP_VU: {
            for (u32Row = 0; u32Row < u32VirHeight; u32Row++) {
                u32ReadSize = fread(pBufy + u32Row * u32VirWidth, 1, u32Width, fp);
                if (u32ReadSize != u32Width) {
                     return RK_FAILURE;
                }
            }

            for (u32Row = 0; u32Row < u32VirHeight / 2; u32Row++) {
                u32ReadSize = fread(pBufu + u32Row * u32VirWidth, 1, u32Width, fp);
                if (u32ReadSize != u32Width) {
                    return RK_FAILURE;
                }
            }
        } break;
        case RK_FMT_RGB888:
        case RK_FMT_BGR888: {
            s32Ret = read_with_pixel_width(pBufy, u32Width, u32VirHeight, u32VirWidth, 3, fp);
        } break;
        default : {
            RK_LOGE("read image do not support fmt %d\n", u32PixFormat);
            return RK_FAILURE;
        } break;
    }

    return s32Ret;
}

RK_U32 unit_test_gdc_get_size(TEST_GDC_CTX_S *ctx) {
    RK_S32 s32Ret = RK_SUCCESS;
    PIC_BUF_ATTR_S stPicBufAttr;
    MB_PIC_CAL_S   stMbPicCalResult;

    if (ctx->u32SrcVirWidth == 0) {
        ctx->u32SrcVirWidth = ctx->u32SrcWidth;
    }
    if (ctx->u32SrcVirHeight == 0) {
        ctx->u32SrcVirHeight = ctx->u32SrcHeight;
    }

    stPicBufAttr.u32Width = ctx->u32SrcVirWidth;
    stPicBufAttr.u32Height = ctx->u32SrcVirHeight;
    stPicBufAttr.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32SrcPixFormat;
    stPicBufAttr.enCompMode = (COMPRESS_MODE_E)ctx->s32SrcCompressMode;
    s32Ret = RK_MPI_CAL_COMM_GetPicBufferSize(&stPicBufAttr, &stMbPicCalResult);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("get picture buffer size failed. err 0x%x", s32Ret);
        return s32Ret;
    }

    return stMbPicCalResult.u32MBSize;
}

RK_VOID test_mpi_gdc_change_parameter(TEST_GDC_CTX_S *ctx, RK_S32 index, TEST_GDC_CHANGE_TYPE type) {
    FISHEYE_REGION_ATTR_S regionAttr;
    memcpy(&regionAttr, &ctx->stFisheyeAttr.astFishEyeRegionAttr[index], sizeof(FISHEYE_REGION_ATTR_S));

    switch (type) {
        case TEST_GDC_CHANGE_TYPE_VIEW_MODE:
        break;
        case TEST_GDC_CHANGE_TYPE_IN_RADIUS:
        break;
        case TEST_GDC_CHANGE_TYPE_OUT_RADIUS:
        break;
        case TEST_GDC_CHANGE_TYPE_PAN:
            if (regionAttr.u32Pan < 360)
                regionAttr.u32Pan++;
            else
                regionAttr.u32Pan = 0;
        break;
        case TEST_GDC_CHANGE_TYPE_TILT:
            if (regionAttr.u32Tilt < 360)
                regionAttr.u32Tilt++;
            else
                regionAttr.u32Tilt = 0;
        break;
        case TEST_GDC_CHANGE_TYPE_HOR_ZOOM:
            if (regionAttr.u32HorZoom < 4095)
                regionAttr.u32HorZoom++;
            else
                regionAttr.u32HorZoom = 1;
        break;
        case TEST_GDC_CHANGE_TYPE_VER_ZOOM:
            if (regionAttr.u32VerZoom < 4095)
                regionAttr.u32VerZoom++;
            else
                regionAttr.u32VerZoom = 1;
        break;
        case TEST_GDC_CHANGE_TYPE_ALL:
        break;
    }
    memcpy(&ctx->stFisheyeAttr.astFishEyeRegionAttr[index], &regionAttr, sizeof(FISHEYE_REGION_ATTR_S));
}

RK_VOID test_mpi_gdc_get_src_point(TEST_GDC_CTX_S *ctx) {
    RK_S32 s32Ret;
    for (RK_U32 region = 1; region < ctx->stFisheyeAttr.u32RegionNum; region++) {
        GDC_FISHEYE_POINT_QUERY_ATTR_S stQuery;
        POINT_S stDstPoint, stSrcPoint;
        memset(&stQuery, 0, sizeof(sizeof(FISHEYE_ATTR_S)));
        stQuery.pstFishEyeAttr = &ctx->stFisheyeAttr;
        stQuery.u32RegionIndex = region;
        stDstPoint.s32X = 0;
        stDstPoint.s32Y = 540;
        s32Ret = RK_MPI_GDC_FisheyePosQueryDst2Src(&stQuery, &ctx->stTask.stImgIn, &stDstPoint, &stSrcPoint);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_GDC_FisheyePosQueryDst2Src err:0x%x!", s32Ret);
        }
        RK_LOGD("region:%d dst:%d,%d src:%d,%d", region, stDstPoint.s32X, stDstPoint.s32Y,
                                                 stSrcPoint.s32X, stSrcPoint.s32Y);
    }
}

RK_VOID test_mpi_gdc_get_pano_point(TEST_GDC_CTX_S *ctx) {
    RK_S32 s32Ret;
    for (RK_U32 region = 1; region < ctx->stFisheyeAttr.u32RegionNum; region++) {
        GDC_FISHEYE_POINT_QUERY_ATTR_S stQuery;
        POINT_S stDstPoint, stPanoPoint;
        memset(&stQuery, 0, sizeof(sizeof(FISHEYE_ATTR_S)));
        stQuery.pstFishEyeAttr = &ctx->stFisheyeAttr;
        stQuery.u32RegionIndex = region;
        stDstPoint.s32X = 0;
        stDstPoint.s32Y = 540;
        s32Ret = RK_MPI_GDC_FisheyePosQueryDst2Pano(&stQuery, 0, &stDstPoint, &stPanoPoint);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_GDC_FisheyePosQueryDst2Pano err:0x%x!", s32Ret);
        }
        RK_LOGD("region:%d dst:%d,%d pano:%d,%d", region, stDstPoint.s32X, stDstPoint.s32Y,
                                                  stPanoPoint.s32X, stPanoPoint.s32Y);
    }
}

RK_S32 test_mpi_gdc_get_src_point_array(TEST_GDC_CTX_S *ctx, RK_U32 region) {
    RK_S32 s32Ret;
    RK_FLOAT step;
    RK_U32 w = ctx->stFisheyeAttr.astFishEyeRegionAttr[region].stOutRect.u32Width;
    RK_U32 h = ctx->stFisheyeAttr.astFishEyeRegionAttr[region].stOutRect.u32Height;

    ctx->u32PointLen = ctx->u32PointLen > ((w + h) * 2) ? (w + h) * 2 : ctx->u32PointLen;
    step = (RK_FLOAT)((w + h) * 2) / ctx->u32PointLen;

    for (RK_U32 i = 0; i < ctx->u32PointLen; i++) {
        if ((step * i) <= w) {
            ctx->pastDstPoint[i].s32X = step * i;
            ctx->pastDstPoint[i].s32Y = 0;
        } else if ((step * i) <= (w + h)) {
            ctx->pastDstPoint[i].s32X = (w - 1);
            ctx->pastDstPoint[i].s32Y = (step * i - w);
            // printf("22 region:%d i%d= %d,%d\n", region, i, ctx->pastDstPoint[i].s32X, ctx->pastDstPoint[i].s32Y);
        } else if ((step * i) <= (2 * w + h)) {
            ctx->pastDstPoint[i].s32X = 2 * w + h - step * i;
            ctx->pastDstPoint[i].s32Y = h - 1;
            // printf("33 region:%d i%d=%d\n", region, i, ctx->pastDstPoint[i].s32X);
        } else {
            ctx->pastDstPoint[i].s32X = 0;
            ctx->pastDstPoint[i].s32Y = (2 * (w + h)- step * i);
            // printf("44 region:%d i%d=%d\n", region, i, ctx->pastDstPoint[i].s32Y);
        }
    }

    // get the array
    GDC_FISHEYE_POINT_QUERY_ATTR_S stQuery;
    memset(&stQuery, 0, sizeof(sizeof(FISHEYE_ATTR_S)));
    stQuery.pstFishEyeAttr = &ctx->stFisheyeAttr;
    stQuery.u32RegionIndex = region;
    s32Ret = RK_MPI_GDC_FisheyePosQueryDst2SrcArray(&stQuery, &ctx->stTask.stImgIn, ctx->u32PointLen,
                                                    ctx->pastDstPoint, ctx->pastSrcPoint);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_GDC_FisheyePosQueryDst2SrcArray err:0x%x!", s32Ret);
    }

    return s32Ret;
}

RK_S32 test_mpi_gdc_get_pano_point_array(TEST_GDC_CTX_S *ctx, RK_U32 region, RK_U32 pano_region) {
    RK_S32 s32Ret;
    RK_FLOAT step;
    RK_U32 w = ctx->stFisheyeAttr.astFishEyeRegionAttr[region].stOutRect.u32Width;
    RK_U32 h = ctx->stFisheyeAttr.astFishEyeRegionAttr[region].stOutRect.u32Height;

    ctx->u32PointLen = ctx->u32PointLen > ((w + h) * 2) ? (w + h) * 2 : ctx->u32PointLen;
    step = (RK_FLOAT)((w + h) * 2) / ctx->u32PointLen;

    for (RK_U32 i = 0; i < ctx->u32PointLen; i++) {
        if ((step * i) <= w) {
            ctx->pastDstPoint[i].s32X = step * i;
            ctx->pastDstPoint[i].s32Y = 0;
        } else if ((step * i) <= (w + h)) {
            ctx->pastDstPoint[i].s32X = (w - 1);
            ctx->pastDstPoint[i].s32Y = (step * i - w);
            // printf("22 region:%d i%d= %d,%d\n", region, i, ctx->pastDstPoint[i].s32X, ctx->pastDstPoint[i].s32Y);
        } else if ((step * i) <= (2 * w + h)) {
            ctx->pastDstPoint[i].s32X = 2 * w + h - step * i;
            ctx->pastDstPoint[i].s32Y = h - 1;
            // printf("33 region:%d i%d=%d\n", region, i, ctx->pastDstPoint[i].s32X);
        } else {
            ctx->pastDstPoint[i].s32X = 0;
            ctx->pastDstPoint[i].s32Y = (2 * (w + h)- step * i);
            // printf("44 region:%d i%d=%d\n", region, i, ctx->pastDstPoint[i].s32Y);
        }
    }

    // get the array
    GDC_FISHEYE_POINT_QUERY_ATTR_S stQuery;
    memset(&stQuery, 0, sizeof(sizeof(FISHEYE_ATTR_S)));
    stQuery.pstFishEyeAttr = &ctx->stFisheyeAttr;
    stQuery.u32RegionIndex = region;
    s32Ret = RK_MPI_GDC_FisheyePosQueryDst2PanoArray(&stQuery, pano_region, ctx->u32PointLen,
                                                     ctx->pastDstPoint, ctx->pastSrcPoint);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("RK_MPI_GDC_FisheyePosQueryDst2PanoArray err:0x%x!", s32Ret);
    }

    return s32Ret;
}

RK_VOID test_mpi_gdc_desktop_ceil_mount_1p_add_1(TEST_GDC_CTX_S *ctx) {
    RK_S32 index = 0;
    RK_U32 fishOptParam_originX, fishOptParam_originY;
    fishOptParam_originX = 4132;
    fishOptParam_originY = 4169;

    ctx->stFisheyeAttr.bEnable = RK_TRUE;
    ctx->stFisheyeAttr.bLMF = RK_FALSE;
    ctx->stFisheyeAttr.bBgColor = RK_TRUE;
    ctx->stFisheyeAttr.u32BgColor = 0;
    ctx->stFisheyeAttr.s32HorOffset = ((fishOptParam_originX >> 3) - 512);
    ctx->stFisheyeAttr.s32VerOffset = ((fishOptParam_originY >> 3) - 512);
    ctx->stFisheyeAttr.u32TrapezoidCoef = 0;
    ctx->stFisheyeAttr.s32FanStrength = 0;
    // ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 1920;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 550;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_360_PANORAMA;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 1920;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    ctx->stFisheyeAttr.u32RegionNum = index;
}

RK_VOID test_mpi_gdc_desktop_ceil_mount_1_add_3(TEST_GDC_CTX_S *ctx) {
    RK_S32 index = 0;
    RK_U32 fishOptParam_originX, fishOptParam_originY;
    fishOptParam_originX = 4132;
    fishOptParam_originY = 4169;

    ctx->stFisheyeAttr.bEnable = RK_TRUE;
    ctx->stFisheyeAttr.bLMF = RK_FALSE;
    ctx->stFisheyeAttr.bBgColor = RK_TRUE;
    ctx->stFisheyeAttr.u32BgColor = 0;
    ctx->stFisheyeAttr.s32HorOffset = ((fishOptParam_originX >> 3) - 512);
    ctx->stFisheyeAttr.s32VerOffset = ((fishOptParam_originY >> 3) - 512);
    ctx->stFisheyeAttr.u32TrapezoidCoef = 0;
    ctx->stFisheyeAttr.s32FanStrength = 0;
    // ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;

    if (ctx->u32ShowPoint == 1)
        ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_NO_TRANSFORMATION;
    else
        ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_360_PANORAMA;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 120; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 960; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 240; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 540;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 0; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 960; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 540;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    // ctx->stFisheyeAttr.u32RegionNum = index;
}

RK_VOID test_mpi_gdc_desktop_ceil_mount_1_add_8(TEST_GDC_CTX_S *ctx) {
    RK_S32 index = 0;
    RK_U32 fishOptParam_originX, fishOptParam_originY;
    fishOptParam_originX = 4132;
    fishOptParam_originY = 4169;

    ctx->stFisheyeAttr.bEnable = RK_TRUE;
    ctx->stFisheyeAttr.bLMF = RK_FALSE;
    ctx->stFisheyeAttr.bBgColor = RK_TRUE;
    ctx->stFisheyeAttr.u32BgColor = 0;
    ctx->stFisheyeAttr.s32HorOffset = ((fishOptParam_originX >> 3) - 512);
    ctx->stFisheyeAttr.s32VerOffset = ((fishOptParam_originY >> 3) - 512);
    ctx->stFisheyeAttr.u32TrapezoidCoef = 0;
    ctx->stFisheyeAttr.s32FanStrength = 0;
    // ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;

    // 0
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 0; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 1
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 45; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 640; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 2
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 90; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 1280; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 3
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 135; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 360;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 4
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_NO_TRANSFORMATION;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 640; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 360;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 5
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 1280; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 360;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 6
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 225; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 7
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 270; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 640; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 8
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 315; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 98; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 1280; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    ctx->stFisheyeAttr.u32RegionNum = index;
}

RK_VOID test_mpi_gdc_desktop_ceil_mount_2p(TEST_GDC_CTX_S *ctx) {
    RK_S32 index = 0;
    RK_U32 fishOptParam_originX, fishOptParam_originY;
    fishOptParam_originX = 4132;
    fishOptParam_originY = 4169;

    ctx->stFisheyeAttr.bEnable = RK_TRUE;
    ctx->stFisheyeAttr.bLMF = RK_FALSE;
    ctx->stFisheyeAttr.bBgColor = RK_TRUE;
    ctx->stFisheyeAttr.u32BgColor = 0;
    ctx->stFisheyeAttr.s32HorOffset = ((fishOptParam_originX >> 3) - 512);
    ctx->stFisheyeAttr.s32VerOffset = ((fishOptParam_originY >> 3) - 512);
    ctx->stFisheyeAttr.u32TrapezoidCoef = 0;
    ctx->stFisheyeAttr.s32FanStrength = 0;
    // ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_360_PANORAMA;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 0; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 2047; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 1920;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_360_PANORAMA;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 474; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 2047; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 540;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 1920;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.u32RegionNum = index;
}

RK_VOID test_mpi_gdc_desktop_mount(TEST_GDC_CTX_S *ctx) {
    switch (ctx->enDeskCeilMode) {
        case TEST_GDC_DESK_CEIL_MODE_1P_ADD_1:
            test_mpi_gdc_desktop_ceil_mount_1p_add_1(ctx);
        break;
        case TEST_GDC_DESK_CEIL_MODE_1_ADD_3:
            test_mpi_gdc_desktop_ceil_mount_1_add_3(ctx);
        break;
        case TEST_GDC_DESK_CEIL_MODE_1_ADD_8:
            test_mpi_gdc_desktop_ceil_mount_1_add_8(ctx);
        break;
        default:
            RK_LOGE("not support this mode:%d", ctx->enDeskCeilMode);
        break;
    }
}

RK_VOID test_mpi_gdc_ceiling_mount(TEST_GDC_CTX_S *ctx) {
    test_mpi_gdc_desktop_mount(ctx);
}

RK_VOID test_mpi_gdc_wall_mount_1p_add_3(TEST_GDC_CTX_S *ctx) {
    RK_S32 index = 0;
    RK_U32 fishOptParam_originX, fishOptParam_originY;
    fishOptParam_originX = 4364;
    fishOptParam_originY = 4136;

    ctx->stFisheyeAttr.bEnable = RK_TRUE;
    ctx->stFisheyeAttr.bLMF = RK_FALSE;
    ctx->stFisheyeAttr.bBgColor = RK_TRUE;
    ctx->stFisheyeAttr.u32BgColor = 0;
    ctx->stFisheyeAttr.s32HorOffset = ((fishOptParam_originX >> 3) - 512);
    ctx->stFisheyeAttr.s32VerOffset = ((fishOptParam_originY >> 3) - 512);
    ctx->stFisheyeAttr.u32TrapezoidCoef = 0;
    ctx->stFisheyeAttr.s32FanStrength = 0;
    // ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_180_PANORAMA;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 960; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 540;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 960; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 540;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 960;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 540;
    index++;

    // ctx->stFisheyeAttr.u32RegionNum = index;
}

RK_VOID test_mpi_gdc_wall_mount_1p_add_8(TEST_GDC_CTX_S *ctx) {
    RK_S32 index = 0;
    RK_U32 fishOptParam_originX, fishOptParam_originY;
    fishOptParam_originX = 4364;
    fishOptParam_originY = 4136;

    ctx->stFisheyeAttr.bEnable = RK_TRUE;
    ctx->stFisheyeAttr.bLMF = RK_FALSE;
    ctx->stFisheyeAttr.bBgColor = RK_TRUE;
    ctx->stFisheyeAttr.u32BgColor = 0;
    ctx->stFisheyeAttr.s32HorOffset = ((fishOptParam_originX >> 3) - 512);
    ctx->stFisheyeAttr.s32VerOffset = ((fishOptParam_originY >> 3) - 512);
    ctx->stFisheyeAttr.u32TrapezoidCoef = 0;
    ctx->stFisheyeAttr.s32FanStrength = 0;
    // ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;

    // 0
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 1
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 640; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 2
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 1280; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 0;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 3
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 360;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 4 scale
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_180_PANORAMA;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 640; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 360;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 5
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 1280; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 360;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 6
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 153; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 0; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 7
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 180; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 640; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    // 8
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].enViewMode = FISHEYE_VIEW_NORMAL;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32InRadius = 0; /* RW; inner radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32OutRadius = 442; /* RW; out radius of gdc correction region */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Pan = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32Tilt = 207; /* RW; Range: [0, 360] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32HorZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].u32VerZoom = 4095; /* RW; Range: [1, 4095] */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32X = 1280; /* RW; out Imge rectangle attribute */
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.s32Y = 720;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Width = 640;
    ctx->stFisheyeAttr.astFishEyeRegionAttr[index].stOutRect.u32Height = 360;
    index++;

    ctx->stFisheyeAttr.u32RegionNum = index;
}

RK_VOID test_mpi_gdc_wall_mount(TEST_GDC_CTX_S *ctx) {
    switch (ctx->enWallMode) {
        case TEST_GDC_WALL_MODE_1P_ADD_3:
            test_mpi_gdc_wall_mount_1p_add_3(ctx);
        break;
        case TEST_GDC_WALL_MODE_1P_ADD_8:
            test_mpi_gdc_wall_mount_1p_add_8(ctx);
        break;
        default:
            RK_LOGE("not support this mode:%d", ctx->enWallMode);
        break;
    }
}

static RK_BOOL test_mpi_gdc_xy_in_valid(RK_U32 xy, RK_U32 max, RK_S32 op) {
     RK_S32 result = xy + op;
     if (result < max && result >= 0)
         return RK_TRUE;
     else
         return RK_FALSE;
}

// tmp code
RK_S32 test_mpi_gdc_api(TEST_GDC_CTX_S *ctx) {
    RK_S32 s32Ret;
    GDC_HANDLE hHandle = -1;
    void *pSrcData = RK_NULL;
    RK_S32 loopCount = 0;
    FILE *pFile = NULL;
    RK_U32 u32ReadSize = 0;
    MB_POOL_CONFIG_S stMbPoolCfg;
    VIDEO_FRAME_INFO_S stFrame;
    VIDEO_FRAME_INFO_S stLineFrame;
    VO_FRAME_INFO_S voLineFrame;
    MB_BLK pLineMbBlk;

    memset(&stFrame, 0, sizeof(VIDEO_FRAME_INFO_S));
    memset(&stMbPoolCfg, 0, sizeof(MB_POOL_CONFIG_S));

    ctx->u32SrcSize = unit_test_gdc_get_size(ctx);
    stMbPoolCfg.u64MBSize = ctx->u32SrcSize;
    stMbPoolCfg.u32MBCnt = 4;
    stMbPoolCfg.enAllocType = MB_ALLOC_TYPE_DMA;
    ctx->inPool = RK_MPI_MB_CreatePool(&stMbPoolCfg);

    pFile = fopen(ctx->srcFilePath, "rb+");
    if (pFile == RK_NULL) {
        RK_LOGE("open path %s failed because %s", ctx->srcFilePath, strerror(errno));
        goto __FAILED;
    }

    // vo init and create
    ctx->s32VoLayer = RK35XX_VOP_LAYER_CLUSTER_0;
    ctx->s32VoDev = RK35XX_VO_DEV_HD0;
    ctx->s32VoChn = RK35XX_VO_GDC_CHN;
    ctx->stLayerAttr.enPixFormat = RK_FMT_BGR888;
    ctx->stLayerAttr.stDispRect.s32X = 0;
    ctx->stLayerAttr.stDispRect.s32Y = 0;
    ctx->stLayerAttr.u32DispFrmRt = 30;
    ctx->stLayerAttr.stDispRect.u32Width = ctx->u32VoWidth;
    ctx->stLayerAttr.stDispRect.u32Height = ctx->u32VoHeight;
    ctx->stLayerAttr.stImageSize.u32Width = ctx->u32VoWidth;
    ctx->stLayerAttr.stImageSize.u32Height = ctx->u32VoHeight;

    ctx->stChnAttr.stRect.s32X = ctx->stChnRect.s32X;
    ctx->stChnAttr.stRect.s32Y = ctx->stChnRect.s32Y;
    ctx->stChnAttr.stRect.u32Width = ctx->stChnRect.u32Width;
    ctx->stChnAttr.stRect.u32Height = ctx->stChnRect.u32Height;
    ctx->stChnAttr.u32Priority = 0;
    ctx->stChnAttr.u32FgAlpha = 128;
    ctx->stChnAttr.u32BgAlpha = 0;
    // ctx->stChnAttr.bGdc = RK_TRUE;  // reserve

    s32Ret = create_vo(ctx, ctx->s32VoChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("create vo dev:%d failed:0x%x", ctx->s32VoDev, s32Ret);
        goto __FAILED;
    }
    // enable vo
    s32Ret = RK_MPI_VO_EnableChn(ctx->s32VoLayer, ctx->s32VoChn);
    if (s32Ret != RK_SUCCESS) {
        RK_LOGE("Enalbe vo chn failed, s32Ret = %d", s32Ret);
        goto __FAILED;
    }
    // test vo chn127 for show line
    if (ctx->u32ShowPoint) {
        /*ctx->u32LineWidth = 640;
        ctx->u32LineHeight = 480;*/
        RK_MPI_VO_CreateGraphicsFrameBuffer(ctx->u32LineWidth, ctx->u32LineHeight, RK_FMT_BGRA5551, &pLineMbBlk);
        stLineFrame.stVFrame.u32Width = ctx->u32LineWidth;
        stLineFrame.stVFrame.u32Height = ctx->u32LineHeight;
        stLineFrame.stVFrame.u32VirWidth = ctx->u32LineWidth;
        stLineFrame.stVFrame.u32VirHeight = ctx->u32LineHeight;
        stLineFrame.stVFrame.enPixelFormat = RK_FMT_BGRA5551;
        stLineFrame.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        stLineFrame.stVFrame.pMbBlk = pLineMbBlk;

        ctx->stChnAttr.stRect.s32X = 0;
        ctx->stChnAttr.stRect.s32Y = 0;
        ctx->stChnAttr.stRect.u32Width = 960;
        ctx->stChnAttr.stRect.u32Height = 540;
        ctx->stChnAttr.u32Priority = RK35XX_VO_LINE_CHN;
        ctx->stChnAttr.u32FgAlpha = 255;
        ctx->stChnAttr.u32BgAlpha = 0;
        // ctx->stChnAttr.bGdc = RK_FALSE;  // reserve
        s32Ret = RK_MPI_VO_SetChnAttr(ctx->s32VoLayer, RK35XX_VO_LINE_CHN, &ctx->stChnAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("set chn Attr failed,s32Ret:%d\n", s32Ret);
            return RK_FAILURE;
        }
        s32Ret = RK_MPI_VO_EnableChn(ctx->s32VoLayer, RK35XX_VO_LINE_CHN);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("Enalbe vo chn failed, s32Ret = %d", s32Ret);
            goto __FAILED;
        }
    }

    // gdc test config
    if (ctx->stFisheyeAttr.enMountMode == FISHEYE_DESKTOP_MOUNT) {
        test_mpi_gdc_desktop_mount(ctx);
    } else if (ctx->stFisheyeAttr.enMountMode == FISHEYE_CEILING_MOUNT) {
        test_mpi_gdc_ceiling_mount(ctx);
    } else if (ctx->stFisheyeAttr.enMountMode == FISHEYE_WALL_MOUNT) {
        test_mpi_gdc_wall_mount(ctx);
    } else {
        RK_LOGE("not support this mount = %d", ctx->stFisheyeAttr.enMountMode);
        goto __FAILED;
    }

    while (1) {
        // img in
        ctx->stTask.stImgIn.stVFrame.pMbBlk = RK_MPI_MB_GetMB(ctx->inPool, ctx->u32SrcSize, RK_TRUE);
        if (RK_NULL == ctx->stTask.stImgIn.stVFrame.pMbBlk) {
            usleep(10000llu);
            continue;
        }
        ctx->stTask.stImgIn.stVFrame.u32Width = ctx->u32SrcWidth;
        ctx->stTask.stImgIn.stVFrame.u32Height = ctx->u32SrcHeight;
        ctx->stTask.stImgIn.stVFrame.u32VirWidth = ctx->u32SrcVirWidth;
        ctx->stTask.stImgIn.stVFrame.u32VirHeight = ctx->u32SrcVirHeight;
        ctx->stTask.stImgIn.stVFrame.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32SrcPixFormat;
        ctx->stTask.stImgIn.stVFrame.u32FrameFlag = 0;
        ctx->stTask.stImgIn.stVFrame.enCompressMode = (COMPRESS_MODE_E)ctx->s32SrcCompressMode;
        // img out
        ctx->stTask.stImgOut.stVFrame.u32Width = ctx->u32DstWidth;
        ctx->stTask.stImgOut.stVFrame.u32Height = ctx->u32DstHeight;
        ctx->stTask.stImgOut.stVFrame.u32VirWidth = ctx->u32DstWidth;
        ctx->stTask.stImgOut.stVFrame.u32VirHeight = ctx->u32DstHeight;
        ctx->stTask.stImgOut.stVFrame.enPixelFormat = (PIXEL_FORMAT_E)ctx->s32SrcPixFormat;
        ctx->stTask.stImgOut.stVFrame.u32FrameFlag = 0;
        ctx->stTask.stImgOut.stVFrame.enCompressMode = (COMPRESS_MODE_E)ctx->s32SrcCompressMode;
        ctx->stTask.stImgOut.stVFrame.pMbBlk = ctx->stTask.stImgIn.stVFrame.pMbBlk;
        // usleep(100*1000);

        // test_mpi_gdc_get_src_point(ctx);
        // test_mpi_gdc_get_pano_point(ctx);

        // read image
        pSrcData = RK_MPI_MB_Handle2VirAddr(ctx->stTask.stImgIn.stVFrame.pMbBlk);
        if (ctx->s32SrcCompressMode == COMPRESS_AFBC_16x16) {
            if (fread(pSrcData, 1, ctx->u32SrcSize, pFile) != ctx->u32SrcSize)
                s32Ret = RK_FAILURE;
            else
                s32Ret = RK_SUCCESS;
        } else {
            s32Ret = read_image(reinterpret_cast<RK_U8 *>(pSrcData), ctx->u32SrcWidth, ctx->u32SrcHeight,
                                ctx->u32SrcVirWidth, ctx->u32SrcVirHeight, ctx->s32SrcPixFormat, pFile);
        }

        if (s32Ret != RK_SUCCESS) {
            if (loopCount < ctx->s32LoopCount) {
                fseek(pFile, 0L, SEEK_SET);
                // RK_LOGI("seek:%d", loopCount);
                RK_MPI_MB_ReleaseMB(ctx->stTask.stImgIn.stVFrame.pMbBlk);
                continue;
            } else {
                RK_LOGD("loop:%d end!", loopCount);
                break;
            }
        }
        RK_MPI_SYS_MmzFlushCache(ctx->stTask.stImgIn.stVFrame.pMbBlk, RK_FALSE);
        RK_LOGD("gdc loop:%d!", loopCount);
#if 1
        // test gdc
        s32Ret = RK_MPI_GDC_BeginJob(&hHandle);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_GDC_BeginJob err:0x%x!", s32Ret);
            goto __FAILED;
        }
        s32Ret = RK_MPI_GDC_SetConfig(hHandle, &ctx->stFisheyeJobConfig);
        if (s32Ret != RK_SUCCESS) {
            RK_MPI_GDC_CancelJob(hHandle);
            RK_LOGE("RK_MPI_GDC_SetConfig err:0x%x!", s32Ret);
            goto __FAILED;
        }
        for (RK_S32 multi = 0; multi < ctx->s32TaskSum; multi++) {
            s32Ret = RK_MPI_GDC_AddCorrectionTask(hHandle, &ctx->stTask, &ctx->stFisheyeAttr);
            if (s32Ret != RK_SUCCESS) {
                RK_MPI_GDC_CancelJob(hHandle);
                RK_LOGE("RK_MPI_GDC_AddCorrectionTask err:0x%x!", s32Ret);
                goto __FAILED;
            }
            // ctx->stTask.u64TaskId = 0;
        }
#if 0  // test multi diff task
        ctx->stTask.u64TaskId = ctx->stTask.u64TaskId + 10;
        s32Ret = RK_MPI_GDC_AddCorrectionTask(hHandle, &ctx->stTask, &ctx->stFisheyeAttr);
        if (s32Ret != RK_SUCCESS) {
            RK_MPI_GDC_CancelJob(hHandle);
            RK_LOGE("RK_MPI_GDC_AddCorrectionTask err:0x%x!", s32Ret);
            goto __FAILED;
        }
#endif
        if (ctx->u32ChangeParam) {
            test_mpi_gdc_change_parameter(ctx, 1, TEST_GDC_CHANGE_TYPE_PAN);
            test_mpi_gdc_change_parameter(ctx, 2, TEST_GDC_CHANGE_TYPE_TILT);
            test_mpi_gdc_change_parameter(ctx, 3, TEST_GDC_CHANGE_TYPE_HOR_ZOOM);
            test_mpi_gdc_change_parameter(ctx, 3, TEST_GDC_CHANGE_TYPE_VER_ZOOM);
        }

        s32Ret = RK_MPI_GDC_EndJob(hHandle);
        if (s32Ret != RK_SUCCESS) {
            RK_MPI_GDC_CancelJob(hHandle);
            RK_LOGE("RK_MPI_GDC_EndJob err:0x%x!", s32Ret);
            goto __FAILED;
        }
#endif
        // gdc out show to vo
        stFrame.stVFrame = ctx->stTask.stImgOut.stVFrame;

        s32Ret = RK_MPI_VO_SendFrame(ctx->s32VoLayer, ctx->s32VoChn, &stFrame, -1);
        if (s32Ret != RK_SUCCESS) {
            RK_LOGE("RK_MPI_VO_SendFrame failed ret:0x%x", s32Ret);
            RK_MPI_VO_DestroyGraphicsFrameBuffer(ctx->stTask.stImgIn.stVFrame.pMbBlk);
        }
        // test chn 127 show point chain
        if (ctx->u32ShowPoint && (loopCount == 0 || ctx->u32ChangeParam)) {
            if (ctx->u32ChangeParam) {
                memset(RK_MPI_MB_Handle2VirAddr(pLineMbBlk), 0x0, ctx->u32LineWidth * ctx->u32LineHeight * 2);
                RK_MPI_SYS_MmzFlushCache(pLineMbBlk, RK_FALSE);
            }
            RK_U16 *pLineData = (RK_U16 *)RK_MPI_MB_Handle2VirAddr(pLineMbBlk);
            if (ctx->u32ShowPoint == 1) {
                RK_U32 x, y;
                for (RK_U32 region = 1; region < ctx->stFisheyeAttr.u32RegionNum; region++) {
                    if (ctx->stFisheyeAttr.enMountMode == FISHEYE_WALL_MOUNT) {  // pano
                        test_mpi_gdc_get_pano_point_array(ctx, region, 0);
                        for (RK_U32 l = 0; l < ctx->u32PointLen; l++) {
                            x = ctx->pastSrcPoint[l].s32X *
                                (ctx->u32LineWidth / ctx->stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Width);
                            y = ctx->pastSrcPoint[l].s32Y *
                                (ctx->u32LineHeight / ctx->stFisheyeAttr.astFishEyeRegionAttr[0].stOutRect.u32Height);
                            if (test_mpi_gdc_xy_in_valid(y * ctx->u32LineWidth + x,
                                                         ctx->u32LineWidth * ctx->u32LineHeight, 0))
                                pLineData[y * ctx->u32LineWidth + x] = gau16PointColor[region - 1];
                        }
                    } else {  // src
                        test_mpi_gdc_get_src_point_array(ctx, region);
                        for (RK_U32 l = 0; l < ctx->u32PointLen; l++) {
                            x = ctx->pastSrcPoint[l].s32X / (ctx->u32SrcWidth / ctx->u32LineWidth);
                            y = ctx->pastSrcPoint[l].s32Y / (ctx->u32SrcHeight / ctx->u32LineHeight);
                            if (test_mpi_gdc_xy_in_valid(y * ctx->u32LineWidth + x,
                                                         ctx->u32LineWidth * ctx->u32LineHeight, 0))
                                pLineData[y * ctx->u32LineWidth + x] = gau16PointColor[region - 1];
                        }
                    }
                }
            }
            RK_MPI_SYS_MmzFlushCache(pLineMbBlk, RK_FALSE);
            s32Ret = RK_MPI_VO_SendFrame(ctx->s32VoLayer, RK35XX_VO_LINE_CHN, &stLineFrame, -1);
            if (s32Ret != RK_SUCCESS) {
                RK_LOGE("RK_MPI_VO_SendFrame failed ret:0x%x", s32Ret);
                RK_MPI_VO_DestroyGraphicsFrameBuffer(pLineMbBlk);
            }
        }
        RK_MPI_MB_ReleaseMB(ctx->stTask.stImgIn.stVFrame.pMbBlk);

        loopCount++;
        if (loopCount >= ctx->s32LoopCount) {
            RK_LOGD("gdc loop:%d end!", loopCount);
            break;
        }
        usleep(1*1000);
    }

__FAILED:
    // disable vo
    RK_MPI_VO_ClearChnBuffer(ctx->s32VoLayer, ctx->s32VoChn, RK_TRUE);
    RK_MPI_VO_DisableChn(ctx->s32VoLayer, ctx->s32VoChn);
    if (ctx->u32ShowPoint) {
        RK_MPI_VO_ReleaseGfxFrameBuffer(ctx->s32VoLayer, RK35XX_VO_LINE_CHN);
        RK_MPI_VO_ClearChnBuffer(ctx->s32VoLayer, RK35XX_VO_LINE_CHN, RK_TRUE);
        RK_MPI_VO_DisableChn(ctx->s32VoLayer, RK35XX_VO_LINE_CHN);
    }
    RK_MPI_VO_DisableLayer(ctx->s32VoLayer);
    RK_MPI_VO_DisableLayer(RK35XX_VOP_LAYER_ESMART_0);
    RK_MPI_VO_DisableLayer(RK35XX_VOP_LAYER_ESMART_1);
    RK_MPI_VO_DisableLayer(RK35XX_VOP_LAYER_SMART_0);
    RK_MPI_VO_DisableLayer(RK35XX_VOP_LAYER_SMART_1);
    RK_MPI_VO_Disable(ctx->s32VoDev);

    RK_LOGE("exit err:0x%x!", s32Ret);
    RK_MPI_GDC_StopJob(hHandle);
    RK_MPI_MB_DestroyPool(ctx->inPool);

    if (pFile) {
        fclose(pFile);
    }

    return s32Ret;
}

static const char *const usages[] = {
    "./rk_mpi_gdc_test -i /data/fisheye_in_s1920_h1080_w1920_14.yuv -w 1920 -h 1080  -f 6 -l 1000"
    " --region_num 4 --show_point 1 --point_len 6000 --change_param 0",
    NULL,
};

static void mpi_gdc_test_show_options(const TEST_GDC_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name       : %s\n", ctx->srcFilePath);
    RK_PRINT("src width              : %d\n", ctx->u32SrcWidth);
    RK_PRINT("src height             : %d\n", ctx->u32SrcHeight);
    RK_PRINT("dst width              : %d\n", ctx->u32DstWidth);
    RK_PRINT("dst height             : %d\n", ctx->u32DstHeight);
    RK_PRINT("line width             : %d\n", ctx->u32LineWidth);
    RK_PRINT("line height            : %d\n", ctx->u32LineHeight);
    RK_PRINT("loop count             : %d\n", ctx->s32LoopCount);
    RK_PRINT("src pixel format       : %d\n", ctx->s32SrcPixFormat);
    RK_PRINT("compress mode          : %d\n", ctx->s32SrcCompressMode);
    RK_PRINT("task_id                : %d\n", ctx->stTask.u64TaskId);
    RK_PRINT("task_sum               : %d\n", ctx->s32TaskSum);
    RK_PRINT("stepx                  : %d\n", ctx->stTask.au64privateData[0]);
    RK_PRINT("stepy                  : %d\n", ctx->stTask.au64privateData[1]);
    RK_PRINT("regionNum              : %d\n", ctx->stFisheyeAttr.u32RegionNum);
    RK_PRINT("mount mode             : %d\n", ctx->stFisheyeAttr.enMountMode);
    RK_PRINT("desk/ceil mode         : %d\n", ctx->enDeskCeilMode);
    RK_PRINT("wall mode              : %d\n", ctx->enWallMode);
    RK_PRINT("show_point             : %d\n", ctx->u32ShowPoint);
    RK_PRINT("point_len              : %d\n", ctx->u32PointLen);
    RK_PRINT("change param           : %d\n", ctx->u32ChangeParam);
    RK_PRINT("vo out resolution      : %d\n", ctx->stVoOutputResolution);
    RK_PRINT("vo chn rect x          : %d\n", ctx->stChnRect.s32X);
    RK_PRINT("vo chn rect y          : %d\n", ctx->stChnRect.s32Y);
    RK_PRINT("vo chn rect w          : %d\n", ctx->stChnRect.u32Width);
    RK_PRINT("vo chn rect h          : %d\n", ctx->stChnRect.u32Height);

    return;
}

int main(int argc, const char **argv) {
    RK_S32 s32Ret = RK_SUCCESS;
    TEST_GDC_CTX_S *ctx;
    ctx = (TEST_GDC_CTX_S *)calloc(1, sizeof(TEST_GDC_CTX_S));
    if (!ctx) {
        RK_LOGE("calloc gdc ctx fail!");
        return 0;
    }

    //  set default params.
    ctx->s32LoopCount = 1;
    ctx->stFisheyeAttr.u32RegionNum = 4;
    ctx->s32TaskSum = 1;
    ctx->u32ChangeParam = 0;
    ctx->u32PointLen = 3000;
    ctx->stFisheyeAttr.enMountMode = FISHEYE_WALL_MOUNT;
    ctx->enWallMode = TEST_GDC_WALL_MODE_1P_ADD_3;
    ctx->enDeskCeilMode = TEST_GDC_DESK_CEIL_MODE_1_ADD_3;
    ctx->stVoOutputResolution = VO_OUTPUT_1080P30;
    ctx->u32VoWidth = 1920;
    ctx->u32VoHeight = 1080;
    ctx->stChnRect.s32X = 0;
    ctx->stChnRect.s32Y = 0;
    ctx->stChnRect.u32Width = 1920;
    ctx->stChnRect.u32Height = 1080;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),
        OPT_STRING('i', "input",  &(ctx->srcFilePath),
                   "input file name. <required>", NULL, 0, 0),
        OPT_INTEGER('w', "src_width", &(ctx->u32SrcWidth),
                    "input source width. <required>", NULL, 0, 0),
        OPT_INTEGER('h', "src_height", &(ctx->u32SrcHeight),
                    "input source height. <required>", NULL, 0, 0),
        OPT_INTEGER('W', "dst_width", &(ctx->u32DstWidth),
                    "output img width.default(src_width)", NULL, 0, 0),
        OPT_INTEGER('H', "dst_height", &(ctx->u32DstHeight),
                    "output img height.default(src_height).", NULL, 0, 0),
        OPT_INTEGER('\0', "line_width", &(ctx->u32LineWidth),
                    "line img width.(default src_width;range(0, src_width))", NULL, 0, 0),
        OPT_INTEGER('\0', "line_height", &(ctx->u32LineHeight),
                    "line img height.(default src_height;range(0, src_height)).", NULL, 0, 0),
        OPT_INTEGER('f', "pixel_format", &(ctx->s32SrcPixFormat),
                    "input source pixel format. default 0; (0:NV12 6:NV21).", NULL, 0, 0),
        OPT_INTEGER('l', "loop_count", &(ctx->s32LoopCount),
                    "loop running count. default(1)", NULL, 0, 0),
        OPT_INTEGER('\0', "src_compress_mode", &(ctx->s32SrcCompressMode),
                    "set input compressmode(default 0; 0:MODE_NONE 1:AFBC_16x16)", NULL, 0, 0),
        OPT_INTEGER('\0', "task_id", &(ctx->stTask.u64TaskId),
                    "set task id(default 0; 0:auto alloc task id >0:specify the task id)", NULL, 0, 0),
        OPT_INTEGER('\0', "stepx", &(ctx->stTask.au64privateData[0]),
                     "set step x(default 0)", NULL, 0, 0),
        OPT_INTEGER('\0', "stepy", &(ctx->stTask.au64privateData[1]),
                     "set step y(default 0)", NULL, 0, 0),
        OPT_INTEGER('\0', "region_num", &(ctx->stFisheyeAttr.u32RegionNum),
                     "set region num(default 4(1+3))", NULL, 0, 0),
        OPT_INTEGER('\0', "mount_type", &(ctx->stFisheyeAttr.enMountMode),
                     "set mount type(default 2;(0:desktop mount 1:ceiling mount 2:wall mount))", NULL, 0, 0),
        OPT_INTEGER('\0', "desk_ceil_mode", &(ctx->enDeskCeilMode),
                      "set desk or ceil mode(default 2;(0:1p+1 1:2p 2:1+3 3:1+4 4:1p+6 5:1+8))", NULL, 0, 0),
        OPT_INTEGER('\0', "wall_mode", &(ctx->enWallMode),
                      "set wall moode(default 1;(0:1p 1:1p+3 2:1p+4 3:1p+8))", NULL, 0, 0),
        OPT_INTEGER('\0', "task_sum", &(ctx->s32TaskSum),
                     "set test task sum(default 1)", NULL, 0, 0),
        OPT_INTEGER('\0', "show_point", &(ctx->u32ShowPoint),
                     "set show point chain(default 0; 0:no show point chain 1:show with scale"
                     "2:show with pano(not unrealized))", NULL, 0, 0),
        OPT_INTEGER('\0', "change_param", &(ctx->u32ChangeParam),
                     "set change param test(default 0)", NULL, 0, 0),
        OPT_INTEGER('\0', "point_len", &(ctx->u32PointLen),
                     "set point len(default 3000, when ctx->u32ShowPoint > 0 valid;"
                     "range:[0, line image perimeter])", NULL, 0, 0),
        OPT_INTEGER('\0', "vo_resolution", &(ctx->stVoOutputResolution),
                     "set change param test(default 4;(4:1080p30 10:1080p60 31:3840*2160p30))", NULL, 0, 0),
        OPT_INTEGER('\0', "vo_chn_x", &(ctx->stChnRect.s32X),
                    "vo chn attr rect x.default(0)", NULL, 0, 0),
        OPT_INTEGER('\0', "vo_chn_y", &(ctx->stChnRect.s32Y),
                    "vo chn attr rect y.default(0).", NULL, 0, 0),
        OPT_INTEGER('\0', "vo_chn_w", &(ctx->stChnRect.u32Width),
                    "vo chn attr rect w.default(1920)", NULL, 0, 0),
        OPT_INTEGER('\0', "vo_chn_h", &(ctx->stChnRect.u32Height),
                    "vo chn attr rect h.default(1080).", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);

    if (ctx->u32DstWidth == 0)
        ctx->u32DstWidth = ctx->u32SrcWidth;
    if (ctx->u32DstHeight == 0)
        ctx->u32DstHeight = ctx->u32SrcHeight;

    if (ctx->u32LineWidth == 0)
        ctx->u32LineWidth = ctx->u32SrcWidth;
    if (ctx->u32LineHeight == 0)
        ctx->u32LineHeight = ctx->u32SrcHeight;

    if (ctx->stVoOutputResolution == 31) {
        ctx->u32VoWidth = 3840;
        ctx->u32VoHeight = 2160;
    }

    mpi_gdc_test_show_options(ctx);
    if (ctx->srcFilePath == RK_NULL ||
        ctx->u32SrcWidth <= 0 ||
        ctx->u32SrcHeight <= 0) {
        argparse_usage(&argparse);
        goto __FAILED;
    }

    if (ctx->u32ShowPoint && ctx->u32PointLen) {
        ctx->pastDstPoint = (POINT_S *)malloc(sizeof(POINT_S) * ctx->u32PointLen);
        ctx->pastSrcPoint = (POINT_S *)malloc(sizeof(POINT_S) * ctx->u32PointLen);
    }

    s32Ret = RK_MPI_SYS_Init();
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    test_mpi_gdc_api(ctx);
    s32Ret = RK_MPI_SYS_Exit();
    if (s32Ret != RK_SUCCESS) {
        goto __FAILED;
    }
    RK_LOGD("gdc test done!");

__FAILED:
    if (ctx->pastDstPoint)
        free(ctx->pastDstPoint);
    if (ctx->pastSrcPoint)
        free(ctx->pastSrcPoint);
    if (ctx)
        free(ctx);

    return 0;
}
