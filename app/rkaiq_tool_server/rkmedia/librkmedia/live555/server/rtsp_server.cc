// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flow.h"

#include <time.h>

#include <mutex>

#include <BasicUsageEnvironment/BasicUsageEnvironment.hh>
#ifndef _RTSP_SERVER_HH
    #include <liveMedia/RTSPServer.hh>
#endif

#if !defined(LIVE555_SERVER_H264) && !defined(LIVE555_SERVER_H265)
    #error "This RTSP !VIDEO! implementation currently only support at least one of h264 and h265!!!"
#endif

#ifdef LIVE555_SERVER_H264
    #include "h264_server_media_subsession.hh"
#endif
#ifdef LIVE555_SERVER_H265
    #include "h265_server_media_subsession.hh"
#endif
#include "aac_server_media_subsession.hh"
#include "live555_media_input.hh"
#include "live555_server.hh"
#include "mjpeg_server_media_subsession.hh"
#include "mp2_server_media_subsession.hh"
#include "simple_server_media_subsession.hh"

#include "buffer.h"
#include "codec.h"
#include "media_config.h"
#include "media_reflector.h"
#include "media_type.h"

namespace easymedia
{
    static bool SendMediaToServer(Flow* f, MediaBufferVector& input_vector);
    class RtspServerFlow : public Flow
    {
      public:
        RtspServerFlow(const char* param);
        virtual ~RtspServerFlow();
        static const char* GetFlowName()
        {
            return "live555_rtsp_server";
        }

      private:
        Live555MediaInput* server_input;
        std::shared_ptr<RtspConnection> rtspConnection;

        std::string channel_name;
        std::string video_type;
        std::string audio_type;
        friend bool SendMediaToServer(Flow* f, MediaBufferVector& input_vector);
        void CallPlayVideoHandler();
        void CallPlayAudioHandler();
    };

    bool SendMediaToServer(Flow* f, MediaBufferVector& input_vector)
    {
        RtspServerFlow* rtsp_flow = (RtspServerFlow*)f;

        for (auto& buffer : input_vector) {
            if (!buffer) {
                continue;
            }
            if (buffer && buffer->IsHwBuffer()) {
                // hardware buffer is limited, copy it
                auto new_buffer = MediaBuffer::Clone(*buffer.get());
                new_buffer->SetType(buffer->GetType());
                buffer = new_buffer;
            }

            if ((buffer->GetUserFlag() & MediaBuffer::kIntra)) {
                std::list<std::shared_ptr<easymedia::MediaBuffer>> spspps;
                if (rtsp_flow->video_type == VIDEO_H264) {
                    spspps = split_h264_separate((const uint8_t*)buffer->GetPtr(), buffer->GetValidSize(),
                                                 buffer->GetUSTimeStamp());
                } else if (rtsp_flow->video_type == VIDEO_H265) {
                    spspps = split_h265_separate((const uint8_t*)buffer->GetPtr(), buffer->GetValidSize(),
                                                 buffer->GetUSTimeStamp());
                }
                // Independently send vps, sps, pps packets to live555.
                for (auto& buf : spspps) {
                    rtsp_flow->server_input->PushNewVideo(buf);
                }
                // The original Intr frame information is sent to live555.
                // At this time it still contains extra information.
                rtsp_flow->server_input->PushNewVideo(buffer);
            } else if (buffer->GetType() == Type::Audio) {
                rtsp_flow->server_input->PushNewAudio(buffer);
            } else if (buffer->GetType() == Type::Video) {
                rtsp_flow->server_input->PushNewVideo(buffer);
            } else {
                // muxer buffer
                rtsp_flow->server_input->PushNewMuxer(buffer);
            }
        }

        return true;
    }

