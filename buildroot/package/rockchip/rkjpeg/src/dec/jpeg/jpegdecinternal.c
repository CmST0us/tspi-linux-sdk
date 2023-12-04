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
--  Description : SW Jpeg Decoder
--
------------------------------------------------------------------------------
--
--
--

------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. External compiler flags
    3. Module defines
    4. Local function prototypes
    5. Functions
        - JpegDecClearStructs
        - JpegDecInitHW
        - JpegDecAllocateResidual
        - JpegDecWriteTables
        - JpegRefreshRegs
        - JpegFlushRegs

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vpu_type.h"
#include "jpegdeccontainer.h"
#include "jpegdecapi.h"
#include "jpegdecmarkers.h"
#include "jpegdecutils.h"
#include "jpegdechdrs.h"
#include "jpegdecscan.h"
#include "jpegregdrv.h"
#include "jpegdecinternal.h"
#include "dwl.h"
#include "deccfg.h"
#define LOG_TAG "JPEG_DEC"


#include "vpu.h"
#include "hw_jpegdecapi.h"
#include "allocator_drm.h"

#define ALOGV printf
#define ALOGD printf
#define ALOGI printf
#define ALOGW printf
#define ALOGE printf

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

static const RK_U8 zzOrder[64] = {
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

#define JPEGDEC_SLICE_START_VALUE 0
#define JPEGDEC_VLC_LEN_START_REG 134
#define JPEGDEC_VLC_LEN_END_REG 147

#ifdef PJPEG_COMPONENT_TRACE
extern RK_U32 pjpegComponentId;
extern RK_U32 *pjpegCoeffBase;
extern RK_U32 pjpegCoeffSize;

#define TRACE_COMPONENT_ID(id) pjpegComponentId = id
#else
#define TRACE_COMPONENT_ID(id)
#endif

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
static void JpegDecWriteTables(JpegDecContainer * pJpegDecCont);
static void JpegDecWriteTablesNonInterleaved(JpegDecContainer * pJpegDecCont);
static void JpegDecWriteTablesProgressive(JpegDecContainer * pJpegDecCont);
static void JpegDecChromaTableSelectors(JpegDecContainer * pJpegDecCont);
static void JpegDecSetHwStrmParams(JpegDecContainer * pJpegDecCont);
static void JpegDecWriteLenBits(JpegDecContainer * pJpegDecCont);
static void JpegDecWriteLenBitsNonInterleaved(JpegDecContainer * pJpegDecCont);
static void JpegDecWriteLenBitsProgressive(JpegDecContainer * pJpegDecCont);

/*------------------------------------------------------------------------------
    5. Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

        Function name: JpegDecClearStructs

        Functional description:
          handles the initialisation of jpeg decoder data structure

        Inputs:

        Outputs:
          Returns OK when successful, NOK in case unknown message type is
          asked

------------------------------------------------------------------------------*/
void JpegDecClearStructs(JpegDecContainer * pJpegDecCont)
{
    RK_U32 i;

    ASSERT(pJpegDecCont);

    /* stream pointers */
    pJpegDecCont->stream.thumbnail = 0;
    pJpegDecCont->stream.streamBus = 0;
    pJpegDecCont->stream.pStartOfStream = NULL;
    pJpegDecCont->stream.pCurrPos = NULL;
    pJpegDecCont->stream.bitPosInByte = 0;
    pJpegDecCont->stream.streamLength = 0;
    pJpegDecCont->stream.readBits = 0;
    pJpegDecCont->stream.appnFlag = 0;
    pJpegDecCont->stream.returnSosMarker = 0;

    /* output image pointers and variables */
    pJpegDecCont->image.pStartOfImage = NULL;
    pJpegDecCont->image.pLum = NULL;
    pJpegDecCont->image.pCr = NULL;
    pJpegDecCont->image.pCb = NULL;
    pJpegDecCont->image.headerReady = 0;
    pJpegDecCont->image.imageReady = 0;
    pJpegDecCont->image.ready = 0;
    pJpegDecCont->image.size = 0;
    pJpegDecCont->image.sizeLuma = 0;
    pJpegDecCont->image.sizeChroma = 0;
    for (i = 0; i < MAX_NUMBER_OF_COMPONENTS; i++) {
        pJpegDecCont->image.columns[i] = 0;
        pJpegDecCont->image.pixelsPerRow[i] = 0;
    }

    /* frame info */
    pJpegDecCont->frame.Lf = 0;
    pJpegDecCont->frame.P = 0;
    pJpegDecCont->frame.Y = 0;
    pJpegDecCont->frame.X = 0;
    pJpegDecCont->frame.hwY = 0;
    pJpegDecCont->frame.hwX = 0;
    pJpegDecCont->frame.Nf = 0; /* Number of components in frame */
    pJpegDecCont->frame.codingType = 0;
    pJpegDecCont->frame.numMcuInFrame = 0;
    pJpegDecCont->frame.numMcuInRow = 0;
    pJpegDecCont->frame.mcuNumber = 0;
    pJpegDecCont->frame.Ri = 0;
    pJpegDecCont->frame.row = 0;
    pJpegDecCont->frame.col = 0;
    pJpegDecCont->frame.driPeriod = 0;
    pJpegDecCont->frame.block = 0;
    pJpegDecCont->frame.cIndex = 0;
    pJpegDecCont->frame.bufferBus = 0;
    pJpegDecCont->frame.pBuffer = NULL;
    pJpegDecCont->frame.pBufferCb = NULL;
    pJpegDecCont->frame.pBufferCr = NULL;
    pJpegDecCont->frame.pTableBase.vir_addr = NULL;
    pJpegDecCont->frame.pTableBase.phy_addr = 0;

    /* asic buffer */
    pJpegDecCont->asicBuff.outLumaBuffer.vir_addr = NULL;
    pJpegDecCont->asicBuff.outChromaBuffer.vir_addr = NULL;
    pJpegDecCont->asicBuff.outChromaBuffer2.vir_addr = NULL;
    pJpegDecCont->asicBuff.outLumaBuffer.phy_addr = 0;
    pJpegDecCont->asicBuff.outChromaBuffer.phy_addr = 0;
    pJpegDecCont->asicBuff.outChromaBuffer2.phy_addr = 0;
    pJpegDecCont->asicBuff.outLumaBuffer.offset = 0;
    pJpegDecCont->asicBuff.outChromaBuffer.offset = 0;
    pJpegDecCont->asicBuff.outChromaBuffer2.offset = 0;

    /* pp instance */
    pJpegDecCont->ppInstance = NULL;
    pJpegDecCont->ppControl.usePipeline = 0;

    /* asic running flag */
    pJpegDecCont->asicRunning = 0;

    /* resolution */
    pJpegDecCont->minSupportedWidth = 0;
    pJpegDecCont->minSupportedHeight = 0;
    pJpegDecCont->maxSupportedWidth = 0;
    pJpegDecCont->maxSupportedHeight = 0;
    pJpegDecCont->maxSupportedPixelAmount = 0;
    pJpegDecCont->maxSupportedSliceSize = 0;

    /* out bus tmp */
    pJpegDecCont->info.outLuma.vir_addr = NULL;
    pJpegDecCont->info.outChroma.vir_addr = NULL;
    pJpegDecCont->info.outChroma2.vir_addr = NULL;

    /* user allocated addresses */
    pJpegDecCont->info.givenOutLuma.vir_addr = NULL;
    pJpegDecCont->info.givenOutChroma.vir_addr = NULL;
    pJpegDecCont->info.givenOutChroma2.vir_addr = NULL;

    /* image handling info */
    pJpegDecCont->info.sliceHeight = 0;
    pJpegDecCont->info.amountOfQTables = 0;
    pJpegDecCont->info.yCbCrMode = 0;
    pJpegDecCont->info.yCbCr422 = 0;
    pJpegDecCont->info.column = 0;
    pJpegDecCont->info.X = 0;
    pJpegDecCont->info.Y = 0;
    pJpegDecCont->info.memSize = 0;
    pJpegDecCont->info.SliceCount = 0;
    pJpegDecCont->info.SliceMBCutValue = 0;
    pJpegDecCont->info.pipeline = 0;
    pJpegDecCont->info.userAllocMem = 0;
    pJpegDecCont->info.sliceStartCount = 0;
    pJpegDecCont->info.amountOfSlices = 0;
    pJpegDecCont->info.noSliceIrqForUser = 0;
    pJpegDecCont->info.SliceReadyForPause = 0;
    pJpegDecCont->info.sliceLimitReached = 0;
    pJpegDecCont->info.sliceMbSetValue = 0;
    pJpegDecCont->info.timeout = (RK_U32) DEC_RK70_TIMEOUT_LENGTH;
    pJpegDecCont->info.rlcMode = 0; /* JPEG always in VLC mode == 0 */
    pJpegDecCont->info.lumaPos = 0;
    pJpegDecCont->info.chromaPos = 0;
    pJpegDecCont->info.fillRight = 0;
    pJpegDecCont->info.fillBottom = 0;
    pJpegDecCont->info.streamEnd = 0;
    pJpegDecCont->info.streamEndFlag = 0;
    pJpegDecCont->info.inputBufferEmpty = 0;
    pJpegDecCont->info.inputStreaming = 0;
    pJpegDecCont->info.inputBufferLen = 0;
    pJpegDecCont->info.decodedStreamLen = 0;
    pJpegDecCont->info.init = 0;
    pJpegDecCont->info.initThumb = 0;
    pJpegDecCont->info.initBufferSize = 0;

    /* progressive */
    pJpegDecCont->info.nonInterleaved = 0;
    pJpegDecCont->info.componentId = 0;
    pJpegDecCont->info.operationType = 0;
    pJpegDecCont->info.operationTypeThumb = 0;
    pJpegDecCont->info.progressiveScanReady = 0;
    pJpegDecCont->info.nonInterleavedScanReady = 0;
    pJpegDecCont->info.pCoeffBase.vir_addr = NULL;
    pJpegDecCont->info.pCoeffBase.phy_addr = 0;
    pJpegDecCont->info.allocated = 0;
    pJpegDecCont->info.yCbCrModeOrig = 0;
    pJpegDecCont->info.getInfoYCbCrMode = 0;
    pJpegDecCont->info.progressiveFinish = 0;
    pJpegDecCont->info.pfCompId = 0;
    for (i = 0; i < MAX_NUMBER_OF_COMPONENTS; i++)
        pJpegDecCont->info.pfNeeded[i] = 0;
    pJpegDecCont->info.tmpStrm.vir_addr = NULL;

    for (i = 0; i < MAX_NUMBER_OF_COMPONENTS; i++) {
        pJpegDecCont->info.components[i] = 0;
        pJpegDecCont->info.pred[i] = 0;
        pJpegDecCont->info.dcRes[i] = 0;
        pJpegDecCont->frame.numBlocks[i] = 0;
        pJpegDecCont->frame.blocksPerRow[i] = 0;
        pJpegDecCont->frame.useAcOffset[i] = 0;
        pJpegDecCont->frame.component[i].C = 0;
        pJpegDecCont->frame.component[i].H = 0;
        pJpegDecCont->frame.component[i].V = 0;
        pJpegDecCont->frame.component[i].Tq = 0;
    }

    /* scan info */
    pJpegDecCont->scan.Ls = 0;
    pJpegDecCont->scan.Ns = 0;
    pJpegDecCont->scan.Ss = 0;
    pJpegDecCont->scan.Se = 0;
    pJpegDecCont->scan.Ah = 0;
    pJpegDecCont->scan.Al = 0;
    pJpegDecCont->scan.index = 0;
    pJpegDecCont->scan.numIdctRows = 0;

    for (i = 0; i < MAX_NUMBER_OF_COMPONENTS; i++) {
        pJpegDecCont->scan.Cs[i] = 0;
        pJpegDecCont->scan.Td[i] = 0;
        pJpegDecCont->scan.Ta[i] = 0;
        pJpegDecCont->scan.pred[i] = 0;
    }

    /* huffman table lengths */
    pJpegDecCont->vlc.acTable0.tableLength = 0;
    pJpegDecCont->vlc.acTable1.tableLength = 0;
    pJpegDecCont->vlc.acTable2.tableLength = 0;
    pJpegDecCont->vlc.acTable3.tableLength = 0;

    pJpegDecCont->vlc.dcTable0.tableLength = 0;
    pJpegDecCont->vlc.dcTable1.tableLength = 0;
    pJpegDecCont->vlc.dcTable2.tableLength = 0;
    pJpegDecCont->vlc.dcTable3.tableLength = 0;

    /* Restart interval */
    pJpegDecCont->frame.Ri = 0;

    /* pointer initialisation */
    pJpegDecCont->vlc.acTable0.vals = NULL;
    pJpegDecCont->vlc.acTable1.vals = NULL;
    pJpegDecCont->vlc.acTable2.vals = NULL;
    pJpegDecCont->vlc.acTable3.vals = NULL;

    pJpegDecCont->vlc.dcTable0.vals = NULL;
    pJpegDecCont->vlc.dcTable1.vals = NULL;
    pJpegDecCont->vlc.dcTable2.vals = NULL;
    pJpegDecCont->vlc.dcTable3.vals = NULL;

    pJpegDecCont->frame.pBuffer = NULL;

    return;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecInitHW

        Functional description:
          Set up HW regs for decode

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          Returns OK when successful, NOK in case unknown message type is
          asked

------------------------------------------------------------------------------*/
JpegDecRet JpegDecInitHW(JpegDecContainer * pJpegDecCont)
{
    RK_U32 i;
    RK_U32 coeffBuffer = 0;
    JpegDecContainer *PTR_JPGC = pJpegDecCont;

    ASSERT(pJpegDecCont);

    TRACE_COMPONENT_ID(PTR_JPGC->info.componentId);

    /* Check if first InitHw call */
    if (PTR_JPGC->info.sliceStartCount == 0) {
        /* Check if HW resource is available */
        ;
    }

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_TIMEOUT_E, 1);

    /* frame size, round up the number of mbs */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_W_EXT,
                       ((((PTR_JPGC->info.X) >> (4)) & 0xE00) >> 9));

    /* frame size, round up the number of mbs */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_WIDTH,
                       ((PTR_JPGC->info.X) >> (4)) & 0x1FF);

    /* frame size, round up the number of mbs */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_H_EXT,
                       ((((PTR_JPGC->info.Y) >> (4)) & 0x700) >> 8));

    /* frame size, round up the number of mbs */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_HEIGHT_P,
                       ((PTR_JPGC->info.Y) >> (4)) & 0x0FF);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set decoding mode: JPEG\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_MODE, JPEG_RK70_MODE_JPEG);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set output write enabled\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_DIS, 0);        //wjm test

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set filtering disabled\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_FILTERING_DIS, 1);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set amount of QP Table\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_QTABLES,
                       PTR_JPGC->info.amountOfQTables);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set input format\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_MODE,
                       PTR_JPGC->info.yCbCrMode);

    /* In case of JPEG: Always VLC mode used (0) */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_RLC_MODE_E, PTR_JPGC->info.rlcMode);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Width is not multiple of 16\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_FILRIGHT_E, PTR_JPGC->info.fillRight);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_FILDOWN_E,
                       PTR_JPGC->info.fillBottom);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set  slice %d\n", PTR_JPGC->info.sliceHeight));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_SLICE_H, //1);    //wjm test
                       PTR_JPGC->info.sliceHeight);

    /* Set JPEG operation mode */
    if (PTR_JPGC->info.operationType != JPEGDEC_PROGRESSIVE) {
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_E, 0);
    } else {
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_E, 1);
    }

    /* Set spectral selection start coefficient */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_SS, PTR_JPGC->scan.Ss);

    /* Set spectral selection end coefficient */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_SE, PTR_JPGC->scan.Se);

    /* Set the point transform used in the preceding scan */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_AH, PTR_JPGC->scan.Ah);

    /* Set the point transform value */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_AL, PTR_JPGC->scan.Al);

    /* Set needed progressive parameters */
    if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE) {
        /* write coeff table base */
        coeffBuffer = PTR_JPGC->info.pCoeffBase.phy_addr;
        /* non-interleaved */
        if (PTR_JPGC->info.nonInterleaved) {
            for (i = 0; i < PTR_JPGC->info.componentId; i++) {
                coeffBuffer += (JPEGDEC_COEFF_SIZE *
                                PTR_JPGC->frame.numBlocks[i]);
            }
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_COEFF_BUF,
                               coeffBuffer);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_DIS, 0);
        }
        /* interleaved components */
        else {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_COEFF_BUF,
                               coeffBuffer);
            coeffBuffer += (JPEGDEC_COEFF_SIZE) * PTR_JPGC->frame.numBlocks[0];
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_DCCB_BASE,
                               coeffBuffer);
            coeffBuffer += (JPEGDEC_COEFF_SIZE) * PTR_JPGC->frame.numBlocks[1];
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_DCCR_BASE,
                               coeffBuffer);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_DIS, 1);
        }
    }

    if (PTR_JPGC->info.operationType == JPEGDEC_BASELINE) {
        /* write "length amounts" */
        JpegDecWriteLenBits(PTR_JPGC);
        /* Create AC/DC/QP tables for HW */
        JpegDecWriteTables(PTR_JPGC);
        //VPUMemClean(&(PTR_JPGC->frame.pTableBase));
        //memset(PTR_JPGC->frame.pTableBase.vir_addr, 0, PTR_JPGC->frame.pTableBase.size);
    } else if (PTR_JPGC->info.operationType == JPEGDEC_NONINTERLEAVED) {
        /* write "length amounts" */
        JpegDecWriteLenBitsNonInterleaved(PTR_JPGC);

        /* Create AC/DC/QP tables for HW */
        JpegDecWriteTablesNonInterleaved(PTR_JPGC);
    } else {    
        /* write "length amounts" */
        JpegDecWriteLenBitsProgressive(PTR_JPGC);

        /* Create AC/DC/QP tables for HW */
        JpegDecWriteTablesProgressive(PTR_JPGC);
    }
    /* Select which tables the chromas use */
    JpegDecChromaTableSelectors(PTR_JPGC);
    /* write table base */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_QTABLE_BASE,
                       PTR_JPGC->frame.pTableBase.phy_addr);

    /* set up stream position for HW decode */
    JpegDecSetHwStrmParams(PTR_JPGC);

    /* set restart interval */
    if (PTR_JPGC->frame.Ri) {
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_SYNC_MARKER_E, 1);
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_REST_FREQ,
                           PTR_JPGC->frame.Ri);
    } else
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_SYNC_MARKER_E, 0);

    /* Handle PP and output base addresses */
    if (PTR_JPGC->ppInstance != NULL && PTR_JPGC->ppControl.usePipeline) {
        JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set output write disabled\n"));
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_DIS, 1);

        /* set output to zero, because of pp */
        /* Luminance output */
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE, 0);

        /* Chrominance output */
        if (PTR_JPGC->image.sizeChroma) {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPG_CH_OUT_BASE, 0);
        }

        PTR_JPGC->info.pipeline = 1;
    } else {
        /* Luminance output */
        if (PTR_JPGC->info.operationType == JPEGDEC_BASELINE) {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE,
                               PTR_JPGC->asicBuff.outLumaBuffer.phy_addr);

            /* Chrominance output */
            if (PTR_JPGC->image.sizeChroma) {
                /* write output base */
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPG_CH_OUT_BASE,
                                   (PTR_JPGC->asicBuff.outChromaBuffer.phy_addr & 0x3FF));
            }
        } else {
            if (PTR_JPGC->info.componentId == 0) {
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE,
                                   PTR_JPGC->asicBuff.outLumaBuffer.phy_addr);
            } else if (PTR_JPGC->info.componentId == 1) {
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE,
                                   PTR_JPGC->asicBuff.outChromaBuffer.phy_addr);
            } else {
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE,
                                   (PTR_JPGC->asicBuff.outChromaBuffer2.phy_addr));
            }
        }

        PTR_JPGC->info.pipeline = 0;
    }

    PTR_JPGC->info.sliceStartCount = 1;
    PTR_JPGC->asicRunning = 1;

    /* Enable jpeg mode and set slice mode */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_E, 1);

    /* Flush regs to hw register */
    JpegFlushRegs(PTR_JPGC);

    return JPEGDEC_OK;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecInitHWContinue

        Functional description:
          Set up HW regs for decode

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          Returns OK when successful, NOK in case unknown message type is
          asked

