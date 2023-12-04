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
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>

#include <linux/videodev2.h>
//#include <drm_fourcc.h>
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <fstream>
#include <iostream>
#include <vector>

//#include "RTMeshGenerator.h"
#include "RTNodeVFilterFec.h"     // NOLINT
#include "RTMediaBuffer.h" // NOLINT
#include "RTNodeCommon.h"

#define ISPP_MAX_VIDEO_NODE 60
#define MESH_PATH "/oem/usr/share/mesh"
#define MESH_PATH_1080 "/oem/usr/share/mesh1080"
#define OPT_FEC_CAMEAR_IDX "cameraIdx"
#define RKISPP_CMD_FEC_IN_OUT                                                  \
  _IOW('V', BASE_VIDIOC_PRIVATE + 10, struct rkispp_fec_in_out)

static int fecFd = -1;
static pthread_mutex_t fecMutex = PTHREAD_MUTEX_INITIALIZER;
#ifdef DEBUG_FEC
#include <sys/syscall.h>
pid_t gettid() { return syscall(SYS_gettid); }
#endif

int readFileList(const char *basePath) {
  DIR *dir;
  int found = -1;
  struct dirent *ptr;
  char filename[1000];

  if ((dir = opendir(basePath)) == NULL) {
    RT_LOGE("Open dir error...\n");
    return found;
  }

  while ((ptr = readdir(dir)) != NULL) {
    if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
      /// current dir OR parrent dir
      continue;
    } else if (ptr->d_type == 8) {
      /// file, check video*/name whether has rkispp_fec info
      if (strcmp(ptr->d_name, "name") == 0) {
        // RT_LOGD("d_name:%s/%s",basePath,ptr->d_name);
        memset(filename, '\0', sizeof(filename));
        strcpy(filename, basePath);
        strcat(filename, "/");
        strcat(filename, ptr->d_name);
        FILE *fp = fopen(filename, "rb");
        if (fp) {
          char buf[128];
          fgets(buf, 128, fp);
          // printf("buf=%s\n", buf);
          if (strstr(buf, "rkispp_fec") != NULL) {
            printf("found ispp fec node");
            found = 1;
            break;
          }
        }
      }
    } else if (ptr->d_type == 10) {
      // TODO: nothing
    } else if (ptr->d_type == 4) {
      // TODO: nothing
    }
  }
  closedir(dir);
  return found;
}

static int findFecEntry(char *entryName) {
  int found = -1;
  char path[128] = {0};
  for (int i = 0; i < ISPP_MAX_VIDEO_NODE; i++) {
    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), "/sys/class/video4linux/video%d", i);
    if (0 == access(path, F_OK)) {
      found = readFileList(path);
      if (found > 0) {
        found = i;
        break;
      }
    }
  }

  if (found >= 0) {
    sprintf(entryName, "/dev/video%d", found);
  } else {
    RT_LOGE("failed to found fec entry:%d", found);
  }
  return found;
}

static int doFecProcess(struct rkispp_fec_in_out *fec) {
  pthread_mutex_lock(&fecMutex);
  int ret = -1;
  if (fecFd <= 0) {
    RT_LOGD("***************************\n");
    char fecEntryNode[32];
    ret = findFecEntry(fecEntryNode);
    RT_LOGD("fecEntryNode = %s", fecEntryNode);
    RT_LOGD("***************************\n");
    fecFd = open(fecEntryNode, O_RDWR, 0);
    // findFecNode("/sys/class/video4linux");
  }
  if (fecFd > 0) {
    ret = ioctl(fecFd, RKISPP_CMD_FEC_IN_OUT, fec);
    if (ret == -EAGAIN) // try again
      ret = ioctl(fecFd, RKISPP_CMD_FEC_IN_OUT, fec);
  }
  pthread_mutex_unlock(&fecMutex);
  return ret;
}

