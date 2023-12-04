#include "RKISP2FecUnit.h"

#include <dlfcn.h>
#include <drm_fourcc.h>
#include <stdio.h>
#include <sys/time.h>

#include <string>

#include "rt_log.h"

#define ALIGN(value, align) ((value + align - 1) & ~(align - 1))
#if 0
#if defined(__LP64__)
#define RK_XXX_PATH "/vendor/lib64/libdistortion_gl.so"
#else
#define RK_XXX_PATH "/vendor/lib/libdistortion_gl.so"
#endif
#else
#define RK_XXX_PATH "/oem/usr/lib/libdistortion.so"
#endif
#define LOGE RT_LOGE
#if 0
RKISP2FecUnit *RKISP2FecUnit::mInstance = nullptr;

RKISP2FecUnit *RKISP2FecUnit::getInstance() {
  if (mInstance == nullptr) {
    mInstance = new RKISP2FecUnit();
  }
  return mInstance;
}
#endif

static int64_t getNowUsTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

RKISP2FecUnit::RKISP2FecUnit()
    : createFenceFd(nullptr), dso(nullptr), done_init(0),
      distortionByGpuInit(nullptr), distortionByGpuDeinit(nullptr),
      distortionByGpuProcess(nullptr), distortionSyncFenceFd(nullptr),
      waitFencfd(nullptr) {
  loadDistortionGlLibray();
}

RKISP2FecUnit::~RKISP2FecUnit() {
  distortionDeinit();
  if (dso)
    dlclose(dso);
#if 0
  if (mInstance != nullptr) {
    delete (mInstance);
    mInstance = nullptr;
  }
#endif
}

void RKISP2FecUnit::loadDistortionGlLibray() {
  if (dso == NULL) {
    dso = dlopen(RK_XXX_PATH, RTLD_NOW| RTLD_GLOBAL);
  }
  RT_LOGE("rk-debug[%s %d]  dso=%p", __FUNCTION__, __LINE__, dso);
  if (dso == 0) {
    LOGE("rk-debug can't not find %s ! error=%s ", RK_XXX_PATH, dlerror());
    return;
  }
  createGLClass = NULL;
  if (createGLClass == NULL)
    createGLClass = (__createGLClass)dlsym(dso, "createGLContext");
  if (createGLClass == NULL) {
    LOGE("rk_debug can't not find CreateGLClass function in "
         "/system/lib64/libdistortion_gl.so !");
    goto err;
  }
  if (distortionByGpuInit == NULL)
    distortionByGpuInit =
        (__distortionByGpuInit)dlsym(dso, "distortionByGpuInit");
  if (distortionByGpuInit == NULL) {
    LOGE("rk_debug can't not find distortionByGpuInit function in "
         "/system/lib64/libdistortion_gl.so !");
    goto err;
  }

  if (distortionByGpuProcess == NULL)
    distortionByGpuProcess =
        (__distortionByGpuProcess)dlsym(dso, "distortionByGpuProcess");
  if (distortionByGpuProcess == NULL) {
    LOGE("rk_debug can't not find distortionByGpuProcess function in "
         "/system/lib64/libdistortion_gl.so !");
    goto err;
  }
  if (distortionByGpuDeinit == NULL)
    distortionByGpuDeinit =
        (__distortionByGpuDeinit)dlsym(dso, "distortionByGpuDeinit");
  if (distortionByGpuDeinit == NULL) {
    LOGE("rk_debug can't not find distortionByGpuDeinit function in "
         "/system/lib64/libdistortion_gl.so !");
    goto err;
  }
  if (createFenceFd == NULL)
    createFenceFd = (__createFenceFd)dlsym(dso, "distortioncreateFence");
  if (createFenceFd == NULL) {
    LOGE("rk_debug can't not find createFencefd function in "
         "/system/lib64/libdistortion_gl.so !");
    goto err;
  }
  if (waitFencfd == NULL)
    waitFencfd = (__waitFence)dlsym(dso, "distortionWaitFence");
  if (waitFencfd == NULL) {
    LOGE("rk_debug can't not find waitFencfd function in "
         "/system/lib64/libdistortion_gl.so !");
    goto err;
  }
  return;
err:
  dlclose(dso);
}

void RKISP2FecUnit::calculateMeshGridSize(int width, int height, int &meshW,
                                          int &meshH) {
  int meshStepW, meshStepH;
  int alignW = ALIGN(width, 32);
  int alignH = ALIGN(height, 32);

  if (width <= 1920) {
    meshStepW = 16;
    meshStepH = 8;
  } else {
    meshStepW = 32;
    meshStepH = 16;
  }

  meshW = (alignW + meshStepW - 1) / meshStepW + 1;
  meshH = (alignH + meshStepH - 1) / meshStepH + 1;
  LOGE("meshW=%d meshH=%d  alignw=%d alignh=%d", meshW, meshH, alignW, alignH);
}

int RKISP2FecUnit::distortionInit(int width, int height) {
  std::unique_lock<std::mutex> lk(mtx);
  int success = 0;
  if (!done_init && distortionByGpuInit) {
    uint32_t w = (width & 0xffff) << 16 | 3840;
    uint32_t h = (height & 0xffff) << 16 | 2160;
    int meshGridW = 0, meshGridH = 0;

    done_init = 1;
    calculateMeshGridSize(width, height, meshGridW, meshGridH);
    glClass = createGLClass();
    if (glClass != nullptr) {
      success = distortionByGpuInit(glClass, w, h, meshGridW, meshGridH);
      LOGE("rk-debug: glclass = %p meshGridW=%d meshGridH=%d", glClass,
           meshGridW, meshGridH);
    } else {
      success = -1;
    }
  }
  return success;
}

int RKISP2FecUnit::distortionDeinit() {
  std::unique_lock<std::mutex> lk(mtx);
  int success = 0;
  if (distortionByGpuDeinit && done_init)
    success = distortionByGpuDeinit(glClass);
  done_init = 0;
  return success;
}

int RKISP2FecUnit::doFecProcess(int inW, int inH, int inFd, int inFormat,
                                int outW, int outH, int outFd, int outFormat,
                                int &fenceFd) {
#if 1
  if (distortionByGpuProcess) {
    void *gpu_fence_fd = NULL;
    distortionByGpuProcess(glClass, inFd, inW, inH, inFormat, outFd, outW, outH,
                           outFormat, 0);
#if 0
    if (fenceFd > 0) {
        int retireFenceFd = sync_merge("fec_and_camera", fenceFd, gpu_fence_fd);
        close(gpu_fence_fd);
        close(fenceFd);
        fenceFd = retireFenceFd
    }
#endif
    if (createFenceFd)
      gpu_fence_fd = createFenceFd(glClass);
    if (waitFencfd)
      waitFencfd(glClass, gpu_fence_fd, 0);
  }
#else
  RT_LOGE("waitFencefd");
  if (waitFencfd)
    waitFencfd(NULL, NULL, 0);
#endif
  return 0;
}
