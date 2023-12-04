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
--  Description : Jpeg Decoder Internal functions
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

#ifndef JPEGDECINTERNAL_H
#define JPEGDECINTERNAL_H

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "vpu_type.h"
#include "jpegdecapi.h"
#include "jpegdeccontainer.h"
#define LOG_TAG "JPEG_DEC"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/
#define JPEGDEC_TRACE_INTERNAL(args) //ALOGE args

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/
void JpegDecClearStructs(JpegDecContainer * pJpegDecCont);
JpegDecRet JpegDecInitHW(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWContinue(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWInputBuffLoad(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWProgressiveContinue(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWNonInterleaved(JpegDecContainer * pJpegDecCont);
JpegDecRet JpegDecAllocateResidual(JpegDecContainer * pJpegDecCont);
void JpegDecSliceSizeCalculation(JpegDecContainer * pJpegDecCont);
JpegDecRet JpegDecNextScanHdrs(JpegDecContainer * pJpegDecCont);
void JpegRefreshRegs(JpegDecContainer * pJpegDecCont);
void JpegFlushRegs(JpegDecContainer * pJpegDecCont);
void JpegDecInitHWEmptyScan(JpegDecContainer * pJpegDecCont, RK_U32 componentId);

#endif /* #endif JPEGDECDATA_H */

