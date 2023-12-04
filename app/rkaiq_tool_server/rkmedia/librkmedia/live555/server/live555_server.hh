// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_LIVE555_SERVER_HH_
#define EASYMEDIA_LIVE555_SERVER_HH_
#include "live555_media_input.hh"
#include <map>
namespace easymedia {
enum CMD_TYPE { NewSession, RemoveSession };
struct message {
  unsigned char cmd_type;
  char channel_name[120];
  char videoType[30];
  char audioType[30];
  int channels;
  int sample_rate;
  unsigned bitrate;
  int profile;
};

class RtspConnection {
public:
  static std::shared_ptr<RtspConnection>
  getInstance(int port, std::string username, std::string userpwd) {
    kMutex.lock();
    if (m_rtspConnection == nullptr) {
      struct make_shared_enabler : public RtspConnection {
        make_shared_enabler(int port, std::string username, std::string userpwd)
            : RtspConnection(port, username, userpwd){};
      };
      m_rtspConnection =
          std::make_shared<make_shared_enabler>(port, username, userpwd);
      if (!init_ok) {
        m_rtspConnection = nullptr;
      }
    }
    kMutex.unlock();
    return m_rtspConnection;
  }
  Live555MediaInput *createNewChannel(std::string channel_name,
                                      std::string video_type,
                                      std::string audio_type, int channels = 0,
                                      int sample_rate = 0, unsigned bitrate = 0,
                                      int profile = 1);
  void removeChannel(std::string channel_name);

  ~RtspConnection();

private:
  static volatile bool init_ok;
  static volatile char out_loop_cond;

  RtspConnection(int port, std::string username, std::string userpwd);

  void service_session_run();
  static void incomingMsgHandler(RtspConnection *rtsp, int mask);
  void incomingMsgHandler1();
  void sendMessage(struct message msg);
  void addSession(struct message msg);
  void removeSession(struct message msg);
  static std::mutex kMutex;
  static std::shared_ptr<RtspConnection> m_rtspConnection;

  TaskScheduler *scheduler;
  UsageEnvironment *env;
  UserAuthenticationDatabase *authDB;
  RTSPServer *rtspServer;
  std::thread *session_thread;
  int msg_fd[2];
  std::map<std::string, Live555MediaInput *> input_map;
  ConditionLockMutex mtx;
  std::mutex lock_msg;
  volatile bool flag;
};

class RKServerMediaSession : public ServerMediaSession {
public:
  static RKServerMediaSession *createNew(UsageEnvironment &env,
                                         char const *streamName,
                                         Live555MediaInput *server_input) {

    time_t t;
    t = time(&t);
    return new RKServerMediaSession(env, streamName, ctime(&t),
                                    "rtsp stream server", False, NULL,
                                    server_input);
  }

protected:
  RKServerMediaSession(UsageEnvironment &env, char const *streamName,
                       char const *info, char const *description, Boolean isSSM,
                       char const *miscSDPLines,
                       Live555MediaInput *server_input)
      : ServerMediaSession(env, streamName, info, description, isSSM,
                           miscSDPLines),
        m_server_input(server_input) {}
  virtual ~RKServerMediaSession() {
    LOG_FILE_FUNC_LINE();
    Medium::close(m_server_input);
  };

private:
  Live555MediaInput *m_server_input;
};

} // namespace easymedia

#endif // #ifndef EASYMEDIA_LIVE555_SERVER_HH_
