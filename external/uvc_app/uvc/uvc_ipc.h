/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __UVC_IPC_H__
#define __UVC_IPC_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "uvc_ipc_ext.h"
#include <stddef.h>
#include <pthread.h>

extern void uvc_read_camera_buffer(void *cam_buf, struct MPP_ENC_INFO *info,
                                   void *extra_data, size_t extra_size);
#ifdef __cplusplus
}
#endif

#include <shmc/shm_queue.h>
#include <stdint.h>
#include "uvc_data.pb.h"
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <string>
using namespace std;


//#include "rockit/RTMediaBuffer.h"

namespace
{
constexpr const char *kShmUVCWriteKey  = "0x20002";//"0x10022";
constexpr const char *kShmUVCReadKey   = "0x20001";//"0x10021";
constexpr size_t      kQueueBufSize    = 1024 * 1024 * 0.5;
} // namespace
using namespace shmc;
//typedef void* (*UVCBufferRecvCB)(RTMediaBuffer *);

#if DBUG
#define UVC_IPC_DYNAMIC_DEBUG_ON 1 //release version can set to 0
#else
#define UVC_IPC_DYNAMIC_DEBUG_ON 0
#endif

#define UVC_IPC_DYNAMIC_DEBUG_FPS "/tmp/uvc_ipc_fps"
#define UVC_IPC_DYNAMIC_DEBUG_STATE "/tmp/uvc_ipc_state"
#define UVC_IPC_DYNAMIC_DEBUG_EPTZ "/tmp/uvc_ipc_eptz"
#define UVC_IPC_DYNAMIC_DEBUG_ISP_FPS "/tmp/uvc_isp_fps"
#define UVC_IPC_DYNAMIC_DEBUG_ISP_STATE "/tmp/uvc_isp_state"

#define UVC_SENDBUFF_USE_INIT_ALLOC 1
#define UVC_STREAM_OFF_NOT_SEND_SETTING 1
#define UVC_ABANDON_FRM_COUNT 0

namespace ShmControl
{

enum ShmUVCMessageType
{
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

enum UVC_IPC_RECV_STATE
{
    UVC_IPC_STATE_RECV_ENTER = 1,
    UVC_IPC_STATE_RECV_ENC_ENTER = 2,
    UVC_IPC_STATE_RECV_ENC_EXIT  = 3,
    UVC_IPC_STATE_RECV_EXIT = 4,
};
enum UVC_IPC_SEND_STATE
{
    UVC_IPC_STATE_SEND_ENTER = 1,
    UVC_IPC_STATE_SEND_ENTER_UNLOCK = 2,
    UVC_IPC_STATE_SEND_READY = 3,
    UVC_IPC_STATE_SEND_EXIT = 4,
};

string shm_meg_name[] = {
    "uvc_start",
    "uvc_stop",
    "uvc_enable_eptz",
    "uvc_set_zoom",
    "uvc_buf",
    "uvc_cfg_camera",
    "uvc_set_eptz",
};

struct SendBufferInfo
{
    int32_t id;
    int32_t size;
    int32_t fd;
    int32_t handle;
    int64_t data;
    int64_t priv_data;
    int32_t seq;
};

struct DrmBuffMap
{
    char *buf;
    int32_t id;
//    int32_t fd;
    int32_t size;
    int32_t handle;
    int buf_fd;
};

class ShmUVCController
{
public:
    ShmUVCController();
    ~ShmUVCController();

    void ShmUVCDrmRelease();
    void sendUVCBuffer(enum ShmUVCMessageType event, void *data);
    void startRecvMessage();
    void clearRecvMessage();
    void stopRecvMessage();
    void recvUVCMessageLoop();
#if UVC_IPC_DYNAMIC_DEBUG_ON
    void uvcIPCDebugLoop();
#endif
private:
    void initialize();
    void handleUVCMessage(std::string &msg);
    void recvUVCBuffer(MediaBufferInfo *bufferInfo);

private:
    ShmQueue<shmc::SVIPC> shmWriteQueue;
    ShmQueue<shmc::SVIPC> shmWriteReadQueue;
    ShmQueue<shmc::SVIPC> shmReadQueue;
    std::mutex            wQueueMtx;
    std::mutex            rQueueMtx;
    std::thread          *recvThread;
#if UVC_IPC_DYNAMIC_DEBUG_ON
    std::thread          *debugThread;
    bool                 debugLooping;
#endif
    map<int, DrmBuffMap>  drm_info;
    bool                  recvLooping;
    int32_t               drmFd;
    int32_t               width;
    int32_t               height;
    int32_t               nv12_size;
    int32_t               frm;
    int32_t               isp_frm;
    float                 fps;
#if UVC_IPC_DYNAMIC_DEBUG_ON
    struct timeval        ipc_enter_time;
    struct timeval        isp_enter_time;
#endif
    UVCMessage            send_message;
    UVC_IPC_SEND_STATE    send_state;
    UVC_IPC_RECV_STATE    recv_state;
    int32_t               norecv_count;
    int32_t               norecv_err_count;
    int32_t               abandon_count;
    enum UVC_IPC_ENC_TYPE encode_type;
    int32_t               recv_seq;
    int32_t               send_seq;
    int32_t               recv_count;
    int32_t               send_count;
    int32_t               isp_buf_count;
    int32_t               set_buf_count;
    int32_t               uvc_fps_set;
};

struct UVC_IPC_INFO
{
    ShmUVCController *shm_control;
    pthread_mutex_t mutex;
    bool start;
    bool stop;
    bool history[4];
    int enable_eptz;
    int zoom;
    int eptz_pan;
    int eptz_tilt;
    struct CAMERA_INFO camera;
};

} // namespace ShmControl

#endif
