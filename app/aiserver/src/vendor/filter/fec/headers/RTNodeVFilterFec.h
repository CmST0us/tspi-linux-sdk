/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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
 *
 * author: added by <rimon.xu@rock-chips.com>
 *   date: 2020-06-08
 * module: eptz task node
 */

#ifndef SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFEC_H_
#define SRC_RT_TASK_TASK_NODE_FILTER_RTNODEVFILTERFEC_H_

#include <thread>

#include "RTFecProcessor.h"
#include "RTMediaRockx.h"
#include "RTTaskNode.h"

#define NODE_NAME_RK_FEC "rkfec"

struct rkispp_fec_in_out {
  int width;
  int height;
  int in_fourcc;
  int out_fourcc;
  int in_pic_fd;
  int out_pic_fd;
  int mesh_xint_fd;
  int mesh_xfra_fd;
  int mesh_yint_fd;
  int mesh_yfra_fd;
};
/***************************************/

struct drm_buf {
  void *map;
  int dmabuf_fd;
  int size;
};

class RTNodeVFilterFec : public RTTaskNode {
public:
  RTNodeVFilterFec();
  virtual ~RTNodeVFilterFec();

  virtual RT_RET open(RTTaskNodeContext *context);
  virtual RT_RET process(RTTaskNodeContext *context);
  virtual RT_RET close(RTTaskNodeContext *context);

private:
  int allocMeshBufs();
  int calFecMeshsize(int width, int height);
  int loadMeshfiles(const char *path);
  RT_RET doGpuProcess(RTTaskNodeContext *context);

  RTFECProcessor *mProcessor;
  struct drm_buf *xintMesh;
  struct drm_buf *xfraMesh;
  struct drm_buf *yintMesh;
  struct drm_buf *yfraMesh;

  RtMutex *mLock;
  std::mutex fecLock;
  std::condition_variable cv;

  char devName[32];
  int meshSize[2];
  int cameraIdx;
  int dumpCnt;
  int drmFd;
  int width;
  int height;
  int loadMeshSuccess;
  int fpsCtr;
  int fpsAbandonSet;
  int fpsInCount;
  int fpsAbandonCount;

protected:
  virtual RT_RET invokeInternal(RtMetaData *meta);
  /*
     virtual RT_RET parserConfig(RtMetaData *meta);
     virtual RT_RET invokeInternal(RtMetaData *meta);
     virtual RT_RET initSupportOptions();
 */
};

#endif
