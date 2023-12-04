// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mjpeg_server_media_subsession.hh"
#include <liveMedia/JPEGVideoRTPSink.hh>

#include "media_type.h"
#include "utils.h"

namespace easymedia
{
    MJPEGServerMediaSubsession* MJPEGServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                      Live555MediaInput& wisInput)
    {
        return new MJPEGServerMediaSubsession(env, wisInput);
    }

    MJPEGServerMediaSubsession::MJPEGServerMediaSubsession(UsageEnvironment& env, Live555MediaInput& mediaInput)
        : OnDemandServerMediaSubsession(env, True /*reuse the first source*/), fMediaInput(mediaInput),
          fEstimatedKbps(1000)
    {
    }

    MJPEGServerMediaSubsession::~MJPEGServerMediaSubsession()
    {
        LOG_FILE_FUNC_LINE();
    }

    void MJPEGServerMediaSubsession::startStream(
        unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
        unsigned short& rtpSeqNum, unsigned& rtpTimestamp,
        ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
        void* serverRequestAlternativeByteHandlerClientData)
    {
        OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken, rtcpRRHandler, rtcpRRHandlerClientData,
                                                   rtpSeqNum, rtpTimestamp, serverRequestAlternativeByteHandler,
                                                   serverRequestAlternativeByteHandlerClientData);
        if (kSessionIdList.empty()) {
            fMediaInput.Start(envir());
        }
        if (fMediaInput.GetStartVideoStreamCallback() != NULL) {
            fMediaInput.GetStartVideoStreamCallback()();
        }
        LOG("%s - clientSessionId: 0x%08x\n", __func__, clientSessionId);
        kSessionIdList.push_back(clientSessionId);
    }
    void MJPEGServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& streamToken)
    {
        LOG("%s - clientSessionId: 0x%08x\n", __func__, clientSessionId);
        kSessionIdList.remove(clientSessionId);
        if (kSessionIdList.empty()) {
            fMediaInput.Stop(envir());
        }
        OnDemandServerMediaSubsession::deleteStream(clientSessionId, streamToken);
    }

    FramedSource* MJPEGServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
    {
        estBitrate = fEstimatedKbps;
        LOG_FILE_FUNC_LINE();
        if (!fSDPLines && clientSessionId != 0) {
            LOG("%s:%s:%d --- you must get sdp first.\n", __FILE__, __func__, __LINE__);
            return NULL;
        }
        // Create a framer for the Video Elementary Stream:
        FramedSource* source = MJPEGVideoSource::createNew(envir(), fMediaInput.videoSource(CODEC_TYPE_JPEG));
        LOG("MJPEG framedsource : %p\n", source);
        return source;
    }

    RTPSink* MJPEGServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char,
                                                          FramedSource* inputSource)
    {
        if (!inputSource) {
            LOG("inputSource is not ready, can not create new rtp sink\n");
            return NULL;
        }
        setVideoRTPSinkBufferSize();
        LOG_FILE_FUNC_LINE();
        RTPSink* rtp_sink = JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
        LOG("MJPEG rtp sink : %p\n", rtp_sink);
        return rtp_sink;
    }

} // namespace easymedia
