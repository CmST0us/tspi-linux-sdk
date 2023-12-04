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
#include "mjpeg_server_media_subsession.hh"
#include "mp2_server_media_subsession.hh"
#include "simple_server_media_subsession.hh"

#include "buffer.h"
#include "codec.h"
#include "live555_server.hh"
#include "media_config.h"
#include "media_reflector.h"
#include "media_type.h"

#include <assert.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <unistd.h>

namespace easymedia
{

    std::mutex RtspConnection::kMutex;
    std::shared_ptr<RtspConnection> RtspConnection::m_rtspConnection = nullptr;
    volatile bool RtspConnection::init_ok = false;
    volatile char RtspConnection::out_loop_cond = 1;

    RtspConnection::RtspConnection(int port, std::string username, std::string userpwd)
        : scheduler(nullptr), env(nullptr), authDB(nullptr), rtspServer(nullptr), session_thread(nullptr)
    {
        if (!username.empty() && !userpwd.empty()) {
            authDB = new UserAuthenticationDatabase;
            if (!authDB) {
                goto err;
            }
            authDB->addUserRecord(username.c_str(), userpwd.c_str());
        }
        scheduler = BasicTaskScheduler::createNew();
        if (!scheduler) {
            goto err;
        }
        env = BasicUsageEnvironment::createNew(*scheduler);
        if (!env) {
            goto err;
        }

        rtspServer = RTSPServer::createNew(*env, port, authDB, 10);

        if (!rtspServer) {
            goto err;
        }

        if (pipe2(msg_fd, O_CLOEXEC)) {
            LOG("create msg_fd error.\n");
            goto err;
        }

        out_loop_cond = 0;
        session_thread = new std::thread(&RtspConnection::service_session_run, this);
        if (!session_thread) {
            LOG_NO_MEMORY();
            goto err;
        }
        init_ok = true;
        return;
    err:
        LOG("=============== RtspConnection error. =================\n");
        init_ok = false;
    }

    void RtspConnection::service_session_run()
    {
        AutoPrintLine apl(__func__);
        LOG("================ service_session_run =================\n");
        prctl(PR_SET_NAME, "live555_server");
        env->taskScheduler().turnOnBackgroundReadHandling(
            msg_fd[0], (TaskScheduler::BackgroundHandlerProc*)&incomingMsgHandler, this);
        env->taskScheduler().doEventLoop(&out_loop_cond);
    }

    Live555MediaInput* RtspConnection::createNewChannel(std::string channel_name, std::string video_type,
                                                        std::string audio_type, int channels, int sample_rate,
                                                        unsigned bitrate, int profile)
    {
        struct message msg;
        msg.cmd_type = CMD_TYPE::NewSession;
        strcpy(msg.channel_name, channel_name.c_str());
        strcpy(msg.videoType, video_type.c_str());
        strcpy(msg.audioType, audio_type.c_str());
        msg.channels = channels;
        msg.sample_rate = sample_rate;
        msg.bitrate = bitrate;
        msg.profile = profile;
        sendMessage(msg);
        auto search = input_map.find(channel_name);
        if (search != input_map.end()) {
            return search->second;
        }
        return nullptr;
    }

    void RtspConnection::removeChannel(std::string channel_name)
    {
        struct message msg;
        msg.cmd_type = CMD_TYPE::RemoveSession;
        strcpy(msg.channel_name, channel_name.c_str());
        sendMessage(msg);
    }

    void RtspConnection::incomingMsgHandler(RtspConnection* rtsp, int)
    {
        rtsp->incomingMsgHandler1();
    }

    void RtspConnection::incomingMsgHandler1()
    {
        struct message msg;
        ssize_t count = read(msg_fd[0], &msg, sizeof(msg));
        if (count < 0) {
            LOG("incomingMsgHandler1 read failed\n");
            return;
        }
        switch (msg.cmd_type) {
            case CMD_TYPE::NewSession:
                addSession(msg);
                break;
            case CMD_TYPE::RemoveSession:
                removeSession(msg);
                break;
            default:
                LOG_FILE_FUNC_LINE();
                LOG("===== message error type====.\n");
                break;
        }

        LOG("%s: before mtx.notify\n", __func__);
        mtx.lock();
        flag = false;
        mtx.notify();
        mtx.unlock();
        LOG("%s: after mtx.notify\n", __func__);
    }

