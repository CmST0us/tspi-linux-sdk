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
--  Description : Hardware decoder system configuration
--
--------------------------------------------------------------------------------
--
--
--
------------------------------------------------------------------------------*/

#ifndef __DEC_RK70_CFG_H__
#define __DEC_RK70_CFG_H__

/* predefined values of HW system parameters. DO NOT ALTER! */
#define DEC_RK70_LITTLE_ENDIAN       1
#define DEC_RK70_BIG_ENDIAN          0

#define DEC_RK70_BUS_BURST_LENGTH_UNDEFINED        0
#define DEC_RK70_BUS_BURST_LENGTH_4                4
#define DEC_RK70_BUS_BURST_LENGTH_8                8
#define DEC_RK70_BUS_BURST_LENGTH_16               16

#define DEC_RK70_ASIC_SERVICE_PRIORITY_DEFAULT      0
#define DEC_RK70_ASIC_SERVICE_PRIORITY_WR_1         1
#define DEC_RK70_ASIC_SERVICE_PRIORITY_WR_2         2
#define DEC_RK70_ASIC_SERVICE_PRIORITY_RD_1         3
#define DEC_RK70_ASIC_SERVICE_PRIORITY_RD_2         4

#define DEC_RK70_OUTPUT_FORMAT_RASTER_SCAN          0
#define DEC_RK70_OUTPUT_FORMAT_TILED                1

/* end of predefined values */

/* now what we use */

#ifndef DEC_RK70_USING_IRQ
/* Control IRQ generation by decoder hardware */
#define DEC_RK70_USING_IRQ                1
#endif

#ifndef DEC_RK70_ASIC_SERVICE_PRIORITY
/* hardware intgernal prioriy scheme. better left unchanged */
#define DEC_RK70_ASIC_SERVICE_PRIORITY    DEC_RK70_ASIC_SERVICE_PRIORITY_DEFAULT
#endif

/* AXI single command multiple data disable not set */
#define DEC_RK70_SCMD_DISABLE             0

/* Advanced prefetch disable flag. If disable flag is set, product shall
 * operate akin to 9190 and earlier products. */
#define DEC_RK70_APF_DISABLE              0

#ifndef DEC_RK70_BUS_BURST_LENGTH
/* how long are the hardware data bursts; better left unchanged    */
#define DEC_RK70_BUS_BURST_LENGTH         DEC_RK70_BUS_BURST_LENGTH_16
#endif

#ifndef DEC_RK70_INPUT_STREAM_ENDIAN
/* this should match the system endianess, so that Decoder reads      */
/* the input stream in the right order                                */
#define DEC_RK70_INPUT_STREAM_ENDIAN      DEC_RK70_LITTLE_ENDIAN
#endif

#ifndef DEC_RK70_OUTPUT_PICTURE_ENDIAN
/* this should match the system endianess, so that Decoder writes     */
/* the output pixel data in the right order                           */
#define DEC_RK70_OUTPUT_PICTURE_ENDIAN    DEC_RK70_LITTLE_ENDIAN
#endif

#ifndef DEC_RK70_LATENCY_COMPENSATION
/* compensation for bus latency; values up to 63 */
#define DEC_RK70_LATENCY_COMPENSATION     0
#endif

#ifndef DEC_RK70_INTERNAL_CLOCK_GATING
/* clock is gated from decoder structures that are not used */
#define DEC_RK70_INTERNAL_CLOCK_GATING    1
#endif

#ifndef DEC_RK70_OUTPUT_FORMAT
/* Decoder output picture format in external memory: Raster-scan or     */
/*macroblock tiled i.e. macroblock data written in consecutive addresses */
#define DEC_RK70_OUTPUT_FORMAT            DEC_RK70_OUTPUT_FORMAT_RASTER_SCAN
#endif

#ifndef DEC_RK70_DATA_DISCARD_ENABLE
#define DEC_RK70_DATA_DISCARD_ENABLE      0
#endif

/* Decoder output data swap for 32bit words*/
#ifndef DEC_RK70_OUTPUT_SWAP_32_ENABLE
#define DEC_RK70_OUTPUT_SWAP_32_ENABLE    1
#endif

/* Decoder input data swap(excluding stream data) for 32bit words*/
#ifndef DEC_RK70_INPUT_DATA_SWAP_32_ENABLE
#define DEC_RK70_INPUT_DATA_SWAP_32_ENABLE     1
#endif

/* Decoder input stream swap for 32bit words */
#ifndef DEC_RK70_INPUT_STREAM_SWAP_32_ENABLE
#define DEC_RK70_INPUT_STREAM_SWAP_32_ENABLE   1
#endif

/* Decoder input data endian. Do not modify this! */
#ifndef DEC_RK70_INPUT_DATA_ENDIAN
#define DEC_RK70_INPUT_DATA_ENDIAN        DEC_RK70_BIG_ENDIAN
#endif


