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

#ifndef INCLUDE_RT_MPI_RK_COMM_GDC_H_
#define INCLUDE_RT_MPI_RK_COMM_GDC_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include "rk_type.h"
#include "rk_common.h"
#include "rk_errno.h"
#include "rk_comm_video.h"

/* failure caused by malloc buffer */
#define RK_GDC_SUCCESS             RK_SUCCESS
#define RK_ERR_GDC_NOBUF           RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_NOBUF)
#define RK_ERR_GDC_BUF_EMPTY       RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_EMPTY)
#define RK_ERR_GDC_NULL_PTR        RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
#define RK_ERR_GDC_ILLEGAL_PARAM   RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
#define RK_ERR_GDC_BUF_FULL        RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_FULL)
#define RK_ERR_GDC_SYS_NOTREADY    RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_NOTREADY)
#define RK_ERR_GDC_NOT_SUPPORT     RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_SUPPORT)
#define RK_ERR_GDC_NOT_PERMITTED   RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_PERM)
#define RK_ERR_GDC_BUSY            RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_BUSY)
#define RK_ERR_GDC_INVALID_CHNID   RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_CHNID)
#define RK_ERR_GDC_CHN_UNEXIST     RK_DEF_ERR(RK_ID_GDC, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)

#define FISHEYE_MAX_REGION_NUM     9
#define FISHEYE_LMFCOEF_NUM        128
#define GDC_PMFCOEF_NUM            9

typedef RK_S32      GDC_HANDLE;

typedef struct rkGDC_TASK_ATTR_S {
    VIDEO_FRAME_INFO_S      stImgIn;             /* Input picture */
    VIDEO_FRAME_INFO_S      stImgOut;            /* Output picture */
    /* RW; Private data of task ; au64privateData[0]: stepx au64privateData[1]: stepy;
           advised to set this parameter to 0.*/
    RK_U64                  au64privateData[4];
    /* RW; Specify a task index, default 0 is not specify;[0,GDC_MAX_TASK_NUM);
           advised to set this parameter to 0*/
    RK_U64                  u64TaskId;
} GDC_TASK_ATTR_S;

/* Mount mode of device*/
typedef enum rkFISHEYE_MOUNT_MODE_E {
    FISHEYE_DESKTOP_MOUNT    = 0,        /* Desktop mount mode */
    FISHEYE_CEILING_MOUNT    = 1,        /* Ceiling mount mode */
    FISHEYE_WALL_MOUNT       = 2,        /* wall mount mode */

    FISHEYE_MOUNT_MODE_BUTT
} FISHEYE_MOUNT_MODE_E;

/* View mode of client*/
typedef enum rkFISHEYE_VIEW_MODE_E {
    FISHEYE_VIEW_360_PANORAMA   = 0,     /* 360 panorama mode of gdc correction */
    FISHEYE_VIEW_180_PANORAMA   = 1,     /* 180 panorama mode of gdc correction */
    FISHEYE_VIEW_NORMAL         = 2,     /* normal mode of gdc correction */
    FISHEYE_NO_TRANSFORMATION   = 3,     /* no gdc correction */

    FISHEYE_VIEW_MODE_BUTT
} FISHEYE_VIEW_MODE_E;

/*Fisheye region correction attribute */
typedef struct rkFISHEYE_REGION_ATTR_S {
    FISHEYE_VIEW_MODE_E     enViewMode;     /* RW; Range: [0, 3];gdc view mode */
    RK_U32                  u32InRadius;    /* RW; inner radius of gdc correction region*/
    RK_U32                  u32OutRadius;   /* RW; out radius of gdc correction region*/
    RK_U32                  u32Pan;         /* RW; Range: [0, 360] */
    RK_U32                  u32Tilt;        /* RW; Range: [0, 360] */
    RK_U32                  u32HorZoom;     /* RW; Range: [1, 4095] */
    RK_U32                  u32VerZoom;     /* RW; Range: [1, 4095] */
    RECT_S                  stOutRect;      /* RW; out Imge rectangle attribute */
} FISHEYE_REGION_ATTR_S;

typedef struct rkFISHEYE_REGION_ATTR_EX_S {
    FISHEYE_VIEW_MODE_E     enViewMode;     /* RW; Range: [0, 3];gdc view mode */
    RK_U32                  u32InRadius;    /* RW; inner radius of gdc correction region*/
    RK_U32                  u32OutRadius;   /* RW; out radius of gdc correction region*/
    RK_U32                  u32X;           /* RW; Range: [0, 4608] */
    RK_U32                  u32Y;           /* RW; Range: [0, 3456] */
    RK_U32                  u32HorZoom;     /* RW; Range: [1, 4095] */
    RK_U32                  u32VerZoom;     /* RW; Range: [1, 4095] */
    RECT_S                  stOutRect;      /* RW; out Imge rectangle attribute */
} FISHEYE_REGION_ATTR_EX_S;

