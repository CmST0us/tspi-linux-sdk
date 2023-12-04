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
--  Description : JPEG decoder API source file
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
        - JpegDecInit
        - JpegDecRelease
        - JpegDecGetImageInfo
        - JpegDecDecode
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "dwl.h"
#include "vpu_type.h"
#include "jpegdecapi.h"
#include "jpegdeccontainer.h"
#include "jpegdecmarkers.h"
#include "jpegdecinternal.h"
#include "jpegdecutils.h"
#include "jpegdechdrs.h"
#include "jpegdecscan.h"
#include "jpegregdrv.h"

#include "vpu.h"
#define LOG_TAG "JPEG_DEC"
#include "allocator_drm.h"

#define ALOGV printf
#define ALOGD printf
#define ALOGI printf
#define ALOGW printf
#define ALOGE printf


/*------------------------------------------------------------------------------
       Version Information - DO NOT CHANGE!
------------------------------------------------------------------------------*/

#define JPG_MAJOR_VERSION 1
#define JPG_MINOR_VERSION 1

#define JPG_BUILD_MAJOR 1
#define JPG_BUILD_MINOR 149
#define JPG_SW_BUILD ((JPG_BUILD_MAJOR * 1000) + JPG_BUILD_MINOR)

/*------------------------------------------------------------------------------
    2. External compiler flags
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/
#define JPEGDEC_API_TRC printf

#define JPEGDEC_CLEAR_IRQ  rk_SetRegisterFile(PTR_JPGC->reghandle, \
                                          HWIF_DEC_IRQ_STAT, 0); \
                           rk_SetRegisterFile(PTR_JPGC->reghandle, \
                                          HWIF_DEC_IRQ, 0);

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    5. Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

    Function name: JpegDecInit

        Functional description:
            Init jpeg decoder

        Inputs:
            JpegDecInst * decInst     a reference to the jpeg decoder instance is
                                         stored here

        Outputs:
            JPEGDEC_OK
            JPEGDEC_INITFAIL
            JPEGDEC_PARAM_ERROR
            JPEGDEC_DWL_ERROR
            JPEGDEC_MEMFAIL

------------------------------------------------------------------------------*/
JpegDecRet JpegDecInit(JpegDecInst * pDecInst, RK_U32 color_conv)
{
    JpegDecContainer *pJpegCont;
    int socket;;
    RK_U32 i = 0;
    RK_U32 asicID;
    RK_U32 fuseStatus = 0;
    RK_U32 extensionsSupported;

    DWLInitParam_t dwlInit;

    //JPEGDEC_API_TRC("JpegDecInit#\n");

    /* check that right shift on negative numbers is performed signed */
    /*lint -save -e* following check causes multiple lint messages */
    if (((-1) >> 1) != (-1)) {
        printf("JpegDecInit# ERROR: Right shift is not signed\n");
        return (JPEGDEC_INITFAIL);
    }

    /*lint -restore */
    if (pDecInst == NULL) {
        printf("JpegDecInit# ERROR: decInst == NULL \n");
        return (JPEGDEC_PARAM_ERROR);
    }
    *pDecInst = NULL;   /* return NULL instance for any error */
    //JPEGDEC_API_TRC("Ready to VPUClientInit#\n");
    socket = VPUClientInit(color_conv ? VPU_DEC_PP : VPU_DEC);
    //ALOGD("VPUClientInit has done.the socket number is %d\n",socket);
    if (socket <= 0) {
        printf("JpegDecInit# ERROR: DWL Init failed,socket = %d\n", socket);
        return (JPEGDEC_DWL_ERROR);
    }

    /* check for proper hardware */
    asicID = 0x91900000;

    {
        /* check that JPEG decoding supported in HW */
        VPUHwDecConfig_t hwCfg;

        memset(&hwCfg, 0, sizeof(VPUHwDecConfig_t));
        hwCfg.jpegESupport = 1;
        hwCfg.jpegSupport = 1;

        //VPUGetDecHwCfg(&hwCfg);
        if (!hwCfg.jpegSupport) {
            JPEGDEC_API_TRC(("JpegDecInit# ERROR: JPEG not supported in HW\n"));
            (void) VPUClientRelease(socket);;
            return JPEGDEC_FORMAT_NOT_SUPPORTED;
        }

        /* check progressive support */
        if ((asicID >> 16) != 0x6731U) {
            /* progressive decoder */
            if (hwCfg.jpegSupport == JPEG_BASELINE)
                fuseStatus = 1;
        }

        extensionsSupported = hwCfg.jpegESupport;

    }

    dwlInit.clientType = DWL_CLIENT_TYPE_JPEG_DEC;

    /* Initialize Wrapper */


    pJpegCont = (JpegDecContainer *) DWLmalloc(sizeof(JpegDecContainer));
    if (pJpegCont == NULL) {
        (void) VPUClientRelease(socket);
        return (JPEGDEC_MEMFAIL);
    }
    memset(pJpegCont, 0, sizeof(JpegDecContainer));
    pJpegCont->socket = socket;
    pJpegCont->vpumem_ctx = NULL;

    pJpegCont->reghandle = get_rkdecregister_handle();

    if ( pJpegCont->reghandle == NULL) {
		printf("pJpegCont->reghandle = NULL, VPUClientRelease \n");
        (void) VPUClientRelease(socket);
        return (JPEGDEC_MEMFAIL);
    }
    set_rkdecregister_map(pJpegCont->reghandle, (RK_U32*)pJpegCont->jpegRegs);

    /* reset internal structures */
    JpegDecClearStructs(pJpegCont);
    /* Reset shadow registers */
    for (i = 1; i < DEC_RK70_REGISTERS; i++) {
        pJpegCont->jpegRegs[i] = 0;
    }
    memset(&pJpegCont->ioctl_info, 0, sizeof(JpegdIocRegInfo));
    pJpegCont->ioctl_info.extra_info.magic = EXTRA_INFO_MAGIC;
    pJpegCont->ioctl_info.extra_info.cnt = 1; /* pp_chroma_out and chroma_out */

    /* these parameters are defined in deccfg.h */
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_OUT_ENDIAN,
                       DEC_RK70_OUTPUT_PICTURE_ENDIAN);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_IN_ENDIAN,
                       DEC_RK70_INPUT_DATA_ENDIAN);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_STRENDIAN_E,
                       DEC_RK70_INPUT_STREAM_ENDIAN);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_MAX_BURST,
                       DEC_RK70_BUS_BURST_LENGTH);
    if ((asicID >> 16) == 0x6731U) {
        rk_SetRegisterFile(pJpegCont->reghandle, HWIF_PRIORITY_MODE,
                           DEC_RK70_ASIC_SERVICE_PRIORITY);
    } else {
        rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_SCMD_DIS,
                           DEC_RK70_SCMD_DISABLE);
    }
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_LATENCY,
                       DEC_RK70_LATENCY_COMPENSATION);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_DATA_DISC_E,
                       DEC_RK70_DATA_DISCARD_ENABLE);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_OUTSWAP32_E,
                       DEC_RK70_OUTPUT_SWAP_32_ENABLE);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_INSWAP32_E,
                       DEC_RK70_INPUT_DATA_SWAP_32_ENABLE);
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_STRSWAP32_E,
                       DEC_RK70_INPUT_STREAM_SWAP_32_ENABLE);

    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_OUT_TILED_E, 0);

    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_TIMEOUT_E, 1);

    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_CLK_GATE_E, 1);

    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_IRQ_DIS, 0);

    /* set AXI RW IDs */
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_AXI_RD_ID,
                       (DEC_RK70_AXI_ID_R & 0xFFU));
    rk_SetRegisterFile(pJpegCont->reghandle, HWIF_DEC_AXI_WR_ID,
                       (DEC_RK70_AXI_ID_W & 0xFFU));

    /* save HW version so we dont need to check it all
     * the time when deciding the control stuff */
    pJpegCont->is8190 = (asicID >> 16) != 0x6731U ? 1 : 0;
    /* set HW related config's */
    if (pJpegCont->is8190) {
        pJpegCont->fuseBurned = fuseStatus;
        /* max */
        pJpegCont->maxSupportedWidth = JPEGDEC_MAX_WIDTH_8190;
        pJpegCont->maxSupportedHeight = JPEGDEC_MAX_HEIGHT_8190;
        pJpegCont->maxSupportedPixelAmount = JPEGDEC_MAX_PIXEL_AMOUNT_8190;
        pJpegCont->maxSupportedSliceSize = JPEGDEC_MAX_SLICE_SIZE_8190;
    } else {
        /* max */
        pJpegCont->maxSupportedWidth = JPEGDEC_MAX_WIDTH;
        pJpegCont->maxSupportedHeight = JPEGDEC_MAX_HEIGHT;
        pJpegCont->maxSupportedPixelAmount = JPEGDEC_MAX_PIXEL_AMOUNT;
        pJpegCont->maxSupportedSliceSize = JPEGDEC_MAX_SLICE_SIZE;
    }

    /* min */
    pJpegCont->minSupportedWidth = JPEGDEC_MIN_WIDTH;
    pJpegCont->minSupportedHeight = JPEGDEC_MIN_HEIGHT;

    pJpegCont->extensionsSupported = extensionsSupported;

    *pDecInst = (JpegDecContainer *) pJpegCont;

    printf("JpegDecInit# OK\n");
    return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

    Function name: JpegDecRelease

        Functional description:
            Release Jpeg decoder

        Inputs:
            JpegDecInst decInst    jpeg decoder instance

            void