int alloc_drm_buffer(int fd, int width, int height, int bpp,
                     struct drm_buf *buf) {
  struct drm_mode_create_dumb alloc_arg;
  struct drm_mode_map_dumb mmap_arg;
  struct drm_mode_destroy_dumb destory_arg;
  void *map;
  int ret;

  memset(&alloc_arg, 0, sizeof(alloc_arg));
  alloc_arg.bpp = bpp;
  alloc_arg.width = width;
  alloc_arg.height = height;

  ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &alloc_arg);
  if (ret) {
    printf("failed to create dumb buffer\n");
    return ret;
  }

  memset(&mmap_arg, 0, sizeof(mmap_arg));
  mmap_arg.handle = alloc_arg.handle;
  ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mmap_arg);
  if (ret) {
    printf("failed to create map dumb\n");
    ret = -EINVAL;
    goto destory_dumb;
  }
  map = mmap(0, alloc_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
             mmap_arg.offset);
  if (map == MAP_FAILED) {
    printf("failed to mmap buffer\n");
    ret = -EINVAL;
    goto destory_dumb;
  }
  ret = drmPrimeHandleToFD(fd, alloc_arg.handle, 0, &buf->dmabuf_fd);
  if (ret) {
    printf("failed to get dmabuf fd\n");
    munmap(map, alloc_arg.size);
    ret = -EINVAL;
    goto destory_dumb;
  }
  buf->size = alloc_arg.size;
  buf->map = map;

destory_dumb:
  memset(&destory_arg, 0, sizeof(destory_arg));
  destory_arg.handle = alloc_arg.handle;
  drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destory_arg);
  return ret;
}

int free_drm_buffer(int fd, struct drm_buf *buf) {
  if (buf) {
    close(buf->dmabuf_fd);
    munmap(buf->map, buf->size);
    free(buf);
  }
  return 0;
}

int fec_drm_open() { return drmOpen("rockchip", NULL); }

void fec_drm_close(int drm_fd) { close(drm_fd); }

int fec_open(char *dev) { return open(dev, O_RDWR, 0); }

void fec_close(int fd) { close(fd); }

int load_file(char *filename, int len, void *outbuf) {
  if (filename == nullptr) {
    RT_LOGE("mesh file is null , please check it\n");
    return -1;
  }
  FILE *fp = fopen(filename, "rb");
  int ret = -1;
  if (fp) {
    ret = fread(outbuf, 1, len, fp);
    fclose(fp);
  }
  return ret;
}

RTNodeVFilterFec::RTNodeVFilterFec()
    : width(1280), height(720), xintMesh(nullptr), xfraMesh(nullptr),
      yintMesh(nullptr), yfraMesh(nullptr), mProcessor(nullptr) {
  dumpCnt = 0;
  mLock = new RtMutex();
  std::string path = "/oem/usr/share/mesh/";
  // generateMapParms(path, width, height);
  RT_LOGE("create fec node\n");
}

RTNodeVFilterFec::~RTNodeVFilterFec() {
  free_drm_buffer(drmFd, yfraMesh);
  yfraMesh = nullptr;
  free_drm_buffer(drmFd, yintMesh);
  yintMesh = nullptr;
  free_drm_buffer(drmFd, xfraMesh);
  xfraMesh = nullptr;
  free_drm_buffer(drmFd, xintMesh);
  xintMesh = nullptr;
  fec_drm_close(drmFd);
  rt_safe_delete(mLock);
}

int RTNodeVFilterFec::allocMeshBufs() {
  int ret = -1;
  drmFd = fec_drm_open();
  if (drmFd < 0) {
    printf("failed to open rockchip drm");
    goto err_alloc;
  }
  meshSize[0] = calFecMeshsize(width, height);
  meshSize[1] = meshSize[0] * 2;

  RT_LOGD("\nmesh_size:%d", meshSize[0]);
  xintMesh = (struct drm_buf *)malloc(sizeof(struct drm_buf));
  ret = alloc_drm_buffer(drmFd, meshSize[0] * 2, 1, 8, xintMesh);
  if (ret)
    goto err_alloc;
  RT_LOGD("xint fd:%d size:%d", xintMesh->dmabuf_fd, xintMesh->size);

  xfraMesh = (struct drm_buf *)malloc(sizeof(struct drm_buf));
  ret = alloc_drm_buffer(drmFd, meshSize[0], 1, 8, xfraMesh);
  if (ret)
    goto err_alloc;
  RT_LOGD("xfra fd:%d size:%d", xfraMesh->dmabuf_fd, xfraMesh->size);

  yintMesh = (struct drm_buf *)malloc(sizeof(struct drm_buf));
  ret = alloc_drm_buffer(drmFd, meshSize[0] * 2, 1, 8, yintMesh);
  if (ret)
    goto err_alloc;
  RT_LOGD("yint fd:%d size:%d", yintMesh->dmabuf_fd, yintMesh->size);

  yfraMesh = (struct drm_buf *)malloc(sizeof(struct drm_buf));
  ret = alloc_drm_buffer(drmFd, meshSize[0], 1, 8, yfraMesh);
  if (ret)
    goto err_alloc;
  RT_LOGD("yfra fd:%d size:%d\n", yfraMesh->dmabuf_fd, yfraMesh->size);

  return 0;
err_alloc:
  free_drm_buffer(drmFd, yfraMesh);
  yfraMesh = nullptr;
  free_drm_buffer(drmFd, yintMesh);
  yintMesh = nullptr;
  free_drm_buffer(drmFd, xfraMesh);
  xfraMesh = nullptr;
  free_drm_buffer(drmFd, xintMesh);
  xintMesh = nullptr;
  fec_drm_close(drmFd);
  return -1;
}

