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
--  Description :  RK70 JPEG Decoder HW register access functions interface
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/

#ifndef _JPG_HWREGDRV_H_
#define _JPG_HWREGDRV_H_

#include "vpu_type.h"
#include "deccfg.h"
#include "rkregdrv.h"

#define JPEG_RK70_MODE_JPEG       3

#define JPEGDEC_RK70_IRQ_DEC_RDY        0x1
#define JPEGDEC_RK70_IRQ_BUS_ERROR      0x2
#define JPEGDEC_RK70_IRQ_BUFFER_EMPTY   0x4
#define JPEGDEC_RK70_IRQ_ASO            0x10
#define JPEGDEC_RK70_IRQ_STREAM_ERROR   0x100
#define JPEGDEC_RK70_IRQ_SLICE_READY    0x20
#define JPEGDEC_RK70_IRQ_TIMEOUT        0x200

#endif /* #define _JPG_HWREGDRV_H_ */

