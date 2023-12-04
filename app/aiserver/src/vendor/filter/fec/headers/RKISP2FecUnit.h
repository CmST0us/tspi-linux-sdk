
/*
 * Copyright (C) 2015-2017 Intel Corporation
 * Copyright (c) 2017, Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef _CAMERA3_RKISP2FecUnit_H_
#define _CAMERA3_RKISP2FecUnit_H_

#include <memory>
#include <mutex>

using __createGLClass = void *(*)();
using __distortionByGpuInit = int (*)(void *p, int sw, int sh, int mapw,
                                      int maph);
using __distortionByGpuDeinit = int (*)(void *p);
using __distortionByGpuProcess = void *(*)(void *p, int inputfd, int inWidth,
                                           int inHeight, int infmt,
                                           int outputfd, int outWidth,
                                           int outHeigh, int outfmt, int op);
using __createFenceFd = void *(*)(void *p);
using __waitFence = int (*)(void *p, void *fence, int value);
using __distortionByGpuSyncFenceFd = int (*)(void *p);

class RKISP2FecUnit {
private:
  void loadDistortionGlLibray();

  void *glClass;
  void *dso;
  int done_init;
  int width_;
  int height_;
  mutable std::mutex mtx;
  // static RKISP2FecUnit *mInstance;

  __createGLClass createGLClass;
  __createFenceFd createFenceFd;
  __waitFence waitFencfd;
  __distortionByGpuInit distortionByGpuInit;
  __distortionByGpuDeinit distortionByGpuDeinit;
  __distortionByGpuSyncFenceFd distortionSyncFenceFd;
  __distortionByGpuProcess distortionByGpuProcess;

public:
  RKISP2FecUnit();
  ~RKISP2FecUnit();
  void calculateMeshGridSize(int width, int height, int &meshW, int &meshH);
  int doFecProcess(int inW, int inH, int inFd, int inFormat, int outW, int outH,
                   int outFd, int outFormat, int &fenceFd);
  int distortionInit(int width, int height);
  int distortionDeinit();

  //  static RKISP2FecUnit *getInstance();
};

#endif
