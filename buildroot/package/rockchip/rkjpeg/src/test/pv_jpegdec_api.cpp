#define LOG_TAG "Jpeg_api"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "jpegdecapi.h"
#include "dwl.h"
#include "jpegdeccontainer.h"

#include "deccfg.h"
#include "rkregdrv.h"

#include "hw_jpegdecapi.h"
#include "pv_jpegdec_api.h"

#include "vpu.h"
#include "jpeg_global.h"
#include <errno.h>
#include <unistd.h>
#ifndef DRM_LINUX
#include "vpu.h"
#include "vpu_api.h"
#endif
#include "vpu_macro.h"
//#include "jpeg_macro.h"

#define ALOGV printf
#define ALOGD printf
#define ALOGI printf
#define ALOGW printf
#define ALOGE printf


extern void PrintRet(JpegDecRet * pJpegRet);

#define DEFAULT                          -1
#define JPEG_INPUT_BUFFER                0x5120

#define PP_IN_FORMAT_YUV422INTERLAVE     0
#define PP_IN_FORMAT_YUV420SEMI          1
#define PP_IN_FORMAT_YUV420PLANAR        2
#define PP_IN_FORMAT_YUV400              3
#define PP_IN_FORMAT_YUV422SEMI          4
#define PP_IN_FORMAT_YUV420SEMITIELED    5
#define PP_IN_FORMAT_YUV440SEMI          6
#define PP_IN_FORMAT_YUV444_SEMI         7
#define PP_IN_FORMAT_YUV411_SEMI         8

#define PP_OUT_FORMAT_RGB565             0
#define PP_OUT_FORMAT_ARGB               1
#define PP_OUT_FORMAT_YUV422INTERLAVE    3
#define PP_OUT_FORMAT_YUV420INTERLAVE    5
#define SZ_4K                            (4*1024)
//#define LocalPrint ALOG
#define LocalPrint(...)
int Rk_JpegDecoder::pv_rkJpegdecoder_init()
{
    int ret;
    JpegDecApiVersion decVer;
    JpegDecBuild decBuild;
	printf("pv_rkJpegdecoder_init 00000000000\n");

    jpegIn.streamBuffer.pVirtualAddress = BLANK;
    jpegIn.streamBuffer.busAddress = 0;
    jpegIn.streamLength = 0;
    jpegIn.pictureBufferY.pVirtualAddress = BLANK;
    jpegIn.pictureBufferY.busAddress = 0;
    jpegIn.pictureBufferCbCr.pVirtualAddress = BLANK;
    jpegIn.pictureBufferCbCr.busAddress = 0;

    /* reset output */
    jpegOut.outputPictureY.pVirtualAddress = BLANK;
    jpegOut.outputPictureY.busAddress = 0;
    jpegOut.outputPictureCbCr.pVirtualAddress = BLANK;
    jpegOut.outputPictureCbCr.busAddress = 0;
    jpegOut.outputPictureCr.pVirtualAddress = BLANK;
    jpegOut.outputPictureCr.busAddress = 0;

    /* reset imageInfo */
    imageInfo.displayWidth = 0;
    imageInfo.displayHeight = 0;
    imageInfo.outputWidth = 0;
    imageInfo.outputHeight = 0;
    imageInfo.version = 0;
    imageInfo.units = 0;
    imageInfo.xDensity = 0;
    imageInfo.yDensity = 0;
    imageInfo.outputFormat = 0;
    imageInfo.thumbnailType = 0;
    imageInfo.displayWidthThumb = 0;
    imageInfo.displayHeightThumb = 0;
    imageInfo.outputWidthThumb = 0;
    imageInfo.outputHeightThumb = 0;
    imageInfo.outputFormatThumb = 0;

    /* set default */
    jpegIn.sliceMbSet = 0;
    jpegIn.bufferSize = 0;
	printf("pv_rkJpegdecoder_init 111111111111\n");

    memset(&pictureMem, 0, sizeof(pictureMem));

    streamMem.vir_addr = BLANK;
    streamMem.phy_addr = 0;
	
	printf("pv_rkJpegdecoder_init os_api=%p  allocator_drm=%p---------\n",os_api, allocator_drm);
	os_api = &allocator_drm;
	printf("pv_rkJpegdecoder_init 1.5 os_api=%p  allocator_drm=%p \n", os_api, allocator_drm);
	ctx = (void*)malloc(sizeof(allocator_ctx_drm));
	printf("pv_rkJpegdecoder_init os_api->open -\n");
	ret = os_api->open(&ctx, 32);

    /* allocate memory for stream buffer. if unsuccessful -> exit */
    stream_size = 1024 * 500;
#ifdef DRM_LINUX
	streamMem.size = stream_size;
	streamMem.type = MPP_BUFFER_TYPE_DRM;
	printf("pv_rkJpegdecoder_init  222222222222222\n");
    ret = os_api->alloc(ctx, &streamMem);
	os_api->mmap(ctx, &streamMem);
#else
	VPUMallocLinear(&streamMem, stream_size);
#endif
	printf("pv_rkJpegdecoder_init os_api->alloc \n");
    if (ret != DWL_OK) {
        return -1;
    }

    decVer = JpegGetAPIVersion();
    decBuild = JpegDecGetBuild();

    jpegInst = NULL;
    return 0;
}

