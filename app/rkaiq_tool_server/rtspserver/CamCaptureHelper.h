#ifndef CAM_CAPTURE_HELPER_
#define CAM_CAPTURE_HELPER_

#include "RKHWEncApi.h"
#include "QMediaBuffer.h"

#define MAX_CAM_BUFCNT 10

class CamCaptureHelper
{
  public:
    CamCaptureHelper();
    ~CamCaptureHelper();

    typedef struct {
        /* v4l2 setup information */
        char video_dev[128]; // v4l2 video device
        int width;           // v4l2 pixel format
        int height;
        int format;
    } MetaInfo;

    typedef struct CamFrame_t {
        void* start;
        size_t length;
        int export_fd;
    } CamFrame;

    bool init(MetaInfo* meta);
    void deinit();

    bool getCameraBuffer(QMediaBuffer* buffer);
    bool putCameraBuffer(QMediaBuffer* buffer);

  private:
    int mDevfd;
    int mPixBufCnt;
    CamFrame mCambuf[MAX_CAM_BUFCNT]; // frame buffers

    int v4l2Ioctl(int fd, int req, void* arg);
    int v4l2DequeueBuf();
    void v4l2QueueBuf(int idx);
};

#endif // CAM_CAPTURE_HELPER_
