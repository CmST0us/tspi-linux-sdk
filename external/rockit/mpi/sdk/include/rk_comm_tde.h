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

#ifndef SRC_RT_MPI_INCLUDE_RK_COMM_TDE_H_
#define SRC_RT_MPI_INCLUDE_RK_COMM_TDE_H_

#include "rk_type.h"
#include "rk_common.h"
#include "rk_comm_mb.h"
#include "rk_errno.h"
#include "rk_comm_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// error code
enum {
    RK_ERR_NOT_ALIGNED = RK_ERR_BUTT - 1,  /* <  aligned error for position, stride, width */
    RK_ERR_MINIFICATION,                   /* <  invalid minification */
    RK_ERR_CLIP_AREA,                      /* <  clip area and operation area have no intersection */
    RK_ERR_JOB_TIMEOUT,                    /* <  blocked job wait timeout */
    RK_ERR_UNSUPPORTED_OPERATION,          /* <  unsupported operation */
    RK_ERR_QUERY_TIMEOUT,                  /* <  query time out */
    RK_ERR_INTERRUPT                       /* blocked job was interrupted */
};

#define RK_ERR_TDE_DEV_NOT_OPEN           RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)
#define RK_ERR_TDE_DEV_OPEN_FAILED        RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_BUSY)
#define RK_ERR_TDE_NULL_PTR               RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
#define RK_ERR_TDE_NO_MEM                 RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_NOMEM)
#define RK_ERR_TDE_INVALID_HANDLE         RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_DEVID)
#define RK_ERR_TDE_INVALID_PARA           RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
#define RK_ERR_TDE_NOT_ALIGNED            RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_ALIGNED)
#define RK_ERR_TDE_MINIFICATION           RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_MINIFICATION)
#define RK_ERR_TDE_CLIP_AREA              RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_CLIP_AREA)
#define RK_ERR_TDE_JOB_TIMEOUT            RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_JOB_TIMEOUT)
#define RK_ERR_TDE_UNSUPPORTED_OPERATION  RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_UNSUPPORTED_OPERATION)
#define RK_ERR_TDE_QUERY_TIMEOUT          RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_QUERY_TIMEOUT)
#define RK_ERR_TDE_INTERRUPT              RK_DEF_ERR(RK_ID_TDE, RK_ERR_LEVEL_ERROR, RK_ERR_INTERRUPT)

/* Definition of the TDE handle */
typedef RK_S32 TDE_HANDLE;

typedef enum rkTDE_ALUCMD_E {
    TDE_OSD_COVER = 0x100,      /* not blend */
    TDE_OSD_DST_ALPHA = 0x105,  /* blend:src data alpha premultipy */
    TDE_OSD_ALL_ALPHA = 0x405   /* blend:src data not alpha premultipy */
} TDE_ALUCMD_E;

/* Structure of the bitmap information set by customers */
typedef struct rkTDE_SURFACE_S {
    MB_BLK pMbBlk;/* <Header address of a bitmap or the Y component */
    PIXEL_FORMAT_E enColorFmt; /* <Color format */
    RK_U32 u32Height; /* <Bitmap height */
    RK_U32 u32Width; /* <Bitmap width */
    COMPRESS_MODE_E enComprocessMode; /* compress type */
    RK_BOOL bAlphaExt1555;  /* <Whether to enable the alpha extension of an ARGB1555 bitmap. */
    RK_U8 u8Alpha0;  /* <Values of alpha0 and alpha1, used as the ARGB1555 format */
    RK_U8 u8Alpha1;  /* <Values of alpha0 and alpha1, used as the ARGB1555 format */
} TDE_SURFACE_S;

/* Definition of the TDE rectangle */
typedef struct rkTDE_RECT_S {
    RK_S32 s32Xpos;   /* <Horizontal coordinate */
    RK_S32 s32Ypos;   /* <Vertical coordinate */
    RK_U32 u32Width;  /* <Width */
    RK_U32 u32Height; /* <Height */
} TDE_RECT_S;

/* Definition of colorkey modes */
typedef enum rkTDE_COLORKEY_MODE_E {
    TDE_COLORKEY_MODE_NONE = 0,   /* <No colorkey */
     /*
      * <When performing the colorkey operation on the foreground bitmap,
      * you need to perform this operation before the CLUT for color extension and perform this operation
      * after the CLUT for color correction.
      */
    TDE_COLORKEY_MODE_FOREGROUND,
    TDE_COLORKEY_MODE_BACKGROUND, /* <Perform the colorkey operation on the background bitmap */
    TDE_COLORKEY_MODE_BUTT
} TDE_COLORKEY_MODE_E;

/* Definition of blit operation options */
typedef struct rkTDE_OPT_S {
    TDE_COLORKEY_MODE_E enColorKeyMode;  /* <Colorkey mode */
    RK_U32              unColorKeyValue; /* <Colorkey value */
    MIRROR_E            enMirror;        /* <Mirror type */
    TDE_RECT_S          stClipRect;      /* <Definition of the clipping area */
    RK_U32              u32GlobalAlpha;   /* <Global alpha value */
} TDE_OPT_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* SRC_RT_MPI_INCLUDE_RK_COMM_TDE_H_ */
