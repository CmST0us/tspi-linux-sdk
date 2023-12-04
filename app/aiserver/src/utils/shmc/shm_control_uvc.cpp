// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/prctl.h>
#include <sys/time.h>

#include "shm_control_uvc.h"
#include "logger/log.h"
#include "drm_helper.h"
#include "RTVideoFrame.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "shm_control_uvc"

namespace rockchip {
namespace aiserver {

void ProcessRecvUVCMessage(void *opaque) {
    prctl(PR_SET_NAME, "aiuvc_recv_msg_thread");
    ShmUVCController* controller = (ShmUVCController *)opaque;
    controller->recvUVCMessageLoop();
}

ShmUVCController::ShmUVCController() {
    initialize();
}

ShmUVCController::~ShmUVCController() {
#if UVC_DYNAMIC_DEBUG
    debugLooping = false;
    if (debugThread)
    {
        debugThread->join();
        delete debugThread;
        debugThread = nullptr;
    }
#endif
    if (drmFd >= 0) {
        drm_close(drmFd);
        drmFd = -1;
    }
}

#if UVC_DYNAMIC_DEBUG
void ProcessDebug(void *opaque)
{
    prctl(PR_SET_NAME, "shm_uvc_debug_thread");
    ShmUVCController *controller = (ShmUVCController *)opaque;
    controller->debugLoop();
}

void ShmUVCController::debugLoop()
{
    LOG_INFO("enter\n");
    while (debugLooping)
    {
        sleep(1);
        if (!access(UVC_IPC_DYNAMIC_DEBUG_STATE, 0))
        {
            LOG_INFO("send_seq:%d, recv_seq:%d send_count:%d, recv_count:%d\n",
                      send_seq, recv_seq, send_count, recv_count);
        }
    }
    LOG_INFO("exit \n");
}
#endif

void ShmUVCController::initialize() {
    recvLooping = false;
    uvcRunning  = false;
    cameraWidth = -1;
    cameraHeight = -1;
    bool shmRet = false;
#ifdef ENABLE_SHM_SERVER
    shmc::SetLogHandler(shmc::kDebug, [](shmc::LogLevel lv, const char *s) {
        LOG_INFO("[%d] %s\n", lv, s);
    });
    shmRet = shmWriteQueue.InitForWrite(kShmUVCWriteKey, kUVCQueueBufSize);
    LOG_INFO("shmWriteQueue InitForWrite(ret=%d)\n", shmRet);

    shmRet = shmReadWriteQueue.InitForWrite(kShmUVCReadKey, kUVCQueueBufSize);
    shmRet = shmReadQueue.InitForRead(kShmUVCReadKey);
    LOG_INFO("shmReadQueue InitForRead(ret=%d)\n", shmRet);
#endif
#if UVC_DYNAMIC_DEBUG
    debugLooping = true;
    debugThread = new std::thread(ProcessDebug, this);
#endif

    drmFd = drm_open();
}

void ShmUVCController::setControlListener(RTGraphListener* listener) {
    graphListener = listener;
}

void ShmUVCController::reset() {
    int ret = 0;
    int length = 0;
    std::string msg;

    do {
        std::lock_guard<std::mutex> lock(readQueueMtx);
        ret = shmReadQueue.Pop(&msg);
        length = msg.length();
        LOG_INFO("stop recv message(ret=%d, length=%d)\n", ret, length);
        msg.clear();
    } while(ret && length > 0);
    //queue_r_.Reset();
}

void ShmUVCController::startRecvMessage() {
    recvLooping = true;
    recvThread = new std::thread(ProcessRecvUVCMessage, this);
}

void ShmUVCController::stopRecvMessage()
{
    recvLooping = false;
    if (recvThread) {
        recvThread->join();
        delete recvThread;
        recvThread = nullptr;
    }

    reset();
}

void ShmUVCController::recvUVCMessageLoop() {
    int ret = 0;
    while (recvLooping) {
        std::string msg;

        do {
            std::lock_guard<std::mutex> lock(readQueueMtx);
            ret = shmReadQueue.Pop(&msg);
        } while (0);

        if (ret) {
            LOG_DEBUG("recv uvc message = %s\n", msg.c_str());
            handleUVCMessage(msg);
            msg.clear();
        } else {
            usleep(5*1000);
        }
    }
    LOG_INFO("recv uvc message thread end\n");
}

void ShmUVCController::handleUVCMessage(std::string &msg) {
    int msgType = 0;
    UVCMessage message;

    message.ParseFromString(msg);
    msgType = message.msg_type();

    switch(msgType) {
      case MSG_UVC_START: {
        doStartUVC();
        break;
      }
      case MSG_UVC_STOP: {
        doStopUVC();
        break;
      }
      case MSG_UVC_ENABLE_ETPTZ: {
        MethodParams methodParams = message.method_params();
        int enabled = methodParams.i32_p();
        graphListener->setEPTZ(AI_UVC_EPTZ_AUTO, enabled);
        break;
      }
      case MSG_UVC_SET_ZOOM: {
        MethodParams methodParams = message.method_params();
        float zoomVal = methodParams.flo_p();
        graphListener->setZoom(zoomVal);
        break;
      }
      case MSG_UVC_TRANSPORT_BUF: {
        MediaBufferInfo bufferInfo = message.buffer_info();
        doRecvUVCBuffer(&bufferInfo);
        LOG_DEBUG("recv uvc buffer message\n");
        break;
      }
      case MSG_UVC_CONFIG_CAMERA: {
        StreamInfo streamInfo = message.stream_info();
        doUpdateCameraParams(&streamInfo);
        break;
      }
      case MSG_UVC_SET_EPTZ_PAN: {
        MethodParams methodParams = message.method_params();
        int pan = methodParams.i32_p();
        graphListener->setEPTZ(AI_UVC_EPTZ_PAN, pan);
        break;
      }
      case MSG_UVC_SET_EPTZ_TILT: {
        MethodParams methodParams = message.method_params();
        int tilt = methodParams.i32_p();
        graphListener->setEPTZ(AI_UVC_EPTZ_TILT, tilt);
        break;
      }
      case MSG_UVC_ENABLE_BYPASS: {
        MethodParams methodParams = message.method_params();
        int enabled = methodParams.i32_p();
        graphListener->setEPTZ(AI_UVC_BYPASS_LINK, enabled);
        break;
      }
      default:
        LOG_ERROR("recv uvc unknown message\n");
        break;
    }
}

void ShmUVCController::doStartUVC() {
    if (graphListener != nullptr) {
        graphListener->start(RT_APP_UVC);
    }
    std::lock_guard<std::mutex> lock(opMutex);
    uvcRunning = true;
#if UVC_DYNAMIC_DEBUG
    send_count = 0;
    recv_count = 0;
#endif
}

void ShmUVCController::doStopUVC() {
    {
        std::lock_guard<std::mutex> lock(opMutex);
        uvcRunning = false;
        LOG_INFO("clear uvc buffer after stopped\n");
        clearUVCBuffer();
        LOG_INFO("clear uvc buffer ok\n");
    }

    if (graphListener != nullptr) {
        graphListener->stop(RT_APP_UVC);
    }
}

void ShmUVCController::doRecvUVCBuffer(MediaBufferInfo* bufferInfo) {
    std::lock_guard<std::mutex> lock(opMutex);
    int32_t uniqueId = bufferInfo->id();
    int64_t privData = bufferInfo->priv_data();
    recv_seq = bufferInfo->seq();
    recv_count ++;

    RTMediaBuffer* mediaBuffer = (RTMediaBuffer *)privData;

    int32_t found = 0;
    std::list<RTMediaBuffer *>::iterator it;
    for (it = bufList.begin(); it != bufList.end();) {
        RTMediaBuffer *buf = *it;
        if (buf == mediaBuffer) {
            it = bufList.erase(it);
            mediaBuffer->release();
            found = 1;
            break;
        } else {
            it++;
        }
    }

    if (found) {
        LOG_DEBUG("recv uvc buffer uniqueId %d, buffer 0x%llx\n", uniqueId, privData);
    } else {
        LOG_ERROR("recv invalid uvc buffer 0x%llx, seq:%d\n", privData, recv_seq);
    }
}

void ShmUVCController::doUpdateCameraParams(StreamInfo* streamInfo) {
    int forceClear = 0;
    if (uvcRunning && (cameraWidth != streamInfo->width() ||
        cameraHeight != streamInfo->height())) {
        LOG_ERROR("config camera in unexpected state\n");
        uvcRunning = false;
        forceClear = 1;
        std::lock_guard<std::mutex> lock(opMutex);
        clearUVCBuffer();
    }

    RtMetaData cameraParams;
    cameraParams.setInt32("opt_width",        streamInfo->width());
    cameraParams.setInt32("opt_height",       streamInfo->height());
    cameraParams.setInt32("opt_vir_width",    streamInfo->vir_width());
    cameraParams.setInt32("opt_vir_height",   streamInfo->vir_height());
    cameraParams.setInt32("node_buff_size",   streamInfo->buf_size());
    cameraParams.setInt32("opt_quantization", streamInfo->range());
    cameraWidth = streamInfo->width();
    cameraHeight = streamInfo->height();
    graphListener->invoke(RT_APP_UVC, RT_ACTION_CONFIG_CAMERA, &cameraParams);

    if (forceClear) {
        uvcRunning = true;
    }
}

void ShmUVCController::clearUVCBuffer() {
    std::list<RTMediaBuffer *>::iterator it;
    for (it = bufList.begin(); it != bufList.end();) {
        RTMediaBuffer *buffer = *it;
        it = bufList.erase(it);
        if (buffer == nullptr) {
            LOG_ERROR("uvc buffer null pointer\n");
        } else {
            LOG_ERROR("force release uniqueId %d, buffer 0x%x\n", buffer->getUniqueID(), buffer);
            buffer->release();
        }
    }
}

void ShmUVCController::sendUVCBuffer(RTMediaBuffer* buffer) {
    int64_t pts = 0;
    int32_t seq = 0;
    std::string sendbuf;
    RTVideoFrame *pVFrame = reinterpret_vframe(buffer);
    UVCMessage message;
    if (buffer == nullptr) {
        return;
    }
    if (pVFrame == nullptr) {
        LOG_ERROR("buffer type is not video frame. release it.");
        buffer->release();
        return;
    }

    MediaBufferInfo *bufferInfo = new MediaBufferInfo();
    message.set_allocated_buffer_info(bufferInfo);
    //MediaBufferInfo bufferInfo = message.buffer_info();

    bufferInfo->set_id(pVFrame->getUniqueID());
    bufferInfo->set_size(pVFrame->getSize());
    bufferInfo->set_fd(pVFrame->getFd());
    bufferInfo->set_handle(pVFrame->getHandle());
    bufferInfo->set_pts(pVFrame->getPts());
    bufferInfo->set_data((int64_t)pVFrame->getData());
    bufferInfo->set_priv_data((int64_t)pVFrame);
    bufferInfo->set_seq(pVFrame->getSeq());
    message.set_msg_type(MSG_UVC_TRANSPORT_BUF);
    message.set_msg_name("uvcbuffer");
    message.SerializeToString(&sendbuf);
    message.ParseFromString(sendbuf);

    std::lock_guard<std::mutex> lock(opMutex);
    if (!uvcRunning) {
        buffer->release();
        return;
    }
    bufList.push_back(buffer);
    shmWriteQueue.Push(sendbuf);

#if UVC_DYNAMIC_DEBUG
    send_seq = pVFrame->getSeq();
    send_count ++;
    if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0)) {
        int32_t use_time_us, now_time_us;
        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us
        use_time_us = now_time_us - pVFrame->getPts();
        LOG_ERROR("isp->aiserver seq:%d latency time:%d us, %d ms\n", send_seq, use_time_us, use_time_us / 1000);
    }

    if (!access(UVC_DYNAMIC_DEBUG_IPC_BUFFER_CHECK, 0)) {
        LOG_ERROR("send uvc buffer uniqueId %d, buffer 0x%llx\n", buffer->getUniqueID(), (int64_t)buffer);
    }
#endif

}

}
}
