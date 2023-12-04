#ifndef GLPORCESS_H
#define GLPORCESS_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#define MAGIC   0x3141592

#define EGL_ITU_REC601_EXT                0x327F
#define EGL_ITU_REC709_EXT                0x3280
#define EGL_ITU_REC2020_EXT               0x3281
#define EGL_YUV_FULL_RANGE_EXT            0x3282
#define EGL_YUV_NARROW_RANGE_EXT          0x3283

#define GRAPHICS_ROT_MIRROR        1800
#define GRAPHICS_ROT_FLIP          900

typedef enum {
    RKGFX_LSF_OP_RENDER_TEXTURE = 0,
    RKGFX_LSF_OP_COLOR_FILL,
    RKGFX_LSF_OP_DRAW_LINES,
    RKGFX_LSF_OP_DRAW_POINTS,
    
} RKGFX_LSF_OP;

#define MAXLayer 128

#define MAXLines 64


typedef struct RKGFX_LSF_POINT_S {
    int x;
    int y;
} RKGFX_LSF_POINT_T;

typedef struct RKGFX_LSF_LINE_S{
    RKGFX_LSF_POINT_T startPt;
    RKGFX_LSF_POINT_T endPt;
    int lineW;
    float color[3];
} RKGFX_LSF_LINE_T;

typedef struct RKGFX_LSF_LINES_S{
    RKGFX_LSF_LINE_T linesArray[MAXLines];
    int numlines;
} RKGFX_LSF_LINES_T;

typedef struct RKGFX_LSF_QUADRL_INFO_S {
    float color[3];
    int coord[8];
    // [0]=x0 [1]=y0, [2]=x1 [3]=y1,  [4]=x2 [5]=y2,  [6]=x3 [7]=y3
    //        (x0,y0)------- (x3,y3)
    //               |     |
    //               |     |
    //               |     |
    //        (x1,y1)------- (x2,y2)
    int lineW;
    int quadBox;   // If you want to render the border this flag is set to 1
} RKGFX_LSF_QUADRL_INFO_T;

typedef struct RKGFX_LSF_MOSAIC_INFO_S{
    int left;
    int top;
    int right;
    int bottom;
    int bsize;
    int mode;
} RKGFX_LSF_MOSAIC_INFO_T;


typedef struct RKGFX_LSF_WIREFRAME_INFO_S{
    int left;
    int top;
    int right;
    int bottom;
    float color[3];
    int lpx;
    int tpx;
    int rpx;
    int bpx;
} RKGFX_LSF_WIREFRAME_INFO_T;

typedef struct RKGFX_LSF_ALPHA_LUT_INFO_S{
   unsigned char map_0;
   unsigned char map_1;
} RKGFX_LSF_ALPHA_LUT_INFO_T;

typedef struct RKGFX_LSF_LAYER_INFO_S{
    int fd;
    unsigned int fd_id;
    unsigned int  chID;
    int afbc_flag;
    int format;
    int color_space;
    int sample_range;
    int width;
    int height;
    int left;
    int top;
    int right;
    int bottom;
    int focuswin;
    int rotation;
    RKGFX_LSF_QUADRL_INFO_T quadril_info;
    RKGFX_LSF_MOSAIC_INFO_T mosic_info;
    RKGFX_LSF_WIREFRAME_INFO_T wireframe_info;
    RKGFX_LSF_ALPHA_LUT_INFO_T alpha_lut_info;
    int reserve[32];
} RKGFX_LSF_LAYER_INFO_T;

typedef struct RKGFX_LSF_LAYER_LIST_S{
    RKGFX_LSF_LAYER_INFO_T srcLayerInfo[MAXLayer];
    RKGFX_LSF_LAYER_INFO_T dstLayerInfo[MAXLayer];
    RKGFX_LSF_LINES_T lineSet;
    int numLayer;
    int op;
    float wfRgb[3];
    float fcfRgb[3];
    int px;
    int pxfc;
    int imgReserve;
    int reserve[32];   // reserve[0], 保留默认清屏的颜色。  reserve[1], 传递几个屏的信息
    int tsize ;
} RKGFX_LSF_LAYER_LIST_T;

#ifdef __cplusplus
extern "C"
{
#endif

void* RKGFX_LSF_Create();
int   RKGFX_LSF_Init(void *p,int screenW,int screenH,int priority);
int   RKGFX_LSF_Composite(void *p,RKGFX_LSF_LAYER_LIST_T *layerinfo);
void* RKGFX_LSF_CreateFence(void *p);
int   RKGFX_LSF_WaitFence(void *p,void *fence);
int   RKGFX_LSF_Destroy(void *p);
int   RKGFX_LSF_ReleaseChannel(void *p, unsigned int  chID);
unsigned int RKGFX_LSF_Calc_Afbc_MemSize(const int width, const int height,const int format);

#ifdef __cplusplus
}
#endif
#endif