/*Fisheye all regions correction attribute */
typedef struct rkFISHEYE_ATTR_S {
    RK_BOOL                 bEnable;    /* RW; Range: [0, 1];whether enable fisheye correction or not */
    /* RW; Range: [0, 1];whether gdc len's LMF coefficient is from user config or from default linear config */
    RK_BOOL                 bLMF;
    RK_BOOL                 bBgColor;   /* RW; Range: [0, 1];whether use background color or not */
    RK_U32                  u32BgColor; /* RW; Range: [0,0xffffff];the background color RGB888*/
    /* RW; Range: [-511, 511];the horizontal offset between image center and physical center of len*/
    RK_S32                  s32HorOffset;
    /* RW; Range: [-511, 511]; the vertical offset between image center and physical center of len*/
    RK_S32                  s32VerOffset;
    RK_U32                  u32TrapezoidCoef;  /* RW; Range: [0, 32];strength coefficient of trapezoid correction */
    RK_S32                  s32FanStrength;    /* RW; Range: [-760, 760];strength coefficient of fan correction */
    FISHEYE_MOUNT_MODE_E    enMountMode;       /* RW; Range: [0, 2];gdc mount mode */
    RK_U32                  u32RegionNum;      /* RW; Range: [1, 9]; gdc correction region number */
    /* RW; attribution of gdc correction region */
    FISHEYE_REGION_ATTR_S   astFishEyeRegionAttr[FISHEYE_MAX_REGION_NUM];
} FISHEYE_ATTR_S;

typedef struct rkFISHEYE_ATTR_EX_S {
    RK_BOOL                    bEnable;      /* RW; Range: [0, 1];whether enable fisheye correction or not */
    /* RW; Range: [0, 1];whether gdc len's LMF coefficient is from user config or from default linear config */
    RK_BOOL                    bLMF;
    RK_BOOL                    bBgColor;     /* RW; Range: [0, 1];whether use background color or not */
    RK_U32                     u32BgColor;   /* RW; Range: [0,0xffffff];the background color RGB888*/
    /* RW; Range: [-511, 511];the horizontal offset between image center and physical center of len*/
    RK_S32                     s32HorOffset;
    /* RW; Range: [-511, 511]; the vertical offset between image center and physical center of len*/
    RK_S32                     s32VerOffset;
    RK_U32                     u32TrapezoidCoef;  /* RW; Range: [0, 32];strength coefficient of trapezoid correction */
    RK_S32                     s32FanStrength;    /* RW; Range: [-760, 760];strength coefficient of fan correction */
    FISHEYE_MOUNT_MODE_E       enMountMode;       /* RW; Range: [0, 2];gdc mount mode */
    RK_U32                     u32RegionNum;      /* RW; Range: [1, 4]; gdc correction region number */
    /* RW; attribution of gdc correction region */
    FISHEYE_REGION_ATTR_EX_S   astFishEyeRegionAttr[FISHEYE_MAX_REGION_NUM];
} FISHEYE_ATTR_EX_S;

/*Spread correction attribute */
typedef struct rkSPREAD_ATTR_S {
    /* RW; Range: [0, 1];whether enable spread or not, When spread on, ldc DistortionRatio range should be [0, 500] */
    RK_BOOL                 bEnable;
    RK_U32                  u32SpreadCoef;      /* RW; Range: [0, 18];strength coefficient of spread correction */
    SIZE_S                  stDestSize;         /* RW; dest size of spread*/
} SPREAD_ATTR_S;

/*Fisheye Job Config */
typedef struct rkFISHEYE_JOB_CONFIG_S {
    RK_U64                  u64LenMapPhyAddr;   /* LMF coefficient Physic Addr*/
} FISHEYE_JOB_CONFIG_S;

/*Fisheye Config */
typedef struct rkFISHEYE_CONFIG_S {
    RK_U16                  au16LMFCoef[FISHEYE_LMFCOEF_NUM];     /*RW;  LMF coefficient of gdc len */
} FISHEYE_CONFIG_S;

/*Gdc PMF Attr */
typedef struct rkGDC_PMF_ATTR_S {
    RK_S64                  as64PMFCoef[GDC_PMFCOEF_NUM];         /*W;  PMF coefficient of gdc */
} GDC_PMF_ATTR_S;

typedef struct rkGDC_FISHEYE_POINT_QUERY_ATTR_S {
    RK_U32          u32RegionIndex;
    FISHEYE_ATTR_S *pstFishEyeAttr;
    RK_U16          au16LMF[FISHEYE_LMFCOEF_NUM];
} GDC_FISHEYE_POINT_QUERY_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __RK_COMM_GDC_H__ */
