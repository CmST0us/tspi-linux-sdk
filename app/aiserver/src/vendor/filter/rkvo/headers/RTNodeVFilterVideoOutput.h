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

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTER_VIDEO_OUTPUT_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTER_VIDEO_OUTPUT_H_

#include "RTTaskNode.h"
#include "RTMediaRockx.h"
#include "RTAIDetectResults.h"

#include "rk_debug.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_vo.h"
#include "rk_mpi_vdec.h"
#include "rk_mpi_mb.h"
#include "rk_common.h"

#define MAX_FRAME_QUEUE              8
#define MAX_TIME_OUT_MS              20

#ifndef VDEC_INT64_MIN
#define VDEC_INT64_MIN               (-0x7fffffffffffffffLL-1)
#endif

#ifndef VDEC_INT64_MAX
#define VDEC_INT64_MAX               INT64_C(9223372036854775807)
#endif

#ifndef rk_safe_free
#define rk_safe_free(p)              { if (p) {free(p); (p)=RK_NULL;} }
#endif

#ifndef rk_safe_delete
#define rk_safe_delete(p)            { if (p) {delete(p); (p)=RK_NULL;} }
#endif

#define VDEC_ARRAY_ELEMS(a)          (sizeof(a) / sizeof((a)[0]))

typedef struct _rkWbcCfg {
    VO_WBC_SOURCE_S stWbcSource;
    VO_WBC_ATTR_S stWbcAttr;
    RK_S32 s32ChnId;
    RK_S32 s32VdecChnId;
} WBC_CFG;

typedef struct _rkVdecCfg {
    RK_U32 u32FrameBufferCnt;
    COMPRESS_MODE_E enCompressMode;
} VDEC_CFG;

typedef struct _rkVOCfg {
    RK_U32 u32Screen0VoLayer;
    RK_U32 u32Screen1VoLayer;

    RK_U32 u32Screen0Rows;
    RK_U32 u32Screen1Rows;
    RK_BOOL bDoubleScreen;
} VO_CFG;

typedef struct _rkParserCfg {
    char *srcFileUri[VDEC_MAX_CHN_NUM];
    RK_CODEC_ID_E enCodecId;
    RK_U32 u32SrcWidth;
    RK_U32 u32SrcHeight;
    RK_U32 u32StreamIndex;
   //  AVFormatContext *pstAvfc;
} PARSER_CFG;

typedef struct _rkBorderCfg {
    RK_U32 u32LeftWidth;
    RK_U32 u32RightWidth;
    RK_U32 u32TopWidth;
    RK_U32 u32BottomWidth;
} Border_CFG;

#define RK356X_VOP_LAYER_CLUSTER_0      0
#define RK356X_VOP_LAYER_CLUSTER_1      2
#define RK356X_VOP_LAYER_ESMART_0       4
#define RK356X_VOP_LAYER_ESMART_1       5
#define RK356X_VOP_LAYER_SMART_0        6
#define RK356X_VOP_LAYER_SMART_1        7

#define RK356X_VO_DEV_HD0               0
#define RK356X_VO_DEV_HD1               1

#define DISPLAY_TYPE_HDMI                0
#define DISPLAY_TYPE_EDP                 1
#define DISPLAY_TYPE_VGA                 2

#define MAX_VO_FORMAT_RGB_NUM            4
#define TEST_VO_FORMAT_ARGB8888          0
#define TEST_VO_FORMAT_ABGR8888          1
#define TEST_VO_FORMAT_RGB888            2
#define TEST_VO_FORMAT_BGR888            3
#define TEST_VO_FORMAT_ARGB1555          4
#define TEST_VO_FORMAT_ABGR1555          5
#define TEST_VO_FORMAT_NV12              6
#define TEST_VO_FORMAT_NV21              7

#define VO_CHANNEL_PLAY_NORMAL           0
#define VO_CHANNEL_PLAY_PAUSE            1
#define VO_CHANNEL_PLAY_STEP             2
#define VO_CHANNEL_PLAY_SPEED            3

#define WBC_SOURCE_DEV                0
#define WBC_SOURCE_VIDEO              1
#define WBC_SOURCE_GRAPHIC            2

#define MAX_WINDOWS_NUM               64
#define MAX_STEP_FRAME_NUM            50

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

typedef struct _VO_MODE {
    RK_U32 mode;
    VO_INTF_SYNC_E enIntfSync;
} ROCKIT_VO_MODE_S;

static VO_DEV VoLayer = RK356X_VOP_LAYER_CLUSTER_0;
static VO_DEV VoLayer_second = RK356X_VOP_LAYER_CLUSTER_1;