int Rk_JpegDecoder::mJpegFreeLinear(MppBufferInfo* mInfo){
	if (mInfo && os_api) {
		os_api->free(ctx, mInfo);
    }
    return 0;
}

int Rk_JpegDecoder::pv_rkJpegdecoder_oneframe(unsigned char * aOutBuffer, unsigned int *aOutputLength,
                                              unsigned char* aInputBuf, unsigned int* aInBufSize, unsigned int out_phyaddr)
{
    JpegDecContainer *jpegC;
    int ret;

    int ppInputFomart = 0;
    RK_U32 imageInfoLength = 0;
    RK_U32 getDataLength = 0;
    RK_U32 mcuSizeDivider = 0, amountOfMCUs = 0, mcuInRow = 0;
    PostProcessInfo ppInfo;
    RK_U32 ppScaleW = 640, ppScaleH = 480;
    RK_U32 input_size = *aInBufSize;
    RK_U32 str_size = (input_size + 255) & (~255);
    RK_U32 pic_size = 0;
    RK_U8 *tmp;
    RK_U8 *dst;
    RK_U8 *end;

    if (input_size < 16) {
        ALOGD("input_size is too short, size=%d\n", input_size);
        *aInBufSize = 0;
        *aOutputLength = 0;
        goto j_end;
    }
	printf("pv_rkJpegdecoder_oneframe: jpegInst=%p open vpu device \n", jpegInst);
    if (NULL == jpegInst) {
        jpegRet = JpegDecInit(&jpegInst, color_conv);
        if (jpegRet != JPEGDEC_OK) {
            LocalPrint(LOG_DEBUG, "jpeghw", ("JpegDecInit Fail!!!\n"));
            PrintRet(&jpegRet);
            return -1;
        }
    }
    jpegC = (JpegDecContainer*)jpegInst;
    jpegC->vpumem_ctx = vpumem_ctx;
	jpegC->ctx = ctx;

	printf("stream_size=%d str_size + SZ_4K=%d \n", stream_size, (str_size + SZ_4K));
    if (str_size + SZ_4K > stream_size) {
        /* make sure that input buffer is always larger than
         * input stream length */
#ifdef DRM_LINUX
        os_api->free(ctx, &streamMem);
		streamMem.size = str_size + SZ_4K;
		ret = os_api->alloc(ctx, &streamMem);
		ret = os_api->mmap(ctx, &streamMem);
		
		printf("alloc streamMem: addr=%p size=%d fd=%d result=%d\n", streamMem.vir_addr, streamMem.size, streamMem.phy_addr, ret);
#else
        if (streamMem.vir_addr)
            VPUFreeLinear(&streamMem);
        ret = VPUMallocLinear(&streamMem, str_size + SZ_4K);
#endif
        if (ret != DWL_OK) {
            goto j_end;
        }
        stream_size = str_size + SZ_4K;
    }
	printf("after alloc *************\n");
    dst = (RK_U8 *)streamMem.vir_addr;
    end = dst + input_size;
    if (aInputBuf[6] == 0x41 && aInputBuf[7] == 0x56 && aInputBuf[8] == 0x49 && aInputBuf[9] == 0x31) {
        /* distinguish 310 series camera from 210 for Logitech */
        int     i;
        RK_U32 copy_len = 0;
        tmp = aInputBuf;

        for (i = 0; i < input_size - 4; i++) {
            if (tmp[i] == 0xff) {
                if (tmp[i + 1] == 0x00 && tmp[i + 2] == 0xff && ((tmp[i + 3] & 0xf0) == 0xd0))
                    i += 2;
            }
            *dst++ = tmp[i];
            copy_len++;
        }
        for (; i < input_size; i++) {
            *dst++ = tmp[i];
            copy_len++;
        }
        if (copy_len < input_size)
            memset(dst, 0, input_size - copy_len);
        input_size = copy_len;
    } else {
        memcpy(dst, aInputBuf, input_size);
        memset(dst + input_size, 0, str_size - input_size);
    }
	if (dst) {
		FILE* fp = fopen("/data/test.jpg", "wb");
		if (fp) {
			fwrite(dst, 1, input_size, fp);
			fclose(fp);
		}
	}
    /* NOTE: on2 hardware bug, need to remove tailing FF 00 before FF D9 end flag */
    dst = (RK_U8 *)streamMem.vir_addr;
    if (end[-1] == 0xD9 && end[-2] == 0xFF) {
        end -= 2;

        do {
            if (end[-1] == 0xFF) {
                end--;
                continue;
            }
            if (end[-1] == 0x00 && end [-2] == 0xFF) {
                end -= 2;
                continue;
            }
            break;
        } while (1);

        end[0] = 0xff;
        end[1] = 0xD9;
    }
	printf("os_api->free ************\n");
    //os_api->free(ctx, &streamMem);
    //VPUMemInvalidate(&streamMem);

    jpegIn.streamLength = input_size;
    jpegIn.bufferSize = input_size;
    jpegIn.pstreamMem = &streamMem;

    /* Pointer to the input JPEG */
    jpegIn.streamBuffer.pVirtualAddress = (RK_U32 *) streamMem.vir_addr;
    jpegIn.streamBuffer.busAddress = streamMem.phy_addr;

    /* Get image information of the JPEG image */
    jpegRet = JpegDecGetImageInfo(jpegInst, &jpegIn, &imageInfo);
    if (jpegRet != JPEGDEC_OK) {
        LocalPrint(LOG_DEBUG, "jpeghw", "JpegDecGetImageInfo fail error code is  %d\n", (int)jpegRet);
        goto j_end;
    }

    if (color_conv) {
        /* Using pp to convert all format to yuv420sp */
        switch (imageInfo.outputFormat) {
        case JPEGDEC_YCbCr400:
            ppInputFomart = PP_IN_FORMAT_YUV400;
            break;
        case JPEGDEC_YCbCr420_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV420SEMI;
            break;
        case JPEGDEC_YCbCr422_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV422SEMI;
            break;
        case JPEGDEC_YCbCr440:
            ppInputFomart = PP_IN_FORMAT_YUV440SEMI;
            break;
        case JPEGDEC_YCbCr411_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV411_SEMI;
            break;
        case JPEGDEC_YCbCr444_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV444_SEMI;
            break;
        }

        /* set pp info */
        memset(&ppInfo, 0, sizeof(ppInfo));
        ppInfo.enable = 1;
        ppInfo.outFomart = 5;   //PP_OUT_FORMAT_YUV420INTERLAVE
        ppScaleW = imageInfo.outputWidth;
        ppScaleH = imageInfo.outputHeight;
        if (ppScaleW > 1920) { // || ppScaleH > 1920) {
            ppScaleW = (ppScaleW + 15) & (~15);
            ppScaleH = (ppScaleH + 15) & (~15);
        } else {
            ppScaleW = (ppScaleW + 7) & (~7); // pp dest width must be dividable by 8
            ppScaleH = (ppScaleH + 1) & (~1); // must be dividable by 2.in pp downscaling ,the output lines always equal (desire lines - 1);
        }

        pic_size = ppScaleW * ppScaleH * 2;
        jpegC->ppInstance = (void *)1;
    } else {
        /* keep original output format */
        memset(&ppInfo, 0, sizeof(ppInfo));
        ppInfo.outFomart = 5;   //PP_OUT_FORMAT_YUV420INTERLAVE
        ppScaleW = imageInfo.outputWidth;
        ppScaleH = imageInfo.outputHeight;

        ppScaleW = (ppScaleW + 15) & (~15);
        ppScaleH = (ppScaleH + 15) & (~15);

        switch (imageInfo.outputFormat) {
        case JPEGDEC_YCbCr400:
            ppInputFomart = PP_IN_FORMAT_YUV400;
            pic_size = ppScaleW * ppScaleH;
            break;
        case JPEGDEC_YCbCr420_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV420SEMI;
            pic_size = ppScaleW * ppScaleH * 3 / 2;
            break;
        case JPEGDEC_YCbCr422_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV422SEMI;
            pic_size = ppScaleW * ppScaleH * 2;
            break;
        case JPEGDEC_YCbCr440:
            ppInputFomart = PP_IN_FORMAT_YUV440SEMI;
            pic_size = ppScaleW * ppScaleH * 2;
            break;
        case JPEGDEC_YCbCr411_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV411_SEMI;
            pic_size = ppScaleW * ppScaleH * 2;
            break;
        case JPEGDEC_YCbCr444_SEMIPLANAR:
            ppInputFomart = PP_IN_FORMAT_YUV444_SEMI;
            pic_size = ppScaleW * ppScaleH * 3;
            break;
        }

        jpegC->ppInstance = (void *)0;
    }

    /* if there is not external buffer use internal buffer instead */
    if (!out_phyaddr) {
        int timeout = 0;

        while (1) {
            if (jpegC->vpumem_ctx) {
                //ret = VPUMallocLinearFromRender(&ppOutMem, (ppScaleW * ppScaleH * 2), jpegC->vpumem_ctx);
                if (ret != DWL_OK) {
                    return (JPEGDEC_MEMFAIL);
                }
            } else {
#ifdef DRM_LINUX
				pictureMem.size = pic_size;
				ret = os_api->alloc(ctx, &pictureMem);
				ret = os_api->mmap(ctx, &pictureMem);
#else
				VPUMallocLinear(&pictureMem, pic_size);;
#endif				
            }
            if (ret == DWL_OK) {
                break;
            }

            usleep(5000);
            timeout++;
            if (timeout > 0xFF) {
                ALOGE("MJPEG Malloc ppoutmem Failed \n");
                return (JPEGDEC_MEMFAIL);
            }
        }
    } else {
        pictureMem.phy_addr = out_phyaddr;
    }

    jpegC->pictureMem = &pictureMem;

    SetPostProcessor(jpegC->reghandle, &pictureMem, imageInfo.outputWidth, imageInfo.outputHeight,
                     ppScaleW, ppScaleH, ppInputFomart, &ppInfo);
#if 1
     if (1) {
        /* default output format is yuv420sp, so we can do in this way */
        JpegdIocExtInfoSlot *slot;
        slot = &(jpegC->ioctl_info.extra_info.slots[0]);
        slot->reg_idx = 22; /* pp_out_chroma */
        slot->offset = ppScaleW * ppScaleH;
    }
#endif
    /* Slice mode */
    jpegIn.sliceMbSet = 0;

    /* calculate MCU's */
    if (imageInfo.outputFormat == JPEGDEC_YCbCr400 ||
        imageInfo.outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR) {
        amountOfMCUs =
            ((imageInfo.outputWidth * imageInfo.outputHeight) / 64);
        mcuInRow = (imageInfo.outputWidth / 8);
    } else if (imageInfo.outputFormat == JPEGDEC_YCbCr420_SEMIPLANAR) {
        /* 265 is the amount of luma samples in MB for 4:2:0 */
        amountOfMCUs =
            ((imageInfo.outputWidth * imageInfo.outputHeight) / 256);
        mcuInRow = (imageInfo.outputWidth / 16);
    } else if (imageInfo.outputFormat == JPEGDEC_YCbCr422_SEMIPLANAR) {
        /* 128 is the amount of luma samples in MB for 4:2:2 */
        amountOfMCUs =
            ((imageInfo.outputWidth * imageInfo.outputHeight) / 128);
        mcuInRow = (imageInfo.outputWidth / 16);
    } else if (imageInfo.outputFormat == JPEGDEC_YCbCr440) {
        /* 128 is the amount of luma samples in MB for 4:4:0 */
        amountOfMCUs =
            ((imageInfo.outputWidth * imageInfo.outputHeight) / 128);
        mcuInRow = (imageInfo.outputWidth / 8);
    } else if (imageInfo.outputFormat == JPEGDEC_YCbCr411_SEMIPLANAR) {
        amountOfMCUs =
            ((imageInfo.outputWidth * imageInfo.outputHeight) / 256);
        mcuInRow = (imageInfo.outputWidth / 32);
    }

    /* set mcuSizeDivider for slice size count */
    if (imageInfo.outputFormat == JPEGDEC_YCbCr400 ||
        imageInfo.outputFormat == JPEGDEC_YCbCr440 ||
        imageInfo.outputFormat == JPEGDEC_YCbCr444_SEMIPLANAR)
        mcuSizeDivider = 2;
    else
        mcuSizeDivider = 1;

    /* 9190 and over 16M ==> force to slice mode */
    if ((jpegIn.sliceMbSet == 0) &&
        ((imageInfo.outputWidth * imageInfo.outputHeight) >
         JPEGDEC_MAX_PIXEL_AMOUNT)) {
        do {
            jpegIn.sliceMbSet++;
        } while (((jpegIn.sliceMbSet * (mcuInRow / mcuSizeDivider)) +
                  (mcuInRow / mcuSizeDivider)) <
                 JPEGDEC_MAX_SLICE_SIZE_8190);
        LocalPrint(LOG_DEBUG, "jpeghw", "Force to slice mode (over 16M) ==> Decoder Slice MB Set %d\n", (int)jpegIn.sliceMbSet);
    }

    jpegIn.decImageType = 0; // FULL MODEs

	printf("before JpegDecDecode(jpegInst, &jpegIn, &jpegOut); \n");
    /* Decode JPEG */
    do {
        jpegRet = JpegDecDecode(jpegInst, &jpegIn, &jpegOut);
        PrintRet(&jpegRet);
		printf("JpegDecDecode  jpegRet =%d\n", jpegRet);
        if ( jpegRet == JPEGDEC_FRAME_READY) {
			
			#ifdef DRM_LINUX
            JPEG_FRAME *frame  = (JPEG_FRAME *)aOutBuffer;
			#else
			VPU_FRAME *frame  = (VPU_FRAME *)aOutBuffer;
			#endif
            frame->FrameBusAddr[0] = pictureMem.phy_addr;
            frame->FrameBusAddr[1] = frame->FrameBusAddr[0] + ((ppScaleW + 15) & 0xfffffff0) * ((ppScaleH + 15) & 0xfffffff0);

            frame->DisplayWidth  = ppScaleW;
            frame->DisplayHeight = ppScaleH;

            frame->FrameWidth = (ppScaleW + 15) & (~15);
            frame->FrameHeight = (ppScaleH + 15) & (~15);

            if (out_phyaddr) {
                frame->ColorType = VPU_OUTPUT_FORMAT_YUV420_SEMIPLANAR;
                *aOutputLength = sizeof(JPEG_FRAME);
            } else {
                if (color_conv) {
                    frame->ColorType = VPU_OUTPUT_FORMAT_YUV420_SEMIPLANAR;
                } else {
                    switch (imageInfo.outputFormat) {
                    case JPEGDEC_YCbCr420_SEMIPLANAR:
                        frame->ColorType = VPU_OUTPUT_FORMAT_YUV420_SEMIPLANAR;
                        break;
                    case JPEGDEC_YCbCr422_SEMIPLANAR:
                        frame->ColorType = VPU_OUTPUT_FORMAT_YUV422_SEMIPLANAR;
                        break;
                    default:
                        frame->ColorType = VPU_OUTPUT_FORMAT_YUV420_SEMIPLANAR;
                        break;
                    }
                }
                #ifdef DRM_LINUX
                frame->jpegmem.phy_addr = pictureMem.phy_addr;
				frame->jpegmem.vir_addr = pictureMem.vir_addr;
				frame->jpegmem.hnd = pictureMem.hnd;
				frame->jpegmem.size = pictureMem.size;
				frame->jpegmem.type = pictureMem.type;
				 *aOutputLength = sizeof(JPEG_FRAME);
				#else
				VPUMemDuplicate(&frame->vpumem, &pictureMem);
                *aOutputLength = sizeof(VPU_FRAME);
				#endif
            }
        } else if ( jpegRet == JPEGDEC_SLICE_READY) {
        } else if ( jpegRet == JPEGDEC_STRM_PROCESSED) {
        } else if ( jpegRet == JPEGDEC_STRM_ERROR) {
            goto j_end;
        } else {
            LocalPrint(LOG_DEBUG, "jpeghw", "DECODE ERROR\n");
            goto j_end;
        }
    } while (jpegRet != JPEGDEC_FRAME_READY );

j_end:
#if 1
    if (jpegRet == JPEGDEC_FRAME_READY ) {
        static FILE *jpg_file;
        static char name[32];
        static int jpeg_count = 0;

        snprintf(name, sizeof(name), "/data/jpg/out%04d.yuv", jpeg_count);
        jpg_file = fopen(name, "wb");
		printf("open:%s ret=%s\n", jpg_file, strerror(errno));
        if (jpg_file) {
            ALOGE("found jpeg error saving to %s\n", name);
            fwrite(pictureMem.vir_addr, pictureMem.size, 1, jpg_file);
            fclose(jpg_file);
            jpeg_count++;
        }
    }
#endif
#ifdef DRM_LINUX
    if (!out_phyaddr)
        mJpegFreeLinear(&pictureMem);
#else
	if (!out_phyaddr)
		VPUFreeLinear(&pictureMem);
#endif
    if (jpegRet == JPEGDEC_UNSUPPORTED) {
        return JPEGDEC_UNSUPPORTED;
    }
    return 0;

}

