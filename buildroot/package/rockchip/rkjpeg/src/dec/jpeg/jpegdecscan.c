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
--  Description : jpeg scan decode source
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
        - JpegDecDecodeScan
        - JpegDecDecodeScanHeader
        - JpegDecDecodeBaselineScan
        - JpegDecDecodeProgressiveScan
        - JpegDecDecodeBaselineBlock

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "jpegdecutils.h"
#include "jpegdecinternal.h"
#include "jpegdecmarkers.h"
#include "jpegdecscan.h"
#include "dwl.h"
#include "allocator_drm.h"

#define ALOGV printf
#define ALOGD printf
#define ALOGI printf
#define ALOGW printf
#define ALOGE printf


/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

_DTRACE:   define this flag to print trace information to stdout

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeScanHeader(JpegDecContainer *);

/*------------------------------------------------------------------------------
    5. Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeScan

        Functional description:
          Handles the top level control of scan decoding

        Inputs:
          JpegDecContainer *pDecData      Pointer to JpegDecContainer structure

        Outputs:
          OK  (0)
          NOK (-1)

------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeScan(JpegDecContainer * pDecData)
{
    JpegDecRet retCode; /* Returned code container */

    retCode = JPEGDEC_ERROR;

    retCode = JpegDecDecodeScanHeader(pDecData);
    if (retCode != JPEGDEC_OK) {
        return (retCode);
    }

    JPEGDEC_TRACE_INTERNAL(("SCAN: Allocate residual buffer\n"));

    /* Allocate buffers if not done already */
    if (!pDecData->info.allocated) {
        retCode = JpegDecAllocateResidual(pDecData);
        if (retCode != JPEGDEC_OK) {
            JPEGDEC_TRACE_INTERNAL(("SCAN: ALLOCATE ERROR\n"));

            return (retCode);
        }
        /* update */
        pDecData->info.allocated = 1;
    } else {
#if 0
        int timeout = 0;

        if (pDecData->asicBuff.outLumaBuffer.phy_addr)
			allocator_drm.free(pDecData->ctx, &pDecData->asicBuff.outLumaBuffer); 
            //VPUFreeLinear(&pDecData->asicBuff.outLumaBuffer);

        while (VPUMallocLinear(&pDecData->asicBuff.outLumaBuffer, (pDecData->image.sizeLuma + (pDecData->image.sizeChroma)))) {
            usleep(5000);
            timeout++;
            if (timeout > 0xFF) {
                ALOGE("MJPEG Malloc outbuf Failed \n");
                return (JPEGDEC_MEMFAIL);
            }
        }
#endif
        pDecData->asicBuff.outLumaBuffer = *pDecData->pictureMem;
        /* luma bus address to output */
        pDecData->info.outLuma = pDecData->asicBuff.outLumaBuffer;

        if (pDecData->image.sizeChroma) {
            pDecData->asicBuff.outChromaBuffer.phy_addr = pDecData->asicBuff.outLumaBuffer.phy_addr + (pDecData->image.sizeLuma << 10);

            if (pDecData->info.operationType != JPEGDEC_BASELINE) {
                pDecData->asicBuff.outChromaBuffer2.phy_addr = pDecData->asicBuff.outChromaBuffer.phy_addr + (pDecData->image.sizeChroma / 2);
            } else {
                pDecData->asicBuff.outChromaBuffer2.phy_addr = 0;
            }

            // chroma bus address to output
            pDecData->info.outChroma = pDecData->asicBuff.outChromaBuffer;
            pDecData->info.outChroma2 = pDecData->asicBuff.outChromaBuffer2;
        }
    }

    JPEGDEC_TRACE_INTERNAL(("SCAN: Scan rlc data\n"));
    JPEGDEC_TRACE_INTERNAL(("SCAN: MODE: %d\n", pDecData->frame.codingType));

    return (JPEGDEC_OK);

}

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeScanHeader

        Functional description:
          Decodes scan headers

        Inputs:
          JpegDecContainer *pDecData       Pointer to JpegDecContainer structure

        Outputs:
          OK  (0)
          NOK (-1)

