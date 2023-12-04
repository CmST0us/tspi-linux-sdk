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
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/
#ifndef _PPDEC_H_
#define _PPDEC_H_

#include "vpu_type.h"

/* PP input types (picStruct) */
/* Frame or top field */
#define DECPP_PIC_FRAME_OR_TOP_FIELD                       0U
/* Bottom field only */
#define DECPP_PIC_BOT_FIELD                                1U
/* top and bottom is separate locations */
#define DECPP_PIC_TOP_AND_BOT_FIELD                        2U
/* top and bottom interleaved */
#define DECPP_PIC_TOP_AND_BOT_FIELD_FRAME                  3U
/* interleaved top only */
#define DECPP_PIC_TOP_FIELD_FRAME                          4U
/* interleaved bottom only */
#define DECPP_PIC_BOT_FIELD_FRAME                          5U

/* Control interface between decoder and pp */
/* decoder writes, pp read-only */

typedef struct DecPpInterface_ {
    enum {
        DECPP_IDLE = 0,
        DECPP_RUNNING,  /* PP was started */
        DECPP_PIC_READY, /* PP has finished a picture */
        DECPP_PIC_NOT_FINISHED /* PP still processing a picture */
    } ppStatus; /* Decoder keeps track of what it asked the pp to do */

    enum {
        MULTIBUFFER_UNINIT = 0, /* buffering mode not yet decided */
        MULTIBUFFER_DISABLED,   /* Single buffer legacy mode */
        MULTIBUFFER_SEMIMODE,   /* enabled but full pipel cannot be used */
        MULTIBUFFER_FULLMODE    /* enabled and full pipeline successful */
    } multiBufStat;

    RK_U32 inputBusLuma;
    RK_U32 inputBusChroma;
    RK_U32 bottomBusLuma;
    RK_U32 bottomBusChroma;
    RK_U32 picStruct;           /* structure of input picture */
    RK_U32 topField;
    RK_U32 inwidth;
    RK_U32 inheight;
    RK_U32 usePipeline;
    RK_U32 littleEndian;
    RK_U32 wordSwap;
    RK_U32 croppedW;
    RK_U32 croppedH;

    RK_U32 bufferIndex;         /* multibuffer, where to put PP output */
    RK_U32 displayIndex;        /* multibuffer, next picture in display order */
    RK_U32 prevAnchorDisplayIndex;

    /* VC-1 */
    RK_U32 rangeRed;
    RK_U32 rangeMapYEnable;
    RK_U32 rangeMapYCoeff;
    RK_U32 rangeMapCEnable;
    RK_U32 rangeMapCCoeff;
} DecPpInterface;

/* Decoder asks with this struct information about pp setup */
/* pp writes, decoder read-only */

typedef struct DecPpQuery_ {
    RK_U32 pipelineAccepted;    /* PP accepts pipeline */
    RK_U32 deinterlace;         /* Deinterlace feature used */
    RK_U32 multiBuffer;         /* multibuffer PP output enabled */
    RK_U32 ppConfigChanged;     /* PP config changed after previous output */
} DecPpQuery;

#endif
