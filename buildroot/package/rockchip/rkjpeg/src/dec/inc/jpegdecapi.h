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
--  Description : JPEG decoder header file
--
------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/
#ifndef __JPEGDECAPI_H__
#define __JPEGDECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. External compiler flags
    3. Module defines
    4. Local function prototypes
    5. Functions

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "vpu_type.h"
#include "decapicommon.h"
#include "allocator_drm.h"
#ifndef DRM_LINUX
#include "vpu.h"
#include "vpu_api.h"
#endif

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/
#define JPEGDEC_YCbCr400 0x080000U
#define JPEGDEC_YCbCr420_SEMIPLANAR 0x020001U
#define JPEGDEC_YCbCr422_SEMIPLANAR 0x010001U
#define JPEGDEC_YCbCr440 0x010004U
#define JPEGDEC_YCbCr411_SEMIPLANAR 0x100000U
#define JPEGDEC_YCbCr444_SEMIPLANAR 0x200000U

#define JPEGDEC_BASELINE 0x0
#define JPEGDEC_PROGRESSIVE 0x1
#define JPEGDEC_NONINTERLEAVED 0x2

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef void *JpegDecInst;

typedef enum {
    JPEGDEC_SLICE_READY = 2,
    JPEGDEC_FRAME_READY = 1,
    JPEGDEC_STRM_PROCESSED = 3,
    JPEGDEC_SCAN_PROCESSED = 4,
    JPEGDEC_OK = 0,
    JPEGDEC_ERROR = -1,
    JPEGDEC_UNSUPPORTED = -2,
    JPEGDEC_PARAM_ERROR = -3,
    JPEGDEC_MEMFAIL = -4,
    JPEGDEC_INITFAIL = -5,
    JPEGDEC_INVALID_STREAM_LENGTH = -6,
    JPEGDEC_STRM_ERROR = -7,
    JPEGDEC_INVALID_INPUT_BUFFER_SIZE = -8,
    JPEGDEC_HW_RESERVED = -9,
    JPEGDEC_INCREASE_INPUT_BUFFER = -10,
    JPEGDEC_SLICE_MODE_UNSUPPORTED = -11,
    JPEGDEC_DWL_HW_TIMEOUT = -253,
    JPEGDEC_DWL_ERROR = -254,
    JPEGDEC_HW_BUS_ERROR = -255,
    JPEGDEC_SYSTEM_ERROR = -256,

    JPEGDEC_FORMAT_NOT_SUPPORTED = -1000
} JpegDecRet;

enum {
    JPEGDEC_NO_UNITS = 0,   /* No units, X and Y specify
                                 * the pixel aspect ratio    */
    JPEGDEC_DOTS_PER_INCH = 1,  /* X and Y are dots per inch */
    JPEGDEC_DOTS_PER_CM = 2 /* X and Y are dots per cm   */
};

enum {
    JPEGDEC_THUMBNAIL_JPEG = 0x10,
    JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT = 0x11,
    JPEGDEC_NO_THUMBNAIL = 0x12
};

enum {
    JPEGDEC_IMAGE = 0,
    JPEGDEC_THUMBNAIL = 1
};

typedef struct {
    RK_U32 *pVirtualAddress;
    RK_U32 busAddress;
} JpegDecLinearMem;

/* Image information */
typedef struct {
    RK_U32 displayWidth;
    RK_U32 displayHeight;
    RK_U32 outputWidth;    /* Number of pixels/line in the image  */
    RK_U32 outputHeight;   /* Number of lines in in the image     */
    RK_U32 version;
    RK_U32 units;
    RK_U32 xDensity;
    RK_U32 yDensity;
    RK_U32 outputFormat;   /* JPEGDEC_YCbCr400
                             * JPEGDEC_YCbCr420
                             * JPEGDEC_YCbCr422
                             */
    RK_U32 codingMode; /* JPEGDEC_BASELINE
                         * JPEGDEC_PROGRESSIVE
                         * JPEGDEC_NONINTERLEAVED
                         */

    RK_U32 thumbnailType;  /* Thumbnail exist or not or not supported */
    RK_U32 displayWidthThumb;
    RK_U32 displayHeightThumb;
    RK_U32 outputWidthThumb;   /* Number of pixels/line in the image  */
    RK_U32 outputHeightThumb;  /* Number of lines in in the image     */
    RK_U32 outputFormatThumb;  /* JPEGDEC_YCbCr400
                                 * JPEGDEC_YCbCr420
                                 * JPEGDEC_YCbCr422
                                 */
    RK_U32 codingModeThumb;    /* JPEGDEC_BASELINE
                                 * JPEGDEC_PROGRESSIVE
                                 * JPEGDEC_NONINTERLEAVED
                                 */

} JpegDecImageInfo;

/* Decoder input JFIF information */
typedef struct {
    JpegDecLinearMem streamBuffer;  /* input stream buffer */
    RK_U32 streamLength;   /* input stream length or buffer size */
    RK_U32 bufferSize; /* input stream buffer size */
    RK_U32 decImageType;   /* Full image or Thumbnail to be decoded */
    RK_U32 sliceMbSet; /* slice mode: mcu rows to decode */
    JpegDecLinearMem pictureBufferY;    /* luma output address ==> if user allocated */
    JpegDecLinearMem pictureBufferCbCr; /* chroma output address ==> if user allocated */
    JpegDecLinearMem pictureBufferCr; /* chroma output address ==> if user allocated */
#ifdef DRM_LINUX
    MppBufferInfo *pstreamMem;
#else
	VPUMemLinear_t *pstreamMem;
#endif
} JpegDecInput;

/* Decoder output */
typedef struct {
    JpegDecLinearMem outputPictureY;    /* Pointer to the Luma output image */
    JpegDecLinearMem outputPictureCbCr; /* Pointer to the Chroma output image */
    JpegDecLinearMem outputPictureCr; /* Pointer to the Chroma output image */

} JpegDecOutput;

typedef struct {
    RK_U32 major;  /* API major version */
    RK_U32 minor;  /* API minor version */

} JpegDecApiVersion;

typedef struct {
    RK_U32 swBuild;    /* Software build ID */
    RK_U32 hwBuild;    /* Hardware build ID */
    DecHwConfig hwConfig;   /* hardware supported configuration */

} JpegDecBuild;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

/* Version information */
JpegDecApiVersion JpegGetAPIVersion(void);

/* Build information */
JpegDecBuild JpegDecGetBuild(void);

/* Initialization */
JpegDecRet JpegDecInit(JpegDecInst * pDecInst, RK_U32 color_conv);

/* Release */
void JpegDecRelease(JpegDecInst decInst);

/* Get image information of the JFIF */
JpegDecRet JpegDecGetImageInfo(JpegDecInst decInst,
                               JpegDecInput * pDecIn,
                               JpegDecImageInfo * pImageInfo);

/* Decode JFIF */
JpegDecRet JpegDecDecode(JpegDecInst decInst,
                         JpegDecInput * pDecIn, JpegDecOutput * pDecOut);

/*------------------------------------------------------------------------------
    Prototype of the API trace funtion. Traces all API entries and returns.
    This must be implemented by the application using the decoder API!
    Argument:
        string - trace message, a null terminated string
------------------------------------------------------------------------------*/
void JpegDecTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif
