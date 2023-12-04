/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Rockchip Products .                             --
--                                                                            --
--                   (C) COPYRIGHT 2012 ROCKCHIP PRODUCTS                     --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description : Common Decoder API definitions
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/
#ifndef __DECAPICOMMON_H__
#define __DECAPICOMMON_H__

#include "vpu_type.h"

#define MPEG4_NOT_SUPPORTED             (RK_U32)(0x00)
#define MPEG4_SIMPLE_PROFILE            (RK_U32)(0x01)
#define MPEG4_ADVANCED_SIMPLE_PROFILE   (RK_U32)(0x02)
#define MPEG4_CUSTOM_NOT_SUPPORTED      (RK_U32)(0x00)
#define MPEG4_CUSTOM_FEATURE_1          (RK_U32)(0x01)
#define H264_NOT_SUPPORTED              (RK_U32)(0x00)
#define H264_BASELINE_PROFILE           (RK_U32)(0x01)
#define H264_MAIN_PROFILE               (RK_U32)(0x02)
#define H264_HIGH_PROFILE               (RK_U32)(0x03)
#define VC1_NOT_SUPPORTED               (RK_U32)(0x00)
#define VC1_SIMPLE_PROFILE              (RK_U32)(0x01)
#define VC1_MAIN_PROFILE                (RK_U32)(0x02)
#define VC1_ADVANCED_PROFILE            (RK_U32)(0x03)
#define MPEG2_NOT_SUPPORTED             (RK_U32)(0x00)
#define MPEG2_MAIN_PROFILE              (RK_U32)(0x01)
#define JPEG_NOT_SUPPORTED              (RK_U32)(0x00)
#define JPEG_BASELINE                   (RK_U32)(0x01)
#define JPEG_PROGRESSIVE                (RK_U32)(0x02)
#define PP_NOT_SUPPORTED                (RK_U32)(0x00)
#define PP_SUPPORTED                    (RK_U32)(0x01)
#define PP_DITHERING                    (RK_U32)(0x10000000)
#define PP_SCALING                      (RK_U32)(0x0C000000)
#define PP_DEINTERLACING                (RK_U32)(0x02000000)
#define PP_ALPHA_BLENDING               (RK_U32)(0x01000000)
#define SORENSON_SPARK_NOT_SUPPORTED    (RK_U32)(0x00)
#define SORENSON_SPARK_SUPPORTED        (RK_U32)(0x01)
#define VP6_NOT_SUPPORTED               (RK_U32)(0x00)
#define VP6_SUPPORTED                   (RK_U32)(0x01)
#define VP7_NOT_SUPPORTED               (RK_U32)(0x00)
#define VP7_SUPPORTED                   (RK_U32)(0x01)
#define VP8_NOT_SUPPORTED               (RK_U32)(0x00)
#define VP8_SUPPORTED                   (RK_U32)(0x01)
#define REF_BUF_NOT_SUPPORTED           (RK_U32)(0x00)
#define REF_BUF_SUPPORTED               (RK_U32)(0x01)
#define REF_BUF_INTERLACED              (RK_U32)(0x02)
#define REF_BUF_DOUBLE                  (RK_U32)(0x04)
#define AVS_NOT_SUPPORTED               (RK_U32)(0x00)
#define AVS_SUPPORTED                   (RK_U32)(0x01)
#define JPEG_EXT_NOT_SUPPORTED          (RK_U32)(0x00)
#define JPEG_EXT_SUPPORTED              (RK_U32)(0x01)
#define RV_NOT_SUPPORTED                (RK_U32)(0x00)
#define RV_SUPPORTED                    (RK_U32)(0x01)
#define MVC_NOT_SUPPORTED               (RK_U32)(0x00)
#define MVC_SUPPORTED                   (RK_U32)(0x01)

#define H264_NOT_SUPPORTED_FUSE                  (RK_U32)(0x00)
#define H264_FUSE_ENABLED                        (RK_U32)(0x01)
#define MPEG4_NOT_SUPPORTED_FUSE                 (RK_U32)(0x00)
#define MPEG4_FUSE_ENABLED                       (RK_U32)(0x01)
#define MPEG2_NOT_SUPPORTED_FUSE                 (RK_U32)(0x00)
#define MPEG2_FUSE_ENABLED                       (RK_U32)(0x01)
#define SORENSON_SPARK_NOT_SUPPORTED_FUSE        (RK_U32)(0x00)
#define SORENSON_SPARK_ENABLED                   (RK_U32)(0x01)
#define JPEG_NOT_SUPPORTED_FUSE                  (RK_U32)(0x00)
#define JPEG_FUSE_ENABLED                        (RK_U32)(0x01)
#define VP6_NOT_SUPPORTED_FUSE                   (RK_U32)(0x00)
#define VP6_FUSE_ENABLED                         (RK_U32)(0x01)
#define VP7_NOT_SUPPORTED_FUSE                   (RK_U32)(0x00)
#define VP7_FUSE_ENABLED                         (RK_U32)(0x01)
#define VP8_NOT_SUPPORTED_FUSE                   (RK_U32)(0x00)
#define VP8_FUSE_ENABLED                         (RK_U32)(0x01)
#define VC1_NOT_SUPPORTED_FUSE                   (RK_U32)(0x00)
#define VC1_FUSE_ENABLED                         (RK_U32)(0x01)
#define JPEG_PROGRESSIVE_NOT_SUPPORTED_FUSE      (RK_U32)(0x00)
#define JPEG_PROGRESSIVE_FUSE_ENABLED            (RK_U32)(0x01)
#define REF_BUF_NOT_SUPPORTED_FUSE               (RK_U32)(0x00)
#define REF_BUF_FUSE_ENABLED                     (RK_U32)(0x01)
#define AVS_NOT_SUPPORTED_FUSE                   (RK_U32)(0x00)
#define AVS_FUSE_ENABLED                         (RK_U32)(0x01)
#define RV_NOT_SUPPORTED_FUSE                    (RK_U32)(0x00)
#define RV_FUSE_ENABLED                          (RK_U32)(0x01)
#define MVC_NOT_SUPPORTED_FUSE                   (RK_U32)(0x00)
#define MVC_FUSE_ENABLED                         (RK_U32)(0x01)

