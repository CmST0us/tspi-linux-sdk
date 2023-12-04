// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_SIMPLE_SERVER_MEDIA_SUBSESSION_HH_
#define EASYMEDIA_SIMPLE_SERVER_MEDIA_SUBSESSION_HH_

#include <list>
#include <mutex>

#include "live555_media_input.hh"
#include <liveMedia/OnDemandServerMediaSubsession.hh>

typedef enum {
  WA_PCM = 0x01,
  WA_PCMA = 0x06,
  WA_PCMU = 0x07,
  WA_IMA_ADPCM = 0x11,
  WA_UNKNOWN
} WAV_AUDIO_FORMAT;

namespace easymedia {
class SIMPLEServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
  static SIMPLEServerMediaSubsession *
  createNew(UsageEnvironment &env, Live555MediaInput &wisInput,
            unsigned samplingFrequency, unsigned numChannels,
            std::string audioFormat, unsigned bitrate);

protected: // we're a virtual base class
  SIMPLEServerMediaSubsession(UsageEnvironment &env,
                              Live555MediaInput &mediaInput,
                              unsigned samplingFrequency, unsigned numChannels,
                              std::string audioFormat, unsigned bitrate);
  virtual ~SIMPLEServerMediaSubsession();

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
  std::string fAudioFormat;
  unsigned fbitrate; //码率bits
  std::list<unsigned int> kSessionIdList;
};
} // namespace easymedia

#endif // #ifndef EASYMEDIA_SIMPLE_SERVER_MEDIA_SUBSESSION_HH_