------------------------------------------------------------------------------*/
void JpegDecRelease(JpegDecInst decInst)
{
    JpegDecContainer *PTR_JPGC = ((JpegDecContainer *) decInst);

    int socket;

    //JPEGDEC_API_TRC("JpegDecRelease#");

    if (PTR_JPGC == NULL) {
        JPEGDEC_API_TRC("JpegDecRelease# ERROR: decInst == NULL");
        return;
    }

    socket = PTR_JPGC->socket;
    if (PTR_JPGC->vlc.acTable0.vals) {
        DWLfree(PTR_JPGC->vlc.acTable0.vals);
    }
    if (PTR_JPGC->vlc.acTable1.vals) {
        DWLfree(PTR_JPGC->vlc.acTable1.vals);
    }
    if (PTR_JPGC->vlc.acTable2.vals) {
        DWLfree(PTR_JPGC->vlc.acTable2.vals);
    }
    if (PTR_JPGC->vlc.acTable3.vals) {
        DWLfree(PTR_JPGC->vlc.acTable3.vals);
    }
    if (PTR_JPGC->vlc.dcTable0.vals) {
        DWLfree(PTR_JPGC->vlc.dcTable0.vals);
    }
    if (PTR_JPGC->vlc.dcTable1.vals) {
        DWLfree(PTR_JPGC->vlc.dcTable1.vals);
    }
    if (PTR_JPGC->vlc.dcTable2.vals) {
        DWLfree(PTR_JPGC->vlc.dcTable2.vals);
    }
    if (PTR_JPGC->vlc.dcTable3.vals) {
        DWLfree(PTR_JPGC->vlc.dcTable3.vals);
    }
    if (PTR_JPGC->frame.pBuffer) {
        DWLfree(PTR_JPGC->frame.pBuffer);
    }
    /* progressive */
    if (PTR_JPGC->info.pCoeffBase.vir_addr) {
#ifdef DRM_LINUX
        if (allocator_drm.free)
        	allocator_drm.free(PTR_JPGC->ctx, &(PTR_JPGC->info.pCoeffBase));
#else
		VPUFreeLinear(&(PTR_JPGC->info.pCoeffBase));
#endif
        PTR_JPGC->info.pCoeffBase.vir_addr = NULL;
    }
    if (PTR_JPGC->info.tmpStrm.vir_addr) {
        //VPUFreeLinear(&(PTR_JPGC->info.tmpStrm));
		if (allocator_drm.free)
        	allocator_drm.free(PTR_JPGC->ctx, &(PTR_JPGC->info.tmpStrm));
        PTR_JPGC->info.tmpStrm.vir_addr = NULL;
    }
    if (PTR_JPGC->frame.pTableBase.vir_addr) {
#ifdef DRM_LINUX
		if (allocator_drm.free)
        	allocator_drm.free(PTR_JPGC->ctx, &(PTR_JPGC->frame.pTableBase));
#else
		VPUFreeLinear(&(PTR_JPGC->frame.pTableBase));
#endif
        PTR_JPGC->frame.pTableBase.vir_addr = NULL;
    }
    /* if not user allocated memories */
    if (!PTR_JPGC->info.userAllocMem) {
        if (PTR_JPGC->asicBuff.outLumaBuffer.vir_addr != NULL) {
#ifdef DRM_LINUX
			if (allocator_drm.free)
				allocator_drm.free(PTR_JPGC->ctx, &(PTR_JPGC->asicBuff.outLumaBuffer));
#else
			VPUFreeLinear( &(PTR_JPGC->asicBuff.outLumaBuffer));
#endif
            PTR_JPGC->asicBuff.outLumaBuffer.vir_addr = NULL;
        }
        /*if(PTR_JPGC->asicBuff.outChromaBuffer.vir_addr != NULL)
        {
            VPUFreeLinear(&(PTR_JPGC->asicBuff.outChromaBuffer));
            PTR_JPGC->asicBuff.outChromaBuffer.vir_addr = NULL;
        }
        if(PTR_JPGC->asicBuff.outChromaBuffer2.vir_addr != NULL)
        {
            VPUFreeLinear(&(PTR_JPGC->asicBuff.outChromaBuffer2));
            PTR_JPGC->asicBuff.outChromaBuffer2.vir_addr = NULL;
        }*/
    } else {
        PTR_JPGC->asicBuff.outLumaBuffer.vir_addr = NULL;
        PTR_JPGC->asicBuff.outChromaBuffer.vir_addr = NULL;
    }

    if (PTR_JPGC->reghandle != NULL) {
        release_rkdecregister_handle(PTR_JPGC->reghandle);
        PTR_JPGC->reghandle = NULL;
    }

    if (decInst) {
        DWLfree(PTR_JPGC);
    }
    VPUClientRelease(socket);

    JPEGDEC_API_TRC("JpegDecRelease# OK\n");

    return;
}

/*------------------------------------------------------------------------------

    Function name: JpegDecGetImageInfo

        Functional description:
            Get image information of the JFIF

        Inputs:
            JpegDecInst decInst     jpeg decoder instance
            JpegDecInput *pDecIn    input stream information
            JpegDecImageInfo *pImageInfo
                    structure where the image info is written

        Outputs:
            JPEGDEC_OK
            JPEGDEC_ERROR
            JPEGDEC_UNSUPPORTED
            JPEGDEC_PARAM_ERROR
            JPEGDEC_INCREASE_BUFFER
            JPEGDEC_INVALID_STREAM_LENGTH
            JPEGDEC_INVALID_INPUT_BUFFER_SIZE

------------------------------------------------------------------------------*/

/* Get image information of the JFIF */
JpegDecRet JpegDecGetImageInfo(JpegDecInst decInst, JpegDecInput * pDecIn,
                               JpegDecImageInfo * pImageInfo)
{
    JpegDecContainer *PTR_JPGC = ((JpegDecContainer *) decInst);

    RK_U32 Nf = 0;
    RK_U32 Ns = 0;
    RK_U32 NsThumb = 0;
    RK_U32 i, j = 0;
    RK_U32 init = 0;
    RK_U32 initThumb = 0;
    RK_U32 H[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 V[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 Htn[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 Vtn[MAX_NUMBER_OF_COMPONENTS];
    RK_U32 Hmax = 0;
    RK_U32 Vmax = 0;
    RK_U32 headerLength = 0;
    RK_U32 currentByte = 0;
    RK_U32 currentBytes = 0;
    RK_U32 appLength = 0;
    RK_U32 appBits = 0;
    RK_U32 thumbnail = 0;
    RK_U32 errorCode = 0;

#ifdef JPEGDEC_ERROR_RESILIENCE
    RK_U32 errorResilience = 0;
    RK_U32 errorResilienceThumb = 0;
#endif /* JPEGDEC_ERROR_RESILIENCE */

    StreamStorage stream;

    //JPEGDEC_API_TRC("JpegDecGetImageInfo check param\n");

    /* check pointers & parameters */
    if (decInst == NULL || pDecIn == NULL || pImageInfo == NULL ||
        RK70_CHECK_VIRTUAL_ADDRESS(pDecIn->streamBuffer.pVirtualAddress)/* ||
       RK70_CHECK_BUS_ADDRESS(pDecIn->streamBuffer.busAddress)*/) {
        JPEGDEC_API_TRC("JpegDecGetImageInfo JPEGDEC_PARAM_ERROR\n");
        return (JPEGDEC_PARAM_ERROR);
    }

    /* Check the stream lenth */
    if (pDecIn->streamLength < 1) {
        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pDecIn->streamLength\n");
        return (JPEGDEC_INVALID_STREAM_LENGTH);
    }

    /* Check the stream lenth */
    if ((pDecIn->streamLength > DEC_RK70_MAX_STREAM) &&
        (pDecIn->bufferSize < JPEGDEC_RK70_MIN_BUFFER ||
         pDecIn->bufferSize > JPEGDEC_RK70_MAX_BUFFER)) {
        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pDecIn->bufferSize = %d,pDecIn->streamLength = %d\n",
                        pDecIn->bufferSize, pDecIn->streamLength);
        return (JPEGDEC_INVALID_INPUT_BUFFER_SIZE);
    }

    /* Check the stream buffer size */
    if (pDecIn->bufferSize && (pDecIn->bufferSize < JPEGDEC_RK70_MIN_BUFFER ||
                               pDecIn->bufferSize > JPEGDEC_RK70_MAX_BUFFER)) {
        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pDecIn->bufferSize = %d\n", pDecIn->bufferSize);
        return (JPEGDEC_INVALID_INPUT_BUFFER_SIZE);
    }

    /* Check the stream buffer size */
    /*if(pDecIn->bufferSize && ((pDecIn->bufferSize % 8) != 0))
    {
        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: JPEGDEC_INVALID_INPUT_BUFFER_SIZE\n");
        return (JPEGDEC_INVALID_INPUT_BUFFER_SIZE);
    }*/

    /* reset sampling factors */
    for (i = 0; i < MAX_NUMBER_OF_COMPONENTS; i++) {
        H[i] = 0;
        V[i] = 0;
        Htn[i] = 0;
        Vtn[i] = 0;
    }

    /* imageInfo initialization */
    pImageInfo->displayWidth = 0;
    pImageInfo->displayHeight = 0;
    pImageInfo->outputWidth = 0;
    pImageInfo->outputHeight = 0;
    pImageInfo->version = 0;
    pImageInfo->units = 0;
    pImageInfo->xDensity = 0;
    pImageInfo->yDensity = 0;
    pImageInfo->outputFormat = 0;

    /* Default value to "Thumbnail" */
    pImageInfo->thumbnailType = JPEGDEC_NO_THUMBNAIL;
    pImageInfo->displayWidthThumb = 0;
    pImageInfo->displayHeightThumb = 0;
    pImageInfo->outputWidthThumb = 0;
    pImageInfo->outputHeightThumb = 0;
    pImageInfo->outputFormatThumb = 0;

    /* utils initialization */
    stream.bitPosInByte = 0;
    stream.pCurrPos = (RK_U8 *) pDecIn->streamBuffer.pVirtualAddress;
    stream.pStartOfStream = (RK_U8 *) pDecIn->streamBuffer.pVirtualAddress;
    stream.readBits = 0;
    stream.appnFlag = 0;

    /* stream length */
    if (!pDecIn->bufferSize)
        stream.streamLength = pDecIn->streamLength;
    else
        stream.streamLength = pDecIn->bufferSize;
    //JPEGDEC_API_TRC("JpegDecGetImageInfo start get marker\n");
    /* Read decoding parameters */
    for (stream.readBits = 0; (stream.readBits / 8) < stream.streamLength;
         stream.readBits++) {
        /* Look for marker prefix byte from stream */
        if ((currentByte == 0xFF) || JpegDecGetByte(&(stream)) == 0xFF) {
            currentByte = JpegDecGetByte(&(stream));

            /* switch to certain header decoding */
            switch (currentByte) {
                /* baseline marker */
            case SOF0:
                /* progresive marker */
            case SOF2:
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get SOF2\n");
                if (currentByte == SOF0)
                    pImageInfo->codingMode = PTR_JPGC->info.operationType =
                                                 JPEGDEC_BASELINE;
                else
                    pImageInfo->codingMode = PTR_JPGC->info.operationType =
                                                 JPEGDEC_PROGRESSIVE;
                /* Frame header */
                i++;
                Hmax = 0;
                Vmax = 0;

                /* SOF0/SOF2 length */
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }

                /* Sample precision (only 8 bits/sample supported) */
                currentByte = JpegDecGetByte(&(stream));
                if (currentByte != 8) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Sample precision");
                    return (JPEGDEC_UNSUPPORTED);
                }

                /* Number of Lines */
                pImageInfo->outputHeight = JpegDecGet2Bytes(&(stream));
                pImageInfo->displayHeight = pImageInfo->outputHeight;
                if (pImageInfo->outputHeight < 1) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pImageInfo->outputHeight Unsupported");
                    return (JPEGDEC_UNSUPPORTED);
                }

#ifdef JPEGDEC_ERROR_RESILIENCE
                if ((pImageInfo->outputHeight & 0xF) &&
                    (pImageInfo->outputHeight & 0xF) <= 8)
                    errorResilience = 1;
#endif /* JPEGDEC_ERROR_RESILIENCE */

                /* round up to next multiple-of-16 */
                pImageInfo->outputHeight += 0xf;
                pImageInfo->outputHeight &= ~(0xf);
                PTR_JPGC->frame.hwY = pImageInfo->outputHeight;

                /* Number of Samples per Line */
                pImageInfo->outputWidth = JpegDecGet2Bytes(&(stream));
                pImageInfo->displayWidth = pImageInfo->outputWidth;
                if (pImageInfo->outputWidth < 1) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pImageInfo->outputWidth unsupported");
                    return (JPEGDEC_UNSUPPORTED);
                }
                pImageInfo->outputWidth += 0xf;
                pImageInfo->outputWidth &= ~(0xf);
                PTR_JPGC->frame.hwX = pImageInfo->outputWidth;

                /* check for minimum and maximum dimensions */
                if (pImageInfo->outputWidth < PTR_JPGC->minSupportedWidth ||
                    pImageInfo->outputHeight < PTR_JPGC->minSupportedHeight ||
                    pImageInfo->outputWidth > PTR_JPGC->maxSupportedWidth ||
                    pImageInfo->outputHeight > PTR_JPGC->maxSupportedHeight ||
                    (pImageInfo->outputWidth * pImageInfo->outputHeight) >
                    PTR_JPGC->maxSupportedPixelAmount) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Unsupported size");
                    return (JPEGDEC_UNSUPPORTED);
                }

                /* Number of Image Components per Frame */
                Nf = JpegDecGetByte(&(stream));
                if (Nf != 3 && Nf != 1) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Number of Image Components per Frame");
                    return (JPEGDEC_UNSUPPORTED);
                }
                for (j = 0; j < Nf; j++) {
                    /* jump over component identifier */
                    if (JpegDecFlushBits(&(stream), 8) == STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }

                    /* Horizontal sampling factor */
                    currentByte = JpegDecGetByte(&(stream));
                    H[j] = (currentByte >> 4);

                    /* Vertical sampling factor */
                    V[j] = (currentByte & 0xF);

                    /* jump over Tq */
                    if (JpegDecFlushBits(&(stream), 8) == STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }

                    if (H[j] > Hmax)
                        Hmax = H[j];
                    if (V[j] > Vmax)
                        Vmax = V[j];
                }
                if (Hmax == 0 || Vmax == 0) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Hmax == 0 || Vmax == 0");
                    return (JPEGDEC_UNSUPPORTED);
                }
