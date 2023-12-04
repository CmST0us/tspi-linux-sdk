#ifndef _H264or5_LIVE_VIDEO_SOURCE_HH
#define _H264or5_LIVE_VIDEO_SOURCE_HH

#include "FramedSource.hh"

class H264LiveVideoSource : public FramedSource
{
  public:
    static H264LiveVideoSource* createNew(UsageEnvironment& env, void* listener);
    virtual unsigned maxFrameSize() const;

  protected:
    H264LiveVideoSource(UsageEnvironment& env, void* listener);
    // called only by createNew()

    virtual ~H264LiveVideoSource();

  private:
    // redefined virtual functions:
    virtual void doGetNextFrame();

  private:
    Boolean fHasTriggerKeyFrame;
    int fSendHeaderCount;
    // char *fTruncatedBytes;

    unsigned int fTruncatedBytesNum;

    void* fListener;
};

#endif
