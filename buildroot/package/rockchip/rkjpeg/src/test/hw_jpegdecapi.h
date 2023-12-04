#ifndef HW_JPEGDECAPI_H
#define HW_JPEGDECAPI_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    void * inStream;
    int wholeStreamLength;
    int (*FillInputBuf) (void *, char *, size_t );
} SourceStreamCtl;

typedef struct {
    int enable;
    int outFomart; /* =0,RGB565;=1,ARGB 8888 */
    //int destWidth;
    //int destHeight;
    int scale_denom;
    int shouldDither;
    int cropX;
    int cropY;
    int cropW;
    int cropH;
} PostProcessInfo;

typedef struct {
    void * decoderHandle;
    char *outAddr;
    int outWidth;
    int outHeight;
} HwJpegOutputInfo;

typedef struct {
    SourceStreamCtl  streamCtl;
    PostProcessInfo  ppInfo;
} HwJpegInputInfo;

extern int hw_jpeg_decode(HwJpegInputInfo *inInfo, HwJpegOutputInfo *outInfo);
extern int hw_jpeg_release(void *decInst);
extern void PrintRet(JpegDecRet * pJpegRet);
#ifdef DRM_LINUX
extern int SetPostProcessor(void * reg, MppBufferInfo *dst, int inWidth, int inHeigth,
                            int outWidth, int outHeight, int inColor, PostProcessInfo *ppInfo);
#else
extern int SetPostProcessor(void * reg, VPUMemLinear_t *dst, int inWidth, int inHeigth,
                            int outWidth, int outHeight, int inColor, PostProcessInfo *ppInfo);

#endif
#ifdef __cplusplus
}
#endif

#endif /* HW_JPEGDECAPI_H */

