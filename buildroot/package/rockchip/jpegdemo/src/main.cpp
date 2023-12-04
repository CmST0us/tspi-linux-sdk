#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h> 
#include <stdint.h>

#include "pv_jpegdec_api.h"
#include "jpeg_global.h"
#include "vpu.h"
#include "vpu_api.h"

int getFileSize(FILE* fp)
{
    int filesize = 0;
    if(fp == NULL)
    {
        printf("fopen jpgFile fail:");
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return filesize;
}

int main(int argc, char** argv){
	char* mPath = argv[1];
	int srcSize = 0;	
	void* srcData=NULL;
	void* mDecoder;

    printf("jpeg_demo: mPath = %s \n", mPath);
	FILE* fp = fopen(mPath, "rb");

	if (fp) {
		srcSize = getFileSize(fp);
		srcData = (void*)malloc(srcSize);
		fread(srcData,1,srcSize,fp);
		fclose(fp);
	}

    printf("srcData=%p srcSize=%d \n", srcData, srcSize);
#ifdef DRM_LINUX
	if (srcData) {
		int aOutputLength=0;
        int ret;
	    JPEG_FRAME mFrame;

		mDecoder = get_class_RkJpegDecoder();
        ret = init_class_RkJpegDecoder(mDecoder);
        printf("get_class_RkJpegDecoder: =%p \n", mDecoder);
		ret = dec_oneframe_class_RkJpegDecoder(mDecoder, (void*)&mFrame, &aOutputLength, srcData, &srcSize);
        printf("jpeg_demo: mFrame.jpegmem.vir_addr=0x%x addr=%p mFrame.jpegmem.phy_addr=0x%x size=%d\n",
                mFrame.jpegmem.vir_addr, mFrame.jpegmem.vir_addr,
                mFrame.jpegmem.phy_addr, mFrame.jpegmem.size);
		if (mFrame.jpegmem.vir_addr != NULL) {
			FILE* outPut = fopen("/data/output.yuv", "wb");
            printf("jpeg_demo: fwrite result ret=%s\n", strerror(errno));
			if (outPut) {
                printf("fwrite ************************** mFrame.jpegmem.size =%d\n", mFrame.jpegmem.size);
				fwrite(mFrame.jpegmem.vir_addr, mFrame.jpegmem.size, 1, outPut);
				fclose(outPut);
				outPut = NULL;
			}
		}
	}
#else
    if (srcData) {
        int aOutputLength=0;
        int ret;
        VPU_FRAME mFrame;

        mDecoder = get_class_RkJpegDecoder();
        ret = init_class_RkJpegDecoder(mDecoder);
        printf("get_class_RkJpegDecoder: =%p \n", mDecoder);
        ret = dec_oneframe_class_RkJpegDecoder(mDecoder, (void*)&mFrame, &aOutputLength, srcData, &srcSize);
        printf("jpeg_demo: mFrame.jpegmem.vir_addr=0x%x addr=%p mFrame.jpegmem.phy_addr=0x%x size=%d\n",
                mFrame.vpumem.vir_addr, mFrame.vpumem.vir_addr,
                mFrame.vpumem.phy_addr, mFrame.vpumem.size);
        if (mFrame.vpumem.vir_addr != NULL) {
            FILE* outPut = fopen("/data/output.yuv", "wb");
            printf("jpeg_demo: fwrite result ret=%s\n", strerror(errno));
            if (outPut) {
                printf("fwrite ************************** mFrame.jpegmem.size =%d\n", mFrame.vpumem.size);
                fwrite(mFrame.vpumem.vir_addr, mFrame.vpumem.size, 1, outPut);
                fclose(outPut);
                outPut = NULL;
            }
        }
        if (mFrame.vpumem.vir_addr != NULL)
            VPUFreeLinear(&mFrame.vpumem);
    }
#endif
	if (srcData)
		free(srcData);
	srcData = NULL;
	deinit_class_RkJpegDecoder(mDecoder);
	return 0;
}
