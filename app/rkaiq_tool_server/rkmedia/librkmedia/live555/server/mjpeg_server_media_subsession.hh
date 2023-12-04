// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MJPEG_SERVER_MEDIA_SUBSESSION_HH_
#define EASYMEDIA_MJPEG_SERVER_MEDIA_SUBSESSION_HH_

#include <list>
#include <mutex>

#include "live555_media_input.hh"
#include "mjpeg_video_source.hh"
#include <liveMedia/OnDemandServerMediaSubsession.hh>

namespace easymedia {
class MJPEGServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
  static MJPEGServerMediaSubsession *createNew(UsageEnvironment &env,
                                               Live555MediaInput &wisInput);

protected: // we're a virtual base class
  MJPEGServerMediaSubsession(UsageEnvironment &env,
                             Live555MediaInput &mediaInput);
  virtual ~MJPEGServerMediaSubsession();
  void startStream(
      unsigned clientSessionId, void *streamToken, TaskFunc *rtcpRRHandler,
      void *rtcpRRHandlerClientData, unsigned short &rtpSeqNum,
      unsigned &rtpTimestamp,
      ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
      void *serverRequestAlternativeByteHandlerClientData) override;
  void deleteStream(unsigned clientSessionId, void *&streamToken) override;

protected:
  Live555MediaInput &fMediaInput;
  unsigned fEstimatedKbps;

private: // redefined virtual functions
  virtual FramedSource *createNewStreamSource(unsigned clientSessionId,
                                              unsigned &estBitrate);
  virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource *inputSource);

private:
  std::list<unsigned int> kSessionIdList;
};
} // namespace easymedia

#endif // #ifndef EASYMEDIA_MJPEG_SERVER_MEDIA_SUBSESSION_HH_