------------------------------------------------------------------------------*/
void JpegDecInitHWContinue(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer* PTR_JPGC = pJpegDecCont;

    ASSERT(pJpegDecCont);

    /* update slice counter */
    PTR_JPGC->info.amountOfSlices++;

    if (PTR_JPGC->ppInstance == NULL &&
        PTR_JPGC->info.userAllocMem == 1 && PTR_JPGC->info.sliceStartCount > 0) {
        /* if user allocated memory ==> new addresses */
        PTR_JPGC->asicBuff.outLumaBuffer.vir_addr =
            PTR_JPGC->info.givenOutLuma.vir_addr;
        PTR_JPGC->asicBuff.outLumaBuffer.phy_addr =
            PTR_JPGC->info.givenOutLuma.phy_addr;
        PTR_JPGC->asicBuff.outChromaBuffer.vir_addr =
            PTR_JPGC->info.givenOutChroma.vir_addr;
        PTR_JPGC->asicBuff.outChromaBuffer.phy_addr =
            PTR_JPGC->info.givenOutChroma.phy_addr;
    }

    /* Update only register/values that might have been changed */

    /*************** Set swreg1 data ************/
    /* clear status bit */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_SLICE_INT, 0);

    /*************** Set swreg5 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set stream last buffer bit\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_STREAM_ALL,
                       PTR_JPGC->info.streamEnd);

    /*************** Set swreg13 data ************/
    /* PP depending register writes */
    if (PTR_JPGC->ppInstance == NULL) {
        /* Luminance output */
        JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set LUMA OUTPUT data base address\n"));
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE,
                           PTR_JPGC->asicBuff.outLumaBuffer.phy_addr);

        /*************** Set swreg14 data ************/

        /* Chrominance output */
        if (PTR_JPGC->image.sizeChroma) {
            /* write output base */
            JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set CHROMA OUTPUT data base address\n"));
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPG_CH_OUT_BASE,
                               PTR_JPGC->asicBuff.outChromaBuffer.phy_addr);
        }

        PTR_JPGC->info.pipeline = 0;
    }

    /*************** Set swreg13 data ************/
    /* PP depending register writes */
    if (PTR_JPGC->ppInstance != NULL && PTR_JPGC->ppControl.usePipeline == 0) {
        if (PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV420) {
            PTR_JPGC->info.lumaPos = (PTR_JPGC->info.X *
                                      (PTR_JPGC->info.sliceMbSetValue * 16));
            PTR_JPGC->info.chromaPos = ((PTR_JPGC->info.X) *
                                        (PTR_JPGC->info.sliceMbSetValue * 8));
        } else if (PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV422) {
            PTR_JPGC->info.lumaPos = (PTR_JPGC->info.X *
                                      (PTR_JPGC->info.sliceMbSetValue * 16));
            PTR_JPGC->info.chromaPos = ((PTR_JPGC->info.X) *
                                        (PTR_JPGC->info.sliceMbSetValue * 16));
        } else if (PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV440) {
            PTR_JPGC->info.lumaPos = (PTR_JPGC->info.X *
                                      (PTR_JPGC->info.sliceMbSetValue * 16));
            PTR_JPGC->info.chromaPos = ((PTR_JPGC->info.X) *
                                        (PTR_JPGC->info.sliceMbSetValue * 16));
        } else {
            PTR_JPGC->info.lumaPos = (PTR_JPGC->info.X *
                                      (PTR_JPGC->info.sliceMbSetValue * 16));
            PTR_JPGC->info.chromaPos = 0;
        }

        /* update luma/chroma position */
        PTR_JPGC->info.lumaPos = (PTR_JPGC->info.lumaPos *
                                  PTR_JPGC->info.amountOfSlices);
        if (PTR_JPGC->info.chromaPos) {
            PTR_JPGC->info.chromaPos = (PTR_JPGC->info.chromaPos *
                                        PTR_JPGC->info.amountOfSlices);
        }

        /* Luminance output */
        JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set LUMA OUTPUT data base address\n"));
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE,
                           PTR_JPGC->asicBuff.outLumaBuffer.phy_addr +
                           PTR_JPGC->info.lumaPos);

        /*************** Set swreg14 data ************/

        /* Chrominance output */
        if (PTR_JPGC->image.sizeChroma) {
            /* write output base */
            JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set CHROMA OUTPUT data base address\n"));
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPG_CH_OUT_BASE,
                               PTR_JPGC->asicBuff.outChromaBuffer.phy_addr +
                               PTR_JPGC->info.chromaPos);
        }

        PTR_JPGC->info.pipeline = 0;
    }

    /*************** Set swreg15 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set slice/full mode: 0 full; other = slice\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_SLICE_H,
                       PTR_JPGC->info.sliceHeight);

    /* Flush regs to hw register */
    JpegFlushRegs(pJpegDecCont);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecInitHWInputBuffLoad

        Functional description:
          Set up HW regs for decode after input buffer load

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          Returns OK when successful, NOK in case unknown message type is
          asked

------------------------------------------------------------------------------*/
void JpegDecInitHWInputBuffLoad(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;

    ASSERT(pJpegDecCont);

    /* Update only register/values that might have been changed */
    /*************** Set swreg4 data ************/
    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame width extension\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_W_EXT,
                       ((((PTR_JPGC->info.X) >> (4)) & 0xE00) >> 9));

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame width\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_WIDTH,
                       ((PTR_JPGC->info.X) >> (4)) & 0x1FF);

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame height extension\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_H_EXT,
                       ((((PTR_JPGC->info.Y) >> (4)) & 0x700) >> 8));

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame height\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_HEIGHT_P,
                       ((PTR_JPGC->info.Y) >> (4)) & 0x0FF);

    /*************** Set swreg5 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Set stream start bit\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_STRM_START_BIT,
                       PTR_JPGC->stream.bitPosInByte);

    /*************** Set swreg6 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Set stream length\n"));

    /* check if all stream will processed in this buffer */
    if ((PTR_JPGC->info.decodedStreamLen) >= PTR_JPGC->stream.streamLength) {
        PTR_JPGC->info.streamEnd = 1;
    }

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_STREAM_LEN,
                       PTR_JPGC->info.inputBufferLen);

    /*************** Set swreg4 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Set stream last buffer bit\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_STREAM_ALL,
                       PTR_JPGC->info.streamEnd);

    /*************** Set swreg12 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Set stream start address\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_RLC_VLC_BASE,
                       PTR_JPGC->stream.streamBus);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Stream bus start 0x%08x\n",
                            PTR_JPGC->stream.streamBus));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Bit position 0x%08x\n",
                            PTR_JPGC->stream.bitPosInByte));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL BUFFER LOAD: Stream length 0x%08x\n",
                            PTR_JPGC->stream.streamLength));

    /* Flush regs to hw register */
    JpegFlushRegs(pJpegDecCont);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecInitHWProgressiveContinue

        Functional description:
          Set up HW regs for decode after progressive scan decoded

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          Returns OK when successful, NOK in case unknown message type is
          asked

