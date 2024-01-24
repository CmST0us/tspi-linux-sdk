#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
#include <linux/videodev2.h>

//c varable
extern struct v4l2_fmtdesc fmtd[20];
extern unsigned char * displaybuf; //v4l2 video buffer
extern int current_video_state;
typedef struct buffer{
    void *start;
    unsigned int length;
}buffer;

//define
#define SRC_WIDTH 640
#define SRC_HEIGHT 480
#define DST_WIDTH 640
#define DST_HEIGHT 480
#define NB_BUFFER 4

#ifdef __cplusplus
}
#endif
#endif