    RtspServerFlow::RtspServerFlow(const char* param)
    {
        std::list<std::string> input_data_types;
        std::map<std::string, std::string> params;
        if (!parse_media_param_map(param, params)) {
            SetError(-EINVAL);
            return;
        }
        std::string value;
        CHECK_EMPTY_SETERRNO(value, params, KEY_INPUTDATATYPE, EINVAL)
        parse_media_param_list(value.c_str(), input_data_types, ',');
        CHECK_EMPTY_SETERRNO(channel_name, params, KEY_CHANNEL_NAME, EINVAL)

        value = params[KEY_PORT_NUM];
        int port = std::stoi(value);
        std::string& username = params[KEY_USERNAME];
        std::string& userpwd = params[KEY_USERPASSWORD];
        rtspConnection = RtspConnection::getInstance(port, username, userpwd);
        int sample_rate = 0, channels = 0, profiles = 0;
        unsigned bitrate = 0;
        value = params[KEY_SAMPLE_RATE];
        if (!value.empty()) {
            sample_rate = std::stoi(value);
        }

        value = params[KEY_CHANNELS];
        if (!value.empty()) {
            channels = std::stoi(value);
        }

        value = params[KEY_PROFILE];
        if (!value.empty()) {
            profiles = std::stoi(value);
        }

        value = params[KEY_SAMPLE_FMT];
        if (!value.empty()) {
            bitrate = std::stoi(value);
        }

        if (rtspConnection) {
            int in_idx = 0;
            std::string markname;
            SlotMap sm;

            for (auto& type : input_data_types) {
                if (type == VIDEO_H264 || type == VIDEO_H265 || type == IMAGE_JPEG) {
                    video_type = type;
                } else if (type == AUDIO_AAC || type == AUDIO_G711A || type == AUDIO_G711U || type == AUDIO_G726 ||
                           type == AUDIO_MP2 || type == MUXER_MPEG_TS || type == MUXER_MPEG_PS) {
                    audio_type = type;
                }
                sm.input_slots.push_back(in_idx);
                in_idx++;
            }
            server_input = rtspConnection->createNewChannel(channel_name, video_type, audio_type, channels, sample_rate,
                                                            bitrate, profiles);
            server_input->SetStartVideoStreamCallback(std::bind(&RtspServerFlow::CallPlayVideoHandler, this));
            server_input->SetStartAudioStreamCallback(std::bind(&RtspServerFlow::CallPlayAudioHandler, this));
            sm.process = SendMediaToServer;
            sm.thread_model = Model::ASYNCCOMMON;
            sm.mode_when_full = InputMode::BLOCKING;
            sm.input_maxcachenum.push_back(0); // no limit
            if (sm.input_slots.size() > 1) {
                sm.input_maxcachenum.push_back(0);
            }
            markname = "rtsp " + channel_name + std::to_string(in_idx);
            if (!InstallSlotMap(sm, markname, 0)) {
                LOG("Fail to InstallSlotMap, %s\n", markname.c_str());
                goto err;
            }
        } else {
            goto err;
        }
        return;
    err:
        SetError(-EINVAL);
    }

    void RtspServerFlow::CallPlayVideoHandler()
    {
        auto handler = GetPlayVideoHandler();
        if (handler != nullptr) {
            handler(this);
        }
    }

    void RtspServerFlow::CallPlayAudioHandler()
    {
        auto handler = GetPlayAudioHandler();
        if (handler) {
            handler(this);
        }
    }

    RtspServerFlow::~RtspServerFlow()
    {
        AutoPrintLine apl(__func__);
        StopAllThread();
        SetDisable();
        if (rtspConnection) {
            rtspConnection->removeChannel(channel_name);
        }
        server_input = nullptr;
    }

    DEFINE_FLOW_FACTORY(RtspServerFlow, Flow)
    const char* FACTORY(RtspServerFlow)::ExpectedInputDataType()
    {
        return "";
    }
    const char* FACTORY(RtspServerFlow)::OutPutDataType()
    {
        return "";
    }

} // namespace easymedia