------------------------------------------------------------------------------*/
void JpegDecInitHWProgressiveContinue(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;

    RK_U32 i;
    RK_U32 coeffBuffer = 0;
    RK_U32 outputBuffer = 0;

    ASSERT(pJpegDecCont);

    if (PTR_JPGC->ppInstance == NULL && PTR_JPGC->info.userAllocMem == 1) {
        /* if user allocated memory ==> new addresses */
        PTR_JPGC->asicBuff.outLumaBuffer.vir_addr =
            PTR_JPGC->info.givenOutLuma.vir_addr;
        PTR_JPGC->asicBuff.outLumaBuffer.phy_addr =
            PTR_JPGC->info.givenOutLuma.phy_addr;
        PTR_JPGC->asicBuff.outChromaBuffer.vir_addr =
            PTR_JPGC->info.givenOutChroma.vir_addr;
        PTR_JPGC->asicBuff.outChromaBuffer.phy_addr =
            PTR_JPGC->info.givenOutChroma.phy_addr;
        PTR_JPGC->asicBuff.outChromaBuffer2.vir_addr =
            PTR_JPGC->info.givenOutChroma2.vir_addr;
        PTR_JPGC->asicBuff.outChromaBuffer2.phy_addr =
            PTR_JPGC->info.givenOutChroma2.phy_addr;
    }

    TRACE_COMPONENT_ID(PTR_JPGC->info.componentId);
    /* Update only register/values that might have been changed */

    /*************** Set swreg13 data ************/
    /* Luminance output */
    if (PTR_JPGC->info.componentId == 0)
        outputBuffer = PTR_JPGC->asicBuff.outLumaBuffer.phy_addr;
    else if (PTR_JPGC->info.componentId == 1)
        outputBuffer = (PTR_JPGC->asicBuff.outChromaBuffer.phy_addr);
    else
        outputBuffer = (PTR_JPGC->asicBuff.outChromaBuffer2.phy_addr);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set LUMA OUTPUT data base address\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_BASE, outputBuffer);

    PTR_JPGC->info.pipeline = 0;

    /* set up stream position for HW decode */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set stream position for HW\n"));
    JpegDecSetHwStrmParams(PTR_JPGC);

    /*************** Set swreg5 data ************/
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set input format\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_MODE,
                       PTR_JPGC->info.yCbCrMode);

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame width extension\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_W_EXT,
                       ((((PTR_JPGC->info.X) >> (4)) & 0xE00) >> 9));

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame width\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_WIDTH,
                       ((PTR_JPGC->info.X) >> (4)) & 0x1FF);

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame height extension\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_H_EXT,
                       ((((PTR_JPGC->info.Y) >> (4)) & 0x700) >> 8));

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame height\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PIC_MB_HEIGHT_P,
                       ((PTR_JPGC->info.Y) >> (4)) & 0x0FF);

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_WDIV8, PTR_JPGC->info.fillX);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_FILRIGHT_E,
                       PTR_JPGC->info.fillX || PTR_JPGC->info.fillRight);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_HDIV8, PTR_JPGC->info.fillY);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_FILDOWN_E,
                       PTR_JPGC->info.fillY || PTR_JPGC->info.fillBottom);

    /*************** Set swreg52 data ************/
    /* Set JPEG operation mode */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
    if (PTR_JPGC->info.operationType != JPEGDEC_PROGRESSIVE) {
        /* Set JPEG operation mode */
        JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_E, 0);
    } else {
        /* Set JPEG operation mode */
        JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_E, 1);
    }

    /* Set spectral selection start coefficient */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_SS, PTR_JPGC->scan.Ss);

    /* Set spectral selection end coefficient */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_SE, PTR_JPGC->scan.Se);

    /* Set the point transform used in the preceding scan */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_AH, PTR_JPGC->scan.Ah);

    /* Set the point transform value */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_AL, PTR_JPGC->scan.Al);

    /* Set needed progressive parameters */
    if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE) {
        JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set coefficient buffer base address\n"));
        coeffBuffer = PTR_JPGC->info.pCoeffBase.phy_addr;
        /* non-interleaved */
        if (PTR_JPGC->info.nonInterleaved) {
            for (i = 0; i < PTR_JPGC->info.componentId; i++) {
                coeffBuffer += (JPEGDEC_COEFF_SIZE *
                                PTR_JPGC->frame.numBlocks[i]);
            }
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_COEFF_BUF,
                               coeffBuffer);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_DIS, 0);
        }
        /* interleaved components */
        else {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_COEFF_BUF,
                               coeffBuffer);
            coeffBuffer += (JPEGDEC_COEFF_SIZE) * PTR_JPGC->frame.numBlocks[0];
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_DCCB_BASE,
                               coeffBuffer);
            coeffBuffer += (JPEGDEC_COEFF_SIZE) * PTR_JPGC->frame.numBlocks[1];
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_PJPEG_DCCR_BASE,
                               coeffBuffer);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_OUT_DIS, 1);
        }
    }

    /* write "length amounts" */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write VLC length amounts to register\n"));
    JpegDecWriteLenBitsProgressive(PTR_JPGC);

    /* Create AC/DC/QP tables for HW */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write AC,DC,QP tables to base\n"));
    JpegDecWriteTablesProgressive(PTR_JPGC);

    /* Select which tables the chromas use */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Select chroma AC,DC tables\n"));
    JpegDecChromaTableSelectors(PTR_JPGC);

    /* write table base */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set AC,DC,QP table base address\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_QTABLE_BASE,
                       PTR_JPGC->frame.pTableBase.phy_addr);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_QTABLES,
                       PTR_JPGC->info.amountOfQTables);

    if (PTR_JPGC->info.sliceMbSetValue) {
        /*************** Set swreg15 data ************/
        JPEGDEC_TRACE_INTERNAL(("INTERNAL CONTINUE: Set slice/full mode: 0 full; other = slice\n"));
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_SLICE_H,
                           PTR_JPGC->info.sliceHeight);
    }

    /* set restart interval */
    if (PTR_JPGC->frame.Ri) {
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_SYNC_MARKER_E, 1);
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_REFER13_BASE,
                           PTR_JPGC->frame.Ri);
    } else
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_SYNC_MARKER_E, 0);

    PTR_JPGC->asicRunning = 1;


    /* Enable jpeg mode and set slice mode */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Enable jpeg\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_E, 1);
    /* Flush regs to hw register */
    JpegFlushRegs(PTR_JPGC);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecSetHwStrmParams

        Functional description:
          set up hw stream start position

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecSetHwStrmParams(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    StreamStorage *JPG_STR = &pJpegDecCont->stream;

    RK_U32 addrTmp = 0;
    RK_U32 amountOfStream = 0;

    /* calculate and set stream start address to hw */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: read bits %d\n", JPG_STR->readBits));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: read bytes %d\n", JPG_STR->readBits / 8));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Stream bus start 0x%08x\n",
                            JPG_STR->streamBus));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Stream virtual start 0x%08x\n",
                            JPG_STR->pStartOfStream));

    /* calculate and set stream start address to hw */
    addrTmp = (((RK_U32) JPG_STR->pStartOfStream & 0x3) +
               (RK_U32) (JPG_STR->pCurrPos - JPG_STR->pStartOfStream)) & (~7);

    JPEGDEC_TRACE_INTERNAL(("pStartOfStream data : 0x%x, 0x%x, 0x%x, 0x%x", JPG_STR->pStartOfStream[JPG_STR->streamLength - 4],
                            JPG_STR->pStartOfStream[JPG_STR->streamLength - 3], JPG_STR->pStartOfStream[JPG_STR->streamLength - 2], JPG_STR->pStartOfStream[JPG_STR->streamLength - 1]));

   /* if (!VPUMemJudgeIommu()) {
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_RLC_VLC_BASE, (RK_U32)JPG_STR->streamBus + addrTmp);
    } else {*/
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_RLC_VLC_BASE, (RK_U32)JPG_STR->streamBus | (addrTmp << 10));
    //}

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Stream bus start 0x%08x\n",
                            JPG_STR->streamBus));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Start Addr 0x%08x\n",
                            (PTR_JPGC->reghandle)));

    /* calculate and set stream start bit to hw */

    /* change current pos to bus address style */
    /* remove three lowest bits and add the difference to bitPosInWord */
    /* used as bit pos in word not as bit pos in byte actually... */
    switch ((RK_U32) JPG_STR->pCurrPos & (7)) {
    case 0:
        break;
    case 1:
        JPG_STR->bitPosInByte += 8;
        break;
    case 2:
        JPG_STR->bitPosInByte += 16;
        break;
    case 3:
        JPG_STR->bitPosInByte += 24;
        break;
    case 4:
        JPG_STR->bitPosInByte += 32;
        break;
    case 5:
        JPG_STR->bitPosInByte += 40;
        break;
    case 6:
        JPG_STR->bitPosInByte += 48;
        break;
    case 7:
        JPG_STR->bitPosInByte += 56;
        break;
    default:
        ASSERT(0);
        break;
    }

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_STRM_START_BIT,
                       JPG_STR->bitPosInByte);

    /* set up stream length for HW.
     * length = size of original buffer - stream we already decoded in SW */
    JPG_STR->pCurrPos = (RK_U8 *) ((RK_U32) JPG_STR->pCurrPos & (~7));

    if (PTR_JPGC->info.inputStreaming) {
        amountOfStream = (PTR_JPGC->info.inputBufferLen -
                          (RK_U32) (JPG_STR->pCurrPos - JPG_STR->pStartOfStream));

        /* NOTE: on2 hardware bug, need increase length of input stream
         *       when height is not aligned as 16 Bytes.
         */
        if ((PTR_JPGC->frame.Y % 16) &&
            (PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV422 ||
             PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV444 ||
             PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV411)) {
            amountOfStream += 100; /* 100 can work so far, but may be inappropriate in some cases */
        }

        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_STREAM_LEN, amountOfStream);
        PTR_JPGC->info.streamEnd = 1;
    } else {
        amountOfStream = (JPG_STR->streamLength -
                          (RK_U32) (JPG_STR->pCurrPos - JPG_STR->pStartOfStream));

        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_STREAM_LEN, amountOfStream);

        /* because no input streaming, frame should be ready during decoding this buffer */
        PTR_JPGC->info.streamEnd = 1;
    }

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set stream last buffer bit\n"));
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_JPEG_STREAM_ALL,    //0);
                       PTR_JPGC->info.streamEnd);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: JPG_STR->streamLength %d\n",
                            JPG_STR->streamLength));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: JPG_STR->pCurrPos 0x%08x\n",
                            (RK_U32) JPG_STR->pCurrPos));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: JPG_STR->pStartOfStream 0x%08x\n",
                            (RK_U32) JPG_STR->pStartOfStream));
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: JPG_STR->bitPosInByte 0x%08x\n",
                            JPG_STR->bitPosInByte));

    return;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecAllocateResidual

        Functional description:
          Allocates residual buffer

        Inputs:
          JpegDecContainer *pJpegDecCont  Pointer to DecData structure

        Outputs:
          OK
          JPEGDEC_MEMFAIL

------------------------------------------------------------------------------*/
JpegDecRet JpegDecAllocateResidual(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    RK_S32 tmp = JPEGDEC_ERROR;
    RK_U32 numBlocks = 0;
    RK_U32 i;
    RK_U32 tableSize = 0;

    ASSERT(PTR_JPGC);

    /*if(PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE)
    {
        for(i = 0; i < PTR_JPGC->frame.Nf; i++)
        {
            numBlocks += PTR_JPGC->frame.numBlocks[i];
        }

        // allocate coefficient buffer
        tmp = VPUMallocLinear(&(PTR_JPGC->info.pCoeffBase), (sizeof(RK_U8) * (JPEGDEC_COEFF_SIZE *
                                                            numBlocks)));
        if(tmp == -1)
        {
            return (JPEGDEC_MEMFAIL);
        }
    #ifdef PJPEG_COMPONENT_TRACE
        pjpegCoeffBase = PTR_JPGC->info.pCoeffBase.vir_addr;
        pjpegCoeffSize = numBlocks * JPEGDEC_COEFF_SIZE;
    #endif

        JPEGDEC_TRACE_INTERNAL(("ALLOCATE: COEFF virtual %x bus %x\n",
                                (RK_U32) PTR_JPGC->info.pCoeffBase.vir_addr,
                                PTR_JPGC->info.pCoeffBase.phy_addr));
        if(PTR_JPGC->frame.Nf > 1)
        {
            tmp = VPUMallocLinear(&PTR_JPGC->info.tmpStrm, sizeof(RK_U8) * 100);
            if(tmp == -1)
            {
                return (JPEGDEC_MEMFAIL);
            }
        }

    }*/

    /* QP/VLC memory size */
    if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE)
        tableSize = JPEGDEC_PROGRESSIVE_TABLE_SIZE;
    else
        tableSize = JPEGDEC_BASELINE_TABLE_SIZE;
    if (PTR_JPGC->frame.pTableBase.vir_addr == NULL) {
        /* allocate VLC/QP table */
#ifdef DRM_LINUX
		if (allocator_drm.alloc) {
			printf("pTableBase alloc ************\n");
			PTR_JPGC->frame.pTableBase.size = sizeof(RK_U8) * tableSize;
			tmp = allocator_drm.alloc(PTR_JPGC->ctx, &(PTR_JPGC->frame.pTableBase));
			allocator_drm.mmap(PTR_JPGC->ctx, &(PTR_JPGC->frame.pTableBase));
			
			printf("pTableBase sucess tmp=%d************\n", tmp);
		}
#else
		tmp =  VPUMallocLinear(&(PTR_JPGC->frame.pTableBase), (sizeof(RK_U8) * tableSize));
#endif
        if (tmp == -1) {
            return (JPEGDEC_MEMFAIL);
        }
    } else {
        if (tableSize != PTR_JPGC->frame.pTableBase.size) {
#ifdef DRM_LINUX
			if (allocator_drm.free) {
	            //VPUFreeLinear(&PTR_JPGC->frame.pTableBase);
				allocator_drm.free(PTR_JPGC->ctx, &PTR_JPGC->frame.pTableBase);
			}
            /* allocate VLC/QP table */
			if (allocator_drm.alloc) {
				printf("pTableBase alloc 2************\n");
				PTR_JPGC->frame.pTableBase.size = sizeof(RK_U8) * tableSize;
				tmp = allocator_drm.alloc(PTR_JPGC->ctx, &(PTR_JPGC->frame.pTableBase));
				allocator_drm.mmap(PTR_JPGC->ctx, &(PTR_JPGC->frame.pTableBase));
            	//tmp = VPUMallocLinear(&(PTR_JPGC->frame.pTableBase), (sizeof(RK_U8) * tableSize));
			}
#else
			VPUFreeLinear(&PTR_JPGC->frame.pTableBase);
			tmp = VPUMallocLinear(&(PTR_JPGC->frame.pTableBase), (sizeof(RK_U8) * tableSize));
#endif
            if (tmp == -1) {
                return (JPEGDEC_MEMFAIL);
            }
        }
    }
	
    printf("ALLOCATE: VLC/QP virtual %p fd= 0x%x\n",
                            PTR_JPGC->frame.pTableBase.vir_addr,
                            PTR_JPGC->frame.pTableBase.phy_addr);

    /*if(PTR_JPGC->ppInstance != NULL)
    {
       // PTR_JPGC->PPConfigQuery(PTR_JPGC->ppInstance, &PTR_JPGC->ppConfigQuery);

        PTR_JPGC->ppControl.usePipeline = 1;

        if(!PTR_JPGC->ppControl.usePipeline)
        {
            PTR_JPGC->image.sizeLuma = (PTR_JPGC->info.X * PTR_JPGC->info.Y);
            if(PTR_JPGC->image.sizeChroma)
            {
                if(PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV420)
                    PTR_JPGC->image.sizeChroma = (PTR_JPGC->image.sizeLuma / 2);
                else if(PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV422 ||
                        PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV440)
                    PTR_JPGC->image.sizeChroma = PTR_JPGC->image.sizeLuma;
            }
        }
    }*/

    /* if pipelined PP -> decoder's output is not written external memory */
    if (PTR_JPGC->ppInstance == NULL ||
        (PTR_JPGC->ppInstance != NULL && !PTR_JPGC->ppControl.usePipeline)) {
        if (PTR_JPGC->info.givenOutLuma.vir_addr == NULL) {
            int timeout = 0;
#if 0
            if (PTR_JPGC->asicBuff.outLumaBuffer.phy_addr)
                VPUFreeLinear(&PTR_JPGC->asicBuff.outLumaBuffer);

            while (VPUMallocLinear(&PTR_JPGC->asicBuff.outLumaBuffer, (PTR_JPGC->image.sizeLuma + (PTR_JPGC->image.sizeChroma)))) {
                usleep(5000);
                timeout++;
                if (timeout > 0xFF) {
                    ALOGE("MJPEG Malloc outbuf Failed \n");
                    return (JPEGDEC_MEMFAIL);
                }
            }
#endif
            PTR_JPGC->asicBuff.outLumaBuffer = *PTR_JPGC->pictureMem;

            /* luma bus address to output */
            PTR_JPGC->info.outLuma = PTR_JPGC->asicBuff.outLumaBuffer;

            if (PTR_JPGC->image.sizeChroma) {
                PTR_JPGC->asicBuff.outChromaBuffer.phy_addr = PTR_JPGC->asicBuff.outLumaBuffer.phy_addr + (PTR_JPGC->image.sizeLuma << 10);

                if (PTR_JPGC->info.operationType != JPEGDEC_BASELINE) {
                    PTR_JPGC->asicBuff.outChromaBuffer2.phy_addr = PTR_JPGC->asicBuff.outChromaBuffer.phy_addr + ((PTR_JPGC->image.sizeChroma / 2) << 10);
                } else {
                    PTR_JPGC->asicBuff.outChromaBuffer2.phy_addr = 0;
                }

                // chroma bus address to output
                PTR_JPGC->info.outChroma = PTR_JPGC->asicBuff.outChromaBuffer;
                PTR_JPGC->info.outChroma2 = PTR_JPGC->asicBuff.outChromaBuffer2;
            }
        } else {
            PTR_JPGC->asicBuff.outLumaBuffer.vir_addr =
                PTR_JPGC->info.givenOutLuma.vir_addr;
            PTR_JPGC->asicBuff.outLumaBuffer.phy_addr =
                PTR_JPGC->info.givenOutLuma.phy_addr;

            PTR_JPGC->asicBuff.outChromaBuffer.vir_addr =
                PTR_JPGC->info.givenOutChroma.vir_addr;
            PTR_JPGC->asicBuff.outChromaBuffer.phy_addr =
                PTR_JPGC->info.givenOutChroma.phy_addr;
            PTR_JPGC->asicBuff.outChromaBuffer2.vir_addr =
                PTR_JPGC->info.givenOutChroma2.vir_addr;
            PTR_JPGC->asicBuff.outChromaBuffer2.phy_addr =
                PTR_JPGC->info.givenOutChroma2.phy_addr;

            PTR_JPGC->asicBuff.outLumaBuffer = *PTR_JPGC->pictureMem;
            /* luma bus address to output */
            PTR_JPGC->info.outLuma = PTR_JPGC->asicBuff.outLumaBuffer;

            if (PTR_JPGC->image.sizeChroma) {
                // chroma bus address to output
                PTR_JPGC->info.outChroma = PTR_JPGC->asicBuff.outChromaBuffer;
                PTR_JPGC->info.outChroma2 = PTR_JPGC->asicBuff.outChromaBuffer2;
            }

            /* flag to release */
            PTR_JPGC->info.userAllocMem = 1;
        }
        printf("ALLOCATE: Luma virtual %p bus 0x%x\n",
                                PTR_JPGC->asicBuff.outLumaBuffer.vir_addr,
                                PTR_JPGC->asicBuff.outLumaBuffer.phy_addr);
		
        printf("ALLOCATE: Chroma virtual 0x%x bus %x\n",
                                (RK_U32) PTR_JPGC->asicBuff.outChromaBuffer.vir_addr,
                                PTR_JPGC->asicBuff.outChromaBuffer.phy_addr);
    }