#define PP_NOT_SUPPORTED_FUSE                    (RK_U32)(0x00)
#define PP_FUSE_ENABLED                          (RK_U32)(0x01)
#define PP_FUSE_DEINTERLACING_ENABLED            (RK_U32)(0x40000000)
#define PP_FUSE_ALPHA_BLENDING_ENABLED           (RK_U32)(0x20000000)
#define MAX_PP_OUT_WIDHT_1920_FUSE_ENABLED       (RK_U32)(0x00008000)
#define MAX_PP_OUT_WIDHT_1280_FUSE_ENABLED       (RK_U32)(0x00004000)
#define MAX_PP_OUT_WIDHT_720_FUSE_ENABLED        (RK_U32)(0x00002000)
#define MAX_PP_OUT_WIDHT_352_FUSE_ENABLED        (RK_U32)(0x00001000)

/* Macro to copy support flags and picture max width from DWL HW config
 * to Decoder HW config */
#define SET_DEC_BUILD_SUPPORT(decHwCfg, dwlHwCfg) \
    decHwCfg.maxDecPicWidth = dwlHwCfg.maxDecPicWidth; \
    decHwCfg.maxPpOutPicWidth = dwlHwCfg.maxPpOutPicWidth; \
    decHwCfg.h264Support = dwlHwCfg.h264Support; \
    decHwCfg.jpegSupport = dwlHwCfg.jpegSupport; \
    decHwCfg.jpegESupport = dwlHwCfg.jpegESupport; \
    decHwCfg.mpeg2Support = dwlHwCfg.mpeg2Support; \
    decHwCfg.mpeg4Support = dwlHwCfg.mpeg4Support; \
    decHwCfg.vc1Support = dwlHwCfg.vc1Support; \
    decHwCfg.ppSupport = dwlHwCfg.ppSupport; \
    decHwCfg.ppConfig = dwlHwCfg.ppConfig; \
    decHwCfg.sorensonSparkSupport = dwlHwCfg.sorensonSparkSupport; \
    decHwCfg.vp6Support = dwlHwCfg.vp6Support; \
    decHwCfg.vp7Support = dwlHwCfg.vp7Support; \
    decHwCfg.vp8Support = dwlHwCfg.vp8Support; \
    decHwCfg.refBufSupport = dwlHwCfg.refBufSupport; \
    decHwCfg.avsSupport = dwlHwCfg.avsSupport; \
    decHwCfg.rvSupport = dwlHwCfg.rvSupport; \
    decHwCfg.customMpeg4Support = dwlHwCfg.customMpeg4Support; \
    decHwCfg.mvcSupport = dwlHwCfg.mvcSupport;

typedef struct DecHwConfig_ {
    RK_U32 mpeg4Support;        /* one of the MPEG4 values defined above */
    RK_U32 customMpeg4Support;  /* one of the MPEG4 custom values defined above */
    RK_U32 h264Support;         /* one of the H264 values defined above */
    RK_U32 vc1Support;          /* one of the VC1 values defined above */
    RK_U32 mpeg2Support;        /* one of the MPEG2 values defined above */
    RK_U32 jpegSupport;         /* one of the JPEG values defined above */
    RK_U32 jpegProgSupport;     /* one of the Progressive JPEG values defined above */
    RK_U32 maxDecPicWidth;      /* maximum picture width in decoder */
    RK_U32 ppSupport;           /* PP_SUPPORTED or PP_NOT_SUPPORTED */
    RK_U32 ppConfig;            /* Bitwise list of PP function */
    RK_U32 maxPpOutPicWidth;    /* maximum post-processor output picture width */
    RK_U32 sorensonSparkSupport;   /* one of the SORENSON_SPARK values defined above */
    RK_U32 refBufSupport;       /* one of the REF_BUF values defined above */
    RK_U32 vp6Support;          /* one of the VP6 values defined above */
    RK_U32 vp7Support;          /* one of the VP7 values defined above */
    RK_U32 vp8Support;          /* one of the VP8 values defined above */
    RK_U32 avsSupport;          /* one of the AVS values defined above */
    RK_U32 jpegESupport;        /* one of the JPEG EXT values defined above */
    RK_U32 rvSupport;           /* one of the HUKKA values defined above */
    RK_U32 mvcSupport;          /* one of the MVC values defined above */
} DecHwConfig;

#endif /* __DECAPICOMMON_H__ */
