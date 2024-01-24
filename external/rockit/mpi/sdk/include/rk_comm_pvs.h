/* GPL-2.0 WITH Linux-syscall-note OR Apache 2.0 */
/* Copyright (c) 2022 Fuzhou Rockchip Electronics Co., Ltd */

#ifndef INCLUDE_RT_MPI_RK_COMM_PVS_H_

#define INCLUDE_RT_MPI_RK_COMM_PVS_H_
#include "rk_type.h"
#include "rk_errno.h"
#include "rk_common.h"
#include "rk_comm_video.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef RK_S32 PVS_DEV;
typedef RK_S32 PVS_CHN;
#define PVS_MAX_DEV_NUM  16
#define PVS_MAX_CHN_NUM  128

typedef enum rkPVS_STITCH_MODE_E {
    STITCH_MODE_PREVIEW = 0,
    STITCH_MODE_PLAYBACK,
    STITCH_MODE_BUTT
} PVS_STITCH_MODE_E;

typedef struct rkPVS_POINT_S {
    RK_S32 s32X;
    RK_S32 s32Y;
} PVS_POINT_S;

typedef struct rkPVS_SIZE_S {
    RK_U32 u32Width;
    RK_U32 u32Height;
} PVS_SIZE_S;

typedef struct rkPVS_RECT_S {
    RK_S32 s32X;
    RK_S32 s32Y;
    RK_U32 u32Width;
    RK_U32 u32Height;
} PVS_RECT_S;

typedef struct rkPVS_CHN_ATTR_S {
    PVS_RECT_S stRect;
} PVS_CHN_ATTR_S;

typedef struct rkPVS_CHN_PARAM_S {
    RK_S32 s32ChnFrmRate;
    RK_S32 s32RecvThreshold;
    PVS_STITCH_MODE_E enStitchMod;
} PVS_CHN_PARAM_S;

typedef struct rkPVS_DEV_ATTR_S {
    PVS_SIZE_S stSize;
    PIXEL_FORMAT_E enPixelFormat;
    COMPRESS_MODE_E enCompMode;
    RK_S32 s32StitchFrmRt;
} PVS_DEV_ATTR_S;

/* invalid device ID */
#define RK_ERR_PVS_INVALID_DEVID     RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_DEVID)
/* invalid channel ID */
#define RK_ERR_PVS_INVALID_CHNID     RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_INVALID_CHNID)
/* at lease one parameter is illegal ,eg, an illegal enumeration value  */
#define RK_ERR_PVS_ILLEGAL_PARAM     RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_ILLEGAL_PARAM)
/* unexist device or channel */
#define RK_ERR_PVS_UNEXIST           RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_UNEXIST)
/* using a NULL point */
#define RK_ERR_PVS_NULL_PTR          RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_NULL_PTR)
/* attr or params not config */
#define RK_ERR_PVS_NOT_CONFIG        RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_NOT_CONFIG)
/* failure caused by malloc buffer */
#define RK_ERR_PVS_NOBUF             RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_NOBUF)
/* no data in buffer */
#define RK_ERR_PVS_BUF_EMPTY         RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_EMPTY)
/* no buffer for new data */
#define RK_ERR_PVS_BUF_FULL          RK_DEF_ERR(RK_ID_PVS, RK_ERR_LEVEL_ERROR, RK_ERR_BUF_FULL)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* INCLUDE_RT_MPI_RK_COMMON_PVS_H_ */
