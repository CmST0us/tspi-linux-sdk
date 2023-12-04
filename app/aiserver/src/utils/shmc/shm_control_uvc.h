// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHM_CONTROL_UVC_H_
#define SHM_CONTROL_UVC_H_

#include <mutex>
#include <shmc/shm_queue.h>
#include <stdint.h>
#include <thread>
#include <vector>
#include <uvc_data.pb.h>
#include "RTMediaBuffer.h"
#include "rt_metadata.h"
#include "dbus_graph_control.h"

#define UVC_DYNAMIC_DEBUG 1 //release version can set to 0
#define UVC_DYNAMIC_DEBUG_USE_TIME_CHECK   "/tmp/uvc_use_time"
#define UVC_DYNAMIC_DEBUG_IPC_BUFFER_CHECK "/tmp/uvc_ipc_buffer"
#define UVC_IPC_DYNAMIC_DEBUG_STATE        "/tmp/uvc_ipc_state"

namespace {
constexpr const char *kShmUVCWriteKey  = "0x20001";
constexpr const char *kShmUVCReadKey   = "0x20002";
constexpr size_t      kUVCQueueBufSize = 1024 * 1024 * 0.5;
} // namespace
using namespace shmc;

typedef void* (*UVCBufferRecvCB)(RTMediaBuffer *);

namespace rockchip {
namespace aiserver {

enum ShmUVCMessageType {
    MSG_UVC_START   = 1,
    MSG_UVC_STOP    = 2,
    MSG_UVC_ENABLE_ETPTZ  = 3,
    MSG_UVC_SET_ZOOM      = 4,
    MSG_UVC_TRANSPORT_BUF = 5,
    MSG_UVC_CONFIG_CAMERA = 6,
    MSG_UVC_SET_EPTZ_PAN = 7,
    MSG_UVC_SET_EPTZ_TILT = 8,
    MSG_UVC_ENABLE_BYPASS  = 9,
};

class ShmUVCController {
  public:
    ShmUVCController();
   ~ShmUVCController();

    void initialize();
    void reset();
    void setControlListener(RTGraphListener* listener);
    void sendUVCBuffer(RTMediaBuffer* buffer);
    void startRecvMessage();
    void stopRecvMessage();
    void recvUVCMessageLoop();
#if UVC_DYNAMIC_DEBUG
    void debugLoop();
#endif

  private:
    void handleUVCMessage(std::string &msg);
    void doStartUVC();
    void doStopUVC();
    void doRecvUVCBuffer(MediaBufferInfo* bufferInfo);
    void doUpdateCameraParams(StreamInfo* streamInfo);
    void clearUVCBuffer();

  private:
    RTGraphListener      *graphListener;
    ShmQueue<shmc::SVIPC> shmWriteQueue;
    ShmQueue<shmc::SVIPC> shmReadWriteQueue;
    ShmQueue<shmc::SVIPC> shmReadQueue;
    std::list<RTMediaBuffer *> bufList;
    std::mutex            readQueueMtx;
    std::mutex            opMutex;
    std::thread          *recvThread;
    bool                  recvLooping;
    bool                  uvcRunning;
    int32_t               drmFd;
    int32_t               cameraWidth;
    int32_t               cameraHeight;
#if UVC_DYNAMIC_DEBUG
    std::thread          *debugThread;
    bool                 debugLooping;
    int32_t              recv_seq;
    int32_t              send_seq;
    int32_t              recv_count;
    int32_t              send_count;
#endif

};
} // namespace aiserver
} // namespace rockchip

#endif // SHM_CONTROL_UVC_H_