int Rk_JpegDecoder::pv_rkJpegdecoder_rest()
{
    return 0;
}

int Rk_JpegDecoder::pv_rkJpegdecoder_deinit()
{
#ifdef DRM_LINUX
    if (streamMem.vir_addr != NULL)
        mJpegFreeLinear(&streamMem);

    if (pictureMem.vir_addr != NULL)
        mJpegFreeLinear(&pictureMem);

    if (jpegInst)
        hw_jpeg_release(jpegInst);

	if (ctx)
		os_api->close(ctx);
#else
	if (streamMem.vir_addr != NULL)
        VPUFreeLinear(&streamMem);

    if (pictureMem.vir_addr != NULL)
        VPUFreeLinear(&pictureMem);

    if (jpegInst)
        hw_jpeg_release(jpegInst);
#endif
    return 0;
}

uint32_t Rk_JpegDecoder::pv_rkJpeg_getoutformat()
{
    if (color_conv) {
        return JPEGDEC_YCbCr420_SEMIPLANAR;
    }
    return imageInfo.outputFormat;
}
int32_t Rk_JpegDecoder::pv_rkJpeg_perform_seting(int cmd, void *param)
{
    int32_t ret = 0;
 /*   JpegDecContainer *jpegC = (JpegDecContainer*)jpegInst;
    switch (cmd) {
    case VPU_API_SET_VPUMEM_CONTEXT: {
        vpumem_ctx = param;
        ALOGE("jpegd recv VPU_API_SET_VPUMEM_CONTEXT, set vpumem_ctx to %p\n", vpumem_ctx);
    } break;
    case VPU_API_SET_CONV_COLOR: {
        color_conv = *((RK_U32 *)param);
        ALOGE("jpeg set color conversion to %d\n", color_conv);
    } break;
    default : {
        ret = -EINVAL;
    } break;
    }*/
    return ret;
}

