#include "H264LiveVideoSource.h"
#include "RtspServer.h"

H264LiveVideoSource* H264LiveVideoSource::createNew(UsageEnvironment& env, void* listener)
{
    return new H264LiveVideoSource(env, listener);
}

H264LiveVideoSource::H264LiveVideoSource(UsageEnvironment& env, void* listener) : FramedSource(env)
{
    env << "H264LiveVideoSource::H264LiveVideoSource"
        << "\n";
    fHasTriggerKeyFrame = False;
    fSendHeaderCount = 1;
    fTruncatedBytesNum = 0;

    // setup callback listener
    fListener = listener;
}

H264LiveVideoSource::~H264LiveVideoSource()
{
}

unsigned int H264LiveVideoSource::maxFrameSize() const
{
    return 1024 * 1024;
}

void H264LiveVideoSource::doGetNextFrame()
{
    QMediaBuffer outBuf;
    RtspServer* server = (RtspServer*)fListener;

    server->onDoGetNextFrame(&outBuf);
    if (outBuf.getData()) {

        fFrameSize = outBuf.getSize();
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fNumTruncatedBytes = fFrameSize - fMaxSize;
        } else {
            fNumTruncatedBytes = 0;
        }

        memmove(fTo, outBuf.getData(), fFrameSize);

        FramedSource::afterGetting(this);
    }
}