int RTNodeVFilterFec::calFecMeshsize(int width, int height) {
  int mesh_size, mesh_left_height;
  int w = 32 * ((width + 31) / 32);
  int h = 32 * ((height + 31) / 32);
  int spb_num = (h + 127) >> 7;
  int left_height = h & 127;
  bool density = (width > 1920) ? true : false;
  int mesh_width = density ? (w / 32 + 1) : (w / 16 + 1);
  int mesh_height = density ? 9 : 17;

  if (!left_height)
    left_height = 128;
  mesh_left_height = density ? (left_height / 16 + 1) : (left_height / 8 + 1);
  mesh_size =
      (spb_num - 1) * mesh_width * mesh_height + mesh_width * mesh_left_height;

  return mesh_size;
}

int RTNodeVFilterFec::loadMeshfiles(const char *path) {
  if (!xintMesh || !xfraMesh || !yintMesh || !yfraMesh)
    return -1;

  char filename[64] = {0};
  int nRead = 0;
  snprintf(filename, sizeof(filename), "%s/meshxi_level%d", path, cameraIdx);
  nRead = load_file(filename, meshSize[1], xintMesh->map);
  RT_LOGD("mesh file: %s %d (%d)", filename, nRead, meshSize[1]);
  if (nRead <= 0)
    goto err_read;
  snprintf(filename, sizeof(filename), "%s/meshxf_level%d", path, cameraIdx);
  nRead = load_file(filename, meshSize[0], xfraMesh->map);
  RT_LOGD("mesh file: %s %d(%d)", filename, nRead, meshSize[0]);
  if (nRead <= 0)
    goto err_read;
  snprintf(filename, sizeof(filename), "%s/meshyi_level%d", path, cameraIdx);
  nRead = load_file(filename, meshSize[1], yintMesh->map);
  RT_LOGD("mesh file: %s %d", filename, nRead);
  if (nRead <= 0)
    goto err_read;
  snprintf(filename, sizeof(filename), "%s/meshyf_level%d", path, cameraIdx);
  nRead = load_file(filename, meshSize[0], yfraMesh->map);
  if (nRead <= 0)
    goto err_read;
  RT_LOGD("mesh file: %s %d", filename, nRead);
  return 0;
err_read:
  RT_LOGE("failed to read mesh files %s\n", path);
  return -1;
}

RT_RET RTNodeVFilterFec::open(RTTaskNodeContext *context) {
  RtMetaData *inputMeta = context->options();
  char fecEntryNode[32];
  RT_RET err = RT_OK;
  int ret = 0;

  fpsCtr = 0;
  fpsAbandonSet = 0;
  fpsInCount = 0;
  fpsAbandonCount = 0;

  cameraIdx = 0;
  inputMeta->findInt32(OPT_FEC_CAMEAR_IDX, &cameraIdx);
  inputMeta->findInt32("opt_width", &width);
  inputMeta->findInt32("opt_height", &height);
  inputMeta->findInt32("opt_fps_ctr", &fpsCtr);
  RT_LOGE("fec mesh files: %s cameraIdx=%d width=%d height=%d fpsCtr=%d\n",
          MESH_PATH, cameraIdx, width, height, fpsCtr);

  ret = findFecEntry(fecEntryNode);
  if (ret >= 0) {
    //   fpsAbandonSet = fpsCtr;
    ret = allocMeshBufs();
    if (ret == 0) {
      if (width == 1280)
        loadMeshSuccess = loadMeshfiles(MESH_PATH);
      else
        loadMeshSuccess = loadMeshfiles(MESH_PATH_1080);
    } else {
      RT_LOGE("failed to allock buffers for mesh files\n");
    }
  } else {
    mProcessor = new RTFECProcessor();
  }
  return err;
}