extern "C"
void *  get_class_RkJpegDecoder(void)
{
    return (void*)new Rk_JpegDecoder();
}
extern "C"
void  destroy_class_RkJpegDecoder(void * JpegDecoder)
{
    delete (Rk_JpegDecoder *)JpegDecoder;
}
extern "C"
int init_class_RkJpegDecoder(void * JpegDecoder)
{
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
	printf("init_class_RkJpegDecoder ******\n");
    return Jpegdec->pv_rkJpegdecoder_init();
}
extern "C"
int deinit_class_RkJpegDecoder(void * JpegDecoder)
{
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
    return Jpegdec->pv_rkJpegdecoder_deinit();
}
extern "C"
int reset_class_RkJpegDecoder(void * JpegDecoder)
{
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
    return Jpegdec->pv_rkJpegdecoder_rest();
}
extern "C"
int dec_oneframe_RkJpegDecoder(void * JpegDecoder, uint8_t* aOutBuffer, uint32_t *aOutputLength,
                               uint8_t* aInputBuf, uint32_t* aInBufSize, uint32_t out_phyaddr)
{
    int ret;
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
    ret = Jpegdec->pv_rkJpegdecoder_oneframe(aOutBuffer, aOutputLength,
                                             aInputBuf, aInBufSize, out_phyaddr);
    return ret;
}

extern "C"
int dec_oneframe_class_RkJpegDecoder(void * JpegDecoder, uint8_t* aOutBuffer, uint32_t *aOutputLength,
                                     uint8_t* aInputBuf, uint32_t* aInBufSize)
{
    int ret;
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
    ret = Jpegdec->pv_rkJpegdecoder_oneframe(aOutBuffer, aOutputLength,
                                             aInputBuf, aInBufSize, 0);
    return ret;
}
extern "C"
int perform_seting_class_RkJpegDecoder(void * JpegDecoder, VPU_API_CMD cmd, void *param)
{
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
    int ret = 0;
    if (Jpegdec != NULL) {
        ret = Jpegdec->pv_rkJpeg_perform_seting(cmd, param);
    } else {
        ret = -1;
    }
    return 0;
}
extern "C"
uint32_t perform_getoutformat_class_RkJpegDecoder(void * JpegDecoder)
{
    Rk_JpegDecoder * Jpegdec = (Rk_JpegDecoder *)JpegDecoder;
    int ret = 0;
    ret = Jpegdec->pv_rkJpeg_getoutformat();
    return ret;
}

