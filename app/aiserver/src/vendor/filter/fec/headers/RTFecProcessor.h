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

#ifndef RT_FEC_PSSROCESSOR_H_
#define RT_FEC_PSSROCESSOR_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include "RKISP2FecUnit.h"
#include "RTMediaBuffer.h"
#include "RTMediaRockx.h"
#include "RTTaskNode.h"

typedef struct _RTFECInfo {
  RTTaskNodeContext *context;
  RTMediaBuffer *inputBuffer;
  RTMediaBuffer *outputBuffer;
  std::mutex *mtx;
  std::condition_variable *cv;
  int inW;
  int inH;
  int inFormat;
  int outW;
  int outH;
  int outFormat;
} RTFECInfo;

class RTFECProcessor {
public:
  RTFECProcessor();
  virtual ~RTFECProcessor();
  virtual RT_RET process(const RTFECInfo &info);

private:
  void rcvMessageThread();
  std::shared_ptr<RKISP2FecUnit> mFecUnit;
  mutable std::mutex mtx;
  std::thread *msgThread;
  std::queue<RTFECInfo> mWorkQueue;
  bool looping;
};

#endif // RT_FEC_PSSROCESSOR_H_
