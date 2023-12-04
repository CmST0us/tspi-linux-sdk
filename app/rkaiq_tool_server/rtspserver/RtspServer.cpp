//#define LOG_NDEBUG 0
#define LOG_TAG "RtspServer"
#include <utils/Log.h>

#include <stdlib.h>
#include <stdio.h>

#include "H264LiveVideoServerMediaSubsession.h"
#include "RtspServer.h"

RtspServer::RtspServer() : mEnv(NULL), mSms(NULL), mRtspServer(NULL), mProcThread(NULL), mCamDev(NULL), mEncoder(NULL)
{
    ALOGD("RtspServer enter");
}

RtspServer::~RtspServer()
{
    ALOGD("~RtspServer enter");
    if (mProcThread != NULL) {
        delete mProcThread;
        mProcThread = NULL;
    }

    if (mCamDev != NULL) {
        mCamDev->deinit();
        delete mCamDev;
        mCamDev = NULL;
    }
    if (mEncoder != NULL) {
        delete mEncoder;
        mEncoder = NULL;
    }
}

bool RtspServer::init(MetaInfo* meta)
{
    if (meta == NULL) {
        ALOGE("Failed to get metaData");
        return false;
    }

    if (mEnv == NULL) {
        // begin by setting up our usage environment.
        TaskScheduler* scheduler = BasicTaskScheduler::createNew();
        mEnv = BasicUsageEnvironment::createNew(*scheduler);

        OutPacketBuffer::maxSize = 1920 * 1080 * 2;
        mRtspServer = RTSPServer::createNew(*mEnv, meta->port_num);
        if (mRtspServer == NULL) {
            ALOGE("Failed to create RTSP server: %s", mEnv->getResultMsg());
            return false;
        }
        strcpy(mStreamName, meta->stream_name);
    }

    return initOther(meta);
}

bool RtspServer::initOther(MetaInfo* meta)
{
    mCamDev = new CamCaptureHelper();
    // start CamCaptureHelper.
    CamCaptureHelper::MetaInfo camInfo;

    strcat(camInfo.video_dev, meta->dev_name);
    camInfo.width = meta->width;
    camInfo.height = meta->height;
    camInfo.format = 0; // nv12

    if (!mCamDev->init(&camInfo)) {
        ALOGE("Failed to init mCamDev");
        return false;
    }

    // start RKHWEncApi.
    mEncoder = new RKHWEncApi();
    RKHWEncApi::EncCfgInfo_t encInfo;
    encInfo.width = meta->width;
    encInfo.height = meta->height;
    encInfo.format = ENC_INPUT_YUV420_SEMIPLANAR;
    encInfo.framerate = 30;    // 30fps
    encInfo.bitRate = 8000000; // 500k default
    encInfo.IDRInterval = 1;
    encInfo.rc_mode = 1;
    encInfo.qp = 20;

    if (!mEncoder->init(&encInfo)) {
        ALOGE("Failed to init mEncoder");
        return false;
    }

    return true;
}

void RtspServer::onDoGetNextFrame(QMediaBuffer* outBuf)
{
    bool ret = true;
    QMediaBuffer camBuf;

    ALOGI("onDoGetNextFrame");

    if (mCamDev->getCameraBuffer(&camBuf)) {
        ret = mEncoder->sendFrame(camBuf.getFd(), camBuf.getSize(), 0, 0);
        if (!ret) {
            ALOGD("Failed to sendFrame");
            mCamDev->putCameraBuffer(&camBuf);
            return;
        }

        EncoderOut_t encOut;
        ret = mEncoder->getOutStream(&encOut);
        if (ret) {
            // fill live source data
            outBuf->setData(encOut.data, encOut.size);
        } else {
            ALOGD("Failed to getOutStream");
        }

        mCamDev->putCameraBuffer(&camBuf);
    } else {
        ALOGD("Failed to get camera buffer");
    }
}

bool RtspServer::start()
{
    ALOGD("Start streaming...");
    resetDoneFlag();
    mProcThread = new std::thread(&RtspServer::doEventLoop, this);

    mSms = ServerMediaSession::createNew(*mEnv, mStreamName, "test", "Session streamed by \"cameraPushTest\"");
    mSms->addSubsession(H264LiveVideoServerMediaSubsession::createNew(*mEnv, True, this));
    mRtspServer->addServerMediaSession(mSms);

    char* url = mRtspServer->rtspURL(mSms);
    ALOGD("Play this stream using the URL: %s", url);
    delete[] url;

    return true;
}

bool RtspServer::stop()
{
    ALOGD("Stop streaming... Exit scheduler eventLoop");
    setDoneFlag();

    ALOGD("Exit procThread");
    if (mProcThread != NULL) {
        mProcThread->join();
        delete mProcThread;
        mProcThread = NULL;
    }

    if (mRtspServer != NULL) {
        mRtspServer->deleteServerMediaSession(mStreamName);
    }

    if (mCamDev != NULL) {
        mCamDev->deinit();
        delete mCamDev;
        mCamDev = NULL;
    }
    if (mEncoder != NULL) {
        delete mEncoder;
        mEncoder = NULL;
    }
    return true;
}

void RtspServer::doEventLoop()
{
    ALOGD("Env doEventLoop");
    mEnv->taskScheduler().doEventLoop(&fDoneFlag);
    ALOGD("Env doEventLoop exit");
}

static RtspServer _g_rtsp_server;

int init_rtsp(const char* video_dev, int width, int height)
{
    RtspServer::MetaInfo info;
    info.width = width;
    info.height = height;
    info.port_num = 1234;
    strcat(info.dev_name, video_dev);
    strcat(info.stream_name, "v");

    ALOGD("init_rtsp: video_dev %s w %d h %d", video_dev, width, height);

    if (!_g_rtsp_server.init(&info)) {
        ALOGE("Failed to init _g_rtsp_server");
        return -1;
    }
    _g_rtsp_server.start();

    return 0;
}

void deinit_rtsp()
{
    ALOGD("deinit_rtsp enter");
    _g_rtsp_server.stop();
}