#ifdef JPEGDEC_RESET_OUTPUT
    {
        (void) DWLmemset(PTR_JPGC->asicBuff.outLumaBuffer.vir_addr,
                         128, PTR_JPGC->image.sizeLuma);
        if (PTR_JPGC->image.sizeChroma) {
            if (PTR_JPGC->info.operationType != JPEGDEC_BASELINE) {
                (void) DWLmemset(PTR_JPGC->asicBuff.outChromaBuffer.
                                 vir_addr, 128,
                                 PTR_JPGC->image.sizeChroma / 2);
                (void) DWLmemset(PTR_JPGC->asicBuff.outChromaBuffer2.
                                 vir_addr, 128,
                                 PTR_JPGC->image.sizeChroma / 2);
            } else
                (void) DWLmemset(PTR_JPGC->asicBuff.outChromaBuffer.
                                 vir_addr, 128,
                                 PTR_JPGC->image.sizeChroma);
        }
        (void) DWLmemset(PTR_JPGC->frame.pTableBase.vir_addr, 0,
                         (sizeof(RK_U8) * tableSize));
        if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE) {
            (void) DWLmemset(PTR_JPGC->info.pCoeffBase.vir_addr, 0,
                             (sizeof(RK_U8) * JPEGDEC_COEFF_SIZE * numBlocks));
        }
    }
#endif /* #ifdef JPEGDEC_RESET_OUTPUT */

    return JPEGDEC_OK;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecSliceSizeCalculation

        Functional description:
          Calculates slice size

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
void JpegDecSliceSizeCalculation(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;

    if (((PTR_JPGC->info.SliceCount +
          1) * (PTR_JPGC->info.sliceMbSetValue * 16)) > PTR_JPGC->info.Y) {
        PTR_JPGC->info.sliceHeight = ((PTR_JPGC->info.Y / 16) -
                                      (PTR_JPGC->info.SliceCount *
                                       PTR_JPGC->info.sliceHeight));
    } else {
        /* TODO! other sampling formats also than YUV420 */
        if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE &&
            PTR_JPGC->info.componentId != 0)
            PTR_JPGC->info.sliceHeight = PTR_JPGC->info.sliceMbSetValue / 2;
        else
            PTR_JPGC->info.sliceHeight = PTR_JPGC->info.sliceMbSetValue;
    }
}

