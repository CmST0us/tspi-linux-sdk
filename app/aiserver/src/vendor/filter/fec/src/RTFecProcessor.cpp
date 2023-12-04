/*
 * Copyright 2021 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RTGlpssProcessor"

#ifdef DEBUG_FLAG
#undef DEBUG_FLAG
#endif
#define DEBUG_FLAG 0x0

#include "RTFecProcessor.h"

#include <functional>
#include <unistd.h>

#include "rt_log.h"

// commands used by integration(player) layer.

void RTFECProcessor::rcvMessageThread() {
  if (mFecUnit)
    mFecUnit->distortionInit(3840, 2160);
  while (looping) {
    if (mWorkQueue.size()) {
      mtx.lock();
      RTFECInfo info = mWorkQueue.front();
      mWorkQueue.pop();
      mtx.unlock();
      int fencefd = -1;
      int format = 0;
      info.inputBuffer->getMetaData()->findInt32(OPT_FILTER_WIDTH, &info.inW);
      info.inputBuffer->getMetaData()->findInt32(OPT_FILTER_HEIGHT, &info.inH);
      info.inputBuffer->getMetaData()->findInt32(kKeyCodecFormat, &format);
      info.outW = info.inW;
      info.outH = info.inH;

      info.outputBuffer->getMetaData()->setInt32(OPT_FILTER_WIDTH, info.inW);
      info.outputBuffer->getMetaData()->setInt32(OPT_FILTER_HEIGHT, info.inH);
      info.outputBuffer->getMetaData()->setCString(OPT_STREAM_FMT_IN,
                                                   "image:nv12");
      info.outputBuffer->getMetaData()->setInt32(OPT_VIDEO_PIX_FORMAT, format);
      info.outputBuffer->getMetaData()->setInt32(kKeyFrameW, info.inW);
      info.outputBuffer->getMetaData()->setInt32(kKeyFrameH, info.inH);
      mFecUnit->distortionInit(info.inW, info.inH);
      mFecUnit->doFecProcess(info.inW, info.inH, info.inputBuffer->getFd(),
                             info.inFormat, info.outW, info.outH,
                             info.outputBuffer->getFd(), info.outFormat,
                             fencefd);
      info.context->queueOutputBuffer(info.outputBuffer);
      RT_LOGV("in: %d %d out: %d %d", info.inW, info.inH, info.outW, info.outH);
#ifdef DEBUG_FEC
      int seq;
      info.outputBuffer->getMetaData()->findInt32(kKeyFrameSequence, &seq);
      RT_LOGE("queueOutputBuffer seq:%d buffer uniqueId %d, buffer 0x%llx\n",
              seq, info.outputBuffer->getUniqueID(),
              (int64_t)info.outputBuffer);
#endif
      info.inputBuffer->release();
      info.mtx->lock();
      info.cv->notify_all();
      info.mtx->unlock();
    } else {
      usleep(10 * 1000);
    }
  }
  return;
}

RTFECProcessor::RTFECProcessor() {
  looping = true;
  mFecUnit = std::make_shared<RKISP2FecUnit>();
  msgThread =
      new std::thread(std::bind(&RTFECProcessor::rcvMessageThread, this));
}

RTFECProcessor::~RTFECProcessor() {
  if (msgThread) {
    looping = false;
    usleep(10*1000);
    msgThread->join();
    delete msgThread;
  }
  if (mFecUnit)
    mFecUnit->distortionDeinit();
  mFecUnit.reset();
}

RT_RET RTFECProcessor::process(const RTFECInfo &info) {
  std::unique_lock<std::mutex> lk(mtx);
  mWorkQueue.push(info);
  return RT_OK;
}