RT_RET RTNodeVFilterFec::close(RTTaskNodeContext *context) {
  RT_RET err = RT_OK;
  if (mProcessor)
    delete mProcessor;
  mProcessor = NULL;
  return err;
}

RT_RET RTNodeVFilterFec::doGpuProcess(RTTaskNodeContext *context) {
  // RT_LOGE("*******%d*", (int)gettid());
  RT_RET err = RT_OK;
  RTMediaBuffer *inputBuffer = RT_NULL;
  RTMediaBuffer *outputBuffer = RT_NULL;
  RTFECInfo mInfos;
  int ret = 0;

  outputBuffer = context->dequeOutputBuffer();
  if (outputBuffer == RT_NULL) {
    RT_LOGE("outputBuffer = RT_NULL");
    return RT_OK;
  }

  inputBuffer = context->dequeInputBuffer();
  if (inputBuffer == RT_NULL) {
    RT_LOGE("inputBuffer = RT_NULL");
    return err;
  }

  if (mProcessor) {
    mInfos.inputBuffer = inputBuffer;
    mInfos.outputBuffer = outputBuffer;
    mInfos.inW = width;
    mInfos.inH = height;
    mInfos.outW = width;
    mInfos.outH = height;
    mInfos.inFormat = DRM_FORMAT_NV12;
    mInfos.outFormat = DRM_FORMAT_NV12;
    mInfos.context = context;
    mInfos.mtx = &fecLock;
    mInfos.cv = &cv;
    mProcessor->process(mInfos);
    std::unique_lock<std::mutex> lck(fecLock);
    cv.wait(lck);
  } else {
    // TODO: send inputBuffer to next node ...
    inputBuffer = context->dequeInputBuffer();
    inputBuffer->getMetaData()->setInt32(OPT_FILTER_WIDTH, width);
    inputBuffer->getMetaData()->setInt32(OPT_FILTER_HEIGHT, height);
    inputBuffer->getMetaData()->setCString(OPT_STREAM_FMT_IN, "image:nv12");
    inputBuffer->getMetaData()->setInt32(kKeyFrameW, width);
    inputBuffer->getMetaData()->setInt32(kKeyFrameH, height);
    context->queueOutputBuffer(inputBuffer);
  }
  RT_LOGD("end");
  return err;
}

