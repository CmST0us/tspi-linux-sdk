#ifndef _JPEG_GLOBAL_
#define _JPEG_GLOBAL_
#ifdef DRM_LINUX
#include "vpu_macro.h"
#else
#include "vpu_api.h"
#endif
#include "allocator_drm.h"

typedef struct tJPEG_FRAME
{
    RK_U32          FrameBusAddr[2];       //0: Y address; 1: UV address;
    RK_U32          FrameWidth;         //16X对齐宽度
    RK_U32          FrameHeight;        //16X对齐高度
    RK_U32          OutputWidth;        //非16X必须
    RK_U32          OutputHeight;       //非16X必须
    RK_U32          DisplayWidth;       //显示宽度
    RK_U32          DisplayHeight;      //显示高度
    RK_U32          ColorType;
    RK_U32          ErrorInfo;          //该帧的错误信息，返回给系统方便调试
    RK_U32	        employ_cnt;
    MppBufferInfo   jpegmem;
    struct tJPEG_FRAME *    next_frame;
    RK_U32          Res[4];
}JPEG_FRAME;
#endif /*_VPU_GLOBAL_*/