/*------------------------------------------------------------------------------

        Function name: JpegDecWriteTables

        Functional description:
          Writes q/ac/dc tables to the HW format as specified in HW regs

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecWriteTables(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    RK_U32 i, j = 0;
    RK_U32 shifter = 32;
    RK_U32 tableWord = 0;
    RK_U32 tableValue = 0;
    RK_U8 tableTmp[64] = { 0 };
    RK_U32 *pTableBase = NULL;

    ASSERT(PTR_JPGC);
    ASSERT(PTR_JPGC->frame.pTableBase.vir_addr);
    ASSERT(PTR_JPGC->frame.pTableBase.phy_addr);
    ASSERT(PTR_JPGC->frame.pTableBase.size);

    pTableBase = PTR_JPGC->frame.pTableBase.vir_addr;

    /* QP tables for all components */
    for (j = 0; j < PTR_JPGC->info.amountOfQTables; j++) {
        if ((JPG_FRM->component[j].Tq) == 0) {
            for (i = 0; i < 64; i++) {
                tableTmp[zzOrder[i]] = (RK_U8) JPG_QTB->table0[i];
            }

            /* update shifter */
            shifter = 32;

            for (i = 0; i < 64; i++) {
                shifter -= 8;

                if (shifter == 24)
                    tableWord = (tableTmp[i] << shifter);
                else
                    tableWord |= (tableTmp[i] << shifter);

                if (shifter == 0) {
                    *(pTableBase) = tableWord;
                    pTableBase++;
                    shifter = 32;
                }
            }
        } else {     
            for (i = 0; i < 64; i++) {
                tableTmp[zzOrder[i]] = (RK_U8) JPG_QTB->table1[i];
            }

            /* update shifter */
            shifter = 32;

            for (i = 0; i < 64; i++) {
                shifter -= 8;

                if (shifter == 24)
                    tableWord = (tableTmp[i] << shifter);
                else
                    tableWord |= (tableTmp[i] << shifter);

                if (shifter == 0) {
                    *(pTableBase) = tableWord;
                    pTableBase++;
                    shifter = 32;
                }
            }
        }
    }

    /* update shifter */
    shifter = 32;

    if (PTR_JPGC->info.yCbCrMode != JPEGDEC_YUV400) {
        /* this trick is done because hw always wants luma table as ac hw table 1 */
        if (JPG_SCN->Ta[0] == 0) {
            /* Write AC Table 1 (as specified in HW regs)
             * NOTE: Not the same as actable[1] (as specified in JPEG Spec) */
            JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC1 (luma)\n"));
            if (JPG_VLC->acTable0.vals) {
                for (i = 0; i < 162; i++) {
                    if (i < JPG_VLC->acTable0.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->acTable0.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 162; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }
            /* Write AC Table 2 */
            JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC2 (not-luma)\n"));
            if (JPG_VLC->acTable1.vals) {
                for (i = 0; i < 162; i++) {
                    if (i < JPG_VLC->acTable1.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->acTable1.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 162; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }
        } else {
            /* Write AC Table 1 (as specified in HW regs)
             * NOTE: Not the same as actable[1] (as specified in JPEG Spec) */

            if (JPG_VLC->acTable1.vals) {
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC1 (luma)\n"));
                for (i = 0; i < 162; i++) {
                    if (i < JPG_VLC->acTable1.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->acTable1.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 162; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }

            /* Write AC Table 2 */
            if (JPG_VLC->acTable0.vals) {
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: writeTables: AC2 (not-luma)\n"));
                for (i = 0; i < 162; i++) {
                    if (i < JPG_VLC->acTable0.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->acTable0.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 162; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }
        }

        /* this trick is done because hw always wants luma table as dc hw table 1 */
        if (JPG_SCN->Td[0] == 0) {
            if (JPG_VLC->dcTable0.vals) {
                for (i = 0; i < 12; i++) {
                    if (i < JPG_VLC->dcTable0.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->dcTable0.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 12; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }

            if (JPG_VLC->dcTable1.vals) {
                for (i = 0; i < 12; i++) {
                    if (i < JPG_VLC->dcTable1.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->dcTable1.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 12; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }

        } else {
            if (JPG_VLC->dcTable1.vals) {
                for (i = 0; i < 12; i++) {
                    if (i < JPG_VLC->dcTable1.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->dcTable1.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 12; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }

            if (JPG_VLC->dcTable0.vals) {
                for (i = 0; i < 12; i++) {
                    if (i < JPG_VLC->dcTable0.tableLength) {
                        tableValue = (RK_U8) JPG_VLC->dcTable0.vals[i];
                    } else {
                        tableValue = 0;
                    }

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                for (i = 0; i < 12; i++) {
                    tableWord = 0;

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }
        }
    } else { /* YUV400 */
        if (!PTR_JPGC->info.nonInterleavedScanReady) {
            /* this trick is done because hw always wants luma table as ac hw table 1 */
            if (JPG_SCN->Ta[0] == 0) {
                /* Write AC Table 1 (as specified in HW regs)
                 * NOTE: Not the same as actable[1] (as specified in JPEG Spec) */
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC1 (luma)\n"));
                if (JPG_VLC->acTable0.vals) {
                    for (i = 0; i < 162; i++) {
                        if (i < JPG_VLC->acTable0.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->acTable0.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 162; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                /* Write AC Table 2 */
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write zero table (YUV400): \n"));
                for (i = 0; i < 162; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                /* Write AC Table 1 (as specified in HW regs)
                 * NOTE: Not the same as actable[1] (as specified in JPEG Spec) */

                if (JPG_VLC->acTable1.vals) {
                    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC1 (luma)\n"));
                    for (i = 0; i < 162; i++) {
                        if (i < JPG_VLC->acTable1.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->acTable1.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 162; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                /* Write AC Table 2 */
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: writeTables: padding zero (YUV400)\n"));
                for (i = 0; i < 162; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }

            /* this trick is done because hw always wants luma table as dc hw table 1 */
            if (JPG_SCN->Td[0] == 0) {
                if (JPG_VLC->dcTable0.vals) {
                    for (i = 0; i < 12; i++) {
                        if (i < JPG_VLC->dcTable0.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->dcTable0.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 12; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                for (i = 0; i < 12; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                if (JPG_VLC->dcTable1.vals) {
                    for (i = 0; i < 12; i++) {
                        if (i < JPG_VLC->dcTable1.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->dcTable1.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 12; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                for (i = 0; i < 12; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }
        } else {
        
            /* this trick is done because hw always wants luma table as ac hw table 1 */
            if (JPG_SCN->Ta[PTR_JPGC->info.componentId] == 0) {
                /* Write AC Table 1 (as specified in HW regs)
                 * NOTE: Not the same as actable[1] (as specified in JPEG Spec) */
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC1 (luma)\n"));
                if (JPG_VLC->acTable0.vals) {
                    for (i = 0; i < 162; i++) {
                        if (i < JPG_VLC->acTable0.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->acTable0.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 162; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                /* Write AC Table 2 */
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write zero table (YUV400): \n"));
                for (i = 0; i < 162; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                /* Write AC Table 1 (as specified in HW regs)
                 * NOTE: Not the same as actable[1] (as specified in JPEG Spec) */

                if (JPG_VLC->acTable1.vals) {
                    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write tables: AC1 (luma)\n"));
                    for (i = 0; i < 162; i++) {
                        if (i < JPG_VLC->acTable1.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->acTable1.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 162; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                /* Write AC Table 2 */
                JPEGDEC_TRACE_INTERNAL(("INTERNAL: writeTables: padding zero (YUV400)\n"));
                for (i = 0; i < 162; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }

            /* this trick is done because hw always wants luma table as dc hw table 1 */
            if (JPG_SCN->Td[PTR_JPGC->info.componentId] == 0) {
                if (JPG_VLC->dcTable0.vals) {
                    for (i = 0; i < 12; i++) {
                        if (i < JPG_VLC->dcTable0.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->dcTable0.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 12; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                for (i = 0; i < 12; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            } else {
                if (JPG_VLC->dcTable1.vals) {
                    for (i = 0; i < 12; i++) {
                        if (i < JPG_VLC->dcTable1.tableLength) {
                            tableValue = (RK_U8) JPG_VLC->dcTable1.vals[i];
                        } else {
                            tableValue = 0;
                        }

                        if (shifter == 32)
                            tableWord = (tableValue << (shifter - 8));
                        else
                            tableWord |= (tableValue << (shifter - 8));

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                } else {
                    for (i = 0; i < 12; i++) {
                        tableWord = 0;

                        shifter -= 8;

                        if (shifter == 0) {
                            *(pTableBase) = tableWord;
                            pTableBase++;
                            shifter = 32;
                        }
                    }
                }

                for (i = 0; i < 12; i++) {
                    tableValue = 0;

                    if (shifter == 32)
                        tableWord = (tableValue << (shifter - 8));
                    else
                        tableWord |= (tableValue << (shifter - 8));

                    shifter -= 8;

                    if (shifter == 0) {
                        *(pTableBase) = tableWord;
                        pTableBase++;
                        shifter = 32;
                    }
                }
            }
        }

    }

    for (i = 0; i < 4; i++) {
        tableValue = 0;

        if (shifter == 32)
            tableWord = (tableValue << (shifter - 8));
        else
            tableWord |= (tableValue << (shifter - 8));

        shifter -= 8;

        if (shifter == 0) {
            *(pTableBase) = tableWord;
            pTableBase++;
            shifter = 32;
        }
    }
	
}

/*------------------------------------------------------------------------------
        Function name: JpegDecWriteTablesNonInterleaved

        Functional description:
          Writes q/ac/dc tables to the HW format as specified in HW regs

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecWriteTablesNonInterleaved(JpegDecContainer * pJpegDecCont)
{

    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    RK_U32 i, j = 0;
    RK_U32 tableWord = 0;
    RK_U8 tableTmp[64] = { 0 };
    RK_U8 *pTmp;
    RK_U32 *pTableBase = NULL;
    RK_U32 first, count;
    RK_U32 len, numWords;
    RK_U32 *vals;
    RK_U32 *pTable;
    RK_U32 dcTable = 0;
    RK_U32 qpTableBase = 0;

    ASSERT(PTR_JPGC);
    ASSERT(PTR_JPGC->frame.pTableBase.vir_addr);
    ASSERT(PTR_JPGC->frame.pTableBase.phy_addr);
    ASSERT(PTR_JPGC->frame.pTableBase.size);
    ASSERT(PTR_JPGC->info.nonInterleaved);

    /* Reset the table memory */
    (void) DWLmemset(PTR_JPGC->frame.pTableBase.vir_addr, 0,
                     (sizeof(RK_U8) * JPEGDEC_BASELINE_TABLE_SIZE));

    pTableBase = PTR_JPGC->frame.pTableBase.vir_addr;

    first = PTR_JPGC->info.componentId;
    count = 1;

    /* QP tables for all components */
    for (j = first; j < first + count; j++) {
        if ((JPG_FRM->component[j].Tq) == 0)
            pTable = JPG_QTB->table0;
        else
            pTable = JPG_QTB->table1;

        for (i = 0; i < 64; i++) {
            tableTmp[zzOrder[i]] = (RK_U8) pTable[i];
        }

        pTmp = tableTmp;
        for (i = 0; i < 16; i++) {
            tableWord = (pTmp[0] << 24) | (pTmp[1] << 16) |
                        (pTmp[2] << 8) | (pTmp[3] << 0);;

            *pTableBase++ = tableWord;
            pTmp += 4;
        }
    }

    /* AC table */
    for (i = first; i < first + count; i++) {
        numWords = 162;
        switch (JPG_SCN->Ta[i]) {
        case 0:
            vals = JPG_VLC->acTable0.vals;
            len = JPG_VLC->acTable0.tableLength;
            break;
        case 1:
            vals = JPG_VLC->acTable1.vals;
            len = JPG_VLC->acTable1.tableLength;
            break;
        case 2:
            vals = JPG_VLC->acTable2.vals;
            len = JPG_VLC->acTable2.tableLength;
            break;
        default:
            vals = JPG_VLC->acTable3.vals;
            len = JPG_VLC->acTable3.tableLength;
            break;
        }

        /* set pointer */
        if (count == 3)
            qpTableBase = 0;
        else
            qpTableBase = JPEGDEC_QP_BASE;

        pTableBase =
            &PTR_JPGC->frame.pTableBase.vir_addr[JPEGDEC_AC1_BASE -
                                                 qpTableBase];

        for (j = 0; j < numWords; j++) {
            tableWord <<= 8;
            if (j < len)
                tableWord |= vals[j];

            if ((j & 0x3) == 0x3)
                *pTableBase++ = tableWord;
        }

        /* fill to border */
        numWords = 164;
        len = 164;
        for (j = 162; j < numWords; j++) {
            tableWord <<= 8;
            if (j < len)
                tableWord |= 0;

            if ((j & 0x3) == 0x3)
                *pTableBase++ = tableWord;
        }
    }

    /* DC table */
    for (i = first; i < first + count; i++) {
        numWords = 12;
        switch (JPG_SCN->Td[i]) {
        case 0:
            vals = JPG_VLC->dcTable0.vals;
            len = JPG_VLC->dcTable0.tableLength;
            break;
        case 1:
            vals = JPG_VLC->dcTable1.vals;
            len = JPG_VLC->dcTable1.tableLength;
            break;
        case 2:
            vals = JPG_VLC->dcTable2.vals;
            len = JPG_VLC->dcTable2.tableLength;
            break;
        default:
            vals = JPG_VLC->dcTable3.vals;
            len = JPG_VLC->dcTable3.tableLength;
            break;
        }

        /* set pointer */
        if (count == 3)
            qpTableBase = 0;
        else
            qpTableBase = JPEGDEC_QP_BASE;

        pTableBase =
            &PTR_JPGC->frame.pTableBase.vir_addr[JPEGDEC_DC1_BASE -
                                                 qpTableBase];

        for (j = 0; j < numWords; j++) {
            tableWord <<= 8;
            if (j < len)
                tableWord |= vals[j];

            if ((j & 0x3) == 0x3)
                *pTableBase++ = tableWord;
        }
    }

    *pTableBase = 0;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecWriteTablesProgressive

        Functional description:
          Writes q/ac/dc tables to the HW format as specified in HW regs

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecWriteTablesProgressive(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    RK_U32 i, j = 0;
    RK_U32 tableWord = 0;
    RK_U8 tableTmp[64] = { 0 };
    RK_U8 *pTmp;
    RK_U32 *pTableBase = NULL;
    RK_U32 first, count;
    RK_U32 len, numWords;
    RK_U32 *vals;
    RK_U32 *pTable;
    RK_U32 dcTable = 0;
    RK_U32 qpTableBase = 0;

    ASSERT(PTR_JPGC);
    ASSERT(PTR_JPGC->frame.pTableBase.vir_addr);
    ASSERT(PTR_JPGC->frame.pTableBase.phy_addr);
    ASSERT(PTR_JPGC->frame.pTableBase.size);

    /* Reset the table memory */
    (void) DWLmemset(PTR_JPGC->frame.pTableBase.vir_addr, 0,
                     (sizeof(RK_U8) * JPEGDEC_PROGRESSIVE_TABLE_SIZE));

    pTableBase = PTR_JPGC->frame.pTableBase.vir_addr;

    if (PTR_JPGC->info.nonInterleaved) {
        first = PTR_JPGC->info.componentId;
        count = 1;
    } else {
        first = 0;
        count = 3;
    }

    /* QP tables for all components */
    for (j = first; j < first + count; j++) {
        if ((JPG_FRM->component[j].Tq) == 0)
            pTable = JPG_QTB->table0;
        else
            pTable = JPG_QTB->table1;

        for (i = 0; i < 64; i++) {
            tableTmp[zzOrder[i]] = (RK_U8) pTable[i];
        }

        pTmp = tableTmp;
        for (i = 0; i < 16; i++) {
            tableWord = (pTmp[0] << 24) | (pTmp[1] << 16) |
                        (pTmp[2] << 8) | (pTmp[3] << 0);;

            *pTableBase++ = tableWord;
            pTmp += 4;
        }
    }

    /* if later stage DC ==> no need for table */
    if (PTR_JPGC->scan.Ah != 0 && PTR_JPGC->scan.Ss == 0)
        return;

    for (i = first; i < first + count; i++) {
        if (PTR_JPGC->scan.Ss == 0) { /* DC */
            dcTable = 1;
            numWords = 12;
            switch (JPG_SCN->Td[i]) {
            case 0:
                vals = JPG_VLC->dcTable0.vals;
                len = JPG_VLC->dcTable0.tableLength;
                break;
            case 1:
                vals = JPG_VLC->dcTable1.vals;
                len = JPG_VLC->dcTable1.tableLength;
                break;
            case 2:
                vals = JPG_VLC->dcTable2.vals;
                len = JPG_VLC->dcTable2.tableLength;
                break;
            default:
                vals = JPG_VLC->dcTable3.vals;
                len = JPG_VLC->dcTable3.tableLength;
                break;
            }
        } else {
            numWords = 162;
            switch (JPG_SCN->Ta[i]) {
            case 0:
                vals = JPG_VLC->acTable0.vals;
                len = JPG_VLC->acTable0.tableLength;
                break;
            case 1:
                vals = JPG_VLC->acTable1.vals;
                len = JPG_VLC->acTable1.tableLength;
                break;
            case 2:
                vals = JPG_VLC->acTable2.vals;
                len = JPG_VLC->acTable2.tableLength;
                break;
            default:
                vals = JPG_VLC->acTable3.vals;
                len = JPG_VLC->acTable3.tableLength;
                break;
            }
        }

        /* set pointer */
        if (count == 3)
            qpTableBase = 0;
        else
            qpTableBase = JPEGDEC_QP_BASE;

        if (dcTable) {
            /* interleaved || non-interleaved */
            if (count == 3) {
                if (i == 0)
                    pTableBase =
                        &PTR_JPGC->frame.pTableBase.
                        vir_addr[JPEGDEC_DC1_BASE - qpTableBase];
                else if (i == 1)
                    pTableBase =
                        &PTR_JPGC->frame.pTableBase.
                        vir_addr[JPEGDEC_DC2_BASE - qpTableBase];
                else
                    pTableBase =
                        &PTR_JPGC->frame.pTableBase.
                        vir_addr[JPEGDEC_DC3_BASE - qpTableBase];
            } else {
                pTableBase =
                    &PTR_JPGC->frame.pTableBase.
                    vir_addr[JPEGDEC_DC1_BASE - qpTableBase];
            }
        } else {
            pTableBase =
                &PTR_JPGC->frame.pTableBase.vir_addr[JPEGDEC_AC1_BASE -
                                                     qpTableBase];
        }

        for (j = 0; j < numWords; j++) {
            tableWord <<= 8;
            if (j < len)
                tableWord |= vals[j];

            if ((j & 0x3) == 0x3)
                *pTableBase++ = tableWord;
        }

        /* fill to border */
        if (i == 0 && dcTable == 0) {
            numWords = 164;
            len = 164;
            for (j = 162; j < numWords; j++) {
                tableWord <<= 8;
                if (j < len)
                    tableWord |= 0;

                if ((j & 0x3) == 0x3)
                    *pTableBase++ = tableWord;
            }
        }

        /* reset */
        dcTable = 0;
    }

    *pTableBase = 0;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecChromaTableSelectors

        Functional description:
          select what tables chromas use

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecChromaTableSelectors(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    /* this trick is done because hw always wants luma table as ac hw table 1 */
    if (JPG_SCN->Ta[0] == 0) {
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_AC_VLCTABLE, JPG_SCN->Ta[2]);
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_AC_VLCTABLE, JPG_SCN->Ta[1]);
    } else {
        if (JPG_SCN->Ta[0] == JPG_SCN->Ta[1])
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_AC_VLCTABLE, 0);
        else
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_AC_VLCTABLE, 1);

        if (JPG_SCN->Ta[0] == JPG_SCN->Ta[2])
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_AC_VLCTABLE, 0);
        else
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_AC_VLCTABLE, 1);
    }

    /* Third DC table selectors */
    if (PTR_JPGC->info.operationType != JPEGDEC_PROGRESSIVE) {
        if (JPG_SCN->Td[0] == 0) {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE,
                               JPG_SCN->Td[2]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE,
                               JPG_SCN->Td[1]);
        } else {
            if (JPG_SCN->Td[0] == JPG_SCN->Td[1])
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE, 0);
            else
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE, 1);

            if (JPG_SCN->Td[0] == JPG_SCN->Td[2])
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE, 0);
            else
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE, 1);
        }

        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE3, 0);
        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE3, 0);
    } else {
        /* if non-interleaved ==> decoding mode YUV400, uses table zero (0) */
        if (PTR_JPGC->info.nonInterleaved) {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE3, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE3, 0);
        } else {
            /* if later stage DC ==> no need for table */
            if (PTR_JPGC->scan.Ah != 0 && PTR_JPGC->scan.Ss == 0) {
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE3, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE3, 0);
            } else {
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CR_DC_VLCTABLE3, 1);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE, 1);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_CB_DC_VLCTABLE3, 0);
            }
        }
    }

    return;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecWriteLenBits

        Functional description:
          tell hw how many vlc words of different lengths we have

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecWriteLenBits(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    VlcTable *pTable1 = NULL;
    VlcTable *pTable2 = NULL;

    /* first select the table we'll use */
    /* this trick is done because hw always wants luma table as ac hw table 1 */
    if (JPG_SCN->Ta[0] == 0) {

        pTable1 = &(JPG_VLC->acTable0);
        pTable2 = &(JPG_VLC->acTable1);

    } else {

        pTable1 = &(JPG_VLC->acTable1);
        pTable2 = &(JPG_VLC->acTable0);
    }

    ASSERT(pTable1);
    ASSERT(pTable2);

    /* write AC table 1 (luma) */

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE1_CNT, pTable1->bits[0]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE2_CNT, pTable1->bits[1]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE3_CNT, pTable1->bits[2]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE4_CNT, pTable1->bits[3]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE5_CNT, pTable1->bits[4]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE6_CNT, pTable1->bits[5]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE7_CNT, pTable1->bits[6]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE8_CNT, pTable1->bits[7]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE9_CNT, pTable1->bits[8]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE10_CNT, pTable1->bits[9]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE11_CNT, pTable1->bits[10]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE12_CNT, pTable1->bits[11]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE13_CNT, pTable1->bits[12]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE14_CNT, pTable1->bits[13]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE15_CNT, pTable1->bits[14]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE16_CNT, pTable1->bits[15]);

    /* table AC2 (the not-luma table) */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE1_CNT, pTable2->bits[0]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE2_CNT, pTable2->bits[1]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE3_CNT, pTable2->bits[2]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE4_CNT, pTable2->bits[3]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE5_CNT, pTable2->bits[4]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE6_CNT, pTable2->bits[5]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE7_CNT, pTable2->bits[6]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE8_CNT, pTable2->bits[7]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE9_CNT, pTable2->bits[8]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE10_CNT, pTable2->bits[9]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE11_CNT, pTable2->bits[10]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE12_CNT, pTable2->bits[11]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE13_CNT, pTable2->bits[12]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE14_CNT, pTable2->bits[13]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE15_CNT, pTable2->bits[14]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC2_CODE16_CNT, pTable2->bits[15]);

    if (JPG_SCN->Td[0] == 0) {

        pTable1 = &(JPG_VLC->dcTable0);
        pTable2 = &(JPG_VLC->dcTable1);

    } else {

        pTable1 = &(JPG_VLC->dcTable1);
        pTable2 = &(JPG_VLC->dcTable0);
    }

    ASSERT(pTable1);
    ASSERT(pTable2);

    /* write DC table 1 (luma) */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE1_CNT, pTable1->bits[0]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE2_CNT, pTable1->bits[1]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE3_CNT, pTable1->bits[2]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE4_CNT, pTable1->bits[3]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE5_CNT, pTable1->bits[4]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE6_CNT, pTable1->bits[5]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE7_CNT, pTable1->bits[6]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE8_CNT, pTable1->bits[7]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE9_CNT, pTable1->bits[8]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE10_CNT, pTable1->bits[9]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE11_CNT, pTable1->bits[10]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE12_CNT, pTable1->bits[11]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE13_CNT, pTable1->bits[12]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE14_CNT, pTable1->bits[13]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE15_CNT, pTable1->bits[14]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE16_CNT, pTable1->bits[15]);

    /* table DC2 (the not-luma table) */

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE1_CNT, pTable2->bits[0]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE2_CNT, pTable2->bits[1]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE3_CNT, pTable2->bits[2]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE4_CNT, pTable2->bits[3]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE5_CNT, pTable2->bits[4]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE6_CNT, pTable2->bits[5]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE7_CNT, pTable2->bits[6]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE8_CNT, pTable2->bits[7]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE9_CNT, pTable2->bits[8]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE10_CNT, pTable2->bits[9]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE11_CNT, pTable2->bits[10]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE12_CNT, pTable2->bits[11]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE13_CNT, pTable2->bits[12]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE14_CNT, pTable2->bits[13]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE15_CNT, pTable2->bits[14]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE16_CNT, pTable2->bits[15]);

    return;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecWriteLenBitsNonInterleaved

        Functional description:
          tell hw how many vlc words of different lengths we have

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecWriteLenBitsNonInterleaved(JpegDecContainer * pJpegDecCont)
{
    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    VlcTable *pTable1 = NULL;
    VlcTable *pTable2 = NULL;

    /* first select the table we'll use */

    /* this trick is done because hw always wants luma table as ac hw table 1 */
    if (JPG_SCN->Ta[PTR_JPGC->info.componentId] == 0) {

        pTable1 = &(JPG_VLC->acTable0);
        pTable2 = &(JPG_VLC->acTable1);

    } else {

        pTable1 = &(JPG_VLC->acTable1);
        pTable2 = &(JPG_VLC->acTable0);
    }

    ASSERT(pTable1);
    ASSERT(pTable2);

    /* write AC table 1 (luma) */

    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE1_CNT, pTable1->bits[0]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE2_CNT, pTable1->bits[1]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE3_CNT, pTable1->bits[2]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE4_CNT, pTable1->bits[3]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE5_CNT, pTable1->bits[4]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE6_CNT, pTable1->bits[5]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE7_CNT, pTable1->bits[6]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE8_CNT, pTable1->bits[7]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE9_CNT, pTable1->bits[8]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE10_CNT, pTable1->bits[9]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE11_CNT, pTable1->bits[10]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE12_CNT, pTable1->bits[11]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE13_CNT, pTable1->bits[12]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE14_CNT, pTable1->bits[13]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE15_CNT, pTable1->bits[14]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE16_CNT, pTable1->bits[15]);

    if (JPG_SCN->Td[PTR_JPGC->info.componentId] == 0) {

        pTable1 = &(JPG_VLC->dcTable0);
        pTable2 = &(JPG_VLC->dcTable1);

    } else {

        pTable1 = &(JPG_VLC->dcTable1);
        pTable2 = &(JPG_VLC->dcTable0);
    }

    ASSERT(pTable1);
    ASSERT(pTable2);

    /* write DC table 1 (luma) */
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE1_CNT, pTable1->bits[0]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE2_CNT, pTable1->bits[1]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE3_CNT, pTable1->bits[2]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE4_CNT, pTable1->bits[3]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE5_CNT, pTable1->bits[4]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE6_CNT, pTable1->bits[5]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE7_CNT, pTable1->bits[6]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE8_CNT, pTable1->bits[7]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE9_CNT, pTable1->bits[8]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE10_CNT, pTable1->bits[9]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE11_CNT, pTable1->bits[10]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE12_CNT, pTable1->bits[11]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE13_CNT, pTable1->bits[12]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE14_CNT, pTable1->bits[13]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE15_CNT, pTable1->bits[14]);
    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE16_CNT, pTable1->bits[15]);

    return;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecWriteLenBitsProgressive

        Functional description:
          tell hw how many vlc words of different lengths we have

        Inputs:
          JpegDecContainer *pJpegDecCont

        Outputs:
          void

------------------------------------------------------------------------------*/
static void JpegDecWriteLenBitsProgressive(JpegDecContainer * pJpegDecCont)
{

    RK_U32 i;

    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    VlcTable *pTable1 = NULL;
    VlcTable *pTable2 = NULL;
    VlcTable *pTable3 = NULL;
    VlcTable *pTable4 = NULL;

    /* reset swregs that contains vlc length information: swregs [16-28] */
    for (i = JPEGDEC_VLC_LEN_START_REG; i < JPEGDEC_VLC_LEN_END_REG; i++)
        PTR_JPGC->jpegRegs[i] = 0;

    /* check if interleaved scan ==> only one table needed */
    if (PTR_JPGC->info.nonInterleaved) {
        /* check if AC or DC coefficient scan */
        if (PTR_JPGC->scan.Ss == 0) { /* DC */
            /* check component ÍD */
            if (PTR_JPGC->info.componentId == 0) {
                if (JPG_SCN->Td[0] == 0)
                    pTable1 = &(JPG_VLC->dcTable0);
                else if (JPG_SCN->Td[0] == 1)
                    pTable1 = &(JPG_VLC->dcTable1);
                else if (JPG_SCN->Td[0] == 2)
                    pTable1 = &(JPG_VLC->dcTable2);
                else
                    pTable1 = &(JPG_VLC->dcTable3);
            } else if (PTR_JPGC->info.componentId == 1) {
                if (JPG_SCN->Td[1] == 0)
                    pTable1 = &(JPG_VLC->dcTable0);
                else if (JPG_SCN->Td[1] == 1)
                    pTable1 = &(JPG_VLC->dcTable1);
                else if (JPG_SCN->Td[1] == 2)
                    pTable1 = &(JPG_VLC->dcTable2);
                else
                    pTable1 = &(JPG_VLC->dcTable3);
            } else {
                if (JPG_SCN->Td[2] == 0)
                    pTable1 = &(JPG_VLC->dcTable0);
                else if (JPG_SCN->Td[2] == 1)
                    pTable1 = &(JPG_VLC->dcTable1);
                else if (JPG_SCN->Td[2] == 2)
                    pTable1 = &(JPG_VLC->dcTable2);
                else
                    pTable1 = &(JPG_VLC->dcTable3);
            }

            ASSERT(pTable1);

            /* if later stage DC ==> no need for table */
            if (PTR_JPGC->scan.Ah == 0) {
                /* write DC table 1 */
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE1_CNT,
                                   pTable1->bits[0]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE2_CNT,
                                   pTable1->bits[1]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE3_CNT,
                                   pTable1->bits[2]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE4_CNT,
                                   pTable1->bits[3]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE5_CNT,
                                   pTable1->bits[4]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE6_CNT,
                                   pTable1->bits[5]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE7_CNT,
                                   pTable1->bits[6]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE8_CNT,
                                   pTable1->bits[7]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE9_CNT,
                                   pTable1->bits[8]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE10_CNT,
                                   pTable1->bits[9]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE11_CNT,
                                   pTable1->bits[10]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE12_CNT,
                                   pTable1->bits[11]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE13_CNT,
                                   pTable1->bits[12]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE14_CNT,
                                   pTable1->bits[13]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE15_CNT,
                                   pTable1->bits[14]);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE16_CNT,
                                   pTable1->bits[15]);
            } else {
                /* write zero table */
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE1_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE2_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE3_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE4_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE5_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE6_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE7_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE8_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE9_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE10_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE11_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE12_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE13_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE14_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE15_CNT, 0);
                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE16_CNT, 0);
            }

        } else { /* AC */
            /* check component ÍD */
            if (PTR_JPGC->info.componentId == 0) {
                if (JPG_SCN->Ta[0] == 0)
                    pTable1 = &(JPG_VLC->acTable0);
                else if (JPG_SCN->Ta[0] == 1)
                    pTable1 = &(JPG_VLC->acTable1);
                else if (JPG_SCN->Ta[0] == 2)
                    pTable1 = &(JPG_VLC->acTable2);
                else
                    pTable1 = &(JPG_VLC->acTable3);
            } else if (PTR_JPGC->info.componentId == 1) {
                if (JPG_SCN->Ta[1] == 0)
                    pTable1 = &(JPG_VLC->acTable0);
                else if (JPG_SCN->Ta[1] == 1)
                    pTable1 = &(JPG_VLC->acTable1);
                else if (JPG_SCN->Ta[1] == 2)
                    pTable1 = &(JPG_VLC->acTable2);
                else
                    pTable1 = &(JPG_VLC->acTable3);
            } else {
                if (JPG_SCN->Ta[2] == 0)
                    pTable1 = &(JPG_VLC->acTable0);
                else if (JPG_SCN->Ta[2] == 1)
                    pTable1 = &(JPG_VLC->acTable1);
                else if (JPG_SCN->Ta[2] == 2)
                    pTable1 = &(JPG_VLC->acTable2);
                else
                    pTable1 = &(JPG_VLC->acTable3);
            }

            ASSERT(pTable1);

            /* write AC table 1 */
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE1_CNT,
                               pTable1->bits[0]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE2_CNT,
                               pTable1->bits[1]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE3_CNT,
                               pTable1->bits[2]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE4_CNT,
                               pTable1->bits[3]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE5_CNT,
                               pTable1->bits[4]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE6_CNT,
                               pTable1->bits[5]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE7_CNT,
                               pTable1->bits[6]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE8_CNT,
                               pTable1->bits[7]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE9_CNT,
                               pTable1->bits[8]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE10_CNT,
                               pTable1->bits[9]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE11_CNT,
                               pTable1->bits[10]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE12_CNT,
                               pTable1->bits[11]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE13_CNT,
                               pTable1->bits[12]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE14_CNT,
                               pTable1->bits[13]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE15_CNT,
                               pTable1->bits[14]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_AC1_CODE16_CNT,
                               pTable1->bits[15]);
        }
    } else { /* interleaved */
        /* first select the table we'll use */
        /* this trick is done because hw always wants luma table as ac hw table 1 */

        if (JPG_SCN->Td[0] == 0)
            pTable1 = &(JPG_VLC->dcTable0);
        else if (JPG_SCN->Td[0] == 1)
            pTable1 = &(JPG_VLC->dcTable1);
        else if (JPG_SCN->Td[0] == 2)
            pTable1 = &(JPG_VLC->dcTable2);
        else
            pTable1 = &(JPG_VLC->dcTable3);

        if (JPG_SCN->Td[1] == 0)
            pTable2 = &(JPG_VLC->dcTable0);
        else if (JPG_SCN->Td[1] == 1)
            pTable2 = &(JPG_VLC->dcTable1);
        else if (JPG_SCN->Td[1] == 2)
            pTable2 = &(JPG_VLC->dcTable2);
        else
            pTable2 = &(JPG_VLC->dcTable3);

        if (JPG_SCN->Td[2] == 0)
            pTable3 = &(JPG_VLC->dcTable0);
        else if (JPG_SCN->Td[2] == 1)
            pTable3 = &(JPG_VLC->dcTable1);
        else if (JPG_SCN->Td[2] == 2)
            pTable3 = &(JPG_VLC->dcTable2);
        else
            pTable3 = &(JPG_VLC->dcTable3);

        ASSERT(pTable1);
        ASSERT(pTable2);
        ASSERT(pTable3);

        /* if later stage DC ==> no need for table */
        if (PTR_JPGC->scan.Ah == 0) {
            /* write DC table 1 (luma) */
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE1_CNT,
                               pTable1->bits[0]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE2_CNT,
                               pTable1->bits[1]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE3_CNT,
                               pTable1->bits[2]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE4_CNT,
                               pTable1->bits[3]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE5_CNT,
                               pTable1->bits[4]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE6_CNT,
                               pTable1->bits[5]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE7_CNT,
                               pTable1->bits[6]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE8_CNT,
                               pTable1->bits[7]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE9_CNT,
                               pTable1->bits[8]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE10_CNT,
                               pTable1->bits[9]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE11_CNT,
                               pTable1->bits[10]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE12_CNT,
                               pTable1->bits[11]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE13_CNT,
                               pTable1->bits[12]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE14_CNT,
                               pTable1->bits[13]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE15_CNT,
                               pTable1->bits[14]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE16_CNT,
                               pTable1->bits[15]);

            /* table DC2 (Cb) */
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE1_CNT,
                               pTable2->bits[0]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE2_CNT,
                               pTable2->bits[1]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE3_CNT,
                               pTable2->bits[2]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE4_CNT,
                               pTable2->bits[3]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE5_CNT,
                               pTable2->bits[4]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE6_CNT,
                               pTable2->bits[5]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE7_CNT,
                               pTable2->bits[6]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE8_CNT,
                               pTable2->bits[7]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE9_CNT,
                               pTable2->bits[8]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE10_CNT,
                               pTable2->bits[9]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE11_CNT,
                               pTable2->bits[10]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE12_CNT,
                               pTable2->bits[11]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE13_CNT,
                               pTable2->bits[12]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE14_CNT,
                               pTable2->bits[13]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE15_CNT,
                               pTable2->bits[14]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE16_CNT,
                               pTable2->bits[15]);

            /* table DC2 (Cr) */
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE1_CNT,
                               pTable3->bits[0]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE2_CNT,
                               pTable3->bits[1]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE3_CNT,
                               pTable3->bits[2]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE4_CNT,
                               pTable3->bits[3]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE5_CNT,
                               pTable3->bits[4]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE6_CNT,
                               pTable3->bits[5]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE7_CNT,
                               pTable3->bits[6]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE8_CNT,
                               pTable3->bits[7]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE9_CNT,
                               pTable3->bits[8]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE10_CNT,
                               pTable3->bits[9]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE11_CNT,
                               pTable3->bits[10]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE12_CNT,
                               pTable3->bits[11]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE13_CNT,
                               pTable3->bits[12]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE14_CNT,
                               pTable3->bits[13]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE15_CNT,
                               pTable3->bits[14]);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE16_CNT,
                               pTable3->bits[15]);
        } else {
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE1_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE2_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE3_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE4_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE5_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE6_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE7_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE8_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE9_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE10_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE11_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE12_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE13_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE14_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE15_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC1_CODE16_CNT, 0);

            /* table DC2 (Cb) */
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE1_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE2_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE3_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE4_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE5_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE6_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE7_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE8_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE9_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE10_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE11_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE12_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE13_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE14_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE15_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC2_CODE16_CNT, 0);

            /* table DC2 (Cr) */
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE1_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE2_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE3_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE4_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE5_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE6_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE7_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE8_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE9_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE10_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE11_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE12_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE13_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE14_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE15_CNT, 0);
            rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DC3_CODE16_CNT, 0);
        }
    }

    return;
}

/*------------------------------------------------------------------------------

        Function name: JpegDecNextScanHdrs

        Functional description:
          Decodes next headers in case of non-interleaved stream

        Inputs:
          JpegDecContainer *pDecData      Pointer to JpegDecContainer structure

        Outputs:
          OK/NOK

------------------------------------------------------------------------------*/
JpegDecRet JpegDecNextScanHdrs(JpegDecContainer * pJpegDecCont)
{

    RK_U32 i;
    RK_U32 currentByte = 0;
    RK_U32 currentBytes = 0;
    JpegDecRet retCode;

    JpegDecContainer *PTR_JPGC = pJpegDecCont;
    ScanInfo         *JPG_SCN = &pJpegDecCont->scan;
    HuffmanTables    *JPG_VLC = &pJpegDecCont->vlc;
    QuantTables      *JPG_QTB = &pJpegDecCont->quant;
    FrameInfo        *JPG_FRM = &pJpegDecCont->frame;

    retCode = JPEGDEC_OK;

    /* reset for new headers */
    PTR_JPGC->image.headerReady = 0;

    /* find markers and go ! */
    do {
        /* Look for marker prefix byte from stream */
        if (JpegDecGetByte(&(PTR_JPGC->stream)) == 0xFF) {
            currentByte = JpegDecGetByte(&(PTR_JPGC->stream));

            /* switch to certain header decoding */
            switch (currentByte) {
            case 0x00:
            case SOF0:
            case SOF2:
                break;
                /* Start of Scan */
            case SOS:
                /* reset image ready */
                PTR_JPGC->image.imageReady = 0;
                retCode = JpegDecDecodeScan(PTR_JPGC);
                PTR_JPGC->image.headerReady = 1;
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Stream error"));
                        return (retCode);
                    } else {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# JpegDecDecodeScan err\n"));
                        return (retCode);
                    }
                }

                if (PTR_JPGC->stream.bitPosInByte) {
                    /* delete stuffing bits */
                    currentByte = (8 - PTR_JPGC->stream.bitPosInByte);
                    if (JpegDecFlushBits
                        (&(PTR_JPGC->stream),
                         8 - PTR_JPGC->stream.bitPosInByte) == STRM_ERROR) {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Stream error"));
                        return (JPEGDEC_STRM_ERROR);
                    }
                }
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# Stuffing bits deleted\n"));
                break;
                /* Start of Huffman tables */
            case DHT:
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# JpegDecDecodeHuffmanTables dec"));
                retCode = JpegDecDecodeHuffmanTables(PTR_JPGC);
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# JpegDecDecodeHuffmanTables stops"));
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Stream error"));
                        return (retCode);
                    } else {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: JpegDecDecodeHuffmanTables err"));
                        return (retCode);
                    }
                }
                break;
                /* start of Quantisation Tables */
            case DQT:
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# JpegDecDecodeQuantTables dec"));
                retCode = JpegDecDecodeQuantTables(PTR_JPGC);
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Stream error"));
                        return (retCode);
                    } else {
                        JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: JpegDecDecodeQuantTables err"));
                        return (retCode);
                    }
                }
                break;
                /* Start of Image */
            case SOI:
                /* no actions needed, continue */
                break;
                /* End of Image */
            case EOI:
                if (PTR_JPGC->image.imageReady) {
                    JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# EOI: OK\n"));
                    return (JPEGDEC_FRAME_READY);
                } else {
                    JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: EOI: NOK\n"));
                    return (JPEGDEC_ERROR);
                }
                /* Define Restart Interval */
            case DRI:
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# DRI"));
                currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                if (currentBytes == STRM_ERROR) {
                    JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Read bits "));
                    return (JPEGDEC_STRM_ERROR);
                }
                PTR_JPGC->frame.Ri = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                break;
                /* Restart with modulo 8 count m */
            case RST0:
            case RST1:
            case RST2:
            case RST3:
            case RST4:
            case RST5:
            case RST6:
            case RST7:
                /* initialisation of DC predictors to zero value !!! */
                for (i = 0; i < MAX_NUMBER_OF_COMPONENTS; i++) {
                    PTR_JPGC->scan.pred[i] = 0;
                }
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# DC predictors init"));
                break;
                /* unsupported features */
            case DNL:
            case SOF1:
            case SOF3:
            case SOF5:
            case SOF6:
            case SOF7:
            case SOF9:
            case SOF10:
            case SOF11:
            case SOF13:
            case SOF14:
            case SOF15:
            case DAC:
            case DHP:
            case TEM:
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Unsupported Features"));
                return (JPEGDEC_UNSUPPORTED);
                /* application data & comments */
            case APP0:
            case APP1:
            case APP2:
            case APP3:
            case APP4:
            case APP5:
            case APP6:
            case APP7:
            case APP8:
            case APP9:
            case APP10:
            case APP11:
            case APP12:
            case APP13:
            case APP14:
            case APP15:
            case COM:
                JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# COM"));
                currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                if (currentBytes == STRM_ERROR) {
                    JPEGDEC_TRACE_INTERNAL(("JpegDecNextScanHdrs# ERROR: Read bits "));
                    return (JPEGDEC_STRM_ERROR);
                }
                /* jump over not supported header */
                if (currentBytes != 0) {
                    PTR_JPGC->stream.readBits += ((currentBytes * 8) - 16);
                    PTR_JPGC->stream.pCurrPos +=
                        (((currentBytes * 8) - 16) / 8);
                }
                break;
            default:
                break;
            }
        } else {
            if (currentByte == 0xFFFFFFFF) {
                break;
            }
        }

        if (PTR_JPGC->image.headerReady)
            break;
    } while ((PTR_JPGC->stream.readBits >> 3) <= PTR_JPGC->stream.streamLength);

    return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------
    Function name   : JpegRefreshRegs
    Description     :
    Return type     : void
    Argument        : PPContainer * ppC
------------------------------------------------------------------------------*/
void JpegRefreshRegs(JpegDecContainer * pJpegDecCont)
{
    return;
}

/*------------------------------------------------------------------------------
    Function name   : JpegFlushRegs
    Description     :
    Return type     : void
    Argument        : PPContainer * ppC
------------------------------------------------------------------------------*/
void JpegFlushRegs(JpegDecContainer * pJpegDecCont)
{
    int i;
    {/*
        int i;
        for (i = 0; i < 159; i++) {
            printf("before reg[%d]=0x%08x \n", i, pJpegDecCont->jpegRegs[i]);
        }*/
    }
    memcpy(pJpegDecCont->ioctl_info.regs, pJpegDecCont->jpegRegs, DEC_RK70_REGISTERS * sizeof(RK_U32));

    RK_U32 *data = (RK_U32*) & (pJpegDecCont->ioctl_info);
    /*for (i = 0; i < sizeof(pJpegDecCont->ioctl_info) / sizeof(RK_U32); i++ ) {
        printf("reg[%03d]=0x%08x\n", i, data[i]);

    }*/

    if (VPUClientSendReg(pJpegDecCont->socket, (RK_U32*) & (pJpegDecCont->ioctl_info),
                         sizeof(pJpegDecCont->ioctl_info) / sizeof(RK_U32)))
        ALOGV("JPEGFlushRegs fail\n");
    else
        ALOGV("JPEGFlushRegs success\n");

}

/*------------------------------------------------------------------------------
    Function name   : JpegDecInitHWEmptyScan
    Description     :
    Return type     : void
    Argument        :
------------------------------------------------------------------------------*/
static RK_U32 NumBits(RK_U32 value)
{

    RK_U32 numBits = 0;

    while (value) {
        value >>= 1;
        numBits++;
    }

    if (!numBits) {
        numBits = 1;
    }

    return (numBits);

}

void JpegDecInitHWEmptyScan(JpegDecContainer * pJpegDecCont, RK_U32 componentId)
{

    RK_U32 i;
    RK_S32 n;
    RK_U32 coeffBuffer = 0;
    RK_U32 outputBuffer = 0;
    RK_U32 numBlocks;
    RK_U32 numMax;
    RK_U8 *pStrm;
    RK_U32 bits;
    RK_U32 bitPos;
    RK_U32 *pTableBase = NULL;

    ASSERT(pJpegDecCont);

    pJpegDecCont->info.nonInterleaved = 1;
    pJpegDecCont->info.componentId = componentId;

    if (pJpegDecCont->ppInstance == NULL && pJpegDecCont->info.userAllocMem == 1) {
        /* if user allocated memory ==> new addresses */
        pJpegDecCont->asicBuff.outLumaBuffer.vir_addr =
            pJpegDecCont->info.givenOutLuma.vir_addr;
        pJpegDecCont->asicBuff.outLumaBuffer.phy_addr =
            pJpegDecCont->info.givenOutLuma.phy_addr;
        pJpegDecCont->asicBuff.outChromaBuffer.vir_addr =
            pJpegDecCont->info.givenOutChroma.vir_addr;
        pJpegDecCont->asicBuff.outChromaBuffer.phy_addr =
            pJpegDecCont->info.givenOutChroma.phy_addr;
        pJpegDecCont->asicBuff.outChromaBuffer2.vir_addr =
            pJpegDecCont->info.givenOutChroma2.vir_addr;
        pJpegDecCont->asicBuff.outChromaBuffer2.phy_addr =
            pJpegDecCont->info.givenOutChroma2.phy_addr;
    }

    /*************** Set swreg13 data ************/
    /* Luminance output */
    if (componentId == 0)
        outputBuffer = pJpegDecCont->asicBuff.outLumaBuffer.phy_addr;
    else if (componentId == 1)
        outputBuffer = (pJpegDecCont->asicBuff.outChromaBuffer.phy_addr);
    else
        outputBuffer = (pJpegDecCont->asicBuff.outChromaBuffer2.phy_addr);

    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_DEC_OUT_BASE, outputBuffer);

    pJpegDecCont->info.yCbCrMode = 0;
    pJpegDecCont->info.X = pJpegDecCont->frame.hwX;
    pJpegDecCont->info.Y = pJpegDecCont->frame.hwY;
    pJpegDecCont->info.fillX = 0;
    pJpegDecCont->info.fillY = 0;
    numBlocks = pJpegDecCont->frame.hwX * pJpegDecCont->frame.hwY / 64;
    coeffBuffer = pJpegDecCont->info.pCoeffBase.phy_addr;
    if (componentId) {
        coeffBuffer += JPEGDEC_COEFF_SIZE * numBlocks;
        if (pJpegDecCont->info.yCbCrModeOrig == JPEGDEC_YUV420) {
            pJpegDecCont->info.X /= 2;
            if (pJpegDecCont->info.X & 0xF) {
                pJpegDecCont->info.X += 8;
                pJpegDecCont->info.fillX = 1;
            }
            pJpegDecCont->info.Y /= 2;
            if (pJpegDecCont->info.Y & 0xF) {
                pJpegDecCont->info.Y += 8;
                pJpegDecCont->info.fillY = 1;
            }
            numBlocks /= 4;
        } else if (pJpegDecCont->info.yCbCrModeOrig == JPEGDEC_YUV422) {
            pJpegDecCont->info.X /= 2;
            if (pJpegDecCont->info.X & 0xF) {
                pJpegDecCont->info.X += 8;
                pJpegDecCont->info.fillX = 1;
            }
            numBlocks /= 2;
        } else if (pJpegDecCont->info.yCbCrModeOrig == JPEGDEC_YUV440) {
            pJpegDecCont->info.Y /= 2;
            if (pJpegDecCont->info.Y & 0xF) {
                pJpegDecCont->info.Y += 8;
                pJpegDecCont->info.fillY = 1;
            }
            numBlocks /= 2;
        }
        if (componentId > 1)
            coeffBuffer += JPEGDEC_COEFF_SIZE * numBlocks;
    }

    pStrm = (RK_U8 *)pJpegDecCont->info.tmpStrm.vir_addr;
    numMax = 0;
    while (numBlocks > 32767) {
        numBlocks -= 32767;
        numMax++;
    }

    n = NumBits(numBlocks);

    /* do we still have correct quantization tables ?? */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Write AC,DC,QP tables to base\n"));
    JpegDecWriteTablesProgressive(pJpegDecCont);

    /* two vlc codes, both with length 1 (can be done?), 0 for largest eob, 1
     * for last eob (EOBn) */
    /* write "length amounts" */
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_AC1_CODE1_CNT, 2);

    /* codeword values 0xE0 (for EOB run of 32767 blocks) and 0xn0 */
    pTableBase = pJpegDecCont->frame.pTableBase.vir_addr;
    pTableBase += 48;   /* start of vlc tables */
    *pTableBase = (0xE0 << 24) | ((n - 1) << 20);

    /* write numMax ext eobs of length 32767 followed by last ext eob */
    bitPos = 0;
    for (i = 0; i < numMax; i++) {
        bits = 0x3FFF << 17;
        *pStrm = (bitPos ? *pStrm : 0) | bits >> (24 + bitPos);
        pStrm++;
        bits <<= 8 - bitPos;
        *pStrm = bits >> 24;
        if (bitPos >= 1) {
            pStrm++;
            bits <<= 8;
            *pStrm = bits >> 24;
        }
        bitPos = (bitPos + 15) & 0x7;
    }

    if (numBlocks) {
        /* codeword to be written:
         * '1' to indicate EOBn followed by number of blocks - 2^(n-1) */
        bits = numBlocks << (32 - n);
        *pStrm = (bitPos ? *pStrm : 0) | bits >> (24 + bitPos);
        pStrm++;
        bits <<= 8 - bitPos;
        n -= 8 - bitPos;
        while (n > 0) {
            *pStrm++ = bits >> 24;
            bits <<= 8;
            n -= 8;
        }
    }

    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_RLC_VLC_BASE,
                       pJpegDecCont->info.tmpStrm.phy_addr);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_STRM_START_BIT, 0);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_STREAM_LEN, 100);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set input format\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_JPEG_MODE, JPEGDEC_YUV400);

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame width extension\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PIC_MB_W_EXT,
                       ((((pJpegDecCont->info.X) >> (4)) & 0xE00) >> 9));

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame width\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PIC_MB_WIDTH,
                       ((pJpegDecCont->info.X) >> (4)) & 0x1FF);

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame height extension\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PIC_MB_H_EXT,
                       ((((pJpegDecCont->info.Y) >> (4)) & 0x700) >> 8));

    /* frame size, round up the number of mbs */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set Frame height\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PIC_MB_HEIGHT_P,
                       ((pJpegDecCont->info.Y) >> (4)) & 0x0FF);

    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_WDIV8,
                       pJpegDecCont->info.fillX);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_JPEG_FILRIGHT_E,
                       pJpegDecCont->info.fillX);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_HDIV8,
                       pJpegDecCont->info.fillY);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_FILDOWN_E,
                       pJpegDecCont->info.fillY);

    /*************** Set swreg52 data ************/
    /* Set JPEG operation mode */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set JPEG operation mode\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_E, 1);

    /* indicate first ac scan for any spectral coeffs, nothing will be changed
     * as every block "skipped" by extended eobs */
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_SS, 1);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_SE, 1);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_AH, 0);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_AL, 0);

    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set coefficient buffer base address\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_PJPEG_COEFF_BUF, coeffBuffer);
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_DEC_OUT_DIS, 0);

    /* write table base */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Set AC,DC,QP table base address\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_QTABLE_BASE,
                       pJpegDecCont->frame.pTableBase.phy_addr);

    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_SYNC_MARKER_E, 0);

    pJpegDecCont->asicRunning = 1;

    /* Enable jpeg mode and set slice mode */
    JPEGDEC_TRACE_INTERNAL(("INTERNAL: Enable jpeg\n"));
    rk_SetRegisterFile(pJpegDecCont->jpegRegs, HWIF_DEC_E, 1);
    /* Flush regs to hw register */
    JpegFlushRegs(pJpegDecCont);
}
#ifdef DRM_LINUX
int SetPostProcessor(void *handle, MppBufferInfo *dst, int inWidth, int inHeigth,
                     int outWidth, int outHeight, int inColor, PostProcessInfo *ppInfo) //,int outColor,HW_BOOL dither)
