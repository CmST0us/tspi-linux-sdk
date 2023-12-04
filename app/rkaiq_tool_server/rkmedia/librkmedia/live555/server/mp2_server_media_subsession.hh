// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MP2_SERVER_MEDIA_SUBSESSION_HH_
#define EASYMEDIA_MP2_SERVER_MEDIA_SUBSESSION_HH_

#include <list>
#include <mutex>

#include "live555_media_input.hh"
#include <liveMedia/OnDemandServerMediaSubsession.hh>

namespace easymedia {
class MP2ServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
  static MP2ServerMediaSubsession *createNew(UsageEnvironment &env,
                                             Live555MediaInput &wisInput);

protected: // we're a virtual base class
  MP2ServerMediaSubsession(UsageEnvironment &env,
                           Live555MediaInput &mediaInput);
  virtual ~MP2ServerMediaSubsession();

protected:
  Live555MediaInput &fMediaInput;
  void startStream(
      unsigned clientSessionId, void *streamToken, TaskFunc *rtcpRRHandler,
      void *rtcpRRHandlerClientData, unsigned short &rtpSeqNum,
      unsigned &rtpTimestamp,
      ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
      void *serverRequestAlternativeByteHandlerClientData) override;
  void deleteStream(unsigned clientSessionId, void *&streamToken) override;

private: // redefined virtual functions
  virtual FramedSource *createNewStreamSource(unsigned clientSessionId,
                                              unsigned &estBitrate);
  virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource *inputSource);
  std::list<unsigned int> kSessionIdList;
  // static std::mutex kMutex;
};
} // namespace easymedia

#endif // #ifndef EASYMEDIA_MP2_SERVER_MEDIA_SUBSESSION_HH_