#ifdef JPEGDEC_ERROR_RESILIENCE
                if (H[0] == 2 && V[0] == 2 &&
                    H[1] == 1 && V[1] == 1 && H[2] == 1 && V[2] == 1) {
                    pImageInfo->outputFormat = JPEGDEC_YCbCr420_SEMIPLANAR;
                } else {
                    /* check if fill needed */
                    if (errorResilience) {
                        pImageInfo->outputHeight -= 16;
                        pImageInfo->displayHeight = pImageInfo->outputHeight;
                    }
                }
#endif /* JPEGDEC_ERROR_RESILIENCE */

                /* check format */
                if (H[0] == 2 && V[0] == 2 &&
                    H[1] == 1 && V[1] == 1 && H[2] == 1 && V[2] == 1) {
                    pImageInfo->outputFormat = JPEGDEC_YCbCr420_SEMIPLANAR;
                    PTR_JPGC->frame.numMcuInRow = (PTR_JPGC->frame.hwX / 16);
                    PTR_JPGC->frame.numMcuInFrame = ((PTR_JPGC->frame.hwX *
                                                      PTR_JPGC->frame.hwY) /
                                                     256);
                } else if (H[0] == 2 && V[0] == 1 &&
                           H[1] == 1 && V[1] == 1 && H[2] == 1 && V[2] == 1) {
                    pImageInfo->outputFormat = JPEGDEC_YCbCr422_SEMIPLANAR;
                    PTR_JPGC->frame.numMcuInRow = (PTR_JPGC->frame.hwX / 16);
                    PTR_JPGC->frame.numMcuInFrame = ((PTR_JPGC->frame.hwX *
                                                      PTR_JPGC->frame.hwY) /
                                                     128);
                } else if (H[0] == 1 && V[0] == 2 &&
                           H[1] == 1 && V[1] == 1 && H[2] == 1 && V[2] == 1) {
                    pImageInfo->outputFormat = JPEGDEC_YCbCr440;
                    PTR_JPGC->frame.numMcuInRow = (PTR_JPGC->frame.hwX / 8);
                    PTR_JPGC->frame.numMcuInFrame = ((PTR_JPGC->frame.hwX *
                                                      PTR_JPGC->frame.hwY) /
                                                     128);
                } else if (H[0] == 1 && V[0] == 1 &&
                           H[1] == 0 && V[1] == 0 && H[2] == 0 && V[2] == 0) {
                    pImageInfo->outputFormat = JPEGDEC_YCbCr400;
                    PTR_JPGC->frame.numMcuInRow = (PTR_JPGC->frame.hwX / 8);
                    PTR_JPGC->frame.numMcuInFrame = ((PTR_JPGC->frame.hwX *
                                                      PTR_JPGC->frame.hwY) /
                                                     64);
                } else if (PTR_JPGC->extensionsSupported &&
                           H[0] == 4 && V[0] == 1 &&
                           H[1] == 1 && V[1] == 1 && H[2] == 1 && V[2] == 1) {
                    /* YUV411 output has to be 32 pixel multiple */
                    if (pImageInfo->outputWidth & 0x1F) {
                        pImageInfo->outputWidth += 16;
                        PTR_JPGC->frame.hwX = pImageInfo->outputWidth;
                    }

                    /* check for maximum dimensions */
                    if (pImageInfo->outputWidth > PTR_JPGC->maxSupportedWidth ||
                        (pImageInfo->outputWidth * pImageInfo->outputHeight) >
                        PTR_JPGC->maxSupportedPixelAmount) {
                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Unsupported size");
                        return (JPEGDEC_UNSUPPORTED);
                    }

                    pImageInfo->outputFormat = JPEGDEC_YCbCr411_SEMIPLANAR;
                    PTR_JPGC->frame.numMcuInRow = (PTR_JPGC->frame.hwX / 32);
                    PTR_JPGC->frame.numMcuInFrame = ((PTR_JPGC->frame.hwX *
                                                      PTR_JPGC->frame.hwY) /
                                                     256);
                } else if (PTR_JPGC->extensionsSupported &&
                           H[0] == 1 && V[0] == 1 &&
                           H[1] == 1 && V[1] == 1 && H[2] == 1 && V[2] == 1) {
                    pImageInfo->outputFormat = JPEGDEC_YCbCr444_SEMIPLANAR;
                    PTR_JPGC->frame.numMcuInRow = (PTR_JPGC->frame.hwX / 8);
                    PTR_JPGC->frame.numMcuInFrame = ((PTR_JPGC->frame.hwX *
                                                      PTR_JPGC->frame.hwY) /
                                                     64);
                } else {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Unsupported YCbCr format");
                    return (JPEGDEC_UNSUPPORTED);
                }

                /* restore output format */
                PTR_JPGC->info.yCbCrMode = PTR_JPGC->info.getInfoYCbCrMode =
                                               pImageInfo->outputFormat;
                break;
            case SOS:
                /* SOS length */
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get SOS\n");
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }

                /* check if interleaved or non-ibnterleaved */
                Ns = JpegDecGetByte(&(stream));
                if (Ns == MIN_NUMBER_OF_COMPONENTS &&
                    pImageInfo->outputFormat != JPEGDEC_YCbCr400 &&
                    pImageInfo->codingMode == JPEGDEC_BASELINE) {
                    pImageInfo->codingMode = PTR_JPGC->info.operationType =
                                                 JPEGDEC_NONINTERLEAVED;
                }

                /* jump over SOS header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }

                if ((stream.readBits + 8) < (8 * stream.streamLength)) {
                    PTR_JPGC->info.init = 1;
                    init = 1;
                } else {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Needs to increase input buffer");
                    return (JPEGDEC_INCREASE_INPUT_BUFFER);
                }
                break;
            case DQT:
                /* DQT length */
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get DQT\n");
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                /* jump over DQT header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }
                break;
            case DHT:
                /* DHT length */
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get DHT\n");
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                /* jump over DHT header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }
                break;
            case DRI:
                /* DRI length */
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get DRI\n");
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
#if 0
                /* jump over DRI header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }
#endif
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                PTR_JPGC->frame.Ri = headerLength;
                break;
                /* application segments */
            case APP0:
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get APP0\n");
                /* reset */
                appBits = 0;
                appLength = 0;
                stream.appnFlag = 0;

                /* APP0 length */
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                appLength = headerLength;
                if (appLength < 16) {
                    stream.appnFlag = 1;
                    if (JpegDecFlushBits(&(stream), ((appLength * 8) - 16)) ==
                        STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }
                    break;
                }
                appBits += 16;

                /* check identifier */
                currentBytes = JpegDecGet2Bytes(&(stream));
                appBits += 16;
                if (currentBytes != 0x4A46) {
                    stream.appnFlag = 1;
                    if (JpegDecFlushBits(&(stream), ((appLength * 8) - appBits))
                        == STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }
                    break;
                }
                currentBytes = JpegDecGet2Bytes(&(stream));
                appBits += 16;
                if (currentBytes != 0x4946 && currentBytes != 0x5858) {
                    stream.appnFlag = 1;
                    if (JpegDecFlushBits(&(stream), ((appLength * 8) - appBits))
                        == STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }
                    break;
                }

                /* APP0 Extended */
                if (currentBytes == 0x5858) {
                    thumbnail = 1;
                }
                currentByte = JpegDecGetByte(&(stream));
                appBits += 8;
                if (currentByte != 0x00) {
                    stream.appnFlag = 1;
                    if (JpegDecFlushBits(&(stream), ((appLength * 8) - appBits))
                        == STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }
                    stream.appnFlag = 0;
                    break;
                }

                /* APP0 Extended thumb type */
                if (thumbnail) {
                    /* extension code */
                    currentByte = JpegDecGetByte(&(stream));
                    if (currentByte == JPEGDEC_THUMBNAIL_JPEG) {
                        pImageInfo->thumbnailType = JPEGDEC_THUMBNAIL_JPEG;
                        appBits += 8;
                        stream.appnFlag = 1;

                        /* check thumbnail data */
                        Hmax = 0;
                        Vmax = 0;

                        /* Read decoding parameters */
                        for (; (stream.readBits / 8) < stream.streamLength;
                             stream.readBits++) {
                            /* Look for marker prefix byte from stream */
                            appBits += 8;
                            if (JpegDecGetByte(&(stream)) == 0xFF) {
                                /* switch to certain header decoding */
                                appBits += 8;

                                currentByte = JpegDecGetByte(&(stream));
                                switch (currentByte) {
                                    /* baseline marker */
                                case SOF0:
                                    /* progresive marker */
                                case SOF2:
                                    if (currentByte == SOF0)
                                        pImageInfo->codingModeThumb =
                                            PTR_JPGC->info.operationTypeThumb =
                                                JPEGDEC_BASELINE;
                                    else
                                        pImageInfo->codingModeThumb =
                                            PTR_JPGC->info.operationTypeThumb =
                                                JPEGDEC_PROGRESSIVE;
                                    /* Frame header */
                                    i++;

                                    /* jump over Lf field */
                                    if (JpegDecFlushBits(&(stream), 16) ==
                                        STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    appBits += 16;

                                    /* Sample precision (only 8 bits/sample supported) */
                                    currentByte = JpegDecGetByte(&(stream));
                                    appBits += 8;
                                    if (currentByte != 8) {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Thumbnail Sample precision");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }

                                    /* Number of Lines */
                                    pImageInfo->outputHeightThumb =
                                        JpegDecGet2Bytes(&(stream));
                                    appBits += 16;
                                    pImageInfo->displayHeightThumb =
                                        pImageInfo->outputHeightThumb;
                                    if (pImageInfo->outputHeightThumb < 1) {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pImageInfo->outputHeightThumb unsupported");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }
#ifdef JPEGDEC_ERROR_RESILIENCE
                                    if ((pImageInfo->outputHeightThumb & 0xF) &&
                                        (pImageInfo->outputHeightThumb & 0xF) <=
                                        8)
                                        errorResilienceThumb = 1;
#endif /* JPEGDEC_ERROR_RESILIENCE */

                                    /* round up to next multiple-of-16 */
                                    pImageInfo->outputHeightThumb += 0xf;
                                    pImageInfo->outputHeightThumb &= ~(0xf);

                                    /* Number of Samples per Line */
                                    pImageInfo->outputWidthThumb =
                                        JpegDecGet2Bytes(&(stream));
                                    appBits += 16;
                                    pImageInfo->displayWidthThumb =
                                        pImageInfo->outputWidthThumb;
                                    if (pImageInfo->outputWidthThumb < 1) {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: pImageInfo->outputWidthThumb unsupported");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }
                                    pImageInfo->outputWidthThumb += 0xf;
                                    pImageInfo->outputWidthThumb &= ~(0xf);
                                    if (pImageInfo->outputWidthThumb <
                                        PTR_JPGC->minSupportedWidth ||
                                        pImageInfo->outputHeightThumb <
                                        PTR_JPGC->minSupportedHeight ||
                                        pImageInfo->outputWidthThumb >
                                        JPEGDEC_MAX_WIDTH_TN ||
                                        pImageInfo->outputHeightThumb >
                                        JPEGDEC_MAX_HEIGHT_TN) {

                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Thumbnail Unsupported size");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }

                                    /* Number of Image Components per Frame */
                                    Nf = JpegDecGetByte(&(stream));
                                    appBits += 8;
                                    if (Nf != 3 && Nf != 1) {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Thumbnail Number of Image Components per Frame");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }
                                    for (j = 0; j < Nf; j++) {

                                        /* jump over component identifier */
                                        if (JpegDecFlushBits(&(stream), 8) ==
                                            STRM_ERROR) {
                                            errorCode = 1;
                                            break;
                                        }
                                        appBits += 8;

                                        /* Horizontal sampling factor */
                                        currentByte = JpegDecGetByte(&(stream));
                                        appBits += 8;
                                        Htn[j] = (currentByte >> 4);

                                        /* Vertical sampling factor */
                                        Vtn[j] = (currentByte & 0xF);

                                        /* jump over Tq */
                                        if (JpegDecFlushBits(&(stream), 8) ==
                                            STRM_ERROR) {
                                            errorCode = 1;
                                            break;
                                        }
                                        appBits += 8;

                                        if (Htn[j] > Hmax)
                                            Hmax = Htn[j];
                                        if (Vtn[j] > Vmax)
                                            Vmax = Vtn[j];
                                    }
                                    if (Hmax == 0 || Vmax == 0) {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Thumbnail Hmax == 0 || Vmax == 0");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }
#ifdef JPEGDEC_ERROR_RESILIENCE
                                    if (Htn[0] == 2 && Vtn[0] == 2 &&
                                        Htn[1] == 1 && Vtn[1] == 1 &&
                                        Htn[2] == 1 && Vtn[2] == 1) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr420_SEMIPLANAR;
                                    } else {
                                        /* check if fill needed */
                                        if (errorResilienceThumb) {
                                            pImageInfo->outputHeightThumb -= 16;
                                            pImageInfo->displayHeightThumb =
                                                pImageInfo->outputHeightThumb;
                                        }
                                    }
#endif /* JPEGDEC_ERROR_RESILIENCE */

                                    /* check format */
                                    if (Htn[0] == 2 && Vtn[0] == 2 &&
                                        Htn[1] == 1 && Vtn[1] == 1 &&
                                        Htn[2] == 1 && Vtn[2] == 1) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr420_SEMIPLANAR;
                                    } else if (Htn[0] == 2 && Vtn[0] == 1 &&
                                               Htn[1] == 1 && Vtn[1] == 1 &&
                                               Htn[2] == 1 && Vtn[2] == 1) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr422_SEMIPLANAR;
                                    } else if (Htn[0] == 1 && Vtn[0] == 2 &&
                                               Htn[1] == 1 && Vtn[1] == 1 &&
                                               Htn[2] == 1 && Vtn[2] == 1) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr440;
                                    } else if (Htn[0] == 1 && Vtn[0] == 1 &&
                                               Htn[1] == 0 && Vtn[1] == 0 &&
                                               Htn[2] == 0 && Vtn[2] == 0) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr400;
                                    } else if (PTR_JPGC->is8190 &&
                                               Htn[0] == 4 && Vtn[0] == 1 &&
                                               Htn[1] == 1 && Vtn[1] == 1 &&
                                               Htn[2] == 1 && Vtn[2] == 1) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr411_SEMIPLANAR;
                                    } else if (PTR_JPGC->is8190 &&
                                               Htn[0] == 1 && Vtn[0] == 1 &&
                                               Htn[1] == 1 && Vtn[1] == 1 &&
                                               Htn[2] == 1 && Vtn[2] == 1) {
                                        pImageInfo->outputFormatThumb =
                                            JPEGDEC_YCbCr444_SEMIPLANAR;
                                    } else {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Thumbnail Unsupported YCbCr format");
                                        return (JPEGDEC_UNSUPPORTED);
                                    }
                                    PTR_JPGC->info.initThumb = 1;
                                    initThumb = 1;
                                    break;
                                case SOS:
                                    /* SOS length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR ||
                                        ((stream.readBits +
                                          ((headerLength * 8) - 16)) >
                                         (8 * stream.streamLength))) {
                                        errorCode = 1;
                                        break;
                                    }

                                    /* check if interleaved or non-ibnterleaved */
                                    NsThumb = JpegDecGetByte(&(stream));
                                    if (NsThumb == MIN_NUMBER_OF_COMPONENTS &&
                                        pImageInfo->outputFormatThumb !=
                                        JPEGDEC_YCbCr400 &&
                                        pImageInfo->codingModeThumb ==
                                        JPEGDEC_BASELINE) {
                                        pImageInfo->codingModeThumb =
                                            PTR_JPGC->info.operationTypeThumb =
                                                JPEGDEC_NONINTERLEAVED;
                                    }

                                    /* jump over SOS header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }

                                    if ((stream.readBits + 8) <
                                        (8 * stream.streamLength)) {
                                        PTR_JPGC->info.init = 1;
                                        init = 1;
                                    } else {
                                        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Needs to increase input buffer");
                                        return (JPEGDEC_INCREASE_INPUT_BUFFER);
                                    }
                                    break;
                                case DQT:
                                    /* DQT length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    /* jump over DQT header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }
                                    appBits += (headerLength * 8);
                                    break;
                                case DHT:
                                    /* DHT length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    /* jump over DHT header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }
                                    appBits += (headerLength * 8);
                                    break;
                                case DRI:
                                    /* DRI length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    /* jump over DRI header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }
                                    appBits += (headerLength * 8);
                                    break;
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
                                    /* APPn length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    /* jump over APPn header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }
                                    appBits += (headerLength * 8);
                                    break;
                                case DNL:
                                    /* DNL length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    /* jump over DNL header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }
                                    appBits += (headerLength * 8);
                                    break;
                                case COM:
                                    /* COM length */
                                    headerLength = JpegDecGet2Bytes(&(stream));
                                    if (headerLength == STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    /* jump over COM header */
                                    if (headerLength != 0) {
                                        stream.readBits +=
                                            ((headerLength * 8) - 16);
                                        stream.pCurrPos +=
                                            (((headerLength * 8) - 16) / 8);
                                    }
                                    appBits += (headerLength * 8);
                                    break;
                                    /* unsupported coding styles */
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
                                    JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR: Unsupported coding styles");
                                    return (JPEGDEC_UNSUPPORTED);
                                default:
                                    break;
                                }
                                if (PTR_JPGC->info.initThumb && initThumb) {
                                    /* flush the rest of thumbnail data */
                                    if (JpegDecFlushBits
                                        (&(stream),
                                         ((appLength * 8) - appBits)) ==
                                        STRM_ERROR) {
                                        errorCode = 1;
                                        break;
                                    }
                                    stream.appnFlag = 0;
                                    break;
                                }
                            } else {
                                if (!PTR_JPGC->info.initThumb &&
                                    pDecIn->bufferSize)
                                    return (JPEGDEC_INCREASE_INPUT_BUFFER);
                                else
                                    return (JPEGDEC_STRM_ERROR);
                            }
                        }
                        break;
                    } else {
                        appBits += 8;
                        pImageInfo->thumbnailType =
                            JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT;
                        stream.appnFlag = 1;
                        if (JpegDecFlushBits
                            (&(stream),
                             ((appLength * 8) - appBits)) == STRM_ERROR) {
                            errorCode = 1;
                            break;
                        }
                        stream.appnFlag = 0;
                        break;
                    }
                } else {
                    /* version */
                    pImageInfo->version = JpegDecGet2Bytes(&(stream));
                    appBits += 16;

                    /* units */
                    currentByte = JpegDecGetByte(&(stream));
                    if (currentByte == 0) {
                        pImageInfo->units = JPEGDEC_NO_UNITS;
                    } else if (currentByte == 1) {
                        pImageInfo->units = JPEGDEC_DOTS_PER_INCH;
                    } else if (currentByte == 2) {
                        pImageInfo->units = JPEGDEC_DOTS_PER_CM;
                    }
                    appBits += 8;

                    /* Xdensity */
                    pImageInfo->xDensity = JpegDecGet2Bytes(&(stream));
                    appBits += 16;

                    /* Ydensity */
                    pImageInfo->yDensity = JpegDecGet2Bytes(&(stream));
                    appBits += 16;

                    /* jump over rest of header data */
                    stream.appnFlag = 1;
                    if (JpegDecFlushBits(&(stream), ((appLength * 8) - appBits))
                        == STRM_ERROR) {
                        errorCode = 1;
                        break;
                    }
                    stream.appnFlag = 0;
                    break;
                }
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
                /* APPn length */
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get other APP\n");
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                /* jump over APPn header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }
                break;
            case DNL:
                /* DNL length */
                //JPEGDEC_API_TRC("JpegDecGetImageInfo get DNL\n");
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                /* jump over DNL header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }
                break;
            case COM:
                headerLength = JpegDecGet2Bytes(&(stream));
                if (headerLength == STRM_ERROR ||
                    ((stream.readBits + ((headerLength * 8) - 16)) >
                     (8 * stream.streamLength))) {
                    errorCode = 1;
                    break;
                }
                /* jump over COM header */
                if (headerLength != 0) {
                    stream.readBits += ((headerLength * 8) - 16);
                    stream.pCurrPos += (((headerLength * 8) - 16) / 8);
                }
                break;
                /* unsupported coding styles */
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
                JPEGDEC_API_TRC("JpegDecGetImageInfo error Unsupported coding styles");
                return (JPEGDEC_UNSUPPORTED);
            default:
                break;
            }
            if (PTR_JPGC->info.init && init)
                break;

            if (errorCode) {
                if (pDecIn->bufferSize) {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo Image info failed!");
                    return (JPEGDEC_INCREASE_INPUT_BUFFER);
                } else {
                    JPEGDEC_API_TRC("JpegDecGetImageInfo Stream error");
                    return (JPEGDEC_STRM_ERROR);
                }
            }
        } else {
            JPEGDEC_API_TRC("JpegDecGetImageInfo Could not get marker");
            if (!PTR_JPGC->info.init)
                return (JPEGDEC_INCREASE_INPUT_BUFFER);
            else
                return (JPEGDEC_STRM_ERROR);
        }
    }
    if (PTR_JPGC->info.init) {
        if (pDecIn->bufferSize)
            PTR_JPGC->info.initBufferSize = pDecIn->bufferSize;

        //JPEGDEC_API_TRC("JpegDecGetImageInfo OK\n");
        return (JPEGDEC_OK);
    } else {
        JPEGDEC_API_TRC("JpegDecGetImageInfo ERROR\n");
        return (JPEGDEC_ERROR);
    }
}

