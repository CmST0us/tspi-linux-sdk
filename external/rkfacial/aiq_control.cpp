/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 * author: Zhihua Wang, hogan.wang@rock-chips.com
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

#include <list>

#include "aiq_control.h"
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>

#define IQFILES_PATH "/etc/iqfiles"
#define AIQ_FILE_PATH_LEN 64
#define AIQ_WIDTH 2688
#define AIQ_HEIGHT 1520
#define IR_SENSOR_MEDIA_MODEL   "rkcif_mipi_lvds"
#define RGB_SENSOR_MEDIA_MODEL  "rkisp0"
#define RGB_ISPP_MEDIA_MODEL  "rkispp0"
#define IR_ISPP_MEDIA_MODEL "rkispp1"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

/* Private v4l2 event */
#define CIFISP_V4L2_EVENT_STREAM_START (V4L2_EVENT_PRIVATE_START + 1)
#define CIFISP_V4L2_EVENT_STREAM_STOP (V4L2_EVENT_PRIVATE_START + 2)

struct aiq_control {
    rk_aiq_working_mode_t mode;
    rk_aiq_sys_ctx_t *ctx;
    aiq_control_type type;
    char sensor[AIQ_FILE_PATH_LEN];
    char ispp[AIQ_FILE_PATH_LEN];
    bool ok;
};

static std::list<struct aiq_control *> g_aiq;

static int xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static int get_devname(struct media_device *device, const char *name, char *dev_name)
{
    const char *devname;
    struct media_entity *entity = NULL;

    entity = media_get_entity_by_name(device, name, strlen(name));
    if (!entity)
        return -1;

    devname = media_entity_get_devname(entity);

    if (!devname)
        return -1;

    strncpy(dev_name, devname, AIQ_FILE_PATH_LEN);

    printf("get %s devname: %s\n", name, dev_name);

    return 0;
}

static int enumrate_modules(struct media_device *device, char *name, size_t size)
{
    uint32_t nents, i;
    const char *dev_name = NULL;
    int active_sensor = -1;

    nents = media_get_entities_count(device);
    for (i = 0; i < nents; ++i) {
        struct media_entity *e;
        const struct media_entity_desc *ef;
        const struct media_link *link;

        e = media_get_entity(device, i);
        ef = media_entity_get_info(e);
        if (ef->type != MEDIA_ENT_T_V4L2_SUBDEV_SENSOR)
            continue;

        if (ef->name[0] != 'm' && ef->name[3] != '_') {
            fprintf(stderr, "sensor entity name format is incorrect\n");
            return -1;
        }

        dev_name = media_entity_get_devname(e);

        switch (ef->type) {
        case MEDIA_ENT_T_V4L2_SUBDEV_SENSOR:
            link = media_entity_get_link(e, 0);
            if (link && (link->flags & MEDIA_LNK_FL_ENABLED)) {
                active_sensor = 1;
                strncpy(name, ef->name, size);
            }
            break;
        default:
            break;
        }
    }

    if (active_sensor < 0) {
        fprintf(stderr, "Not sensor link is enabled, does sensor probe correctly?\n");
        return -1;
    }

    return 0;
}

static int aiq_control_get_sensor_entity_name(aiq_control_type type, char *name, size_t size, char *ispp)
{
    char mdev_path[AIQ_FILE_PATH_LEN];
    int ret = -1;
    struct media_device *device = NULL;

    for (int i = 0;; i++) {
        sprintf(mdev_path, "/dev/media%d", i);
        if (access(mdev_path, F_OK))
            break;

        device = media_device_new(mdev_path);
        if (device == NULL) {
            printf("Failed to create media %s\n", mdev_path);
            continue;
        }

        if (media_device_enumerate(device) < 0) {
            printf("Failed to enumerate %s (%d)\n", mdev_path, ret);
            media_device_unref(device);
            continue;
        }

        const struct media_device_info *info = media_get_info(device);
        if (AIQ_CONTROL_IR == type && !strcmp(info->model, IR_SENSOR_MEDIA_MODEL) ||
                AIQ_CONTROL_RGB == type && !strcmp(info->model, RGB_SENSOR_MEDIA_MODEL))
            ret = enumrate_modules(device, name, size);

        if (AIQ_CONTROL_IR == type && !strcmp(info->model, IR_ISPP_MEDIA_MODEL) ||
                AIQ_CONTROL_RGB == type && !strcmp(info->model, RGB_ISPP_MEDIA_MODEL))
            get_devname(device, "rkispp_input_params", ispp);

        media_device_unref(device);
    }

    return ret;
}

static int wait_stream_event(int fd, unsigned int event_type, int time_out_ms)
{
    int ret;
    struct v4l2_event event;

    CLEAR(event);

    do {
        /*
         * xioctl instead of poll.
         * Since poll() cannot wait for input before stream on,
         * it will return an error directly. So, use ioctl to
         * dequeue event and block until sucess.
         */
        ret = xioctl(fd, VIDIOC_DQEVENT, &event);
        if (ret == 0 && event.type == event_type) {
            return 0;
        }
    } while (true);

    return -1;
}

