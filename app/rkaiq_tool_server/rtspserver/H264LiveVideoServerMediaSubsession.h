#ifndef _H264or5_LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH
#define _H264or5_LIVE_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#include <OnDemandServerMediaSubsession.hh>

class H264LiveVideoServerMediaSubsession : public OnDemandServerMediaSubsession
{
  public:
    static H264LiveVideoServerMediaSubsession* createNew(UsageEnvironment& env, Boolean reuseFirstSource,
                                                         void* listener);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();

  protected:
    H264LiveVideoServerMediaSubsession(UsageEnvironment& env, Boolean reuseFirstSource, void* listener);
    // called only by createNew();
    virtual ~H264LiveVideoServerMediaSubsession();

    void setDoneFlag()
    {
        fDoneFlag = ~0;
    }

  protected: // redefined virtual functions
    virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource);

  private:
    char* fAuxSDPLine;
    char fDoneFlag;         // used when setting up "fAuxSDPLine"
    RTPSink* fDummyRTPSink; // ditto

    void* fListener;
};

#endif