/*------------------------------------------------------------------------------

    Function name: JpegDecDecode

        Functional description:
            Decode JFIF

        Inputs:
            JpegDecInst decInst     jpeg decoder instance
            JpegDecInput *pDecIn    pointer to structure where the decoder
                                    stores input information
            JpegDecOutput *pDecOut  pointer to structure where the decoder
                                    stores output frame information

        Outputs:
            JPEGDEC_FRAME_READY
            JPEGDEC_PARAM_ERROR
            JPEGDEC_INVALID_STREAM_LENGTH
            JPEGDEC_INVALID_INPUT_BUFFER_SIZE
            JPEGDEC_UNSUPPORTED
            JPEGDEC_ERROR
            JPEGDEC_STRM_ERROR
            JPEGDEC_HW_BUS_ERROR
            JPEGDEC_DWL_HW_TIMEOUT
            JPEGDEC_SYSTEM_ERROR
            JPEGDEC_HW_RESERVED
            JPEGDEC_STRM_PROCESSED

  ------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecode(JpegDecInst decInst, JpegDecInput * pDecIn,
                         JpegDecOutput * pDecOut)
{

    JpegDecContainer* PTR_JPGC = ((JpegDecContainer *) decInst);
    FrameInfo *JPG_FRM = &PTR_JPGC->frame;
    DecInfo *JPG_INFO = &PTR_JPGC->info;

    RK_S32 dwlret = -1;
    RK_U32 i = 0;
    RK_U32 currentByte = 0;
    RK_U32 currentBytes = 0;
    RK_U32 appLength = 0;
    RK_U32 appBits = 0;
    RK_U32 asic_status = 0;
    RK_U32 HINTdec = 0;
    RK_U32 asicSliceBit = 0;
    RK_U32 intDec = 0;
    RK_U32 currentPos = 0;
    RK_U32 endOfImage = 0;
    RK_U32 nonInterleavedRdy = 0;
    JpegDecRet infoRet;
    JpegDecRet retCode; /* Returned code container */
    JpegDecImageInfo infoTmp;
    RK_U32 mcuSizeDivider = 0;
    RK_U32 findhufftable = 0;

    /* check null */
    if (decInst == NULL || pDecIn == NULL ||
        RK70_CHECK_VIRTUAL_ADDRESS(pDecIn->streamBuffer.pVirtualAddress) /*||
       RK70_CHECK_BUS_ADDRESS(pDecIn->streamBuffer.busAddress)*/ ||
        pDecOut == NULL) {
        JPEGDEC_API_TRC("JpegDecDecode# ERROR: NULL parameter");
        return (JPEGDEC_PARAM_ERROR);
    }

    /* check image decoding type,full image or thumbnail */
    if (pDecIn->decImageType != 0 && pDecIn->decImageType != 1) {
        JPEGDEC_API_TRC("JpegDecDecode# ERROR: decImageType");
        return (JPEGDEC_PARAM_ERROR);
    }

    /* check user allocated null */
    if ((pDecIn->pictureBufferY.pVirtualAddress == NULL &&
         pDecIn->pictureBufferY.busAddress != 0) ||
        (pDecIn->pictureBufferY.pVirtualAddress != NULL &&
         pDecIn->pictureBufferY.busAddress == 0) ||
        (pDecIn->pictureBufferCbCr.pVirtualAddress == NULL &&
         pDecIn->pictureBufferCbCr.busAddress != 0) ||
        (pDecIn->pictureBufferCbCr.pVirtualAddress != NULL &&
         pDecIn->pictureBufferCbCr.busAddress == 0)) {
        JPEGDEC_API_TRC("JpegDecDecode# ERROR: NULL parameter");
        return (JPEGDEC_PARAM_ERROR);
    }

    /* Check the stream lenth */
    if (pDecIn->streamLength < 1) {
        JPEGDEC_API_TRC("JpegDecDecode# ERROR: pDecIn->streamLength");
        return (JPEGDEC_INVALID_STREAM_LENGTH);
    }

    /* check the input buffer settings ==>
     * checks are discarded for last buffer */
    if (!PTR_JPGC->info.streamEndFlag) {
        /* Check the stream lenth */
        if (!PTR_JPGC->info.inputBufferEmpty &&
            (pDecIn->streamLength > DEC_RK70_MAX_STREAM) &&
            (pDecIn->bufferSize < JPEGDEC_RK70_MIN_BUFFER ||
             pDecIn->bufferSize > JPEGDEC_RK70_MAX_BUFFER)) {
            JPEGDEC_API_TRC("JpegDecDecode# ERROR: pDecIn->bufferSize");
            return (JPEGDEC_INVALID_INPUT_BUFFER_SIZE);
        }

        /* Check the stream buffer size */
        if (!PTR_JPGC->info.inputBufferEmpty &&
            pDecIn->bufferSize && (pDecIn->bufferSize < JPEGDEC_RK70_MIN_BUFFER
                                   || pDecIn->bufferSize >
                                   JPEGDEC_RK70_MAX_BUFFER)) {
            JPEGDEC_API_TRC("JpegDecDecode# ERROR: pDecIn->bufferSize");
            return (JPEGDEC_INVALID_INPUT_BUFFER_SIZE);
        }

        if (!PTR_JPGC->info.inputBufferEmpty &&
            PTR_JPGC->info.init &&
            (pDecIn->bufferSize < PTR_JPGC->info.initBufferSize)) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: Increase input buffer size!\n");
            return (JPEGDEC_INVALID_INPUT_BUFFER_SIZE);
        }
    }

    if (!PTR_JPGC->info.init && !PTR_JPGC->info.SliceReadyForPause &&
        !PTR_JPGC->info.inputBufferEmpty) {
        infoRet = JpegDecGetImageInfo(decInst, pDecIn, &infoTmp);
        if (infoRet != JPEGDEC_OK) {
            JPEGDEC_API_TRC(("JpegDecDecode# ERROR: Image info failed!"));
            return (infoRet);
        }
    }

    /* Store the stream parameters */
    if (PTR_JPGC->info.progressiveScanReady == 0 &&
        PTR_JPGC->info.nonInterleavedScanReady == 0) {
        PTR_JPGC->stream.bitPosInByte = 0;
        PTR_JPGC->stream.pCurrPos = (RK_U8 *) pDecIn->streamBuffer.pVirtualAddress;
        PTR_JPGC->stream.streamBus = pDecIn->streamBuffer.busAddress;
        PTR_JPGC->stream.pStartOfStream =
            (RK_U8 *) pDecIn->streamBuffer.pVirtualAddress;
        PTR_JPGC->stream.readBits = 0;
        PTR_JPGC->stream.streamLength = pDecIn->streamLength;
        PTR_JPGC->stream.appnFlag = 0;
        pDecOut->outputPictureY.pVirtualAddress = NULL;
        pDecOut->outputPictureY.busAddress = 0;
        pDecOut->outputPictureCbCr.pVirtualAddress = NULL;
        pDecOut->outputPictureCbCr.busAddress = 0;
        pDecOut->outputPictureCr.pVirtualAddress = NULL;
        pDecOut->outputPictureCr.busAddress = 0;
    } else {
        PTR_JPGC->image.headerReady = 0;
    }

    /* set mcu/slice value */
    PTR_JPGC->info.sliceMbSetValue = pDecIn->sliceMbSet;

    /* check HW supported features */
    if (!PTR_JPGC->is8190) {
        /* return if not valid HW and unsupported operation type */
        if (PTR_JPGC->info.operationType == JPEGDEC_NONINTERLEAVED ||
            PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: Operation type not supported");
            return (JPEGDEC_UNSUPPORTED);
        }

        if (PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr400 ||
            PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr440 ||
            PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr444_SEMIPLANAR)
            mcuSizeDivider = 2;
        else
            mcuSizeDivider = 1;

        /* check slice config */
        if ((pDecIn->sliceMbSet * (JPG_FRM->numMcuInRow / mcuSizeDivider)) >
            PTR_JPGC->maxSupportedSliceSize) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: sliceMbSet  > JPEGDEC_MAX_SLICE_SIZE");
            return (JPEGDEC_PARAM_ERROR);
        }

        /* check frame size */
        if ((!pDecIn->sliceMbSet) &&
            JPG_FRM->numMcuInFrame > PTR_JPGC->maxSupportedSliceSize) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: mcuInFrame > JPEGDEC_MAX_SLICE_SIZE");
            return (JPEGDEC_PARAM_ERROR);
        }
    } else {
        /* check if fuse was burned */
        if (PTR_JPGC->fuseBurned) {
            /* return if not valid HW and unsupported operation type */
            if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE) {
                JPEGDEC_API_TRC
                ("JpegDecDecode# ERROR: Operation type not supported");
                return (JPEGDEC_UNSUPPORTED);
            }
        }

        /* check slice config */
        if ((pDecIn->sliceMbSet && pDecIn->decImageType == JPEGDEC_IMAGE &&
             PTR_JPGC->info.operationType != JPEGDEC_BASELINE) ||
            (pDecIn->sliceMbSet && pDecIn->decImageType == JPEGDEC_THUMBNAIL &&
             PTR_JPGC->info.operationTypeThumb != JPEGDEC_BASELINE)) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: Slice mode not supported for this operation type");
            return (JPEGDEC_SLICE_MODE_UNSUPPORTED);
        }

        /* check if frame size over 16M */
        if ((!pDecIn->sliceMbSet) &&
            ((JPG_FRM->hwX * JPG_FRM->hwY) > JPEGDEC_MAX_PIXEL_AMOUNT)) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: Resolution > 16M ==> use slice mode!");
            return (JPEGDEC_PARAM_ERROR);
        }

        if (PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr400 ||
            PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr440 ||
            PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr444_SEMIPLANAR)
            mcuSizeDivider = 2;
        else
            mcuSizeDivider = 1;

        /* check slice config */
        if ((pDecIn->sliceMbSet * (JPG_FRM->numMcuInRow / mcuSizeDivider)) >
            PTR_JPGC->maxSupportedSliceSize) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: sliceMbSet  > JPEGDEC_MAX_SLICE_SIZE");
            return (JPEGDEC_PARAM_ERROR);
        }
    }

    /* check slice size */
    if (pDecIn->sliceMbSet &&
        !PTR_JPGC->info.SliceReadyForPause && !PTR_JPGC->info.inputBufferEmpty) {
        if (PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr400 ||
            PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr440 ||
            PTR_JPGC->info.getInfoYCbCrMode == JPEGDEC_YCbCr444_SEMIPLANAR)
            mcuSizeDivider = 2;
        else
            mcuSizeDivider = 1;

        if ((pDecIn->sliceMbSet * (JPG_FRM->numMcuInRow / mcuSizeDivider)) >
            JPG_FRM->numMcuInFrame) {
            JPEGDEC_API_TRC
            ("JpegDecDecode# ERROR: (sliceMbSet * Number of MCU's in row) > Number of MCU's in frame");
            return (JPEGDEC_PARAM_ERROR);
        }
    }

    /* Handle stream/hw parameters after buffer empty */
    if (PTR_JPGC->info.inputBufferEmpty) {
        /* Store the stream parameters */
        PTR_JPGC->stream.bitPosInByte = 0;
        PTR_JPGC->stream.pCurrPos = (RK_U8 *) pDecIn->streamBuffer.pVirtualAddress;
        PTR_JPGC->stream.streamBus = pDecIn->streamBuffer.busAddress;
        PTR_JPGC->stream.pStartOfStream =
            (RK_U8 *) pDecIn->streamBuffer.pVirtualAddress;

        /* update hw parameters */
        PTR_JPGC->info.inputStreaming = 1;
        if (pDecIn->bufferSize)
            PTR_JPGC->info.inputBufferLen = pDecIn->bufferSize;
        else
            PTR_JPGC->info.inputBufferLen = pDecIn->streamLength;

        /* decoded stream */
        PTR_JPGC->info.decodedStreamLen += PTR_JPGC->info.inputBufferLen;
    }

    /* update user allocated output */
    PTR_JPGC->info.givenOutLuma.vir_addr = pDecIn->pictureBufferY.pVirtualAddress;
    PTR_JPGC->info.givenOutLuma.phy_addr = pDecIn->pictureBufferY.busAddress;
    PTR_JPGC->info.givenOutChroma.vir_addr = pDecIn->pictureBufferCbCr.pVirtualAddress;
    PTR_JPGC->info.givenOutChroma.phy_addr = pDecIn->pictureBufferCbCr.busAddress;
    PTR_JPGC->info.givenOutChroma2.vir_addr = pDecIn->pictureBufferCr.pVirtualAddress;
    PTR_JPGC->info.givenOutChroma2.phy_addr = pDecIn->pictureBufferCr.busAddress;

    if (PTR_JPGC->info.progressiveFinish) {
        /* output set */
        if (PTR_JPGC->ppInstance == NULL) {
            pDecOut->outputPictureY.pVirtualAddress =
                PTR_JPGC->info.outLuma.vir_addr;
            ASSERT(pDecOut->outputPictureY.pVirtualAddress);

            /* output set */
            pDecOut->outputPictureY.busAddress =
                PTR_JPGC->info.outLuma.phy_addr;
            ASSERT(pDecOut->outputPictureY.busAddress);

            /* if not grayscale */
            if (PTR_JPGC->image.sizeChroma) {
                pDecOut->outputPictureCbCr.pVirtualAddress =
                    PTR_JPGC->info.outChroma.vir_addr;
                ASSERT(pDecOut->outputPictureCbCr.pVirtualAddress);

                pDecOut->outputPictureCbCr.busAddress =
                    PTR_JPGC->info.outChroma.phy_addr;
                ASSERT(pDecOut->outputPictureCbCr.busAddress);

                pDecOut->outputPictureCr.pVirtualAddress =
                    PTR_JPGC->info.outChroma2.vir_addr;
                pDecOut->outputPictureCr.busAddress =
                    PTR_JPGC->info.outChroma2.phy_addr;
            }
        }
		printf("JpegDecInitHWEmptyScan ******************\n");
        JpegDecInitHWEmptyScan(PTR_JPGC, PTR_JPGC->info.pfCompId);
        {
            VPU_CMD_TYPE cmd;
            RK_S32 len;
            dwlret = VPUClientWaitResult(PTR_JPGC->socket, PTR_JPGC->jpegRegs, DEC_RK70_REGISTERS, &cmd, &len);
            if ((VPU_SUCCESS != dwlret) || (cmd != VPU_SEND_CONFIG_ACK_OK))
                dwlret = DWL_HW_WAIT_ERROR;

        }

        ASSERT(dwlret == DWL_HW_WAIT_OK);
        JpegRefreshRegs(PTR_JPGC);
        asic_status = rk_GetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_IRQ_STAT);
        ASSERT(asic_status == 1);

        i = PTR_JPGC->info.pfCompId + 1;
        while (i < 3 && PTR_JPGC->info.pfNeeded[i] == 0)
            i++;
        if (i == 3) {
            PTR_JPGC->info.progressiveFinish = 0;
            return (JPEGDEC_FRAME_READY);
        } else {
            PTR_JPGC->info.pfCompId = i;
            return (JPEGDEC_SCAN_PROCESSED);
        }
    }
	printf("hw_decode %d******************\n", __LINE__);

    /* check if input streaming used */
    if (!PTR_JPGC->info.SliceReadyForPause &&
        !PTR_JPGC->info.inputBufferEmpty && pDecIn->bufferSize) {
        PTR_JPGC->info.inputStreaming = 1;
        PTR_JPGC->info.inputBufferLen = pDecIn->bufferSize;
        PTR_JPGC->info.decodedStreamLen += PTR_JPGC->info.inputBufferLen;
    }

    /* find markers and go ! */
    do {
        /* if slice mode/slice done return to hw handling */
        if (PTR_JPGC->image.headerReady && PTR_JPGC->info.SliceReadyForPause)
            break;

        /* Look for marker prefix byte from stream */
        if ((currentByte == 0xFF) || (JpegDecGetByte(&(PTR_JPGC->stream)) == 0xFF)) {
            currentByte = JpegDecGetByte(&(PTR_JPGC->stream));

            /* switch to certain header decoding */
            switch (currentByte) {
            case 0x00:
                break;
            case SOF0:
            case SOF2:
                /* Baseline/Progressive */
                PTR_JPGC->frame.codingType = currentByte;
                /* Set operation type */
                if (PTR_JPGC->frame.codingType == SOF0)
                    PTR_JPGC->info.operationType = JPEGDEC_BASELINE;
                else
                    PTR_JPGC->info.operationType = JPEGDEC_PROGRESSIVE;

                retCode = JpegDecDecodeFrameHdr(PTR_JPGC);
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_API_TRC("JpegDecDecode# ERROR: Stream error \n");
                        return (retCode);
                    } else {
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# ERROR: JpegDecDecodeFrameHdr err \n");
                        return (retCode);
                    }
                }
                break;
                /* Start of Scan */
            case SOS:
                /* reset image ready */
                PTR_JPGC->image.imageReady = 0;

                retCode = JpegDecDecodeScan(PTR_JPGC);
                PTR_JPGC->image.headerReady = 1;
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_API_TRC("JpegDecDecode# ERROR: Stream error \n");
                        return (retCode);
                    } else {
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# JpegDecDecodeScan err\n");
                        return (retCode);
                    }
                }

                if (PTR_JPGC->stream.bitPosInByte) {
                    /* delete stuffing bits */
                    currentByte = (8 - PTR_JPGC->stream.bitPosInByte);
                    if (JpegDecFlushBits
                        (&(PTR_JPGC->stream),
                         8 - PTR_JPGC->stream.bitPosInByte) == STRM_ERROR) {
                        JPEGDEC_API_TRC("JpegDecDecode# ERROR: Stream error \n");
                        return (JPEGDEC_STRM_ERROR);
                    }
                }
                JPEGDEC_API_TRC("JpegDecDecode# Stuffing bits deleted\n");
                if (PTR_JPGC->dri_en == 0xffd0) {
                    /*ALOGI("STREAM HAS dri marker!");
                    RK_U8 *dst, *tmp;
                    tmp = PTR_JPGC->stream.pCurrPos;
                    dst = PTR_JPGC->stream.pCurrPos;
                    for(i=0;i<PTR_JPGC->stream.streamLength;i++)
                    {
                        if(tmp[i]==0xff && tmp[i+1]==0x00 && tmp[i+2]==0xff)
                            i += 2;
                        *dst++ = tmp[i];
                    }
                    VPUMemClean(pDecIn->pstreamMem);
                    VPUMemInvalidate(pDecIn->pstreamMem);*/
                }
                break;
                /* Start of Huffman tables */
            case DHT:
                retCode = JpegDecDecodeHuffmanTables(PTR_JPGC);
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_API_TRC("JpegDecDecode# ERROR: Stream error\n");
                        return (retCode);
                    } else {
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# ERROR: JpegDecDecodeHuffmanTables err\n");
                        return (retCode);
                    }
                }
                findhufftable = 1;
                break;
                /* start of Quantisation Tables */
            case DQT:
                retCode = JpegDecDecodeQuantTables(PTR_JPGC);
                if (retCode != JPEGDEC_OK) {
                    if (retCode == JPEGDEC_STRM_ERROR) {
                        JPEGDEC_API_TRC("JpegDecDecode# ERROR: Stream error\n");
                        return (retCode);
                    } else {
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# ERROR: JpegDecDecodeQuantTables err\n");
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
                    JPEGDEC_API_TRC("JpegDecDecode# EOI: OK\n");
                    return (JPEGDEC_FRAME_READY);
                } else {
                    JPEGDEC_API_TRC("JpegDecDecode# ERROR: EOI: NOK\n");
                    return (JPEGDEC_ERROR);
                }
                /* Define Restart Interval */
            case DRI:
                currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                if (currentBytes == STRM_ERROR) {
                    JPEGDEC_API_TRC("JpegDecDecode# ERROR: Read bits\n");
                    return (JPEGDEC_STRM_ERROR);
                }
                PTR_JPGC->frame.Ri = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                PTR_JPGC->dri_en = 0xffd0;  //find dri
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
                return (JPEGDEC_UNSUPPORTED);
                /* application data & comments */
            case APP0:
                //JPEGDEC_API_TRC("JpegDecDecode# APP0 in Decode");
                /* APP0 Extended Thumbnail */
                if (pDecIn->decImageType == JPEGDEC_THUMBNAIL) {
                    /* reset */
                    appBits = 0;
                    appLength = 0;

                    /* length */
                    appLength = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                    appBits += 16;

                    /* check identifier */
                    currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                    appBits += 16;
                    if (currentBytes != 0x4A46) {
                        PTR_JPGC->stream.appnFlag = 1;
                        if (JpegDecFlushBits
                            (&(PTR_JPGC->stream),
                             ((appLength * 8) - appBits)) == STRM_ERROR) {
                            JPEGDEC_API_TRC
                            ("JpegDecDecode# ERROR: Stream error\n");
                            return (JPEGDEC_STRM_ERROR);
                        }
                        PTR_JPGC->stream.appnFlag = 0;
                        break;
                    }
                    currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                    appBits += 16;
                    if (currentBytes != 0x5858) {
                        PTR_JPGC->stream.appnFlag = 1;
                        if (JpegDecFlushBits
                            (&(PTR_JPGC->stream),
                             ((appLength * 8) - appBits)) == STRM_ERROR) {
                            JPEGDEC_API_TRC
                            ("JpegDecDecode# ERROR: Stream error\n");
                            return (JPEGDEC_STRM_ERROR);
                        }
                        PTR_JPGC->stream.appnFlag = 0;
                        break;
                    }
                    currentByte = JpegDecGetByte(&(PTR_JPGC->stream));
                    appBits += 8;
                    if (currentByte != 0x00) {
                        PTR_JPGC->stream.appnFlag = 1;
                        if (JpegDecFlushBits
                            (&(PTR_JPGC->stream),
                             ((appLength * 8) - appBits)) == STRM_ERROR) {
                            JPEGDEC_API_TRC
                            ("JpegDecDecode# ERROR: Stream error");
                            return (JPEGDEC_STRM_ERROR);
                        }
                        PTR_JPGC->stream.appnFlag = 0;
                        break;
                    }
                    /* extension code */
                    currentByte = JpegDecGetByte(&(PTR_JPGC->stream));
                    PTR_JPGC->stream.appnFlag = 0;
                    if (currentByte != JPEGDEC_THUMBNAIL_JPEG) {
                        JPEGDEC_API_TRC(("JpegDecDecode# ERROR: thumbnail unsupported"));
                        return (JPEGDEC_UNSUPPORTED);
                    }
                    /* thumbnail mode */
                    JPEGDEC_API_TRC("JpegDecDecode# Thumbnail data ok!");
                    PTR_JPGC->stream.thumbnail = 1;
                    break;
                } else {
                    /* Flush unsupported thumbnail */
                    currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                    PTR_JPGC->stream.appnFlag = 1;
                    if (JpegDecFlushBits
                        (&(PTR_JPGC->stream),
                         ((currentBytes - 2) * 8)) == STRM_ERROR) {
                        JPEGDEC_API_TRC("JpegDecDecode# ERROR: Stream error");
                        return (JPEGDEC_STRM_ERROR);
                    }
                    PTR_JPGC->stream.appnFlag = 0;
                    break;
                }
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
                currentBytes = JpegDecGet2Bytes(&(PTR_JPGC->stream));
                if (currentBytes == STRM_ERROR) {
                    JPEGDEC_API_TRC("JpegDecDecode# ERROR: Read bits ");
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

    if (!findhufftable) {
        JpegDefaultHuffmanTables(PTR_JPGC);
    }

    retCode = JPEGDEC_OK;
    /* Handle decoded image here */
    if (PTR_JPGC->image.headerReady) {
        /* loop until decoding control should return for user */
        do {		
            if (PTR_JPGC->ppInstance != NULL) {
                PTR_JPGC->ppControl.usePipeline = 1;
            }

            /* check if we had to load imput buffer or not */
            if (!PTR_JPGC->info.inputBufferEmpty) {
                /* if slice mode ==> set slice height */
                if (PTR_JPGC->info.sliceMbSetValue &&
                    PTR_JPGC->ppControl.usePipeline == 0) {
                    
                    JpegDecSliceSizeCalculation(PTR_JPGC);
                    ALOGV(" JpegDecSliceSizeCalculation sliceHeight is %d.PTR_JPGC->info.sliceMbSetValue is %d\n",
                          PTR_JPGC->info.sliceHeight, PTR_JPGC->info.sliceMbSetValue);
                }

                /* Start HW or continue after pause */
                if (!PTR_JPGC->info.SliceReadyForPause) {
                    if (!PTR_JPGC->info.progressiveScanReady ||
                        PTR_JPGC->info.nonInterleavedScanReady) {
                        retCode = JpegDecInitHW(PTR_JPGC);
                        PTR_JPGC->info.nonInterleavedScanReady = 0;
                        if (retCode != JPEGDEC_OK) {
                            /* return JPEGDEC_HW_RESERVED */
                            return retCode;
                        }

                    } else {
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# Continue HW decoding after progressive scan ready\n");
                        JpegDecInitHWProgressiveContinue(PTR_JPGC);
                        PTR_JPGC->info.progressiveScanReady = 0;

                    }
                } else {
                    JPEGDEC_API_TRC
                    ("JpegDecDecode# Continue HW decoding after slice ready\n");
                    JpegDecInitHWContinue(PTR_JPGC);
                }

                PTR_JPGC->info.SliceCount++;
            } else {
                JPEGDEC_API_TRC("JpegDecDecode# Continue HW decoding after input buffer has been loaded\n");
                JpegDecInitHWInputBuffLoad(PTR_JPGC);

                /* buffer loaded ==> reset flag */
                PTR_JPGC->info.inputBufferEmpty = 0;
            }
				
#ifdef JPEGDEC_PERFORMANCE
            dwlret = DWL_HW_WAIT_OK;
#else
            /* wait hw ready */
            {
                VPU_CMD_TYPE cmd;
                RK_S32 len;
                dwlret = VPUClientWaitResult(PTR_JPGC->socket, PTR_JPGC->jpegRegs, DEC_RK70_REGISTERS, &cmd, &len);
				
                if ((VPU_SUCCESS != dwlret) || (cmd != VPU_SEND_CONFIG_ACK_OK))
                    dwlret = DWL_HW_WAIT_ERROR;
            }

            printf("VPUClientWaitResult 0 mjpeg! \n");
            //ALOGE("after reg[1]=0x%x\n",PTR_JPGC->jpegRegs[1]);

            if (0){
                int i;
                for (i = 0; i < 159; i++) {
                    printf("after reg[%d]=0x%08x\n", i, PTR_JPGC->jpegRegs[i]);
                }
            }

#endif /* #ifdef JPEGDEC_PERFORMANCE */

            /* Refresh regs */
            JpegRefreshRegs(PTR_JPGC);

            if (dwlret == DWL_HW_WAIT_OK) {
                /* check && reset status */
                asic_status = rk_GetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_IRQ_STAT);
                //ALOGE("asic_status : 0x%x", asic_status);

                if (asic_status & JPEGDEC_RK70_IRQ_BUS_ERROR) {
                    JPEGDEC_API_TRC
                    ("JpegDecDecode# JPEGDEC_RK70_IRQ_BUS_ERROR");
                    /* clear interrupts */
                    JPEGDEC_CLEAR_IRQ;

                    /* update asicRunning */
                    PTR_JPGC->asicRunning = 0;
                    PTR_JPGC->image.imageReady = 0;
                    PTR_JPGC->image.headerReady = 0;    //clear status for next decoder, add by wjm
                    return JPEGDEC_HW_BUS_ERROR;
                } else if (asic_status & JPEGDEC_RK70_IRQ_STREAM_ERROR ||
                           asic_status & JPEGDEC_RK70_IRQ_TIMEOUT) {
                    if (asic_status & JPEGDEC_RK70_IRQ_STREAM_ERROR)
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# JPEGDEC_RK70_IRQ_STREAM_ERROR \n");
                    else
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# JPEGDEC_RK70_IRQ_TIMEOUT \n");

                    /* clear interrupts */
                    JPEGDEC_CLEAR_IRQ;

                    /* update asicRunning */
                    PTR_JPGC->asicRunning = 0;

                    /* output set */
                    if (PTR_JPGC->ppInstance == NULL) {
                        pDecOut->outputPictureY.pVirtualAddress =
                            PTR_JPGC->info.outLuma.vir_addr;
                        ASSERT(pDecOut->outputPictureY.pVirtualAddress);

                        /* output set */
                        pDecOut->outputPictureY.busAddress =
                            PTR_JPGC->info.outLuma.phy_addr;
                        ASSERT(pDecOut->outputPictureY.busAddress);

                        /* if not grayscale */
                        if (PTR_JPGC->image.sizeChroma) {
                            pDecOut->outputPictureCbCr.pVirtualAddress =
                                PTR_JPGC->info.outChroma.vir_addr;
                            ASSERT(pDecOut->outputPictureCbCr.pVirtualAddress);

                            pDecOut->outputPictureCbCr.busAddress =
                                PTR_JPGC->info.outChroma.phy_addr;
                            ASSERT(pDecOut->outputPictureCbCr.busAddress);

                            pDecOut->outputPictureCr.pVirtualAddress =
                                PTR_JPGC->info.outChroma2.vir_addr;
                            pDecOut->outputPictureCr.busAddress =
                                PTR_JPGC->info.outChroma2.phy_addr;
                        }
                    }

                    PTR_JPGC->image.imageReady = 0;
                    PTR_JPGC->image.headerReady = 0;    //clear status for next decoder, add by wjm

                    if ((asic_status & JPEGDEC_RK70_IRQ_TIMEOUT) &&
                        (PTR_JPGC->frame.Y % 16) &&
                        (PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV422 ||
                         PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV444 ||
                         PTR_JPGC->info.yCbCrMode == JPEGDEC_YUV411)) {
                        return JPEGDEC_FRAME_READY;
                    } else {
                        return JPEGDEC_STRM_ERROR;
                    }
                } else if (asic_status & JPEGDEC_RK70_IRQ_BUFFER_EMPTY) {
                    /* check if frame is ready */
                    if (!(asic_status & JPEGDEC_RK70_IRQ_DEC_RDY)) {
                        JPEGDEC_API_TRC
                        ("JpegDecDecode# JPEGDEC_RK70_IRQ_BUFFER_EMPTY/STREAM PROCESSED");

                        /* clear interrupts */
                        rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_BUFFER_INT,
                                           0);

                        /* flag to load buffer */
                        PTR_JPGC->info.inputBufferEmpty = 1;

                        /* check if all stream should be processed with the
                         * next buffer ==> may affect to some API checks */
                        if ((PTR_JPGC->info.decodedStreamLen +
                             PTR_JPGC->info.inputBufferLen) >=
                            PTR_JPGC->stream.streamLength) {
                            PTR_JPGC->info.streamEndFlag = 1;
                        }

                        /* output set */
                        if (PTR_JPGC->ppInstance == NULL) {
                            pDecOut->outputPictureY.pVirtualAddress =
                                PTR_JPGC->info.outLuma.vir_addr;
                            ASSERT(pDecOut->outputPictureY.pVirtualAddress);

                            /* output set */
                            pDecOut->outputPictureY.busAddress =
                                PTR_JPGC->info.outLuma.phy_addr;
                            ASSERT(pDecOut->outputPictureY.busAddress);

                            /* if not grayscale */
                            if (PTR_JPGC->image.sizeChroma) {
                                pDecOut->outputPictureCbCr.pVirtualAddress =
                                    PTR_JPGC->info.outChroma.vir_addr;
                                ASSERT(pDecOut->outputPictureCbCr.
                                       pVirtualAddress);

                                pDecOut->outputPictureCbCr.busAddress =
                                    PTR_JPGC->info.outChroma.phy_addr;
                                ASSERT(pDecOut->outputPictureCbCr.busAddress);

                                pDecOut->outputPictureCr.pVirtualAddress =
                                    PTR_JPGC->info.outChroma2.vir_addr;
                                pDecOut->outputPictureCr.busAddress =
                                    PTR_JPGC->info.outChroma2.phy_addr;
                            }
                        }

                        return JPEGDEC_STRM_PROCESSED;
                    }
                }

                rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_RDY_INT, 0);
                HINTdec = rk_GetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_IRQ);
                if (HINTdec) {
                    JPEGDEC_API_TRC("JpegDecDecode# CLEAR interrupt");
                    rk_SetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_IRQ, 0);
                }

                /* check if slice ready */
                asicSliceBit = rk_GetRegisterFile(PTR_JPGC->reghandle,
                                                  HWIF_JPEG_SLICE_H);
                intDec = rk_GetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_SLICE_INT);

                /* slice ready ==> reset interrupt */
                if (asicSliceBit && intDec) {
                    /* if RK70 pp not in use */
                    if (PTR_JPGC->ppInstance == NULL)
                        PTR_JPGC->info.SliceReadyForPause = 1;
                    else
                        PTR_JPGC->info.SliceReadyForPause = 0;

                    if (PTR_JPGC->ppInstance != NULL &&
                        !PTR_JPGC->ppControl.usePipeline) {
                        PTR_JPGC->info.SliceReadyForPause = 1;
                    }

                    /* if user allocated memory return given base */
                    if (PTR_JPGC->info.userAllocMem == 1 &&
                        PTR_JPGC->info.SliceReadyForPause == 1) {
                        /* output addresses */
                        pDecOut->outputPictureY.pVirtualAddress =
                            PTR_JPGC->info.givenOutLuma.vir_addr;
                        pDecOut->outputPictureY.busAddress =
                            PTR_JPGC->info.givenOutLuma.phy_addr;
                        if (PTR_JPGC->image.sizeChroma) {
                            pDecOut->outputPictureCbCr.pVirtualAddress =
                                PTR_JPGC->info.givenOutChroma.vir_addr;
                            pDecOut->outputPictureCbCr.busAddress =
                                PTR_JPGC->info.givenOutChroma.phy_addr;
                            pDecOut->outputPictureCr.pVirtualAddress =
                                PTR_JPGC->info.givenOutChroma2.vir_addr;
                            pDecOut->outputPictureCr.busAddress =
                                PTR_JPGC->info.givenOutChroma2.phy_addr;
                        }
                    }

                    /* if not user allocated memory return slice base */
                    if (PTR_JPGC->info.userAllocMem == 0 &&
                        PTR_JPGC->info.SliceReadyForPause == 1) {
                        /* output addresses */
                        pDecOut->outputPictureY.pVirtualAddress =
                            PTR_JPGC->info.outLuma.vir_addr;
                        pDecOut->outputPictureY.busAddress =
                            PTR_JPGC->info.outLuma.phy_addr;
                        if (PTR_JPGC->image.sizeChroma) {
                            pDecOut->outputPictureCbCr.pVirtualAddress =
                                PTR_JPGC->info.outChroma.vir_addr;
                            pDecOut->outputPictureCbCr.busAddress =
                                PTR_JPGC->info.outChroma.phy_addr;
                            pDecOut->outputPictureCr.pVirtualAddress =
                                PTR_JPGC->info.outChroma2.vir_addr;
                            pDecOut->outputPictureCr.busAddress =
                                PTR_JPGC->info.outChroma2.phy_addr;
                        }
                    }

                    /* No slice output in case decoder + PP (no pipeline) */
                    if (PTR_JPGC->ppInstance != NULL &&
                        PTR_JPGC->ppControl.usePipeline == 0) {
                        /* output addresses */
                        pDecOut->outputPictureY.pVirtualAddress = NULL;
                        pDecOut->outputPictureY.busAddress = 0;
                        if (PTR_JPGC->image.sizeChroma) {
                            pDecOut->outputPictureCbCr.pVirtualAddress = NULL;
                            pDecOut->outputPictureCbCr.busAddress = 0;
                            pDecOut->outputPictureCr.pVirtualAddress = NULL;
                            pDecOut->outputPictureCr.busAddress = 0;
                        }

                        JPEGDEC_API_TRC(("JpegDecDecode# Decoder + PP (Rotation/Flip), Slice ready\n"));
                        /* PP not in pipeline, continue do <==> while */
                        PTR_JPGC->info.noSliceIrqForUser = 1;
                    } else {
                        JPEGDEC_API_TRC(("JpegDecDecode# Slice ready"));
                        return JPEGDEC_SLICE_READY;
                    }
					
                } else {
                    if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE ||
                        PTR_JPGC->info.operationType == JPEGDEC_NONINTERLEAVED) {
                        currentPos =
                            rk_GetRegisterFile(PTR_JPGC->reghandle,
                                               HWIF_RLC_VLC_BASE);

                        /* update input buffer address */
                        PTR_JPGC->stream.pCurrPos = ((RK_U8 *) currentPos - 10);
                        PTR_JPGC->stream.bitPosInByte = 0;
                        PTR_JPGC->stream.readBits =
                            ((PTR_JPGC->stream.pCurrPos -
                              PTR_JPGC->stream.pStartOfStream) * 8);

                        /* default if data ends */
                        endOfImage = 1;

                        /* check if last scan is decoded */
                        for (i = 0;
                             i <
                             ((PTR_JPGC->stream.streamLength -
                               (PTR_JPGC->stream.readBits / 8))); i++) {
                            currentByte = PTR_JPGC->stream.pCurrPos[i];
                            if (currentByte == 0xFF) {
                                currentByte = PTR_JPGC->stream.pCurrPos[i + 1];
                                if (currentByte == 0xD9) {
                                    endOfImage = 1;
                                    break;
                                } else if (currentByte == 0xC4 ||
                                           currentByte == 0xDA) {
                                    endOfImage = 0;
                                    break;
                                }
                            }
                        }

                        currentByte = 0;
                        PTR_JPGC->info.SliceCount = 0;
                        PTR_JPGC->info.SliceReadyForPause = 0;

                        /* if not the last scan of the stream */
                        if (endOfImage == 0) {
                            /* output set */
                            if (PTR_JPGC->ppInstance == NULL &&
                                !PTR_JPGC->info.noSliceIrqForUser) {
                                pDecOut->outputPictureY.pVirtualAddress =
                                    PTR_JPGC->info.outLuma.vir_addr;
                                ASSERT(pDecOut->outputPictureY.pVirtualAddress);

                                /* output set */
                                pDecOut->outputPictureY.busAddress =
                                    PTR_JPGC->info.outLuma.phy_addr;
                                ASSERT(pDecOut->outputPictureY.busAddress);

                                /* if not grayscale */
                                if (PTR_JPGC->image.sizeChroma) {
                                    pDecOut->outputPictureCbCr.pVirtualAddress =
                                        PTR_JPGC->info.outChroma.vir_addr;
                                    ASSERT(pDecOut->outputPictureCbCr.
                                           pVirtualAddress);

                                    pDecOut->outputPictureCbCr.busAddress =
                                        PTR_JPGC->info.outChroma.phy_addr;
                                    ASSERT(pDecOut->outputPictureCbCr.
                                           busAddress);

                                    pDecOut->outputPictureCr.pVirtualAddress =
                                        PTR_JPGC->info.outChroma2.
                                        vir_addr;
                                    pDecOut->outputPictureCr.busAddress =
                                        PTR_JPGC->info.outChroma2.phy_addr;
                                }
                            }

                            /* PP not in pipeline, continue do <==> while */
                            PTR_JPGC->info.noSliceIrqForUser = 0;

                            if (PTR_JPGC->info.operationType ==
                                JPEGDEC_PROGRESSIVE)
                                PTR_JPGC->info.progressiveScanReady = 1;
                            else
                                PTR_JPGC->info.nonInterleavedScanReady = 1;

                            /* return control to application if progressive */
                            if (PTR_JPGC->info.operationType !=
                                JPEGDEC_NONINTERLEAVED) {
                                /* non-interleaved scan ==> no output */
                                if (PTR_JPGC->info.nonInterleaved == 0)
                                    PTR_JPGC->info.noSliceIrqForUser = 1;
                                else {
                                    JPEGDEC_API_TRC
                                    ("JpegDecDecode# SCAN PROCESSED");
                                    return (JPEGDEC_SCAN_PROCESSED);
                                }
                            } else {
                                /* set decoded component */
                                PTR_JPGC->info.components[PTR_JPGC->info.
                                                          componentId] = 1;

                                /* check if we have decoded all components */
                                if (PTR_JPGC->info.components[0] == 1 &&
                                    PTR_JPGC->info.components[1] == 1 &&
                                    PTR_JPGC->info.components[2] == 1) {
                                    /* continue decoding next scan */
                                    PTR_JPGC->info.noSliceIrqForUser = 0;
                                    nonInterleavedRdy = 0;
                                } else {
                                    /* continue decoding next scan */
                                    PTR_JPGC->info.noSliceIrqForUser = 1;
                                    nonInterleavedRdy = 0;
                                }
                            }
                        } else {
                            if (PTR_JPGC->info.operationType ==
                                JPEGDEC_NONINTERLEAVED) {
                                /* set decoded component */
                                PTR_JPGC->info.components[PTR_JPGC->info.
                                                          componentId] = 1;

                                /* check if we have decoded all components */
                                if (PTR_JPGC->info.components[0] == 1 &&
                                    PTR_JPGC->info.components[1] == 1 &&
                                    PTR_JPGC->info.components[2] == 1) {
                                    /* continue decoding next scan */
                                    PTR_JPGC->info.noSliceIrqForUser = 0;
                                    nonInterleavedRdy = 1;
                                } else {
                                    /* continue decoding next scan */
                                    PTR_JPGC->info.noSliceIrqForUser = 1;
                                    nonInterleavedRdy = 0;
                                }
                            }
                        }
                    } else {
                        /* PP not in pipeline, continue do <==> while */
                        PTR_JPGC->info.noSliceIrqForUser = 0;
                    }
                }

                /* output set */
                if (PTR_JPGC->ppInstance == NULL &&
                    !PTR_JPGC->info.noSliceIrqForUser) {
                    pDecOut->outputPictureY.pVirtualAddress =
                        PTR_JPGC->info.outLuma.vir_addr;
                    ASSERT(pDecOut->outputPictureY.pVirtualAddress);

                    /* output set */
                    pDecOut->outputPictureY.busAddress =
                        PTR_JPGC->info.outLuma.phy_addr;
                    ASSERT(pDecOut->outputPictureY.busAddress);

                    /* if not grayscale */
                    if (PTR_JPGC->image.sizeChroma) {
                        pDecOut->outputPictureCbCr.pVirtualAddress =
                            PTR_JPGC->info.outChroma.vir_addr;
                        ASSERT(pDecOut->outputPictureCbCr.pVirtualAddress);

                        pDecOut->outputPictureCbCr.busAddress =
                            PTR_JPGC->info.outChroma.phy_addr;
                        ASSERT(pDecOut->outputPictureCbCr.busAddress);

                        pDecOut->outputPictureCr.pVirtualAddress =
                            PTR_JPGC->info.outChroma2.vir_addr;
                        pDecOut->outputPictureCr.busAddress =
                            PTR_JPGC->info.outChroma2.phy_addr;
                    }
                }

                if (PTR_JPGC->info.noSliceIrqForUser == 0) {
                    /* update asicRunning */
                    PTR_JPGC->asicRunning = 0;

                    /* set image ready */
                    PTR_JPGC->image.imageReady = 1;
                    PTR_JPGC->info.noSliceIrqForUser = 0;
                }

                /* get the current stream address  */
                if (PTR_JPGC->info.operationType == JPEGDEC_PROGRESSIVE ||
                    (PTR_JPGC->info.operationType == JPEGDEC_NONINTERLEAVED &&
                     nonInterleavedRdy == 0)) {
                    retCode = JpegDecNextScanHdrs(PTR_JPGC);
                    if (retCode != JPEGDEC_OK && retCode != JPEGDEC_FRAME_READY) {
                        /* return */
                        return retCode;
                    }
                }
            } else if (dwlret == DWL_HW_WAIT_TIMEOUT) {
                JPEGDEC_API_TRC("SCAN: DWL HW TIMEOUT\n");

                /* update asicRunning */
                PTR_JPGC->asicRunning = 0;

                return (JPEGDEC_DWL_HW_TIMEOUT);
            } else if (dwlret == DWL_HW_WAIT_ERROR) {
                JPEGDEC_API_TRC("SCAN: DWL HW ERROR\n");

                /* update asicRunning */
                PTR_JPGC->asicRunning = 0;

                return (JPEGDEC_SYSTEM_ERROR);
            }
        } while (!PTR_JPGC->image.imageReady);
    }

    if (PTR_JPGC->image.imageReady) {
        /* reset image status */
        PTR_JPGC->image.imageReady = 0;
        PTR_JPGC->image.headerReady = 0;

        if (PTR_JPGC->info.operationType == JPEGDEC_BASELINE) {
            return (JPEGDEC_FRAME_READY);
        } else {
            if (endOfImage == 0)
                return (JPEGDEC_SCAN_PROCESSED);
            else {
                if (PTR_JPGC->frame.Nf != 1) {
                    /* determine first component that needs to be cheated */
                    i = 0;
                    while (i < 3 && PTR_JPGC->info.pfNeeded[i] == 0)
                        i++;
                    if (i == 3)
                        return (JPEGDEC_FRAME_READY);

                    JpegDecInitHWEmptyScan(PTR_JPGC, i++);
                    /* wait hw ready */
                    {
                        VPU_CMD_TYPE cmd;
                        RK_S32 len;
                        dwlret = VPUClientWaitResult(PTR_JPGC->socket, PTR_JPGC->reghandle, DEC_RK70_REGISTERS, &cmd, &len);
                        if ((VPU_SUCCESS != dwlret) || (cmd != VPU_SEND_CONFIG_ACK_OK))
                            dwlret = DWL_HW_WAIT_ERROR;

                    }
                    //printf("VPUClientWaitResult 2! \n");
                    ASSERT(dwlret == DWL_HW_WAIT_OK);
                    JpegRefreshRegs(PTR_JPGC);
                    asic_status =
                        rk_GetRegisterFile(PTR_JPGC->reghandle, HWIF_DEC_IRQ_STAT);
                    ASSERT(asic_status == 1);

                    while (i < 3 && PTR_JPGC->info.pfNeeded[i] == 0)
                        i++;
                    if (i == 3)
                        return (JPEGDEC_FRAME_READY);
                    else {
                        PTR_JPGC->info.progressiveFinish = 1;
                        PTR_JPGC->info.pfCompId = i;
                        return (JPEGDEC_SCAN_PROCESSED);
                    }
                } else
                    return (JPEGDEC_FRAME_READY);
            }
        }
    } else {
        JPEGDEC_API_TRC("JpegDecDecode# ERROR\n");
        return (JPEGDEC_ERROR);
    }
}

