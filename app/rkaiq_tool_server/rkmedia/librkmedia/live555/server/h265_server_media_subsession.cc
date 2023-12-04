// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "h265_server_media_subsession.hh"

#include <liveMedia/H265VideoRTPSink.hh>
#include <liveMedia/H265VideoStreamDiscreteFramer.hh>

#include "media_type.h"
#include "utils.h"

namespace easymedia
{
    H265ServerMediaSubsession* H265ServerMediaSubsession::createNew(UsageEnvironment& env, Live555MediaInput& wisInput)
    {
        return new H265ServerMediaSubsession(env, wisInput);
    }

    H265ServerMediaSubsession::H265ServerMediaSubsession(UsageEnvironment& env, Live555MediaInput& mediaInput)
        : OnDemandServerMediaSubsession(env, True /*reuse the first source*/), fMediaInput(mediaInput),
          fEstimatedKbps(1000), fDoneFlag(0), fDummyRTPSink(NULL), fGetSdpCount(10), fAuxSDPLine(NULL)
    {
    }

    H265ServerMediaSubsession::~H265ServerMediaSubsession()
    {
        LOG_FILE_FUNC_LINE();
        if (fAuxSDPLine != NULL) {
            delete[] fAuxSDPLine;
            fAuxSDPLine = NULL;
        }
    }

    // std::mutex H265ServerMediaSubsession::kMutex;
    // std::list<unsigned int> H265ServerMediaSubsession::kSessionIdList;

    void H265ServerMediaSubsession::startStream(
        unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
        unsigned short& rtpSeqNum, unsigned& rtpTimestamp,
        ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
        void* serverRequestAlternativeByteHandlerClientData)
    {
        OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken, rtcpRRHandler, rtcpRRHandlerClientData,
                                                   rtpSeqNum, rtpTimestamp, serverRequestAlternativeByteHandler,
                                                   serverRequestAlternativeByteHandlerClientData);
        // kMutex.lock();
        if (fMediaInput.GetStartVideoStreamCallback() != NULL) {
            fMediaInput.GetStartVideoStreamCallback()();
        }
        if (kSessionIdList.empty()) {
            fMediaInput.Start(envir());
        }
        LOG("%s:%s:%p - clientSessionId: 0x%08x\n", __FILE__, __func__, this, clientSessionId);
        kSessionIdList.push_back(clientSessionId);
        // kMutex.unlock();
    }
    void H265ServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& streamToken)
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

    static void afterPlayingDummy(void* clientData)
    {
        H265ServerMediaSubsession* subsess = (H265ServerMediaSubsession*)clientData;
        LOG("%s, set done.\n", __func__);
        // Signal the event loop that we're done:
        subsess->afterPlayingDummy1();
    }

    void H265ServerMediaSubsession::afterPlayingDummy1()
    {
        // Unschedule any pending 'checking' task:
        envir().taskScheduler().unscheduleDelayedTask(nextTask());
        // Signal the event loop that we're done:
        setDoneFlag();
    }

    static void checkForAuxSDPLine(void* clientData)
    {
        H265ServerMediaSubsession* subsess = (H265ServerMediaSubsession*)clientData;
        subsess->checkForAuxSDPLine1();
    }

    void H265ServerMediaSubsession::checkForAuxSDPLine1()
    {
        nextTask() = NULL;

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
            if (fGetSdpCount-- < 0) {
                setDoneFlag();
                LOG("%s:%s:%p: get sdp time out.\n", __FILE__, __func__, this);
            } else {
                // try again after a brief delay:
                int uSecsToDelay = 100000; // 100 ms
                LOG_FILE_FUNC_LINE();
                nextTask() =
                    envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
            }
        }
    }

    char const* H265ServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
    {
        // Note: For MPEG-4 video buffer, the 'config' information isn't known
        // until we start reading the Buffer.  This means that "rtpSink"s
        // "auxSDPLine()" will be NULL initially, and we need to start reading
        // data from our buffer until this changes.
        if (fAuxSDPLine != NULL) {
            return fAuxSDPLine;
        }
        if (fDummyRTPSink == NULL) {
            // force I framed
            if (fMediaInput.GetStartVideoStreamCallback() != NULL) {
                fMediaInput.GetStartVideoStreamCallback()();
            }
            fDummyRTPSink = rtpSink;
            fGetSdpCount = 10;
            fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
            checkForAuxSDPLine(this);
        }
        envir().taskScheduler().doEventLoop(&fDoneFlag);
        return fAuxSDPLine;
    }

    FramedSource* H265ServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
    {
        LOG("%s:%s:%p - clientSessionId: 0x%08x\n", __FILE__, __func__, this, clientSessionId);
        estBitrate = fMediaInput.getMaxIdrSize();
        if (estBitrate < fEstimatedKbps) {
            estBitrate = fEstimatedKbps;
        }
        // Create a framer for the Video Elementary Stream:
        FramedSource* source =
            H265VideoStreamDiscreteFramer::createNew(envir(), fMediaInput.videoSource(CODEC_TYPE_H265));
        LOG("H265 framedsource : %p, estBitrate = %u\n", source, estBitrate);
        return source;
    }

    RTPSink* H265ServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                                         FramedSource* inputSource)
    {
        if (!inputSource) {
            LOG("inputSource is not ready, can not create new rtp sink\n");
            return NULL;
        }
        setVideoRTPSinkBufferSize();
        LOG_FILE_FUNC_LINE();
        RTPSink* rtp_sink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
        LOG("H265 rtp sink : %p\n", rtp_sink);
        return rtp_sink;
    }

} // namespace easymedia
