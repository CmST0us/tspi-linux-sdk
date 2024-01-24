#include "camerathread1.h"
#include "../v4l2/v4l2.h"

CameraThread1::CameraThread1()
{

}

void CameraThread1::run()
{
    getCam1Buf();
}

void CameraThread1::getCam1Buf()
{
    unsigned char *yuvBuffer = (unsigned char*)malloc(SRC_WIDTH * SRC_HEIGHT * 3);
    unsigned char *rgbBuffer = (unsigned char*)malloc(SRC_WIDTH * SRC_HEIGHT * 3);
    buffer * vbuffer;
    vbuffer = (buffer*)calloc (NB_BUFFER, sizeof (*vbuffer));

    int fd = c_OpenDevice(camera1Dev.toLocal8Bit().data());
    if(fd < 0) return;

    c_FormatDevice(V4L2_PIX_FMT_NV12, fd);
    c_RequestBuffer(vbuffer, fd);

    while (previewCam1) {
        if(c_GetBuffer(yuvBuffer, vbuffer, fd) != 0) return;
        c_NV12_TO_RGB24(yuvBuffer, rgbBuffer, SRC_WIDTH, SRC_HEIGHT);
        emit showCamera1(rgbBuffer);
        msleep(1000/30);
    }

    //close camera
    c_DeintDevice(fd, vbuffer);
    c_CloseDevice(fd);
}
