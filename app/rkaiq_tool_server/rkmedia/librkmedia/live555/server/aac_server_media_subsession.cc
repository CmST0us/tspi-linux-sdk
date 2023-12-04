// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "aac_server_media_subsession.hh"
#include "MPEG4GenericRTPSink.hh"

#include "utils.h"

namespace easymedia
{
    static unsigned const samplingFrequencyTable[16] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
                                                        16000, 12000, 11025, 8000,  7350,  0,     0,     0};

    AACServerMediaSubsession* AACServerMediaSubsession::createNew(UsageEnvironment& env, Live555MediaInput& wisInput,
                                                                  unsigned samplingFrequency, unsigned numChannels,
                                                                  unsigned char profile)
    {
        return new AACServerMediaSubsession(env, wisInput, samplingFrequency, numChannels, profile);
    }

    AACServerMediaSubsession::AACServerMediaSubsession(UsageEnvironment& env, Live555MediaInput& mediaInput,
                                                       unsigned samplingFrequency, unsigned numChannels,
                                                       unsigned char profile)
        : OnDemandServerMediaSubsession(env, True /*reuse the first source*/), fMediaInput(mediaInput),
          fSamplingFrequency(samplingFrequency), fNumChannels(numChannels)
    {
        unsigned char audioSpecificConfig[2];
        unsigned char const audioObjectType = profile + 1;
        unsigned char samplingFrequencyIndex = 0;
        for (unsigned char i = 0; i < 16; i++) {
            if (samplingFrequencyTable[i] == fSamplingFrequency) {
                samplingFrequencyIndex = i;
                break;
            }
        }
        audioSpecificConfig[0] = (audioObjectType << 3) | (samplingFrequencyIndex >> 1);
        // audioSpecificConfig[1] = (samplingFrequencyIndex<<7) |
        // (channelConfiguration<<3);
        audioSpecificConfig[1] = (samplingFrequencyIndex << 7) | (fNumChannels << 3);
        sprintf(fConfigStr, "%02X%02x", audioSpecificConfig[0], audioSpecificConfig[1]);
    }

    AACServerMediaSubsession::~AACServerMediaSubsession()
    {
        LOG_FILE_FUNC_LINE();
    }

    FramedSource* AACServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
    {
        LOG_FILE_FUNC_LINE();
        estBitrate = 96; // kbps, estimate
        return fMediaInput.audioSource();
    }

    RTPSink* AACServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                                        FramedSource* inputSource)
    {
        if (!inputSource) {
            LOG("inputSource is not ready, can not create new rtp sink\n");
            return NULL;
        }

        // ADTSAudioFileSource* adtsSource = (ADTSAudioFileSource*)inputSource;
        setAudioRTPSinkBufferSize();
        RTPSink* rtpsink =
            MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, fSamplingFrequency, "audio",
                                           "AAC-hbr", fConfigStr, fNumChannels);
        setVideoRTPSinkBufferSize();
        return rtpsink;
    }

    // std::mutex AACServerMediaSubsession::kMutex;
    void AACServerMediaSubsession::startStream(unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler,
                                               void* rtcpRRHandlerClientData, unsigned short& rtpSeqNum,
                                               unsigned& rtpTimestamp,
                                               ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                                               void* serverRequestAlternativeByteHandlerClientData)
    {
        OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken, rtcpRRHandler, rtcpRRHandlerClientData,
                                                   rtpSeqNum, rtpTimestamp, serverRequestAlternativeByteHandler,
                                                   serverRequestAlternativeByteHandlerClientData);
        // kMutex.lock();
        if (kSessionIdList.empty()) {
            fMediaInput.Start(envir());
        }
        if (fMediaInput.GetStartAudioStreamCallback() != NULL) {
            fMediaInput.GetStartAudioStreamCallback()();
        }
        LOG("%s:%s:%p - clientSessionId: 0x%08x\n", __FILE__, __func__, this, clientSessionId);
        kSessionIdList.push_back(clientSessionId);
        // kMutex.unlock();
    }
    void AACServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& streamToken)
    {
        // kMutex.lock();
        LOG("%s - clientSessionId: 0x%08x\n", __func__, clientSessionId);
        kSessionIdList.remove(clientSessionId);
        if (kSessionIdList.empty()) {
            fMediaInput.Stop(envir());
        }
        // kMutex.unlock();
        OnDemandServerMediaSubsession::deleteStream(clientSessionId, streamToken);
    }

} // namespace easymedia
