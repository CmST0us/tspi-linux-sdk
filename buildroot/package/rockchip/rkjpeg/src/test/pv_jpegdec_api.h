#ifndef PV_JPEGDECAPI_H
#define PV_JPEGDECAPI_H

#include "jpegdecapi.h"
//#include "vpu_api_private.h"
#include "allocator_drm.h"
#ifndef DRM_LINUX
#include "vpu.h"
#include "vpu_api.h"
#else

typedef enum VPU_API_CMD {
    VPU_API_ENC_SETCFG,
    VPU_API_ENC_GETCFG,
    VPU_API_ENC_SETFORMAT,
    VPU_API_ENC_SETIDRFRAME,
    VPU_API_ENABLE_DEINTERLACE,
    VPU_API_SET_VPUMEM_CONTEXT,
    VPU_API_USE_PRESENT_TIME_ORDER,
    VPU_API_SET_DEFAULT_WIDTH_HEIGH,
    VPU_API_SET_INFO_CHANGE,
    VPU_API_SET_CONV_COLOR,
    VPU_API_DEC_GET_STREAM_COUNT,

    VPU_API_SET_IMMEDIATE_OUT = 0x1000,
    VPU_API_SET_CODEC_TAG = 0x1001,
} VPU_API_CMD;
#endif
typedef struct {
    RK_U32  alignment;
    RK_S32  drm_device;
} allocator_ctx_drm;

#ifdef __cplusplus
extern "C"
{
#endif
extern void*         get_class_RkJpegDecoder(void);
extern void      destroy_class_RkJpegDecoder(void * JpegDecoder);
extern int          init_class_RkJpegDecoder(void * JpegDecoder);
extern int        deinit_class_RkJpegDecoder(void * JpegDecoder);
extern int         reset_class_RkJpegDecoder(void * JpegDecoder);
extern int  dec_oneframe_class_RkJpegDecoder(void * JpegDecoder, unsigned char* aOutBuffer, unsigned int *aOutputLength,
											unsigned char* aInputBuf, unsigned int* aInBufSize);
extern int   perform_seting_class_RkJpegDecoder(void * JpegDecoder, VPU_API_CMD cmd, void *param);

#ifdef __cplusplus
}
#endif


class Rk_JpegDecoder
{
public:
    Rk_JpegDecoder() { jpegInst = NULL; color_conv = 1; vpumem_ctx = NULL;};
    ~Rk_JpegDecoder() { };
    int pv_rkJpegdecoder_init();
    int pv_rkJpegdecoder_oneframe(unsigned char * aOutBuffer, unsigned int *aOutputLength,
                                  unsigned char* aInputBuf, unsigned int* aInBufSize, unsigned int out_phyaddr = 0);
    int pv_rkJpegdecoder_rest();
    int pv_rkJpegdecoder_deinit();
    int32_t pv_rkJpeg_perform_seting(int cmd, void *param);
    uint32_t pv_rkJpeg_getoutformat();
	int mJpegFreeLinear(MppBufferInfo* mInfo);
private:
    //Jpeg_Decoder       *Jpeg_dec;
    JpegDecImageInfo imageInfo;
    JpegDecInput jpegIn;
    JpegDecOutput jpegOut;
    JpegDecInst jpegInst;
    JpegDecRet jpegRet;
#ifdef DRM_LINUX
    MppBufferInfo streamMem;
    MppBufferInfo pictureMem;
#else
	VPUMemLinear_t streamMem;
	VPUMemLinear_t pictureMem;
#endif
    uint32_t stream_size;

    RK_U32 color_conv;
    void* vpumem_ctx;
	void* ctx;
	os_allocator* os_api;
};


#endif /* PV_JPEGDECAPI_H */

