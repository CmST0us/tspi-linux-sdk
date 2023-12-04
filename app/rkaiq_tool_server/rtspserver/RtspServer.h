#ifndef RTSP_SENDER_NODE_
#define RTSP_SENDER_NODE_

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <thread>

#include "CamCaptureHelper.h"
#include "RKHWEncApi.h"

class RtspServer
{
  public:
    RtspServer();
    ~RtspServer();

    typedef struct {
        char dev_name[64];    // camera v4l2 device name
        int width;            // camera v4l2 vfmt width
        int height;           // camera v4l2 vfmt height
        char stream_name[64]; // rtsp url stream name
        int port_num;         // rtsp port number
    } MetaInfo;

    bool init(MetaInfo* meta);

    void setDoneFlag()
    {
        fDoneFlag = ~0;
    }
    void resetDoneFlag()
    {
        fDoneFlag = 0;
    }
    void doEventLoop();

    bool start();
    bool stop();

    // It is called every time sent occurs.
    void onDoGetNextFrame(QMediaBuffer* outBuf);

  private:
    UsageEnvironment* mEnv;
    ServerMediaSession* mSms;
    RTSPServer* mRtspServer;
    char mStreamName[64];

    char fDoneFlag;

    std::thread* mProcThread;

    CamCaptureHelper* mCamDev;
    RKHWEncApi* mEncoder;

    bool initOther(MetaInfo* meta);
};

int init_rtsp(const char* video_dev, int width, int height);
void deinit_rtsp();

#endif // RTSP_SENDER_NODE_
