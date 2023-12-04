/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Rockchip Products .                             --
--                                                                            --
--                   (C) COPYRIGHT 2014 ROCKCHIP PRODUCTS                     --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Description : The video post processor API
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/

#ifndef __PPAPI_H__
#define __PPAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "vpu_type.h"
#include "decapicommon.h"

#define PP_PIPELINE_DISABLED                            0U
#define PP_PIPELINED_DEC_TYPE_H264                      1U
#define PP_PIPELINED_DEC_TYPE_MPEG4                     2U
#define PP_PIPELINED_DEC_TYPE_JPEG                      3U
#define PP_PIPELINED_DEC_TYPE_VC1                       4U
#define PP_PIPELINED_DEC_TYPE_MPEG2                     5U
#define PP_PIPELINED_DEC_TYPE_VP6                       6U
#define PP_PIPELINED_DEC_TYPE_AVS                       7U  /* not supported yet */
#define PP_PIPELINED_DEC_TYPE_RV                        8U
#define PP_PIPELINED_DEC_TYPE_VP8                       9U

#define PP_PIX_FMT_YCBCR_4_0_0                          0x080000U

#define PP_PIX_FMT_YCBCR_4_2_2_INTERLEAVED              0x010001U
#define PP_PIX_FMT_YCRYCB_4_2_2_INTERLEAVED             0x010005U
#define PP_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED             0x010006U
#define PP_PIX_FMT_CRYCBY_4_2_2_INTERLEAVED             0x010007U
#define PP_PIX_FMT_YCBCR_4_2_2_SEMIPLANAR               0x010002U

#define PP_PIX_FMT_YCBCR_4_4_0                          0x010004U

#define PP_PIX_FMT_YCBCR_4_2_0_PLANAR                   0x020000U
#define PP_PIX_FMT_YCBCR_4_2_0_SEMIPLANAR               0x020001U
#define PP_PIX_FMT_YCBCR_4_2_0_TILED                    0x020002U

#define PP_PIX_FMT_YCBCR_4_1_1_SEMIPLANAR               0x100001U
#define PP_PIX_FMT_YCBCR_4_4_4_SEMIPLANAR               0x200001U

#define PP_PIX_FMT_RGB16_CUSTOM                         0x040000U
#define PP_PIX_FMT_RGB16_5_5_5                          0x040001U
#define PP_PIX_FMT_RGB16_5_6_5                          0x040002U
#define PP_PIX_FMT_BGR16_5_5_5                          0x040003U
#define PP_PIX_FMT_BGR16_5_6_5                          0x040004U

#define PP_PIX_FMT_RGB32_CUSTOM                         0x041000U
#define PP_PIX_FMT_RGB32                                0x041001U
#define PP_PIX_FMT_BGR32                                0x041002U

#define PP_YCBCR2RGB_TRANSFORM_CUSTOM                   0U
#define PP_YCBCR2RGB_TRANSFORM_BT_601                   1U
#define PP_YCBCR2RGB_TRANSFORM_BT_709                   2U

#define PP_ROTATION_NONE                                0U
#define PP_ROTATION_RIGHT_90                            1U
#define PP_ROTATION_LEFT_90                             2U
#define PP_ROTATION_HOR_FLIP                            3U
#define PP_ROTATION_VER_FLIP                            4U
#define PP_ROTATION_180                                 5U

#define PP_PIC_FRAME_OR_TOP_FIELD                       0U
#define PP_PIC_BOT_FIELD                                1U
#define PP_PIC_TOP_AND_BOT_FIELD                        2U
#define PP_PIC_TOP_AND_BOT_FIELD_FRAME                  3U
#define PP_PIC_TOP_FIELD_FRAME                          4U
#define PP_PIC_BOT_FIELD_FRAME                          5U

#define PP_MAX_MULTIBUFFER                              17

typedef const void *PPInst;

typedef enum PPResult_ {
    PP_OK = 0,
    PP_PARAM_ERROR = -1,
    PP_MEMFAIL = -4,

    PP_SET_IN_SIZE_INVALID = -64,
    PP_SET_IN_ADDRESS_INVALID = -65,
    PP_SET_IN_FORMAT_INVALID = -66,
    PP_SET_CROP_INVALID = -67,
    PP_SET_ROTATION_INVALID = -68,
    PP_SET_OUT_SIZE_INVALID = -69,
    PP_SET_OUT_ADDRESS_INVALID = -70,
    PP_SET_OUT_FORMAT_INVALID = -71,
    PP_SET_VIDEO_ADJUST_INVALID = -72,
    PP_SET_RGB_BITMASK_INVALID = -73,
    PP_SET_FRAMEBUFFER_INVALID = -74,
    PP_SET_MASK1_INVALID = -75,
    PP_SET_MASK2_INVALID = -76,
    PP_SET_DEINTERLACE_INVALID = -77,
    PP_SET_IN_STRUCT_INVALID = -78,
    PP_SET_IN_RANGE_MAP_INVALID = -79,
    PP_SET_ABLEND_UNSUPPORTED = -80,
    PP_SET_DEINTERLACING_UNSUPPORTED = -81,
    PP_SET_DITHERING_UNSUPPORTED = -82,
    PP_SET_SCALING_UNSUPPORTED = -83,

    PP_BUSY = -128,
    PP_HW_BUS_ERROR = -256,
    PP_HW_TIMEOUT = -257,
    PP_DWL_ERROR = -258,
    PP_SYSTEM_ERROR = -259,
    PP_DEC_COMBINED_MODE_ERROR = -512,
    PP_DEC_RUNTIME_ERROR = -513
} PPResult;

