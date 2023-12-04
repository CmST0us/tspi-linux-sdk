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
--  Description : Jpeg decoder header decoding source code
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
        - JpegDecDecodeFrame
        - JpegDecDecodeQuantTables
        - JpegDecDecodeHuffmanTables

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "jpegdechdrs.h"
#include "jpegdecutils.h"
#include "jpegdecmarkers.h"
#include "jpegdecinternal.h"
#include "jpegdecscan.h"
#include "jpegdecapi.h"
#include "dwl.h"
#include <stdint.h>
/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    5. Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeFrame

        Functional description:
          Decodes frame headers

        Inputs:
          JpegDecContainer *pDecData      Pointer to JpegDecContainer structure

        Outputs:
          OK/NOK

------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeFrameHdr(JpegDecContainer * pDecData)
{
    RK_U32 i;
    RK_U32 width, height;
    RK_U32 tmp1, tmp2;
    RK_U32 Hmax = 0;
    RK_U32 Vmax = 0;
    RK_U32 size = 0;
    JpegDecRet retCode;

    retCode = JPEGDEC_OK;

    /* frame header length */
    pDecData->frame.Lf = JpegDecGet2Bytes(&(pDecData->stream));

    /* check if there is enough data */
    if (((pDecData->stream.readBits / 8) + pDecData->frame.Lf) >
        pDecData->stream.streamLength)
        return (JPEGDEC_STRM_ERROR);

    /* Sample precision */
    pDecData->frame.P = JpegDecGetByte(&(pDecData->stream));
    if (pDecData->frame.P != 8) {
        JPEGDEC_TRACE_INTERNAL(("if ( pDecData->frame.P != 8)\n"));
        return (JPEGDEC_UNSUPPORTED);
    }
    /* Number of Lines */
    pDecData->frame.Y = JpegDecGet2Bytes(&(pDecData->stream));
    if (pDecData->frame.Y < 1) {
        return (JPEGDEC_UNSUPPORTED);
    }
    pDecData->frame.hwY = pDecData->frame.Y;

    /* round up to next multiple-of-16 */
    pDecData->frame.hwY += 0xf;
    pDecData->frame.hwY &= ~(0xf);

    /* Number of samples per line */
    pDecData->frame.X = JpegDecGet2Bytes(&(pDecData->stream));
    if (pDecData->frame.X < 1) {
        return (JPEGDEC_UNSUPPORTED);
    }
    pDecData->frame.hwX = pDecData->frame.X;

    /* round up to next multiple-of-16 */
    pDecData->frame.hwX += 0xf;
    pDecData->frame.hwX &= ~(0xf);

    /* for internal() */
    pDecData->info.X = pDecData->frame.hwX;
    pDecData->info.Y = pDecData->frame.hwY;

    /* check for minimum and maximum dimensions */
    if (pDecData->frame.hwX < pDecData->minSupportedWidth ||
        pDecData->frame.hwY < pDecData->minSupportedHeight ||
        pDecData->frame.hwX > pDecData->maxSupportedWidth ||
        pDecData->frame.hwY > pDecData->maxSupportedHeight ||
        (pDecData->frame.hwX * pDecData->frame.hwY) >
        pDecData->maxSupportedPixelAmount) {
        JPEGDEC_TRACE_INTERNAL(("FRAME: Unsupported size\n"));
        return (JPEGDEC_UNSUPPORTED);
    }

    /* Number of components */
    pDecData->frame.Nf = JpegDecGetByte(&(pDecData->stream));
    if ((pDecData->frame.Nf != 3) && (pDecData->frame.Nf != 1)) {
        JPEGDEC_TRACE_INTERNAL(("pDecData->frame.Nf != 3 && pDecData->frame.Nf != 1\n"));
        return (JPEGDEC_UNSUPPORTED);
    }

    /* save component specific data */
    /* Nf == number of components */
    for (i = 0; i < pDecData->frame.Nf; i++) {
        pDecData->frame.component[i].C = JpegDecGetByte(&(pDecData->stream));
        if (i == 0) { /* for the first component */
            /* if first component id is something else than 1 (jfif) */
            pDecData->scan.index = pDecData->frame.component[i].C;
        } else {
            /* if component ids 'jumps' */
            if ((pDecData->frame.component[i - 1].C + 1) !=
                pDecData->frame.component[i].C) {
                JPEGDEC_TRACE_INTERNAL(("component ids 'jumps'\n"));
                return (JPEGDEC_UNSUPPORTED);
            }
        }
        tmp1 = JpegDecGetByte(&(pDecData->stream));
        pDecData->frame.component[i].H = tmp1 >> 4;
        if (pDecData->frame.component[i].H > Hmax) {
            Hmax = pDecData->frame.component[i].H;
        }
        pDecData->frame.component[i].V = tmp1 & 0xF;
        if (pDecData->frame.component[i].V > Vmax) {
            Vmax = pDecData->frame.component[i].V;
        }

        pDecData->frame.component[i].Tq = JpegDecGetByte(&(pDecData->stream));
    }

    if (pDecData->frame.Nf == 1) {
        Hmax = Vmax = 1;
        pDecData->frame.component[0].H = 1;
        pDecData->frame.component[0].V = 1;
    } else if (Hmax == 0 || Vmax == 0) {
        JPEGDEC_TRACE_INTERNAL(("Hmax == 0 || Vmax == 0 \n"));
        return (JPEGDEC_UNSUPPORTED);
    }

    /* JPEG_YCBCR411 horizontal size has to be multiple of 32 pels */
    if (Hmax == 4 && (pDecData->frame.hwX & 0x1F)) {
        /* round up to next multiple-of-32 */
        pDecData->frame.hwX += 16;
        pDecData->info.X = pDecData->frame.hwX;

        /* check for minimum and maximum dimensions */
        if (pDecData->frame.hwX > pDecData->maxSupportedWidth ||
            (pDecData->frame.hwX * pDecData->frame.hwY) >
            pDecData->maxSupportedPixelAmount) {
            JPEGDEC_TRACE_INTERNAL(("FRAME: Unsupported size\n"));
            return (JPEGDEC_UNSUPPORTED);
        }
    }

    /* set image pointers, calculate pixelPerRow for each component */
    width = ((pDecData->frame.hwX + Hmax * 8 - 1) / (Hmax * 8)) * Hmax * 8;
    height = ((pDecData->frame.hwY + Vmax * 8 - 1) / (Vmax * 8)) * Vmax * 8;

    /* calculate numMcuInRow and numMcuInFrame */
    ASSERT(Hmax != 0);
    ASSERT(Vmax != 0);
    pDecData->frame.numMcuInRow = width / (8 * Hmax);
    pDecData->frame.numMcuInFrame = pDecData->frame.numMcuInRow *
                                    (height / (8 * Vmax));

    /* reset mcuNumbers */
    pDecData->frame.mcuNumber = 0;
    pDecData->frame.row = pDecData->frame.col = 0;

    for (i = 0; i < pDecData->frame.Nf; i++) {

        ASSERT(i <= 2);

        tmp1 = (width * pDecData->frame.component[i].H + Hmax - 1) / Hmax;
        tmp2 = (height * pDecData->frame.component[i].V + Vmax - 1) / Vmax;
        size += tmp1 * tmp2;
        /* pixels per row */

        pDecData->image.pixelsPerRow[i] = tmp1;
        pDecData->image.columns[i] = tmp2;
        pDecData->frame.numBlocks[i] =
            (((pDecData->frame.hwX * pDecData->frame.component[i].H) / Hmax +
              7) >> 3) * (((pDecData->frame.hwY *
                            pDecData->frame.component[i].V) / Vmax + 7) >> 3);

        if (i == 0) {
            pDecData->image.sizeLuma = size;
        }
    }

    pDecData->image.size = size;
    pDecData->image.sizeChroma = size - pDecData->image.sizeLuma;

    /* set YUV mode & calculate rlc tmp size */
    retCode = JpegDecMode(pDecData);
    if (retCode != JPEGDEC_OK) {
        return (retCode);
    }

    return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeQuantTables

        Functional description:
          Decodes quantisation tables from the stream

        Inputs:
          JpegDecContainer *pDecData      Pointer to JpegDecContainer structure

        Outputs:
          OK    (0)
          NOK   (-1)

------------------------------------------------------------------------------*/

JpegDecRet JpegDecDecodeQuantTables(JpegDecContainer * pDecData)
{
    RK_U32 t, tmp, i;
    StreamStorage *pStream = &(pDecData->stream);

    pDecData->quant.Lq = JpegDecGet2Bytes(pStream);

    /* check if there is enough data for tables */
    if (((pStream->readBits / 8) + pDecData->quant.Lq) > pStream->streamLength)
        return (JPEGDEC_STRM_ERROR);

    t = 4;

    while (t < pDecData->quant.Lq) {
        /* Tq value selects what table the components use */

        /* read tables and write to decData->quant */
        tmp = JpegDecGetByte(pStream);
        t++;
        /* supporting only 8 bits / sample */
        if ((tmp >> 4) != 0) {
            return (JPEGDEC_UNSUPPORTED);
        }
        tmp &= 0xF;
        /* set the quantisation table pointer */

        if (tmp == 0) {
            JPEGDEC_TRACE_INTERNAL(("qtable0\n"));
            pDecData->quant.table = pDecData->quant.table0;
        } else if (tmp == 1) {
            JPEGDEC_TRACE_INTERNAL(("qtable1\n"));
            pDecData->quant.table = pDecData->quant.table1;
        } else if (tmp == 2) {
            JPEGDEC_TRACE_INTERNAL(("qtable2\n"));
            pDecData->quant.table = pDecData->quant.table2;
        } else if (tmp == 3) {
            JPEGDEC_TRACE_INTERNAL(("qtable3\n"));
            pDecData->quant.table = pDecData->quant.table3;
        } else {
            return (JPEGDEC_UNSUPPORTED);
        }
        for (i = 0; i < 64; i++) {
            pDecData->quant.table[i] = JpegDecGetByte(pStream);
            t++;
        }

    }

    return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeHuffmanTables

        Functional description:
          Decodes huffman tables from the stream

        Inputs:
          DecData *JpegDecContainer      Pointer to JpegDecContainer structure

        Outputs:
          OK    (0)
          NOK   (-1)

------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeHuffmanTables(JpegDecContainer * pDecData)
{
    RK_U32 i, len, Tc, Th, tmp;
    RK_S32 j;
    StreamStorage *pStream = &(pDecData->stream);

    pDecData->vlc.Lh = JpegDecGet2Bytes(pStream);

    /* check if there is enough data for tables */
    if (((pStream->readBits / 8) + pDecData->vlc.Lh) > pStream->streamLength)
        return (JPEGDEC_STRM_ERROR);

    /* four bytes already read in */
    len = 4;

    while (len < pDecData->vlc.Lh) {
        tmp = JpegDecGetByte(pStream);
        len++;
        Tc = tmp >> 4;  /* Table class */
        if (Tc != 0 && Tc != 1) {
            return (JPEGDEC_UNSUPPORTED);
        }
        Th = tmp & 0xF; /* Huffman table identifier */
        /* only two tables in baseline allowed */
        if ((pDecData->frame.codingType == SOF0) && (Th > 1)) {
            return (JPEGDEC_UNSUPPORTED);
        }
        /* four tables in progressive allowed */
        if ((pDecData->frame.codingType == SOF2) && (Th > 3)) {
            return (JPEGDEC_UNSUPPORTED);
        }
        /* set the table pointer */
        if (Tc) {
            /* Ac table */
            switch (Th) {
            case 0:
                JPEGDEC_TRACE_INTERNAL(("ac0\n"));
                pDecData->vlc.table = &(pDecData->vlc.acTable0);
                break;
            case 1:
                JPEGDEC_TRACE_INTERNAL(("ac1\n"));
                pDecData->vlc.table = &(pDecData->vlc.acTable1);
                break;
            case 2:
                JPEGDEC_TRACE_INTERNAL(("ac2\n"));
                pDecData->vlc.table = &(pDecData->vlc.acTable2);
                break;
            case 3:
                JPEGDEC_TRACE_INTERNAL(("ac3\n"));
                pDecData->vlc.table = &(pDecData->vlc.acTable3);
                break;
            default:
                return (JPEGDEC_UNSUPPORTED);
            }
        } else {
            /* Dc table */
            switch (Th) {
            case 0:
                JPEGDEC_TRACE_INTERNAL(("dc0\n"));
                pDecData->vlc.table = &(pDecData->vlc.dcTable0);
                break;
            case 1:
                JPEGDEC_TRACE_INTERNAL(("dc1\n"));
                pDecData->vlc.table = &(pDecData->vlc.dcTable1);
                break;
            case 2:
                JPEGDEC_TRACE_INTERNAL(("dc2\n"));
                pDecData->vlc.table = &(pDecData->vlc.dcTable2);
                break;
            case 3:
                JPEGDEC_TRACE_INTERNAL(("dc3\n"));
                pDecData->vlc.table = &(pDecData->vlc.dcTable3);
                break;
            default:
                return (JPEGDEC_UNSUPPORTED);
            }
        }

        tmp = 0;
        /* read in the values of list BITS */
        for (i = 0; i < 16; i++) {
            tmp += pDecData->vlc.table->bits[i] = JpegDecGetByte(pStream);
            len++;
        }
        /* allocate memory for HUFFVALs */
        if (pDecData->vlc.table->vals != NULL) {
            /* free previously reserved table */
            DWLfree(pDecData->vlc.table->vals);
        }

        pDecData->vlc.table->vals = (RK_U32 *) DWLmalloc(sizeof(RK_U32) * tmp);

        /* set the table length */
        pDecData->vlc.table->tableLength = tmp;
        /* read in the HUFFVALs */
        for (i = 0; i < tmp; i++) {
            pDecData->vlc.table->vals[i] = JpegDecGetByte(pStream);
            len++;
        }
        /* first and last lengths */
        for (i = 0; i < 16; i++) {
            if (pDecData->vlc.table->bits[i] != 0) {
                pDecData->vlc.table->start = i;
                break;
            }
        }
        for (j = 15; j >= 0; j--) {
            if (pDecData->vlc.table->bits[j] != 0) {
                pDecData->vlc.table->last = ((RK_U32) j + 1);
                break;
            }
        }
    }

    return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDefaultHuffmanTables

        Functional description:
          if jpeg file not find huffman table, use it!

        Inputs:
          DecData *JpegDecContainer      Pointer to JpegDecContainer structure

        Outputs:
          OK    (0)
          NOK   (-1)

------------------------------------------------------------------------------*/
JpegDecRet JpegDefaultHuffmanTables(JpegDecContainer * pDecData)
{

    /* Set up the standard Huffman tables (cf. JPEG standard section K.3) */
    /* IMPORTANT: these are only valid for 8-bit data precision! */
    uint8_t ff_mjpeg_bits_dc_luminance[17] =
    { /* 0-base */ 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t ff_mjpeg_val_dc[12] =       //luminance and chrominance all use it
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    uint8_t ff_mjpeg_bits_dc_chrominance[17] =
    { /* 0-base */ 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

    uint8_t ff_mjpeg_bits_ac_luminance[17] =
    { /* 0-base */ 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
    uint8_t ff_mjpeg_val_ac_luminance[] = {
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
        0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
        0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
        0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
        0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
        0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
        0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
        0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
        0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa
    };

    uint8_t ff_mjpeg_bits_ac_chrominance[17] =
    { /* 0-base */ 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };

    uint8_t ff_mjpeg_val_ac_chrominance[] = {
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
        0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
        0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
        0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
        0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
        0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
        0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
        0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
        0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
        0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
        0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa
    };

    RK_U32 i, len, tmp, k;
    RK_S32 j;
    uint8_t *bitstable[4] = {ff_mjpeg_bits_dc_luminance, ff_mjpeg_bits_ac_luminance, ff_mjpeg_bits_dc_chrominance, ff_mjpeg_bits_ac_chrominance};
    uint8_t *valtable[4] = {ff_mjpeg_val_dc, ff_mjpeg_val_ac_luminance, ff_mjpeg_val_dc, ff_mjpeg_val_ac_chrominance};
    uint8_t *bitstmp;
    uint8_t *valtmp;

    for (k = 0; k < 4; k++) {
        bitstmp = bitstable[k];
        valtmp = valtable[k];

        if (k == 0)
            pDecData->vlc.table = &(pDecData->vlc.dcTable0);
        else if (k == 1)
            pDecData->vlc.table = &(pDecData->vlc.acTable0);
        else if (k == 2)
            pDecData->vlc.table = &(pDecData->vlc.dcTable1);
        else
            pDecData->vlc.table = &(pDecData->vlc.acTable1);

        tmp = 0;
        /* read in the values of list BITS */
        for (i = 0; i < 16; i++) {
            tmp += pDecData->vlc.table->bits[i] = bitstmp[i + 1];
            len++;
        }
        /* allocate memory for HUFFVALs */
        if (pDecData->vlc.table->vals != NULL) {
            /* free previously reserved table */
            DWLfree(pDecData->vlc.table->vals);
        }

        pDecData->vlc.table->vals = (RK_U32 *) DWLmalloc(sizeof(RK_U32) * tmp);

        /* set the table length */
        pDecData->vlc.table->tableLength = tmp;
        /* read in the HUFFVALs */
        for (i = 0; i < tmp; i++) {
            pDecData->vlc.table->vals[i] = valtmp[i];
            len++;
        }
        /* first and last lengths */
        for (i = 0; i < 16; i++) {
            if (pDecData->vlc.table->bits[i] != 0) {
                pDecData->vlc.table->start = i;
                break;
            }
        }
        for (j = 15; j >= 0; j--) {
            if (pDecData->vlc.table->bits[j] != 0) {
                pDecData->vlc.table->last = ((RK_U32) j + 1);
                break;
            }
        }
    }

    return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecMode

        Functional description:
          check YCBCR mode

        Inputs:
          JpegDecContainer *pDecData      Pointer to JpegDecContainer structure

        Outputs:
          OK/NOK

------------------------------------------------------------------------------*/
JpegDecRet JpegDecMode(JpegDecContainer * pDecData)
{
    /*  check input format */
    if (pDecData->frame.Nf == 3) {
        /* JPEG_YCBCR420 */
        if (pDecData->frame.component[0].H == 2 &&
            pDecData->frame.component[0].V == 2 &&
            pDecData->frame.component[1].H == 1 &&
            pDecData->frame.component[1].V == 1 &&
            pDecData->frame.component[2].H == 1 &&
            pDecData->frame.component[2].V == 1) {
            pDecData->info.yCbCrMode = JPEGDEC_YUV420;
            pDecData->info.X = pDecData->frame.hwX;
            pDecData->info.Y = pDecData->frame.hwY;

            /* calculate new output size if slice mode used */
            if (pDecData->info.sliceMbSetValue) {
                /* Y */
                pDecData->image.sizeLuma = (pDecData->info.X *
                                            (pDecData->info.sliceMbSetValue *
                                             16));

                /* CbCr */
                pDecData->image.sizeChroma = pDecData->image.sizeLuma / 2;

            }
        }
        /* JPEG_YCBCR422 */
        else if (pDecData->frame.component[0].H == 2 &&
                 pDecData->frame.component[0].V == 1 &&
                 pDecData->frame.component[1].H == 1 &&
                 pDecData->frame.component[1].V == 1 &&
                 pDecData->frame.component[2].H == 1 &&
                 pDecData->frame.component[2].V == 1) {
            pDecData->info.yCbCrMode = JPEGDEC_YUV422;
            pDecData->info.X = (pDecData->frame.hwX);
            pDecData->info.Y = (pDecData->frame.hwY);

            /* check if fill needed */
            if ((pDecData->frame.Y & 0xF) && (pDecData->frame.Y & 0xF) <= 8)
                pDecData->info.fillBottom = 1;

            /* calculate new output size if slice mode used */
            if (pDecData->info.sliceMbSetValue) {
                /* Y */
                pDecData->image.sizeLuma = (pDecData->info.X *
                                            (pDecData->info.sliceMbSetValue *
                                             16));

                /* CbCr */
                pDecData->image.sizeChroma = pDecData->image.sizeLuma;
            }
        }
        /* JPEG_YCBCR440 */
        else if (pDecData->frame.component[0].H == 1 &&
                 pDecData->frame.component[0].V == 2 &&
                 pDecData->frame.component[1].H == 1 &&
                 pDecData->frame.component[1].V == 1 &&
                 pDecData->frame.component[2].H == 1 &&
                 pDecData->frame.component[2].V == 1) {
            pDecData->info.yCbCrMode = JPEGDEC_YUV440;
            pDecData->info.X = (pDecData->frame.hwX);
            pDecData->info.Y = (pDecData->frame.hwY);

            /* check if fill needed */
            if ((pDecData->frame.X & 0xF) && (pDecData->frame.X & 0xF) <= 8)
                pDecData->info.fillRight = 1;

            /* calculate new output size if slice mode used */
            if (pDecData->info.sliceMbSetValue) {
                /* Y */
                pDecData->image.sizeLuma = (pDecData->info.X *
                                            (pDecData->info.sliceMbSetValue *
                                             16));

                /* CbCr */
                pDecData->image.sizeChroma = pDecData->image.sizeLuma;
            }
        }
        /* JPEG_YCBCR444 : NOT SUPPORTED */
        else if (pDecData->frame.component[0].H == 1 &&
                 pDecData->frame.component[0].V == 1 &&
                 pDecData->frame.component[1].H == 1 &&
                 pDecData->frame.component[1].V == 1 &&
                 pDecData->frame.component[2].H == 1 &&
                 pDecData->frame.component[2].V == 1) {
            pDecData->info.yCbCrMode = JPEGDEC_YUV444;
            pDecData->info.X = pDecData->frame.hwX;
            pDecData->info.Y = pDecData->frame.hwY;

            /* check if fill needed */
            if ((pDecData->frame.X & 0xF) && (pDecData->frame.X & 0xF) <= 8)
                pDecData->info.fillRight = 1;

            if ((pDecData->frame.Y & 0xF) && (pDecData->frame.Y & 0xF) <= 8)
                pDecData->info.fillBottom = 1;

            /* calculate new output size if slice mode used */
            if (pDecData->info.sliceMbSetValue) {
                /* Y */
                pDecData->image.sizeLuma = (pDecData->info.X *
                                            (pDecData->info.sliceMbSetValue *
                                             16));

                /* CbCr */
                pDecData->image.sizeChroma = pDecData->image.sizeLuma * 2;
            }
        }
        /* JPEG_YCBCR411 */
        else if (pDecData->frame.component[0].H == 4 &&
                 pDecData->frame.component[0].V == 1 &&
                 pDecData->frame.component[1].H == 1 &&
                 pDecData->frame.component[1].V == 1 &&
                 pDecData->frame.component[2].H == 1 &&
                 pDecData->frame.component[2].V == 1) {
            pDecData->info.yCbCrMode = JPEGDEC_YUV411;
            pDecData->info.X = (pDecData->frame.hwX);
            pDecData->info.Y = (pDecData->frame.hwY);

            /* check if fill needed */
            if ((pDecData->frame.Y & 0xF) && (pDecData->frame.Y & 0xF) <= 8)
                pDecData->info.fillBottom = 1;

            /* calculate new output size if slice mode used */
            if (pDecData->info.sliceMbSetValue) {
                /* Y */
                pDecData->image.sizeLuma = (pDecData->info.X *
                                            (pDecData->info.sliceMbSetValue *
                                             16));

                /* CbCr */
                pDecData->image.sizeChroma = pDecData->image.sizeLuma / 2;
            }
        } else {
            return (JPEGDEC_UNSUPPORTED);
        }
    } else if (pDecData->frame.Nf == 1) {
        /* 4:0:0 */
        if ((pDecData->frame.component[0].V == 1) ||
            (pDecData->frame.component[0].H == 1)) {
            pDecData->info.yCbCrMode = JPEGDEC_YUV400;
            pDecData->info.X = (pDecData->frame.hwX);
            pDecData->info.Y = (pDecData->frame.hwY);

            /* check if fill needed */
            if ((pDecData->frame.X & 0xF) && (pDecData->frame.X & 0xF) <= 8)
                pDecData->info.fillRight = 1;

            if ((pDecData->frame.Y & 0xF) && (pDecData->frame.Y & 0xF) <= 8)
                pDecData->info.fillBottom = 1;

            /* calculate new output size if slice mode used */
            if (pDecData->info.sliceMbSetValue) {
                /* Y */
                pDecData->image.sizeLuma =
                    ((((pDecData->info.X +
                        15) / 16) * 16) * (pDecData->info.sliceMbSetValue *
                                           16));

                /* CbCr */
                pDecData->image.sizeChroma = 0;
            }
        } else {
            return (JPEGDEC_UNSUPPORTED);
        }
    } else {
        return (JPEGDEC_UNSUPPORTED);
    }

#ifdef JPEGDEC_ERROR_RESILIENCE
    if (pDecData->info.fillBottom) {
        pDecData->info.Y -= 16;
        pDecData->frame.hwY -= 16;
    }
#endif /* JPEGDEC_ERROR_RESILIENCE */

    /* save the original sampling format for progressive use */
    pDecData->info.yCbCrModeOrig = pDecData->info.yCbCrMode;

    return (JPEGDEC_OK);
}