#else
int SetPostProcessor(void *handle, VPUMemLinear_t *dst, int inWidth, int inHeigth,
                     int outWidth, int outHeight, int inColor, PostProcessInfo *ppInfo)
#endif
{
    //modify following values in special product
#define BRIGHTNESS 0    // -128 ~ 127
#define CONTRAST 0      // -64 ~ 64
#define SATURATION 0    // -64 ~ 128
#define PP_IN_FORMAT_YUV422INTERLAVE            0
#define PP_IN_FORMAT_YUV420SEMI                     1
#define PP_IN_FORMAT_YUV420PLANAR                  2
#define PP_IN_FORMAT_YUV400                             3
#define PP_IN_FORMAT_YUV422SEMI                     4
#define PP_IN_FORMAT_YUV420SEMITIELED           5
#define PP_IN_FORMAT_YUV440SEMI                     6
#define PP_IN_FORMAT_YUV444_SEMI                                 7
#define PP_IN_FORMAT_YUV411_SEMI                                 8

#define PP_OUT_FORMAT_RGB565                    0
#define PP_OUT_FORMAT_ARGB                       1
#define PP_OUT_FORMAT_YUV422INTERLAVE    3
#define PP_OUT_FORMAT_YUV420INTERLAVE    5

    int outColor = ppInfo->outFomart;
    int dither = ppInfo->shouldDither;
	RK_U32 dstLum;

    rk_SetRegisterFile(handle, HWIF_PP_AXI_RD_ID, 0);
    rk_SetRegisterFile(handle, HWIF_PP_AXI_WR_ID, 0);
    rk_SetRegisterFile(handle, HWIF_PP_AHB_HLOCK_E, 1);
    rk_SetRegisterFile(handle, HWIF_PP_SCMD_DIS, 1);
    rk_SetRegisterFile(handle, HWIF_PP_IN_A2_ENDSEL, 1);
    rk_SetRegisterFile(handle, HWIF_PP_IN_A1_SWAP32, 1);
    rk_SetRegisterFile(handle, HWIF_PP_IN_A1_ENDIAN, 1);
    rk_SetRegisterFile(handle, HWIF_PP_IN_SWAP32_E, 1);
    rk_SetRegisterFile(handle, HWIF_PP_DATA_DISC_E, 1);
  	rk_SetRegisterFile(handle, HWIF_PP_CLK_GATE_E, 1);
    //rk_SetRegisterFile(handle, HWIF_PP_CLK_GATE_E, 0);
    rk_SetRegisterFile(handle, HWIF_PP_IN_ENDIAN, 1);
    rk_SetRegisterFile(handle, HWIF_PP_OUT_ENDIAN, 1);
    rk_SetRegisterFile(handle, HWIF_PP_OUT_SWAP32_E, 1);
    rk_SetRegisterFile(handle, HWIF_PP_MAX_BURST, 16);

    rk_SetRegisterFile(handle, HWIF_PP_IN_LU_BASE, 0);

    rk_SetRegisterFile(handle, HWIF_EXT_ORIG_WIDTH, inWidth >> 4);

    if (ppInfo->cropW <= 0) {
        rk_SetRegisterFile(handle, HWIF_PP_IN_W_EXT,
                           (((inWidth / 16) & 0xE00) >> 9));
        rk_SetRegisterFile(handle, HWIF_PP_IN_WIDTH,
                           ((inWidth / 16) & 0x1FF));
        rk_SetRegisterFile(handle, HWIF_PP_IN_H_EXT,
                           (((inHeigth / 16) & 0x700) >> 8));
        rk_SetRegisterFile(handle, HWIF_PP_IN_HEIGHT,
                           ((inHeigth / 16) & 0x0FF));
    } else {
        rk_SetRegisterFile(handle, HWIF_PP_IN_W_EXT,
                           (((ppInfo->cropW / 16) & 0xE00) >> 9));
        rk_SetRegisterFile(handle, HWIF_PP_IN_WIDTH,
                           ((ppInfo->cropW / 16) & 0x1FF));
        rk_SetRegisterFile(handle, HWIF_PP_IN_H_EXT,
                           (((ppInfo->cropH / 16) & 0x700) >> 8));
        rk_SetRegisterFile(handle, HWIF_PP_IN_HEIGHT,
                           ((ppInfo->cropH / 16) & 0x0FF));
        rk_SetRegisterFile(handle, HWIF_CROP_STARTX_EXT,
                           (((ppInfo->cropX / 16) & 0xE00) >> 9));
        rk_SetRegisterFile(handle, HWIF_CROP_STARTX,
                           ((ppInfo->cropX / 16) & 0x1FF));
        rk_SetRegisterFile(handle, HWIF_CROP_STARTY_EXT,
                           (((ppInfo->cropY / 16) & 0x700) >> 8));
        rk_SetRegisterFile(handle, HWIF_CROP_STARTY,
                           ((ppInfo->cropY / 16) & 0x0FF));
        if (ppInfo->cropW & 0x0F) {
            rk_SetRegisterFile(handle, HWIF_PP_CROP8_R_E, 1);
        } else {
            rk_SetRegisterFile(handle, HWIF_PP_CROP8_R_E, 0);
        }
        if (ppInfo->cropH & 0x0F) {
            rk_SetRegisterFile(handle, HWIF_PP_CROP8_D_E, 1);
        } else {
            rk_SetRegisterFile(handle, HWIF_PP_CROP8_D_E, 0);
        }
        inWidth = ppInfo->cropW;
        inHeigth = ppInfo->cropH;
    }

    rk_SetRegisterFile(handle, HWIF_DISPLAY_WIDTH, outWidth);
    rk_SetRegisterFile(handle, HWIF_PP_OUT_WIDTH, outWidth);
    rk_SetRegisterFile(handle, HWIF_PP_OUT_HEIGHT, outHeight);
    rk_SetRegisterFile(handle, HWIF_PP_OUT_LU_BASE, dst->phy_addr);

    switch (inColor) {
    case    PP_IN_FORMAT_YUV422INTERLAVE:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 0);
        break;
    case    PP_IN_FORMAT_YUV420SEMI:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 1);
        break;
    case    PP_IN_FORMAT_YUV420PLANAR:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 2);
        break;
    case    PP_IN_FORMAT_YUV400:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 3);
        break;
    case    PP_IN_FORMAT_YUV422SEMI:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 4);
        break;
    case    PP_IN_FORMAT_YUV420SEMITIELED:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 5);
        break;
    case    PP_IN_FORMAT_YUV440SEMI:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 6);
        break;
    case    PP_IN_FORMAT_YUV444_SEMI:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 7);
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT_ES, 0);
        break;
    case    PP_IN_FORMAT_YUV411_SEMI:
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT, 7);
        rk_SetRegisterFile(handle, HWIF_PP_IN_FORMAT_ES, 1);
        break;
    default:
        return -1;
    }