static int subscrible_stream_event(char *ispp, int fd, bool subs) {
    struct v4l2_event_subscription sub;
    int ret = 0;

    CLEAR(sub);
    sub.type = CIFISP_V4L2_EVENT_STREAM_START;
    ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret) {
        printf("can't subscribe %s start event!\n", ispp);
        exit(EXIT_FAILURE);
    }

    CLEAR(sub);
    sub.type = CIFISP_V4L2_EVENT_STREAM_STOP;
    ret = xioctl(fd, subs ? VIDIOC_SUBSCRIBE_EVENT : VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (ret) {
        printf("can't subscribe %s stop event!\n", ispp);
    }

    printf("subscribe events from %s success !\n", ispp);

    return 0;
}

static void *aiq_thread(void *arg)
{
    struct aiq_control *aiq = (struct aiq_control *)arg;

    while (1) {
        int isp_fd = open(aiq->ispp, O_RDWR);
        if (isp_fd < 0) {
            printf("open %s failed %s\n", aiq->ispp, strerror(errno));
            pthread_exit(NULL);
        }
        subscrible_stream_event(aiq->ispp, isp_fd, true);

        rk_aiq_sys_ctx_t *ctx = rk_aiq_uapi_sysctl_init(aiq->sensor, IQFILES_PATH, NULL, NULL);
        if (!ctx) {
            printf("%s: rk_aiq_uapi_sysctl_init fail!\n", __func__);
            pthread_exit(NULL);
        }
        rk_aiq_uapi_sysctl_setMulCamConc(ctx, true);
        if (rk_aiq_uapi_sysctl_prepare(ctx, AIQ_WIDTH, AIQ_HEIGHT, aiq->mode)) {
            printf("%s: rk_aiq_uapi_sysctl_prepare fail!\n", __func__);
            rk_aiq_uapi_sysctl_deinit(ctx);
            pthread_exit(NULL);
        }
        aiq->ok = true;
        wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_START, -1);
        aiq->ctx = ctx;
        if (rk_aiq_uapi_sysctl_start(ctx) < 0) {
            printf("%s: rk_aiq_uapi_sysctl_start fail!\n", __func__);
            rk_aiq_uapi_sysctl_deinit(ctx);
            pthread_exit(NULL);
        }
        wait_stream_event(isp_fd, CIFISP_V4L2_EVENT_STREAM_STOP, -1);
        aiq->ctx = NULL;
        aiq->ok = false;
        rk_aiq_uapi_sysctl_stop(ctx, false);
        rk_aiq_uapi_sysctl_deinit(ctx);
        subscrible_stream_event(aiq->ispp, isp_fd, false);
        close(isp_fd);
    }
    pthread_detach(pthread_self());
    pthread_exit(NULL);
}

static void aiq_alloc(aiq_control_type type)
{
    char name[AIQ_FILE_PATH_LEN] = "";
    char ispp[AIQ_FILE_PATH_LEN] = "";
    if (!aiq_control_get_sensor_entity_name(type, name, sizeof(name), ispp) && strlen(ispp)) {
        struct aiq_control *aiq = (struct aiq_control *)calloc(1, sizeof(struct aiq_control));
        if (!aiq) {
            printf("%s alloc fail!\n", __func__);
            return;
        }
        if (type == AIQ_CONTROL_RGB)
            aiq->mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
        else
            aiq->mode = RK_AIQ_WORKING_MODE_NORMAL;
        aiq->type = type;
        strncpy(aiq->ispp, ispp, sizeof(aiq->ispp));
        strncpy(aiq->sensor, name, sizeof(aiq->sensor));
        g_aiq.push_back(aiq);
        pthread_t tid;
        if (pthread_create(&tid, NULL, aiq_thread, aiq))
            printf("create aiq_thread fail!\n");
    }
}

int aiq_control_alloc(void)
{
    aiq_alloc(AIQ_CONTROL_RGB);
    aiq_alloc(AIQ_CONTROL_IR);
    return 0;
}

void aiq_control_setExpGainRange(enum aiq_control_type type, paRange_t range)
{
    for (auto &aiq : g_aiq) {
        if (aiq->type == type) {
            rk_aiq_uapi_getExpGainRange(aiq->ctx, &range);
            rk_aiq_uapi_setExpGainRange(aiq->ctx, &range);
            break;
        }
    }
}

bool aiq_control_get_status(enum aiq_control_type type)
{
    bool ok = false;
    for (auto &aiq : g_aiq) {
        if (aiq->type == type) {
            ok = aiq->ok;
            break;
        }
    }
    return ok;
}
