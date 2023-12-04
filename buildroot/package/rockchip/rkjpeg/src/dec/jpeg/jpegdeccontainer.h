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
--  Description : Jpeg Decoder Container
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. Module defines
    3. Data types
    4. Function prototypes

------------------------------------------------------------------------------*/

#ifndef JPEGDECCONT_H
#define JPEGDECCONT_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "vpu_type.h"
#include "jpegdecapi.h"
#include "dwl.h"
#include "deccfg.h"
#include "decppif.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/
#ifdef _ASSERT_USED
#include <assert.h>
#endif

/* macro for assertion, used only if compiler flag _ASSERT_USED is defined */
#ifdef _ASSERT_USED
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

#define MIN_NUMBER_OF_COMPONENTS 1
#define MAX_NUMBER_OF_COMPONENTS 3

#define JPEGDEC_RK70_MIN_BUFFER 256//5120
#define JPEGDEC_RK70_MAX_BUFFER 16776960
#define JPEGDEC_MAX_SLICE_SIZE 4096
#define JPEGDEC_TABLE_SIZE 544
#define JPEGDEC_MIN_WIDTH 48
#define JPEGDEC_MIN_HEIGHT 48
#define JPEGDEC_MAX_WIDTH 4672
#define JPEGDEC_MAX_HEIGHT 4672
#define JPEGDEC_MAX_PIXEL_AMOUNT 16370688
#define JPEGDEC_MAX_WIDTH_8190 8176
#define JPEGDEC_MAX_HEIGHT_8190 8176
#define JPEGDEC_MAX_PIXEL_AMOUNT_8190 66846976
#define JPEGDEC_MAX_SLICE_SIZE_8190 8100
#define JPEGDEC_MAX_WIDTH_TN 256
#define JPEGDEC_MAX_HEIGHT_TN 256
#define JPEGDEC_YUV400 0
#define JPEGDEC_YUV420 2
#define JPEGDEC_YUV422 3
#define JPEGDEC_YUV444 4
#define JPEGDEC_YUV440 5
#define JPEGDEC_YUV411 6
#define JPEGDEC_BASELINE_TABLE_SIZE 544
#define JPEGDEC_PROGRESSIVE_TABLE_SIZE 576
#define JPEGDEC_QP_BASE 32
#define JPEGDEC_AC1_BASE 48
#define JPEGDEC_AC2_BASE 88
#define JPEGDEC_DC1_BASE 129
#define JPEGDEC_DC2_BASE 132
#define JPEGDEC_DC3_BASE 135

/* progressive */
#define JPEGDEC_COEFF_SIZE 96

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef struct {
    RK_U32 C;  /* Component id */
    RK_U32 H;  /* Horizontal sampling factor */
    RK_U32 V;  /* Vertical sampling factor */
    RK_U32 Tq; /* Quantization table destination selector */
} Components;

typedef struct {
    RK_U8 *pStartOfStream;
    RK_U8 *pCurrPos;
    RK_U32 streamBus;
    RK_U32 bitPosInByte;
    RK_U32 streamLength;
    RK_U32 readBits;
    RK_U32 appnFlag;
    RK_U32 thumbnail;
    RK_U32 returnSosMarker;
} StreamStorage;