#define VIDEORANGE 1    //0 or 1
    int videoRange = VIDEORANGE;
    rk_SetRegisterFile(handle, HWIF_RANGEMAP_COEF_Y, 9);
    rk_SetRegisterFile(handle, HWIF_RANGEMAP_COEF_C, 9);

    /*  brightness */
    rk_SetRegisterFile(handle, HWIF_COLOR_COEFFF, BRIGHTNESS);

    if (outColor <= PP_OUT_FORMAT_ARGB) {
        /*Bt.601*/
        unsigned int    a = 298;
        unsigned int    b = 409;
        unsigned int    c = 208;
        unsigned int    d = 100;
        unsigned int    e = 516;

        /*Bt.709
        unsigned int    a = 298;
        unsigned int    b = 459;
        unsigned int    c = 137;
        unsigned int    d = 55;
        unsigned int    e = 544;*/

        int satur = 0, tmp;
        if (videoRange != 0) {
            /*Bt.601*/
            a = 256;
            b = 350;
            c = 179;
            d = 86;
            e = 443;
            /*Bt.709
            a = 256;
            b = 403;
            c = 120;
            d = 48;
            e = 475;*/
            rk_SetRegisterFile(handle, HWIF_YCBCR_RANGE, videoRange);
        }
        int contrast = CONTRAST;
        if (contrast != 0) {
            int thr1y, thr2y, off1, off2, thr1, thr2, a1, a2;
            if (videoRange == 0) {
                int tmp1, tmp2;
                /* Contrast */
                thr1 = (219 * (contrast + 128)) / 512;
                thr1y = (219 - 2 * thr1) / 2;
                thr2 = 219 - thr1;
                thr2y = 219 - thr1y;

                tmp1 = (thr1y * 256) / thr1;
                tmp2 = ((thr2y - thr1y) * 256) / (thr2 - thr1);
                off1 = ((thr1y - ((tmp2 * thr1) / 256)) * a) / 256;
                off2 = ((thr2y - ((tmp1 * thr2) / 256)) * a) / 256;

                tmp1 = (64 * (contrast + 128)) / 128;
                tmp2 = 256 * (128 - tmp1);
                a1 = (tmp2 + off2) / thr1;
                a2 = a1 + (256 * (off2 - 1)) / (thr2 - thr1);
            } else {
                /* Contrast */
                thr1 = (64 * (contrast + 128)) / 128;
                thr1y = 128 - thr1;
                thr2 = 256 - thr1;
                thr2y = 256 - thr1y;
                a1 = (thr1y * 256) / thr1;
                a2 = ((thr2y - thr1y) * 256) / (thr2 - thr1);
                off1 = thr1y - (a2 * thr1) / 256;
                off2 = thr2y - (a1 * thr2) / 256;
            }

            if (a1 > 1023)
                a1 = 1023;
            else if (a1 < 0)
                a1 = 0;

            if (a2 > 1023)
                a2 = 1023;
            else if (a2 < 0)
                a2 = 0;

            if (thr1 > 255)
                thr1 = 255;
            else if (thr1 < 0)
                thr1 = 0;

            if (thr2 > 255)
                thr2 = 255;
            else if (thr2 < 0)
                thr2 = 0;

            if (off1 > 511)
                off1 = 511;
            else if (off1 < -512)
                off1 = -512;

            if (off2 > 511)
                off2 = 511;
            else if (off2 < -512)
                off2 = -512;

            rk_SetRegisterFile(handle, HWIF_CONTRAST_THR1, thr1);
            rk_SetRegisterFile(handle, HWIF_CONTRAST_THR2, thr2);

            rk_SetRegisterFile(handle, HWIF_CONTRAST_OFF1, off1);
            rk_SetRegisterFile(handle, HWIF_CONTRAST_OFF2, off2);

            rk_SetRegisterFile(handle, HWIF_COLOR_COEFFA1, a1);
            rk_SetRegisterFile(handle, HWIF_COLOR_COEFFA2, a2);
        } else {
            rk_SetRegisterFile(handle, HWIF_CONTRAST_THR1, 55);
            rk_SetRegisterFile(handle, HWIF_CONTRAST_THR2, 165);
            rk_SetRegisterFile(handle, HWIF_CONTRAST_OFF1, 0);
            rk_SetRegisterFile(handle, HWIF_CONTRAST_OFF2, 0);
            tmp = a;
            if (tmp > 1023)
                tmp = 1023;
            else if (tmp < 0)
                tmp = 0;
            rk_SetRegisterFile(handle, HWIF_COLOR_COEFFA1, tmp);
            rk_SetRegisterFile(handle, HWIF_COLOR_COEFFA2, tmp);
        }

        rk_SetRegisterFile(handle, HWIF_PP_OUT_ENDIAN, 0);

        /* saturation */
        satur = 64 + SATURATION;

        tmp = (satur * (int) b) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        rk_SetRegisterFile(handle, HWIF_COLOR_COEFFB, (unsigned int) tmp);

        tmp = (satur * (int) c) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        rk_SetRegisterFile(handle, HWIF_COLOR_COEFFC, (unsigned int) tmp);

        tmp = (satur * (int) d) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        rk_SetRegisterFile(handle, HWIF_COLOR_COEFFD, (unsigned int) tmp);

        tmp = (satur * (int) e) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;

        rk_SetRegisterFile(handle, HWIF_COLOR_COEFFE, (unsigned int) tmp);
    }

    switch (outColor) {
    case    PP_OUT_FORMAT_RGB565:
        rk_SetRegisterFile(handle, HWIF_R_MASK, 0xF800F800);
        rk_SetRegisterFile(handle, HWIF_G_MASK, 0x07E007E0);
        rk_SetRegisterFile(handle, HWIF_B_MASK, 0x001F001F);

        rk_SetRegisterFile(handle, HWIF_RGB_R_PADD, 0);
        rk_SetRegisterFile(handle, HWIF_RGB_G_PADD, 5);
        rk_SetRegisterFile(handle, HWIF_RGB_B_PADD, 11);
        if (dither) { //always do dither
            //WHLOG("we do dither.");
            rk_SetRegisterFile(handle, HWIF_DITHER_SELECT_R, 2);
            rk_SetRegisterFile(handle, HWIF_DITHER_SELECT_G, 3);
            rk_SetRegisterFile(handle, HWIF_DITHER_SELECT_B, 2);
        } else {
            //WHLOG("we do not dither.");
        }
        rk_SetRegisterFile(handle, HWIF_RGB_PIX_IN32, 1);
        rk_SetRegisterFile(handle, HWIF_PP_OUT_SWAP16_E, 1);
        rk_SetRegisterFile(handle, HWIF_PP_OUT_FORMAT, 0);
        break;
    case    PP_OUT_FORMAT_ARGB:
        rk_SetRegisterFile(handle, HWIF_R_MASK, 0x000000FF | (0xff << 24));
        rk_SetRegisterFile(handle, HWIF_G_MASK, 0x0000FF00 | (0xff << 24));
        rk_SetRegisterFile(handle, HWIF_B_MASK, 0x00FF0000 | (0xff << 24));
        rk_SetRegisterFile(handle, HWIF_RGB_R_PADD, 24);
        rk_SetRegisterFile(handle, HWIF_RGB_G_PADD, 16);
        rk_SetRegisterFile(handle, HWIF_RGB_B_PADD, 8);

        rk_SetRegisterFile(handle, HWIF_RGB_PIX_IN32, 0);
        rk_SetRegisterFile(handle, HWIF_PP_OUT_FORMAT, 0);
        break;
    case    PP_OUT_FORMAT_YUV422INTERLAVE:
        rk_SetRegisterFile(handle, HWIF_PP_OUT_FORMAT, 3);
        break;
    case    PP_OUT_FORMAT_YUV420INTERLAVE: {
        RK_U32 phy_addr = phy_addr = dst->phy_addr;
#if 0
        if (!VPUMemJudgeIommu()) {
            rk_SetRegisterFile(handle, HWIF_PP_OUT_CH_BASE, (phy_addr + outWidth * outHeight));
        } else {
            rk_SetRegisterFile(handle, HWIF_PP_OUT_CH_BASE, phy_addr);
        }
#else
		dstLum = dst->phy_addr | ((outWidth*outHeight)<<10);
		//rk_SetRegisterFile(handle, HWIF_PP_OUT_CH_BASE, dstLum);
		rk_SetRegisterFile(handle, HWIF_PP_OUT_CH_BASE, phy_addr);
#endif
        rk_SetRegisterFile(handle, HWIF_PP_OUT_FORMAT, 5);
    }
    break;
    default:
        return -1;
    }

    rk_SetRegisterFile(handle, HWIF_ROTATION_MODE, 0);


    {
        unsigned int        inw, inh;
        unsigned int        outw, outh;

        inw = inWidth - 1;
        inh = inHeigth - 1;
        outw = outWidth - 1;
        outh = outHeight - 1;


        if (inw < outw) {
            rk_SetRegisterFile(handle, HWIF_HOR_SCALE_MODE, 1);
            rk_SetRegisterFile(handle, HWIF_SCALE_WRATIO, (outw << 16) / inw);
            rk_SetRegisterFile(handle, HWIF_WSCALE_INVRA, (inw << 16) / outw);
        } else if (inw > outw) {
            rk_SetRegisterFile(handle, HWIF_HOR_SCALE_MODE, 2);
            rk_SetRegisterFile(handle, HWIF_WSCALE_INVRA, ((outw + 1) << 16) / (inw + 1));
        } else
            rk_SetRegisterFile(handle, HWIF_HOR_SCALE_MODE, 0);

        if (inh < outh) {
            rk_SetRegisterFile(handle, HWIF_VER_SCALE_MODE, 1);
            rk_SetRegisterFile(handle, HWIF_SCALE_HRATIO, (outh << 16) / inh);
            rk_SetRegisterFile(handle, HWIF_HSCALE_INVRA, (inh << 16) / outh);
        } else if (inh > outh) {
            rk_SetRegisterFile(handle, HWIF_VER_SCALE_MODE, 2);
            rk_SetRegisterFile(handle, HWIF_HSCALE_INVRA, ((outh + 1) << 16) / (inh + 1) + 1);
        } else
            rk_SetRegisterFile(handle, HWIF_VER_SCALE_MODE, 0);
    }

    rk_SetRegisterFile(handle, HWIF_PP_PIPELINE_E, ppInfo->enable);
    return 0;
}
