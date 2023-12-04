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
--  Description : Hardware PP system configuration
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/

#ifndef __PPCFG_H__
#define __PPCFG_H__

/* predefined values of HW system parameters. DO NOT ALTER!      */
#define PP_RK70_PICTURE_LITTLE_ENDIAN               1
#define PP_RK70_PICTURE_BIG_ENDIAN                  0

#define PP_RK70_BUS_BURST_LENGTH_UNDEFINED         0
#define PP_RK70_BUS_BURST_LENGTH_4                 4
#define PP_RK70_BUS_BURST_LENGTH_8                 8
#define PP_RK70_BUS_BURST_LENGTH_16                16

#define PP_RK70_DATA_BUS_WIDTH_32                   4
#define PP_RK70_DATA_BUS_WIDTH_64                   8
/* end of predefined values */

/* now what we use */

#ifndef PP_RK70_USING_IRQ
/* use the HW IRQ or not; set to non-zero to enable                */
/* if set to zero the software will disable the PP IRQ             */
/* generation in the control register                              */
#define PP_RK70_USING_IRQ                           1
#endif

#ifndef PP_RK70_SWAP_32_WORDS
/* in 64 bit bus environment the 32 bit words can be swapped       */
#define PP_RK70_SWAP_32_WORDS                       0
#endif

#ifndef PP_RK70_INPUT_PICTURE_ENDIAN
/* this should match the system endianess, so that PP reads        */
/* the input pixel data in the right order                         */
#define PP_RK70_INPUT_PICTURE_ENDIAN    PP_RK70_PICTURE_LITTLE_ENDIAN
#endif

#ifndef PP_RK70_OUTPUT_PICTURE_ENDIAN
/* this should match the system endianess, so that PP writes       */
/* the output pixel data in the right order                        */
#define PP_RK70_OUTPUT_PICTURE_ENDIAN     PP_RK70_PICTURE_LITTLE_ENDIAN
#endif

#ifndef PP_RK70_BUS_BURST_LENGTH
/* how long are the hardware data bursts; better left unchanged    */
#define PP_RK70_BUS_BURST_LENGTH        PP_RK70_BUS_BURST_LENGTH_16
#endif

#ifndef PP_RK70_DATA_BUS_WIDTH
/* data bus width of the PP hardware; note that this might not be   */
/* the same with the CPU's data bus width                           */
#define PP_RK70_DATA_BUS_WIDTH           PP_RK70_DATA_BUS_WIDTH_32
#endif

#ifndef PP_RK70_LATENCY_COMPENSATION
/* compensation for bus latency; values up to 63 */
#define PP_RK70_LATENCY_COMPENSATION    0
#endif

#ifndef PP_RK70_INTERNAL_CLOCK_GATING
/* clock is gated from PP structures that are not used */
#define PP_RK70_INTERNAL_CLOCK_GATING    0
#endif

#ifndef PP_RK70_DATA_DISCARD_ENABLE
#define PP_RK70_DATA_DISCARD_ENABLE     0
#endif

#ifndef PP_RK70_SWAP_32_WORDS_INPUT
#define PP_RK70_SWAP_32_WORDS_INPUT     0
#endif

/* AXI bus read and write ID values used by HW. 0 - 255 */
#ifndef PP_RK70_AXI_ID_R
#define PP_RK70_AXI_ID_R                      0
#endif

#ifndef PP_RK70_AXI_ID_W
#define PP_RK70_AXI_ID_W                      0
#endif

/* AXI single command multiple data disable not set */
#define PP_RK70_SCMD_DISABLE                  0

/* flag to ignore endianness when reading RGB format alpha blend picture.
 * DO NOT ALTER! */
#define PP_RK70_IGNORE_ABLEND_ENDIANNESS      0

#endif /* __PPCFG_H__ */
