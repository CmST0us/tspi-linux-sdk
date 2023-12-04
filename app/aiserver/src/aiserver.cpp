// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "aiserver.h"
#include "getopt.h"
#include "logger/log.h"
#include "ai_scene_director.h"
#include "nn_vision_rockx.h"

#define HAVE_SIGNAL_PROC 1

typedef struct _AIServerCtx {
    // minilog flags
    int  mFlagMinilog;
    int  mFlagMinilogBacktrace;
    int  mFlagMinilogLevel;
    // dbus flags
    int  mFlagDBusServer;
    int  mFlagDBusDbServer;
    int  mFlagDBusConn;
    // server flags
    bool mQuit;
    std::string mConfigUri;
} AIServerCtx;

static AIServerCtx mAIServerCtx;
static void        parse_args(int argc, char **argv);

namespace rockchip {
namespace aiserver {

AIServer::AIServer() {
    mAIDirector.reset(nullptr);
    mDbusServer.reset(nullptr);
}

void AIServer::setupTaskGraph() {
    mAIDirector.reset(new AISceneDirector());

    LOG_INFO("AIServer: mFlagDBusServer   = %d\n", mAIServerCtx.mFlagDBusServer);
    LOG_INFO("AIServer: mFlagDBusDbServer = %d\n", mAIServerCtx.mFlagDBusDbServer);
    if (mAIServerCtx.mFlagDBusServer) {
        mDbusServer.reset(new DBusServer(mAIServerCtx.mFlagDBusConn, mAIServerCtx.mFlagDBusDbServer));
        assert(mDbusServer);
        mDbusServer->RegisterMediaControl(mAIDirector.get());
        mDbusServer->start();
        LOG_INFO("AIServer: RegisterMediaControl start ok\n");
    }

    mAIDirector->setup();
}

AIServer::~AIServer() {
    if ((mAIServerCtx.mFlagDBusServer) && (nullptr != mDbusServer)) {
        mDbusServer->stop();
    }
    mAIDirector.reset();
    mDbusServer.reset();
}

void AIServer::interrupt() {
    mAIDirector->stop(RT_APP_NN);
    mAIDirector->stop(RT_APP_UVC);
    mAIDirector->interrupt();
}

void AIServer::waitUntilDone() {
    mAIDirector->waitUntilDone();
}

} // namespace aiserver
} // namespace rockchip


//=======================================================================//
//=========================AIServer Main Program=========================//
//=======================================================================//
using namespace rockchip::aiserver;
static AIServer* mAIServerInstance = nullptr;

static void sigterm_handler(int sig) {
    LOG_WARN("quit signal(%d) is caught\n", sig);
    mAIServerCtx.mQuit = true;
    if (nullptr != mAIServerInstance) {
        mAIServerInstance->interrupt();
    }
}

int main(int argc, char *argv[]) {
    mAIServerCtx.mQuit             = false;
    mAIServerCtx.mFlagDBusServer   = true;
    mAIServerCtx.mFlagDBusDbServer = false;
    mAIServerCtx.mFlagDBusConn     = false;

    parse_args(argc, argv);

    LOG_INFO("parse_args done!\n");

    // __minilog_log_init(argv[0], NULL, false, mAIServerCtx.mFlagMinilogBacktrace,
    //                   argv[0], "1.0.0");

    // install signal handlers.
#if HAVE_SIGNAL_PROC
    signal(SIGINT,  sigterm_handler);  // SIGINT  = 2
    signal(SIGQUIT, sigterm_handler);  // SIGQUIT = 3
    signal(SIGTERM, sigterm_handler);  // SIGTERM = 15
    signal(SIGXCPU, sigterm_handler);  // SIGXCPU = 24
    signal(SIGPIPE, SIG_IGN);          // SIGPIPE = 13 is ingnored
#endif

    mAIServerInstance = new AIServer();
    LOG_INFO("create aiserver instance done\n");

    // rt_mem_record_reset();
    mAIServerInstance->setupTaskGraph();
    LOG_INFO("aiserver->setupTaskGraph(); done\n");

    while (!mAIServerCtx.mQuit) {
        usleep(2000000ll);
    }

    mAIServerInstance->waitUntilDone();
    LOG_INFO("aiserver->waitUntilDone(); done\n");

    delete mAIServerInstance;
    mAIServerInstance = nullptr;

    rt_mem_record_dump();
    return 0;
}

static void usage_tip(FILE *fp, int argc, char **argv) {
  fprintf(fp, "Usage: %s [options]\n"
              "Version %s\n"
              "Options:\n"
              "-c | --config      AIServer confg file \n"
              "-o | --dbus_conn   0:system,  1:session \n"
              "-d | --dbus_db     0:disable, 1:enable \n"
              "-s | --dbus_server 0:disable, 1:enable \n"
              "-h | --help        For help \n"
              "\n",
          argv[0], "V1.1");
}

static const char short_options[] = "c:odsh";
static const struct option long_options[] = {
    {"ai_config",   required_argument, NULL, 'c'},
    {"dbus_conn",   optional_argument, 0,    'o'},
    {"dbus_db",     optional_argument, 0,    'd'},
    {"dbus_server", optional_argument, 0,    's'},
    {"help",        no_argument,       0,    'h'},
    {0, 0, 0, 0}
};

static void parse_args(int argc, char **argv) {
    int opt;
    int idx;
    while ((opt = getopt_long(argc, argv, short_options, long_options, &idx))!= -1) {
      switch (opt) {
        case 0: /* getopt_long() flag */
          break;
        case 'c':
          mAIServerCtx.mConfigUri = optarg;
          break;
        case 'o':
          mAIServerCtx.mFlagDBusConn  = atoi(argv[optind]);
          break;
        case 'd':
          mAIServerCtx.mFlagDBusDbServer = atoi(argv[optind]);
          break;
        case 's':
          mAIServerCtx.mFlagDBusServer = atoi(argv[optind]);
          break;
        case 'h':
          usage_tip(stdout, argc, argv);
          exit(EXIT_SUCCESS);
        default:
          usage_tip(stderr, argc, argv);
          exit(EXIT_FAILURE);
      }
    }
}

