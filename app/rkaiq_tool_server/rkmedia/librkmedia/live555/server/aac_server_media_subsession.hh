// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_AAC_SERVER_MEDIA_SUBSESSION_HH_
#define EASYMEDIA_AAC_SERVER_MEDIA_SUBSESSION_HH_

#include <list>
#include <mutex>

#include "live555_media_input.hh"
#include <liveMedia/OnDemandServerMediaSubsession.hh>

namespace easymedia {
class AACServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
  static AACServerMediaSubsession *createNew(UsageEnvironment &env,
                                             Live555MediaInput &wisInput,
                                             unsigned samplingFrequency,
                                             unsigned numChannels,
                                             unsigned char profile);

protected: // we're a virtual base class
  AACServerMediaSubsession(UsageEnvironment &env, Live555MediaInput &mediaInput,
                           unsigned samplingFrequency, unsigned numChannels,
                           unsigned char profile);
  virtual ~AACServerMediaSubsession();

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
  unsigned fSamplingFrequency;
  unsigned fNumChannels;
  unsigned fuSecsPerFrame;
  char fConfigStr[5];
  std::list<unsigned int> kSessionIdList;
  // static std::mutex kMutex;
};
} // namespace easymedia

#endif // #ifndef EASYMEDIA_AAC_SERVER_MEDIA_SUBSESSION_HH_
