#ifndef V4L2_H
#define V4L2_H

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

//function
int c_OpenDevice(char* video);
void c_CloseDevice(int videofd);
int c_FormatDevice(unsigned int pixformat, int videofd);
int c_RequestBuffer(buffer *vbuffer, int videofd);
int c_GetBuffer(unsigned char* yuvBuffer, buffer *vbuffer, int videofd);
void c_DeintDevice(int videofd, buffer *vbuffer);
void c_NV12_TO_RGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height);

//varable


#ifdef __cplusplus
}
#endif
#endif
