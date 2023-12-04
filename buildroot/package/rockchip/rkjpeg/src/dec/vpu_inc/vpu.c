/*
 *
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
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

#define LOG_TAG "vpu"
#include "vpu_type.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
//#include <sys/timeb.h>
#include <sys/poll.h>

#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

#include <cutils/sockets.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vpu.h"
#include "vpu_drv.h"
#define ALOGV printf
#define ALOGD printf
#define ALOGI printf
#define ALOGW printf
#define ALOGE printf

#define VPU_IOC_MAGIC                       'l'
#define VPU_IOC_SET_CLIENT_TYPE             _IOW(VPU_IOC_MAGIC, 1, unsigned long)
#define VPU_IOC_GET_HW_FUSE_STATUS          _IOW(VPU_IOC_MAGIC, 2, unsigned long)
#define VPU_IOC_SET_REG                     _IOW(VPU_IOC_MAGIC, 3, unsigned long)
#define VPU_IOC_GET_REG                     _IOW(VPU_IOC_MAGIC, 4, unsigned long)
#define VPU_IOC_PROBE_IOMMU_STATUS          _IOR(VPU_IOC_MAGIC, 5, unsigned long)
typedef struct VPUReq {
    RK_U32 *req;
    RK_U32  size;
} VPUReq_t;
static int vpu_service_status = -1;
static int vpu_service_iommu_status = -1;

static const char *vpu_dev[] = {
    "/dev/vpu_service",
    "/dev/vpu-service",
};

int open_vpu_device()
{
    int fd = -1;
    RK_U32 i = 0;
    RK_U32 size = (sizeof(vpu_dev) / sizeof(vpu_dev[0]));
    for (i = 0; i < size; i++) {
        fd = open(vpu_dev[i], O_RDWR);
		printf("open_vpu_device: %s result=%d errno=%s \n", vpu_dev[i], fd, strerror(errno));
        if (fd >= 0) {
            return fd;
        }
    }

    return fd;
}

int check_vpu_device()
{
    RK_U32 i;
    RK_U32 size = (sizeof(vpu_dev) / sizeof(vpu_dev[0]));
    for (i = 0; i < size; i++) {
        if (!access(vpu_dev[i], F_OK))
            return 1;
    }
    return 0;
}

#define VPU_SERVICE_TEST    \
    do { \
        if (vpu_service_status < 0) { \
            vpu_service_status = check_vpu_device(); \
        } \
    } while (0)

int VPUClientInit(VPU_CLIENT_TYPE type)
{
    VPU_SERVICE_TEST;
    int ret;
    int fd;
	printf("VPUClientInit: type=%d \n", type);
    if (type != VPU_DEC_HEVC) {
        fd = open_vpu_device();
    } else {
        fd = open("/dev/hevc_service", O_RDWR);
        type = VPU_DEC;
    }
    if (fd == -1) {
        char *msg = NULL;
        msg = strerror(errno);
        printf("errno = %d, Error Msg:%s\n", errno, msg);
        printf("VPUClient: failed to open vpu_service\n");
        return -1;
    }
    ret = ioctl(fd, VPU_IOC_SET_CLIENT_TYPE, (RK_U32)type);
    if (ret) {
        printf("VPUClient: ioctl VPU_IOC_SET_CLIENT_TYPE failed ret %d errno %d\n", ret, errno);
        return -2;
    }
    return fd;
}

RK_S32 VPUClientRelease(int socket)
{
    VPU_SERVICE_TEST;
    int fd = socket;
    if (fd > 0) {
        close(fd);
    }
    return VPU_SUCCESS;
}

RK_S32 VPUClientSendReg(int socket, RK_U32 *regs, RK_U32 nregs)
{
    VPU_SERVICE_TEST;
    int fd = socket;
    RK_S32 ret;
    VPUReq_t req;
    nregs *= sizeof(RK_U32);
    req.req     = regs;
    req.size    = nregs;
    ret = (RK_S32)ioctl(fd, VPU_IOC_SET_REG, &req);
    if (ret) {
        ALOGE("VPUClient: ioctl VPU_IOC_SET_REG failed ret %d errno %d %s\n", ret, errno, strerror(errno));
    }
    return ret;
}

// TODO: 看是否客户端需要返回消息的长度
RK_S32 VPUClientWaitResult(int socket, RK_U32 *regs, RK_U32 nregs, VPU_CMD_TYPE *cmd, RK_S32 *len)
{
    VPU_SERVICE_TEST;
    int fd = socket;
    RK_S32 ret;
    VPUReq_t req;
    nregs *= sizeof(RK_U32);
    req.req     = regs;
    req.size    = nregs;
    ret = (RK_S32)ioctl(fd, VPU_IOC_GET_REG, &req);
    if (ret) {
        printf("VPUClient: ioctl VPU_IOC_GET_REG failed ret %d errno %d %s\n", ret, errno, strerror(errno));
        *cmd = VPU_SEND_CONFIG_ACK_FAIL;
    } else {
        *cmd = VPU_SEND_CONFIG_ACK_OK;
    }
    return ret;
}

RK_S32 VPUClientGetHwCfg(int socket, RK_U32 *cfg, RK_U32 cfg_size)
{
    VPU_SERVICE_TEST;
    int fd = socket;
    RK_S32 ret;
    VPUReq_t req;
    req.req     = cfg;
    req.size    = cfg_size;
    ret = (RK_S32)ioctl(fd, VPU_IOC_GET_HW_FUSE_STATUS, &req);
    if (ret) {
        ALOGE("VPUClient: ioctl VPU_IOC_GET_HW_FUSE_STATUS failed ret %d\n", ret);
    }
    return ret;
}

RK_S32 VPUClientGetIOMMUStatus()
{
    int ret = 0;
    if (vpu_service_iommu_status < 0) {
        int fd = -1;
        fd = open_vpu_device();
        if (fd >= 0) {
            ret = (RK_S32)ioctl(fd, VPU_IOC_PROBE_IOMMU_STATUS, &vpu_service_iommu_status);
            if (ret) {
                vpu_service_iommu_status = 0;
                ALOGE("VPUClient: ioctl VPU_IOC_PROBE_IOMMU_STATUS failed ret %d, disable iommu\n", ret);
            }
            close(fd);
        } else {
            vpu_service_iommu_status = 0;
        }
    }
    ALOGV("vpu_service_iommu_status %d", vpu_service_iommu_status);
    return vpu_service_iommu_status;
}
RK_U32 VPUCheckSupportWidth()
{
    VPUHwDecConfig_t hwCfg;
    int fd = -1;
    fd = open_vpu_device();
    memset(&hwCfg, 0, sizeof(VPUHwDecConfig_t));
    if (fd >= 0) {
        if (VPUClientGetHwCfg(fd, (RK_U32*)&hwCfg, sizeof(hwCfg))) {
            ALOGE("Get HwCfg failed\n");
            close(fd);
            return -1;
        }
        close(fd);
        fd = -1;
    }
    return hwCfg.maxDecPicWidth;
}
#if BUILD_VPU_TEST
#include <pthread.h>
#define MAX_THREAD_NUM      10
void *vpu_test(void *pdata)
{
    for (;;) {
        int fd = open_vpu_device();
        if (fd < 0) {
            ALOGV("failed to open /dev/vpu_service ret %d\n", fd);
            return NULL;
        }
        close(fd);
    }
    return NULL;
}

int main()
{
    int i;
    ALOGV("vpu test start\n");
    pthread_t threads[MAX_THREAD_NUM];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < MAX_THREAD_NUM; i++) {
        pthread_create(&thread[i], &attr, vpu_test, NULL);
    }
    pthread_attr_destroy(&attr);
    for (i = 0; i < 10; i++)
        sleep(1);

    void *dummy;

    for (i = 0; i < MAX_THREAD_NUM; i++) {
        pthread_join(&thread[i], &dummy);
    }
    ALOGV("vpu test end\n");
    return 0;
}
#endif