typedef struct PPInImage_ {
    RK_U32 pixFormat;
    RK_U32 picStruct;
    RK_U32 videoRange;
    RK_U32 width;
    RK_U32 height;
    RK_U32 bufferBusAddr;
    RK_U32 bufferCbBusAddr;
    RK_U32 bufferCrBusAddr;
    RK_U32 bufferBusAddrBot;
    RK_U32 bufferBusAddrChBot;
    RK_U32 vc1MultiResEnable;
    RK_U32 vc1RangeRedFrm;
    RK_U32 vc1RangeMapYEnable;
    RK_U32 vc1RangeMapYCoeff;
    RK_U32 vc1RangeMapCEnable;
    RK_U32 vc1RangeMapCCoeff;
} PPInImage;

typedef struct PPOutImage_ {
    RK_U32 pixFormat;
    RK_U32 width;
    RK_U32 height;
    RK_U32 bufferBusAddr;
    RK_U32 bufferChromaBusAddr;
} PPOutImage;

typedef struct PPRgbTransform_ {
    RK_U32 a;
    RK_U32 b;
    RK_U32 c;
    RK_U32 d;
    RK_U32 e;
} PPRgbTransform;

typedef struct PPRgbBitmask_ {
    RK_U32 maskR;
    RK_U32 maskG;
    RK_U32 maskB;
    RK_U32 maskAlpha;
} PPRgbBitmask;

typedef struct PPOutRgb_ {
    RK_U32 rgbTransform;
    RK_S32 contrast;
    RK_S32 brightness;
    RK_S32 saturation;
    RK_U32 alpha;
    RK_U32 transparency;
    PPRgbTransform rgbTransformCoeffs;
    PPRgbBitmask rgbBitmask;
    RK_U32 ditheringEnable;
} PPOutRgb;

typedef struct PPInCropping_ {
    RK_U32 enable;
    /* NOTE: these are coordinates relative to the input picture */
    RK_U32 originX;
    RK_U32 originY;
    RK_U32 height;
    RK_U32 width;
} PPInCropping;

typedef struct PPOutMask1_ {
    RK_U32 enable;
    /* NOTE: these are coordinates relative to the output image */
    RK_S32 originX;
    RK_S32 originY;
    RK_U32 height;
    RK_U32 width;
    RK_U32 alphaBlendEna;
    RK_U32 blendComponentBase;
} PPOutMask1;

typedef struct PPOutMask2_ {
    RK_U32 enable;
    /* NOTE: these are coordinates relative to the output image */
    RK_S32 originX;
    RK_S32 originY;
    RK_U32 height;
    RK_U32 width;
    RK_U32 alphaBlendEna;
    RK_U32 blendComponentBase;
} PPOutMask2;

typedef struct PPOutFrameBuffer_ {
    RK_U32 enable;
    /* NOTE: these are coordinates relative to the framebuffer */
    RK_S32 writeOriginX;
    RK_S32 writeOriginY;
    RK_U32 frameBufferWidth;
    RK_U32 frameBufferHeight;
} PPOutFrameBuffer;

typedef struct PPInRotation_ {
    RK_U32 rotation;
} PPInRotation;

typedef struct PPOutDeinterlace_ {
    RK_U32 enable;
} PPOutDeinterlace;

typedef struct PPConfig_ {
    PPInImage ppInImg;
    PPInCropping ppInCrop;
    PPInRotation ppInRotation;
    PPOutImage ppOutImg;
    PPOutRgb ppOutRgb;
    PPOutMask1 ppOutMask1;
    PPOutMask2 ppOutMask2;
    PPOutFrameBuffer ppOutFrmBuffer;
    PPOutDeinterlace ppOutDeinterlace;
} PPConfig;

/* Version information */
typedef struct PPApiVersion_ {
    RK_U32 major;           /* PP API major version */
    RK_U32 minor;           /* PP API minor version */
} PPApiVersion;

typedef struct PPBuild_ {
    RK_U32 swBuild;         /* Software build ID */
    RK_U32 hwBuild;         /* Hardware build ID */
    DecHwConfig hwConfig;   /* hardware supported configuration */
} PPBuild;

typedef struct PPOutput_ {
    RK_U32 bufferBusAddr;
    RK_U32 bufferChromaBusAddr;
} PPOutput;

typedef struct PPOutputBuffers_ {
    RK_U32 nbrOfBuffers;
    PPOutput ppOutputBuffers[PP_MAX_MULTIBUFFER];
} PPOutputBuffers;

/*------------------------------------------------------------------------------
    Prototypes of PP API functions
------------------------------------------------------------------------------*/
PPResult PPInit(PPInst * pPostPInst);

PPResult PPDecCombinedModeEnable(PPInst postPInst, const void *pDecInst,
                                 RK_U32 decType);

PPResult PPDecCombinedModeDisable(PPInst postPInst, const void *pDecInst);

PPResult PPGetConfig(PPInst postPInst, PPConfig * pPpConf);

PPResult PPSetConfig(PPInst postPInst, PPConfig * pPpConf);

PPResult PPDecSetMultipleOutput(PPInst postPInst,
                                const PPOutputBuffers * pBuffers);

PPResult PPGetNextOutput(PPInst postPInst, PPOutput * pOut);

void PPRelease(PPInst pPpInst);

PPResult PPGetResult(PPInst postPInst);

PPApiVersion PPGetAPIVersion(void);

PPBuild PPGetBuild(void);

void PPTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif                       /* __PPAPI_H__ */