    void RtspConnection::addSession(struct message msg)
    {
        // 1. server_input
        Live555MediaInput* server_input = Live555MediaInput::createNew(*env);
        auto search = input_map.find(msg.channel_name);
        if (search != input_map.end()) {
            LOG("%s:%s:: input_map, %s already exists, so we have to delete it.\n", __FILE__, __func__,
                msg.channel_name);
            input_map.erase(msg.channel_name);
        }

        input_map.insert(std::pair<std::string, Live555MediaInput*>(msg.channel_name, server_input));
        time_t t;
        t = time(&t);
        ServerMediaSession* sms = RKServerMediaSession::createNew(*(env), msg.channel_name, server_input);

        if (rtspServer != nullptr && sms != nullptr) {
            char* url = nullptr;
            rtspServer->addServerMediaSession(sms);
            url = rtspServer->rtspURL(sms);
            *env << "Play this stream using the URL:\n\t" << url << "\n";
            if (url) {
                delete[] url;
            }
        }

        // video
        ServerMediaSubsession* subsession = nullptr;
        if (strcmp(msg.videoType, VIDEO_H264) == 0) {
            subsession = H264ServerMediaSubsession::createNew(*env, *server_input);
        } else if (strcmp(msg.videoType, VIDEO_H265) == 0) {
#ifdef LIVE555_SERVER_H265
            subsession = H265ServerMediaSubsession::createNew(*env, *server_input);
#endif
        } else if (strcmp(msg.videoType, IMAGE_JPEG) == 0) {
            subsession = MJPEGServerMediaSubsession::createNew(*env, *server_input);
        } else {
            LOG(" %s : no video. videoType = %s \n", __func__, msg.videoType);
        }
        if (subsession) {
            sms->addSubsession(subsession);
        }

        // audio or muxer MUXER_MPEG_TS
        if (strcmp(msg.audioType, AUDIO_AAC) == 0) {
            subsession =
                AACServerMediaSubsession::createNew(*env, *server_input, msg.sample_rate, msg.channels, msg.profile);
        } else if (strcmp(msg.audioType, AUDIO_MP2) == 0) {
            subsession = MP2ServerMediaSubsession::createNew(*env, *server_input);
        } else if (strcmp(msg.audioType, AUDIO_G711A) == 0 || strcmp(msg.audioType, AUDIO_G711U) == 0 ||
                   strcmp(msg.audioType, AUDIO_G726) == 0 || strcmp(msg.audioType, MUXER_MPEG_TS) == 0 ||
                   strcmp(msg.audioType, MUXER_MPEG_PS) == 0) {
            subsession = SIMPLEServerMediaSubsession::createNew(*env, *server_input, msg.sample_rate, msg.channels,
                                                                msg.audioType, msg.bitrate);
        } else {
            LOG(" %s : no audio. audioType = %s \n", __func__, msg.audioType);
        }
        if (subsession) {
            sms->addSubsession(subsession);
        }
    }

    void RtspConnection::removeSession(struct message msg)
    {
        if (rtspServer != nullptr) {
            rtspServer->deleteServerMediaSession(msg.channel_name);
            input_map.erase(msg.channel_name);
            LOG("RtspConnection delete %s.\n", msg.channel_name);
        }
    }
    void RtspConnection::sendMessage(struct message msg)
    {
        lock_msg.lock();
        mtx.lock();
        flag = true;
        ssize_t count = write(msg_fd[1], (void*)&msg, sizeof(msg));
        if (count < 0) {
            LOG("%s: write filed %s\n", __func__, strerror(errno));
        }
        LOG("%s: before mtx.wait.\n", __func__);
        while (flag) {
            mtx.wait();
        }
        mtx.unlock();
        lock_msg.unlock();
        LOG("%s: after mtx.wait.\n", __func__);
    }

    RtspConnection::~RtspConnection()
    {
        out_loop_cond = 1;
        if (msg_fd[0] > 0) {
            env->taskScheduler().turnOffBackgroundReadHandling(msg_fd[0]);
        }
        if (msg_fd[0] >= 0) {
            ::close(msg_fd[0]);
            msg_fd[0] = -1;
        }
        if (msg_fd[1] >= 0) {
            ::close(msg_fd[1]);
            msg_fd[1] = -1;
        }
        if (session_thread) {
            session_thread->join();
            delete session_thread;
            session_thread = nullptr;
        }
        if (rtspServer) {
            // will also reclaim ServerMediaSession and ServerMediaSubsessions
            Medium::close(rtspServer);
            rtspServer = nullptr;
        }
        if (authDB) {
            delete authDB;
            authDB = nullptr;
        }
        if (env && env->reclaim() == True) {
            env = nullptr;
        }
        if (scheduler) {
            delete scheduler;
            scheduler = nullptr;
        }
    }

} // namespace easymedia