typedef struct {
    RK_U8 *pStartOfImage;
    RK_U8 *pLum;
    RK_U8 *pCr;
    RK_U8 *pCb;
    RK_U32 imageReady;
    RK_U32 headerReady;
    RK_U32 size;
    RK_U32 sizeLuma;
    RK_U32 sizeChroma;
    RK_U32 ready;
    RK_U32 columns[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 pixelsPerRow[MAX_NUMBER_OF_COMPONENTS];
} ImageData;

typedef struct {
    RK_U32 Lf;
    RK_U32 P;
    RK_U32 Y;
    RK_U32 hwY;
    RK_U32 X;
    RK_U32 hwX;
    RK_U32 Nf; /* Number of components in frame */
    RK_U32 codingType;
    RK_U32 numMcuInFrame;
    RK_U32 numMcuInRow;
    RK_U32 mcuNumber;
    RK_U32 nextRstNumber;
    RK_U32 Ri;
    RK_U32 driPeriod;
    RK_U32 block;
    RK_U32 row;
    RK_U32 col;
    RK_U32 cIndex;
    RK_U32 *pBuffer;
    RK_U32 bufferBus;
    RK_S32 *pBufferCb;
    RK_S32 *pBufferCr;
#ifdef DRM_LINUX
    MppBufferInfo pTableBase;
#else
	VPUMemLinear_t pTableBase;
#endif
    RK_U32 numBlocks[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 blocksPerRow[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 useAcOffset[MAX_NUMBER_OF_COMPONENTS];
    Components component[MAX_NUMBER_OF_COMPONENTS];
} FrameInfo;

typedef struct {
    RK_U32 Ls;
    RK_U32 Ns;
    RK_U32 Cs[MAX_NUMBER_OF_COMPONENTS];   /* Scan component selector */
    RK_U32 Td[MAX_NUMBER_OF_COMPONENTS];   /* Selects table for DC */
    RK_U32 Ta[MAX_NUMBER_OF_COMPONENTS];   /* Selects table for AC */
    RK_U32 Ss;
    RK_U32 Se;
    RK_U32 Ah;
    RK_U32 Al;
    RK_U32 index;
    RK_S32 numIdctRows;
    RK_S32 pred[MAX_NUMBER_OF_COMPONENTS];
} ScanInfo;

typedef struct {
    RK_U32 sliceHeight;
    RK_U32 amountOfQTables;
    RK_U32 yCbCrMode;
    RK_U32 yCbCr422;
    RK_U32 column;
    RK_U32 X;
    RK_U32 Y;
    RK_U32 memSize;
    RK_U32 SliceCount;
    RK_U32 SliceReadyForPause;
    RK_U32 SliceMBCutValue;
    RK_U32 pipeline;
    RK_U32 userAllocMem;
    RK_U32 sliceMbSetValue;
    RK_U32 timeout;
    RK_U32 rlcMode;
    RK_U32 lumaPos;
    RK_U32 chromaPos;
    RK_U32 sliceStartCount;
    RK_U32 amountOfSlices;
    RK_U32 noSliceIrqForUser;
    RK_U32 sliceLimitReached;
    RK_U32 inputBufferEmpty;
    RK_U32 fillRight;
    RK_U32 fillBottom;
    RK_U32 streamEnd;
    RK_U32 streamEndFlag;
    RK_U32 inputBufferLen;
    RK_U32 inputStreaming;
    RK_U32 decodedStreamLen;
    RK_U32 init;
    RK_U32 initThumb;
    RK_U32 initBufferSize;
    RK_S32 dcRes[MAX_NUMBER_OF_COMPONENTS];
#ifdef DRM_LINUX
    MppBufferInfo outLuma;
    MppBufferInfo outChroma;
    MppBufferInfo outChroma2;
    MppBufferInfo givenOutLuma;
    MppBufferInfo givenOutChroma;
    MppBufferInfo givenOutChroma2;
#else
    VPUMemLinear_t outLuma;
    VPUMemLinear_t outChroma;
    VPUMemLinear_t outChroma2;
    VPUMemLinear_t givenOutLuma;
    VPUMemLinear_t givenOutChroma;
    VPUMemLinear_t givenOutChroma2;
#endif
    RK_S32 pred[MAX_NUMBER_OF_COMPONENTS];
    /* progressive parameters */
    RK_U32 nonInterleaved;
    RK_U32 componentId;
    RK_U32 operationType;
    RK_U32 operationTypeThumb;
    RK_U32 progressiveScanReady;
    RK_U32 nonInterleavedScanReady;
    RK_U32 allocated;
    RK_U32 yCbCrModeOrig;
    RK_U32 getInfoYCbCrMode;
    RK_U32 components[MAX_NUMBER_OF_COMPONENTS];
#ifdef DRM_LINUX
    MppBufferInfo pCoeffBase;
#else
	VPUMemLinear_t pCoeffBase;
#endif
    RK_U32 fillX;
    RK_U32 fillY;

    RK_U32 progressiveFinish;
    RK_U32 pfCompId;
    RK_U32 pfNeeded[MAX_NUMBER_OF_COMPONENTS];
#ifdef DRM_LINUX
    MppBufferInfo tmpStrm;
#else
	VPUMemLinear_t tmpStrm;
#endif
} DecInfo;

typedef struct {
#ifdef DRM_LINUX
    MppBufferInfo outLumaBuffer;
    MppBufferInfo outChromaBuffer;
    MppBufferInfo outChromaBuffer2;
#else
	VPUMemLinear_t outLumaBuffer;
    VPUMemLinear_t outChromaBuffer;
    VPUMemLinear_t outChromaBuffer2;
#endif
} JpegAsicBuffers;

typedef struct {
    RK_U32 bits[16];
    RK_U32 *vals;
    RK_U32 tableLength;
    RK_U32 start;
    RK_U32 last;
} VlcTable;

typedef struct {
    RK_U32 Lh;
    VlcTable acTable0;
    VlcTable acTable1;
    VlcTable acTable2;
    VlcTable acTable3;
    VlcTable dcTable0;
    VlcTable dcTable1;
    VlcTable dcTable2;
    VlcTable dcTable3;
    VlcTable *table;
} HuffmanTables;

typedef struct {
    RK_U32 Lq; /* Quantization table definition length */
    RK_U32 table0[64];
    RK_U32 table1[64];
    RK_U32 table2[64];
    RK_U32 table3[64];
    RK_U32 *table;
} QuantTables;

#define EXTRA_INFO_MAGIC    (0x4C4A46)

typedef struct JpegdIocExtInfoSlot_t {
    RK_U32       reg_idx;
    RK_U32       offset;
} JpegdIocExtInfoSlot;

typedef struct JpegdIocExtInfo_t {
    RK_U32                 magic; /* tell kernel that it is extra info */
    RK_U32                 cnt;
    JpegdIocExtInfoSlot    slots[5];
} JpegdIocExtInfo;

typedef struct JpegdIocRegInfo_t {
    RK_U32                 regs[/*DEC_RK70_REGISTERS*/184];
    JpegdIocExtInfo        extra_info;
} JpegdIocRegInfo;

typedef struct {
    RK_U32 jpegRegs[DEC_RK70_REGISTERS]; // add pp registers;
    JpegdIocRegInfo ioctl_info;
    RK_U32 asicRunning;
    StreamStorage stream;
    FrameInfo frame;
    ImageData image;
    ScanInfo scan;
    DecInfo info;
    HuffmanTables vlc;
    QuantTables quant;
    RK_U32 is8190;
    RK_U32 fuseBurned;
    RK_U32 minSupportedWidth;
    RK_U32 minSupportedHeight;
    RK_U32 maxSupportedWidth;
    RK_U32 maxSupportedHeight;
    RK_U32 maxSupportedPixelAmount;
    RK_U32 maxSupportedSliceSize;
    RK_U32 extensionsSupported;
    JpegAsicBuffers asicBuff;
    DecPpInterface ppControl;
    RK_U32 ppStatus;
    int socket;    /* socket instance */
    void* vpumem_ctx;

    const void *ppInstance;
	void* ctx;
#ifdef DRM_LINUX
    MppBufferInfo *pictureMem;
#else
	VPUMemLinear_t *pictureMem;
#endif
    int dri_en;
    void *reghandle;
} JpegDecContainer;

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

#endif /* #endif JPEGDECDATA_H */

