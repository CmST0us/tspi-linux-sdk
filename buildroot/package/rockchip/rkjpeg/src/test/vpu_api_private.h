#ifndef VPU_API_PRIVATE_H_
#define VPU_API_PRIVATE_H_
#include "vpu_api.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
extern void*         get_class_RkJpegDecoder(void);
extern void      destroy_class_RkJpegDecoder(void * JpegDecoder);
extern int          init_class_RkJpegDecoder(void * JpegDecoder);
extern int        deinit_class_RkJpegDecoder(void * JpegDecoder);
extern int         reset_class_RkJpegDecoder(void * JpegDecoder);
extern int  dec_oneframe_class_RkJpegDecoder(void * JpegDecoder, unsigned char* aOutBuffer, unsigned int *aOutputLength,
 unsigned char* aInputBuf, unsigned int* aInBufSize);
extern int   perform_seting_class_RkJpegDecoder(void * JpegDecoder, VPU_API_CMD cmd, void *param);
#endif