/* AXI bus read and write ID values used by HW. 0 - 255 */
#ifndef DEC_RK70_AXI_ID_R
#define DEC_RK70_AXI_ID_R                      0xff
#endif

#ifndef DEC_RK70_AXI_ID_W
#define DEC_RK70_AXI_ID_W                      0
#endif

/* Check validity of values */

/* data discard and tiled mode can not be on simultaneously */
#if (DEC_RK70_DATA_DISCARD_ENABLE && (DEC_RK70_OUTPUT_FORMAT == DEC_RK70_OUTPUT_FORMAT_TILED))
#error "Bad value specified: DEC_RK70_DATA_DISCARD_ENABLE && (DEC_RK70_OUTPUT_FORMAT == DEC_RK70_OUTPUT_FORMAT_TILED)"
#endif

#if (DEC_RK70_OUTPUT_PICTURE_ENDIAN > 1)
#error "Bad value specified for DEC_RK70_OUTPUT_PICTURE_ENDIAN"
#endif

#if (DEC_RK70_OUTPUT_FORMAT > 1)
#error "Bad value specified for DEC_RK70_OUTPUT_FORMAT"
#endif

#if (DEC_RK70_BUS_BURST_LENGTH > 31)
#error "Bad value specified for DEC_RK70_AMBA_BURST_LENGTH"
#endif

#if (DEC_RK70_ASIC_SERVICE_PRIORITY > 4)
#error "Bad value specified for DEC_RK70_ASIC_SERVICE_PRIORITY"
#endif

#if (DEC_RK70_LATENCY_COMPENSATION > 63)
#error "Bad value specified for DEC_RK70_LATENCY_COMPENSATION"
#endif

#if (DEC_RK70_OUTPUT_SWAP_32_ENABLE > 1)
#error "Bad value specified for DEC_RK70_OUTPUT_SWAP_32_ENABLE"
#endif

#if (DEC_RK70_INPUT_DATA_SWAP_32_ENABLE > 1)
#error "Bad value specified for DEC_RK70_INPUT_DATA_SWAP_32_ENABLE"
#endif

#if (DEC_RK70_INPUT_STREAM_SWAP_32_ENABLE > 1)
#error "Bad value specified for DEC_RK70_INPUT_STREAM_SWAP_32_ENABLE"
#endif

#if (DEC_RK70_OUTPUT_SWAP_32_ENABLE > 1)
#error "Bad value specified for DEC_RK70_INPUT_DATA_ENDIAN"
#endif

#if (DEC_RK70_DATA_DISCARD_ENABLE > 1)
#error "Bad value specified for DEC_RK70_DATA_DISCARD_ENABLE"
#endif

/* Common defines for the decoder */

/* Number registers for the decoder */
#define DEC_RK70_REGISTERS          159

/* Max amount of stream */
#define DEC_RK70_MAX_STREAM         ((1<<24)-1)

/* Timeout value for the VPUWaitHwReady() call. */
/* Set to -1 for an unspecified value */
#ifndef DEC_RK70_TIMEOUT_LENGTH
#define DEC_RK70_TIMEOUT_LENGTH     -1
#endif

/* Enable HW internal watchdog timeout IRQ */
#define DEC_RK70_HW_TIMEOUT_INT_ENA  1

/* Memory wait states for reference buffer */
#define DEC_RK70_REFBU_WIDTH        64
#define DEC_RK70_REFBU_LATENCY      20
#define DEC_RK70_REFBU_NONSEQ       8
#define DEC_RK70_REFBU_SEQ          1

#define DEC_SET_APF_THRESHOLD(reghandle) \
    { \
        u32 apfTmpThreshold = 0; \
        rk_SetRegisterFile(reghandle, HWIF_DEC_ADV_PRE_DIS, DEC_RK70_APF_DISABLE ); \
        if((DEC_RK70_APF_DISABLE) == 0) \
        { \
            if(DEC_RK70_REFBU_SEQ) \
                apfTmpThreshold = DEC_RK70_REFBU_NONSEQ/DEC_RK70_REFBU_SEQ; \
            else \
                apfTmpThreshold = DEC_RK70_REFBU_NONSEQ; \
            if( apfTmpThreshold > 63 ) \
                apfTmpThreshold = 63; \
        } \
        rk_SetRegisterFile(reghandle, HWIF_APF_THRESHOLD, apfTmpThreshold ); \
    }



/* Check validity of the stream addresses */

#define RK70_CHECK_BUS_ADDRESS(d)   ((d) < 64 ? 1 : 0)
#define RK70_CHECK_VIRTUAL_ADDRESS(d)   (((void*)(d) < (void*)64) ? 1 : 0)

#endif /* __DEC_RK70_CFG_H__ */