RT_RET RTNodeVFilterFec::process(RTTaskNodeContext *context) {
  RT_RET err = RT_OK;
  RTMediaBuffer *inputBuffer = RT_NULL;
  RTMediaBuffer *outputBuffer = RT_NULL;
  RT_BOOL doFec = RT_TRUE;
  int ret = 0;
  RtMutex::RtAutolock autoLock(mLock);
  if (mProcessor) {
    doGpuProcess(context);
    return err;
  }

  if (!xintMesh || !xfraMesh || !yintMesh || !yfraMesh)
    return err;
  if (fpsCtr) {
    if (fpsAbandonCount >= fpsAbandonSet) {
      fpsAbandonCount = 0;
    } else {
      doFec = RT_FALSE;
      if (!context->inputIsEmpty()) {
        fpsAbandonCount++;
        inputBuffer = context->dequeInputBuffer();
        inputBuffer->release();
      }
    }
  }

  if (doFec) {
    if (!context->inputIsEmpty() && loadMeshSuccess == 0) {
      outputBuffer = context->dequeOutputBuffer();
      if (outputBuffer == RT_NULL) {
        RT_LOGD("outputBuffer = RT_NULL");
        return RT_OK;
      }

      inputBuffer = context->dequeInputBuffer();

      struct rkispp_fec_in_out fec;
      fec.width = width;
      fec.height = height;
      fec.in_fourcc = V4L2_PIX_FMT_NV12;
      fec.out_fourcc = V4L2_PIX_FMT_NV12;
      fec.in_pic_fd = inputBuffer->getFd();
      fec.out_pic_fd = outputBuffer->getFd();
      fec.mesh_xint_fd = xintMesh->dmabuf_fd;
      fec.mesh_xfra_fd = xfraMesh->dmabuf_fd;
      fec.mesh_yint_fd = yintMesh->dmabuf_fd;
      fec.mesh_yfra_fd = yfraMesh->dmabuf_fd;

      outputBuffer->getMetaData()->setInt32(OPT_FILTER_WIDTH, width);
      outputBuffer->getMetaData()->setInt32(OPT_FILTER_HEIGHT, height);
      outputBuffer->getMetaData()->setCString(OPT_STREAM_FMT_IN, "image:nv12");
      outputBuffer->getMetaData()->setInt32(kKeyFrameW, width);
      outputBuffer->getMetaData()->setInt32(kKeyFrameH, height);

      ret = doFecProcess(&fec);
      if (ret == 0) {
        context->queueOutputBuffer(outputBuffer);
        inputBuffer->release();
      } else {
        RT_LOGE("failed to do fec");
        inputBuffer->getMetaData()->setInt32(OPT_FILTER_WIDTH, width);
        inputBuffer->getMetaData()->setInt32(OPT_FILTER_HEIGHT, height);
        inputBuffer->getMetaData()->setCString(OPT_STREAM_FMT_IN, "image:nv12");
        inputBuffer->getMetaData()->setInt32(kKeyFrameW, width);
        inputBuffer->getMetaData()->setInt32(kKeyFrameH, height);
        context->queueOutputBuffer(inputBuffer);
        outputBuffer->release();
        // RT_LOGD("doFilter fail, release inputBuffer");
        err = RT_OK;
      }
    } else if (!context->inputIsEmpty()) {
      // TODO: send inputBuffer to next node ...
      inputBuffer = context->dequeInputBuffer();
      inputBuffer->getMetaData()->setInt32(OPT_FILTER_WIDTH, width);
      inputBuffer->getMetaData()->setInt32(OPT_FILTER_HEIGHT, height);
      inputBuffer->getMetaData()->setCString(OPT_STREAM_FMT_IN, "image:nv12");
      inputBuffer->getMetaData()->setInt32(kKeyFrameW, width);
      inputBuffer->getMetaData()->setInt32(kKeyFrameH, height);
      context->queueOutputBuffer(inputBuffer);
      if (loadMeshSuccess != 0) {
        if (width == 1280)
          loadMeshSuccess = loadMeshfiles(MESH_PATH);
        else
          loadMeshSuccess = loadMeshfiles(MESH_PATH_1080);
      }
    } else {
      RT_LOGD("found no input buffer");
      err = RT_OK;
    }
  }
  return err;
}

RT_RET RTNodeVFilterFec::invokeInternal(RtMetaData *meta) {
  if (fpsCtr) {
    const char *command;
    meta->findCString(kKeyPipeInvokeCmd, &command);
    RT_LOGD("invoke(%s) internally.", command);
    RTSTRING_SWITCH(command) {
      RTSTRING_CASE("nn_fps_set")
          : meta->findInt32("nn_fps_set", &fpsAbandonSet);
      RT_LOGE("fpsAbandonSet %d", fpsAbandonSet);
      break;
    default:
      RT_LOGD("unsupported command=%d", command);
      break;
    }
  }
  return RT_OK;
}

static RTTaskNode *createFecFilter() { return new RTNodeVFilterFec(); }

/*****************************************
 * register node stub to RTTaskNodeFactory
 *****************************************/
RTNodeStub node_stub_filter_fec{
    .mUid = MKTAG('f', 'e', 'c', 'p'),
    .mName = NODE_NAME_RK_FEC,
    .mVersion = "v1.0",
    .mCreateObj = createFecFilter,
    .mCapsSrc      = { "video/x-raw", RT_PAD_SRC, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL}, },
    .mCapsSink     = { "video/x-raw", RT_PAD_SINK, RT_MB_TYPE_VFRAME, {RT_NULL, RT_NULL} },
};

RT_NODE_FACTORY_REGISTER_STUB(node_stub_filter_fec);
