#include <H264VideoRTPSink.hh>
#include <H264VideoStreamFramer.hh>

#include "H264LiveVideoServerMediaSubsession.h"
#include "H264LiveVideoSource.h"

H264LiveVideoServerMediaSubsession* H264LiveVideoServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                  Boolean reuseFirstSource,
                                                                                  void* listener)
{
    return new H264LiveVideoServerMediaSubsession(env, reuseFirstSource, listener);
}

H264LiveVideoServerMediaSubsession::H264LiveVideoServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource,
                                                                       void* listener)
    : OnDemandServerMediaSubsession(env, reuseFirstSource), fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL)
{
    fListener = listener;
}

H264LiveVideoServerMediaSubsession::~H264LiveVideoServerMediaSubsession()
{
}

static void afterPlayingDummy(void* clientData)
{
    H264LiveVideoServerMediaSubsession* subsess = (H264LiveVideoServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void H264LiveVideoServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData)
{
    H264LiveVideoServerMediaSubsession* subsess = (H264LiveVideoServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void H264LiveVideoServerMediaSubsession::checkForAuxSDPLine1()
{
    char const* dasl;
    if (fAuxSDPLine != NULL) {
        // Signal the event loop that we're done:
        setDoneFlag();
    } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
        fAuxSDPLine = strDup(dasl);
        fDummyRTPSink = NULL;

        // Signal the event loop that we're done:
        setDoneFlag();
    } else if (!fDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 100;
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* H264LiveVideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    if (fAuxSDPLine != NULL)
        return fAuxSDPLine; // it's already been set up (for a previous client)

    if (fDummyRTPSink == NULL) {
        fDummyRTPSink = rtpSink;

        // Start reading the file:
        fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&fDoneFlag);

    return fAuxSDPLine;
}

FramedSource* H264LiveVideoServerMediaSubsession::createNewStreamSource(unsigned, unsigned& estBitrate)
{
    estBitrate = 500; // kbps, estimate

    // Create the video source:
    H264LiveVideoSource* liveSource = H264LiveVideoSource::createNew(envir(), fListener);
    if (liveSource == NULL)
        return NULL;

    // Create a framer for the Video Elementary Stream:
    return H264VideoStreamFramer::createNew(envir(), liveSource);
}

RTPSink* H264LiveVideoServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                                              unsigned char rtpPayloadTypeIfDynamic,
                                                              FramedSource* /*inputSource*/)
{
    // setVideoRTPSinkBufferSize
    OutPacketBuffer::maxSize = 1920 * 1088 * 2;

    return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