ROCKIT_VO_MODE_S test_mode_table[] = {
    {0,     VO_OUTPUT_640x480_60},
    {1,     VO_OUTPUT_NTSC},
    {2,     VO_OUTPUT_PAL},
    {3,     VO_OUTPUT_480P60},
    {4,     VO_OUTPUT_576P50},
    {5,     VO_OUTPUT_800x600_60},
    {6,     VO_OUTPUT_1024x768_60},
    {7,     VO_OUTPUT_720P50},
    {8,     VO_OUTPUT_720P60},
    {9,     VO_OUTPUT_1280x800_60},
    {10,    VO_OUTPUT_1280x1024_60},
    {11,    VO_OUTPUT_1366x768_60},
    {12,    VO_OUTPUT_1440x900_60},
    {13,    VO_OUTPUT_1600x1200_60},
    {14,    VO_OUTPUT_1680x1050_60},
    {15,    VO_OUTPUT_1080I50},
    {16,    VO_OUTPUT_1080I60},
    {17,    VO_OUTPUT_1080P50},
    {18,    VO_OUTPUT_1080P60},
    {19,    VO_OUTPUT_1920x1200_60},
    {20,    VO_OUTPUT_3840x2160_24},
    {21,    VO_OUTPUT_3840x2160_25},
    {22,    VO_OUTPUT_3840x2160_30},
    {23,    VO_OUTPUT_3840x2160_50},
    {24,    VO_OUTPUT_3840x2160_60},
};

typedef struct _rkMpiVOCtx {
    RK_S32  VoDev;
    RK_S32  VoLayer;
    RK_S32  VoLayerMode;

    RK_S32  u32Windows;
    RK_U32  enIntfType; /* 0 HDMI 1 edp */
    RK_U32  enIntfSync; /* VO_OUTPUT_1080P50 */
    RK_U32  s32X;
    RK_U32  s32Y;
    RK_U32  u32DispWidth;
    RK_U32  u32DispHeight;
    RK_U32  u32ImgeWidth;
    RK_U32  u32ImageHeight;
    RK_S32  s32PixFormat;
    RK_U32  u32DispFrmRt;
    RK_U32  u32DispFrmRtRatio;
    RK_U32  uEnMode;

    RK_U32  wbc_enable;
    RK_BOOL wbc_auto;
    RK_U32  ui;
    RK_U32  ui_alpha;
    RK_U32  u32WbcWidth;
    RK_S32  u32WbcHeight;
    COMPRESS_MODE_E  u32WbcCompressMode;
    RK_S32  u32WbcPixFormat;
    RK_U32  u32WbcSourceType;
    RK_U32  u32WbcSourceId;

    VO_LAYER   VoVideoLayer;
    RK_U32     u32VideoWindows;
    RK_U32     u32GFXLayers;
    VO_LAYER   VOGfxLayer;
    RK_BOOL    threadExit;

    RK_BOOL     bVoPlay;
    const char *cfgFileUri;
    RK_S32      s32LoopCount;
    RK_U32      u32ChnIndex;
    RK_U32      u32Screen0Chn;
    RK_U32      u32ChnDismode;
    RK_U32      u32Screen1Chn;
    RK_BOOL     bThreadExit;

    RK_BOOL     bEnWbc;
    RK_BOOL     bEnWbcToVO;
    RK_BOOL     bChnPriority;

    PARSER_CFG  stParserCfg;
    VDEC_CFG    stVdecCfg;
    VO_CFG      stVoCfg;
    WBC_CFG     stWbcCfg;
    Border_CFG  stBorderCfg;
} ROCKIT_VO_CTX_S;

typedef struct _VO_Send_Thread_Param {
    RK_U32 u32VideoWindows;
    VO_LAYER VoVideoLayer;
    RK_U32 u32GFXLayers;
    VO_LAYER VOGfxLayer;
    RK_U32 u32Exit;
} VoThreadParam;

#define HDMI_RKVO_DEBUG_FPS "/tmp/hdmi_rkvo_fps"

class RTNodeVFilterVideoOutput : public RTTaskNode {
 public:
    RTNodeVFilterVideoOutput();
    virtual ~RTNodeVFilterVideoOutput();

    virtual RT_RET open(RTTaskNodeContext *context);
    virtual RT_RET process(RTTaskNodeContext *context);
    virtual RT_RET close(RTTaskNodeContext *context);
 protected:
    virtual RT_RET invokeInternal(RtMetaData *meta);

 private:
    RtMutex        *mLock;
    INT32           mSrcWidth;
    INT32           mSrcHeight;
    float           mClipRatio;
    INT32           mClipWidth;
    INT32           mClipHeight;
    ROCKIT_VO_CTX_S mCtx;
    INT64           frameCount;
    timeval         beginTime;
    INT32           drmInitSuccess;
    VIDEO_FRAME_INFO_S      *pstVFrame;
};

#endif  // SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTER_VIDEO_OUTPUT