------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeScanHeader(JpegDecContainer * pDecData)
{
    RK_U32 i;
    RK_U32 tmp;

    StreamStorage *pStream = &(pDecData->stream);

    pDecData->scan.Ls = JpegDecGet2Bytes(pStream);

    /* check if there is enough data */
    if (((pStream->readBits / 8) + pDecData->scan.Ls) > pStream->streamLength)
        return (JPEGDEC_STRM_ERROR);

    pDecData->scan.Ns = JpegDecGetByte(pStream);

    pDecData->info.fillX = pDecData->info.fillY = 0;
    if (pDecData->scan.Ns == 1) {
        /* Reset to non-interleaved baseline operation type */
        if (pDecData->info.operationType == JPEGDEC_BASELINE &&
            pDecData->info.yCbCrMode != JPEGDEC_YUV400)
            pDecData->info.operationType = JPEGDEC_NONINTERLEAVED;

        tmp = JpegDecGetByte(&(pDecData->stream));
        pDecData->frame.cIndex = tmp - 1;
        pDecData->info.componentId = pDecData->frame.cIndex;
        pDecData->scan.Cs[pDecData->frame.cIndex] = tmp;
        tmp = JpegDecGetByte(pStream);
        pDecData->scan.Td[pDecData->frame.cIndex] = tmp >> 4;
        pDecData->scan.Ta[pDecData->frame.cIndex] = tmp & 0x0F;

        /* check/update component info */
        if (pDecData->frame.Nf == 3) {
            pDecData->info.fillRight = 0;
            pDecData->info.fillBottom = 0;
            pDecData->info.
            pfNeeded[pDecData->scan.Cs[pDecData->frame.cIndex] - 1] = 0;
            if (pDecData->scan.Cs[pDecData->frame.cIndex] == 2 ||
                pDecData->scan.Cs[pDecData->frame.cIndex] == 3) {
                if (pDecData->info.operationType == JPEGDEC_PROGRESSIVE ||
                    pDecData->info.operationType == JPEGDEC_NONINTERLEAVED ||
                    pDecData->info.nonInterleavedScanReady) {
                    if (pDecData->info.yCbCrModeOrig == JPEGDEC_YUV420) {
                        pDecData->info.X = pDecData->frame.hwX >> 1;
                        pDecData->info.Y = pDecData->frame.hwY >> 1;
                    } else if (pDecData->info.yCbCrModeOrig == JPEGDEC_YUV422) {
                        pDecData->info.X = pDecData->frame.hwX >> 1;
                    } else if (pDecData->info.yCbCrModeOrig == JPEGDEC_YUV440) {
                        pDecData->info.Y = pDecData->frame.hwY >> 1;
                    } else {
                        pDecData->info.yCbCrMode = 0;
                        return (JPEGDEC_UNSUPPORTED);
                    }
                }

                pDecData->info.yCbCrMode = 0;
            } else if (pDecData->scan.Cs[pDecData->frame.cIndex] == 1) { /* YCbCr 4:2:0 */
                pDecData->info.X = pDecData->frame.hwX;
                pDecData->info.Y = pDecData->frame.hwY;
                if (pDecData->info.yCbCrMode == JPEGDEC_YUV420) {
                    pDecData->info.yCbCrMode = 1;
                } else if (pDecData->info.yCbCrMode == JPEGDEC_YUV422) {
                    pDecData->info.yCbCrMode = 0;
                    if (pDecData->frame.cIndex == 0) {
                        JPEGDEC_TRACE_INTERNAL(("SCAN: #YUV422 FLAG\n"));
                        pDecData->info.yCbCr422 = 1;
                    }
                } else if (pDecData->info.yCbCrMode == JPEGDEC_YUV444) {
                    pDecData->info.yCbCrMode = 0;
                    return (JPEGDEC_UNSUPPORTED);
                }
            } else {
                pDecData->info.yCbCrMode = 0;
                return (JPEGDEC_UNSUPPORTED);
            }

            if (pDecData->info.X & 0xF) {
                pDecData->info.X += 8;
                pDecData->info.fillX = 1;
            } else if ((pDecData->scan.Cs[pDecData->frame.cIndex] == 1 ||
                        pDecData->info.yCbCrModeOrig == JPEGDEC_YUV440) &&
                       (pDecData->frame.X & 0xF) && (pDecData->frame.X & 0xF) <= 8) {
                pDecData->info.fillRight = 1;
            }

            if (pDecData->info.Y & 0xF) {
                pDecData->info.Y += 8;
                pDecData->info.fillY = 1;
            } else if ((pDecData->scan.Cs[pDecData->frame.cIndex] == 1 ||
                        pDecData->info.yCbCrModeOrig == JPEGDEC_YUV422) &&
                       (pDecData->frame.Y & 0xF) && (pDecData->frame.Y & 0xF) <= 8) {
                pDecData->info.fillBottom = 1;
            }
        } else if (pDecData->frame.Nf == 1) {
            JPEGDEC_TRACE_INTERNAL(("SCAN: #YUV422 FLAG\n"));
            pDecData->info.yCbCr422 = 0;
        }

        /* decoding info */
        if (pDecData->info.operationType == JPEGDEC_PROGRESSIVE ||
            pDecData->info.operationType == JPEGDEC_NONINTERLEAVED) {
            pDecData->info.yCbCrMode = 0;
        }
    } else {
        for (i = 0; i < pDecData->scan.Ns; i++) {
            pDecData->scan.Cs[i] = JpegDecGetByte(&(pDecData->stream));
            tmp = JpegDecGetByte(pStream);
            pDecData->scan.Td[i] = tmp >> 4;    /* which DC table */
            pDecData->scan.Ta[i] = tmp & 0x0F;  /* which AC table */
            pDecData->info.pfNeeded[i] = 1;
        }
        pDecData->info.X = pDecData->frame.hwX;
        pDecData->info.Y = pDecData->frame.hwY;
        pDecData->frame.cIndex = 0;
        pDecData->info.yCbCrMode = pDecData->info.yCbCrModeOrig;
    }

    pDecData->scan.Ss = JpegDecGetByte(pStream);
    pDecData->scan.Se = JpegDecGetByte(pStream);
    tmp = JpegDecGetByte(pStream);
    pDecData->scan.Ah = tmp >> 4;
    pDecData->scan.Al = tmp & 0x0F;

    if (pDecData->frame.codingType == SOF0) {
        /* baseline */
        if (pDecData->scan.Ss != 0)
            return (JPEGDEC_UNSUPPORTED);
        if (pDecData->scan.Se != 63)
            return (JPEGDEC_UNSUPPORTED);
        if (pDecData->scan.Ah != 0)
            return (JPEGDEC_UNSUPPORTED);
        if (pDecData->scan.Al != 0)
            return (JPEGDEC_UNSUPPORTED);

        /* update scan decoding parameters */
        /* interleaved/non-interleaved */
        if (pDecData->info.operationType == JPEGDEC_BASELINE)
            pDecData->info.nonInterleaved = 0;
        else
            pDecData->info.nonInterleaved = 1;
        /* decoding info */
        if ((pDecData->frame.Nf == 3 && pDecData->scan.Ns == 1) ||
            (pDecData->frame.Nf == 1 && pDecData->scan.Ns == 1))
            pDecData->info.amountOfQTables = 1;
        else
            pDecData->info.amountOfQTables = 3;
    }

    if (pDecData->frame.codingType == SOF2) {
        /* progressive */
        if (pDecData->scan.Ss == 0 && pDecData->scan.Se != 0)
            return (JPEGDEC_UNSUPPORTED);
        if (pDecData->scan.Ah < 0 || pDecData->scan.Ah > 13)
            return (JPEGDEC_UNSUPPORTED);
        if (pDecData->scan.Al < 0 || pDecData->scan.Al > 13)
            return (JPEGDEC_UNSUPPORTED);

        /* update scan decoding parameters */
        /* TODO! What if 2 components, possible??? */
        /* interleaved/non-interleaved */
        if (pDecData->scan.Ns == 1) {
            pDecData->info.nonInterleaved = 1;
            /* component ID */
            pDecData->info.componentId = pDecData->frame.cIndex;
            pDecData->info.amountOfQTables = 1;
        } else {
            pDecData->info.nonInterleaved = 0;
            /* component ID ==> set to luma ==> interleaved */
            pDecData->info.componentId = 0;
            pDecData->info.amountOfQTables = 3;
        }

    }

    return (JPEGDEC_OK);
}