/*------------------------------------------------------------------------------

    5.6. Function name: JpegGetAPIVersion

         Purpose:       Returns version information about this API

         Input:         void

         Output:        JpegDecApiVersion

------------------------------------------------------------------------------*/
JpegDecApiVersion JpegGetAPIVersion()
{
    JpegDecApiVersion ver;

    ver.major = JPG_MAJOR_VERSION;
    ver.minor = JPG_MINOR_VERSION;
    //JPEGDEC_API_TRC("JpegGetAPIVersion# OK\n");
    return ver;
}

/*------------------------------------------------------------------------------

    5.7. Function name: JpegDecGetBuild

         Purpose:       Returns the SW and HW build information

         Input:         void

         Output:        JpegDecGetBuild

------------------------------------------------------------------------------*/
JpegDecBuild JpegDecGetBuild(void)
{
    JpegDecBuild ver;
    VPUHwDecConfig_t hwCfg;

    DWLmemset(&ver, 0, sizeof(ver));

    ver.swBuild = JPG_SW_BUILD;
    ver.hwBuild = 0x91900000;

    memset(&hwCfg, 0, sizeof(VPUHwDecConfig_t));
    hwCfg.jpegESupport = 1;
    hwCfg.jpegSupport = 1;

    // while (VPUGetDecHwCfg(&hwCfg))
    //  usleep(10);

    SET_DEC_BUILD_SUPPORT(ver.hwConfig, hwCfg);

    //JPEGDEC_API_TRC("JpegDecGetBuild# OK\n");

    return (ver);
}

