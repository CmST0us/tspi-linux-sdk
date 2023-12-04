// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mp2_server_media_subsession.hh"
#include "MPEG1or2AudioRTPSink.hh"

#include "utils.h"
namespace easymedia
{
    MP2ServerMediaSubsession* MP2ServerMediaSubsession::createNew(UsageEnvironment& env, Live555MediaInput& wisInput)
    {
        return new MP2ServerMediaSubsession(env, wisInput);
    }

    MP2ServerMediaSubsession::MP2ServerMediaSubsession(UsageEnvironment& env, Live555MediaInput& mediaInput)
        : OnDemandServerMediaSubsession(env, True /*reuse the first source*/), fMediaInput(mediaInput)
    {
    }

    MP2ServerMediaSubsession::~MP2ServerMediaSubsession()
    {
        LOG_FILE_FUNC_LINE();
    }

    FramedSource* MP2ServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
    {
        LOG_FILE_FUNC_LINE();
        estBitrate = 128; // kbps, estimate
        return fMediaInput.audioSource();
    }

    RTPSink* MP2ServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
                                                        unsigned char /*rtpPayloadTypeIfDynamic*/,
                                                        FramedSource* inputSource)
    {
        if (!inputSource) {
            LOG("inputSource is not ready, can not create new rtp sink\n");
            return NULL;
        }
        setAudioRTPSinkBufferSize();
        RTPSink* rtpsink = MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
        setVideoRTPSinkBufferSize();
        return rtpsink;
    }

    // std::mutex MP2ServerMediaSubsession::kMutex;
    void MP2ServerMediaSubsession::startStream(unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler,
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
        LOG("%s - clientSessionId: 0x%08x\n", __func__, clientSessionId);
        kSessionIdList.push_back(clientSessionId);
        // kMutex.unlock();
    }
    void MP2ServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& streamToken)
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
