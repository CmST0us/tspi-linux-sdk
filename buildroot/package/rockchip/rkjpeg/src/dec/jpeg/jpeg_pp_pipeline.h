/*------------------------------------------------------------------------------
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                                                                            --
--                            Hantro Products Oy.                             --
--                                                                            --
--                   (C) COPYRIGHT 2006 HANTRO PRODUCTS OY                    --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
--------------------------------------------------------------------------------
--
--  Abstract : JPEG decoder and PP pipeline support
--
--------------------------------------------------------------------------------
--
--  Version control information, please leave untouched.
--
--  $RCSfile: jpeg_pp_pipeline.h,v $
--  $Date: 2007/10/22 08:21:36 $
--  $Revision: 1.2 $
--
------------------------------------------------------------------------------*/

#ifndef JPEG_PP_PIPELINE_H
#define JPEG_PP_PIPELINE_H

#include "decppif.h"

RK_S32 jpegRegisterPP(const void *decInst, const void *ppInst,
                      void (*PPRun) (const void *, DecPpInterface *),
                      void (*PPEndCallback) (const void *),
                      void (*PPConfigQuery) (const void *, DecPpQuery *));

RK_S32 jpegUnregisterPP(const void *decInst, const void *ppInst);

#endif /* #ifdef JPEG_PP_PIPELINE_H */
