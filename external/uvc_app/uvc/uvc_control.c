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

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include "uvc_control.h"
#include "uvc_encode.h"
#include "uvc_video.h"
#include "uevent.h"
#include "uvc_log.h"
#include "uvc_configfs.h"

//#include "camera_control.h"

#define UVC_STREAMING_INTF_PATH "/sys/kernel/config/usb_gadget/rockchip/functions/uvc.gs6/streaming/bInterfaceNumber"

#define UVC_STREAMING_MAXPACKET_PATH "/sys/kernel/config/usb_gadget/rockchip/functions/uvc.gs6/streaming_maxpacket"
int enable_minilog;
int uvc_app_log_level;
int app_quit;
int uvc_depth_cnt;
int uvc_rgb_cnt;


static void (*camera_start_callback)(int fd, int width, int height, int fps, int format, int eptz);
static void (*camera_stop_callback)();

uvc_pu_control_callback_t uvc_pu_control_cb = NULL;
void register_uvc_pu_control_callback(uvc_pu_control_callback_t cb)
{
    uvc_pu_control_cb = cb;
}

struct uvc_ctrl
{
    int id;
    bool start;
    bool stop;
    int width;
    int height;
    int fps;
    int format;
    int eptz;
    int suspend;
    struct uvc_function_config *fc;
};

static struct uvc_ctrl uvc_ctrl[3] =
{
    { -1, false, false, -1, -1, -1, 1, 0, 0, NULL},
    { -1, false, false, -1, -1, -1, 1, 0, 0, NULL},
    { -1, false, false, -1, -1, -1, 1, 0, 0, NULL}, //isp
};

struct uvc_encode uvc_enc;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int uvc_streaming_intf = -1;
static int uvc_streaming_maxpacket = 1024;

bool uvc_encode_init_flag = false;

static pthread_t run_id = 0;
static bool uvc_restart = false;
static bool run_flag = true;
static uint32_t uvc_flags = UVC_CONTROL_CAMERA;

static pthread_mutex_t run_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t run_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t video_added = PTHREAD_COND_INITIALIZER;

static bool is_uvc_video(void *buf)
{
    if (strstr(buf, "usb") || strstr(buf, "gadget"))
        return true;
    else
        return false;
}

static void query_uvc_streaming_maxpacket(void)
{
    int fd;

    fd = open(UVC_STREAMING_MAXPACKET_PATH, O_RDONLY);
    if (fd >= 0)
    {
        char intf[32] = {0};
        read(fd, intf, sizeof(intf) - 1);
        uvc_streaming_maxpacket = atoi(intf);
        if(uvc_streaming_maxpacket < 1023)
             uvc_streaming_maxpacket = 1024;
        LOG_DEBUG("uvc_streaming_maxpacket = %d\n", uvc_streaming_maxpacket);
        close(fd);
    }
    else
    {
        LOG_ERROR("open %s failed!\n", UVC_STREAMING_MAXPACKET_PATH);
    }
}

int get_uvc_streaming_maxpacket(void)
{
    return uvc_streaming_maxpacket;
}

static void query_uvc_streaming_intf(void)
{
    int fd;

    fd = open(UVC_STREAMING_INTF_PATH, O_RDONLY);
    if (fd >= 0)
    {
        char intf[32] = {0};
        read(fd, intf, sizeof(intf) - 1);
        uvc_streaming_intf = atoi(intf);
        LOG_DEBUG("uvc_streaming_intf = %d\n", uvc_streaming_intf);
        close(fd);
    }
    else
    {
        LOG_ERROR("open %s failed!\n", UVC_STREAMING_INTF_PATH);
    }
}

int get_uvc_streaming_intf(void)
{
    return uvc_streaming_intf;
}

void uvc_control_start_setcallback(int (*callback)(int fd, int width, int height, int fps, int format, int eptz))
{
    camera_start_callback = callback;
}

void uvc_control_stop_setcallback(void (*callback)())
{
    camera_stop_callback = callback;
}

static void get_uvc_index_to_name(unsigned int index, char *name)
{
    if (strstr(name, "depth") || strstr(name, "DEPTH") || strstr(name, "ir") || strstr(name, "IR")) {
        uvc_depth_cnt = index;
        LOG_DEBUG("uvc_depth_cnt = %d, name:%s\n",uvc_depth_cnt, name);
    } else if (strstr(name, "rgb") || strstr(name, "RGB")) {
        uvc_rgb_cnt = index;
        LOG_DEBUG("uvc_rgb_cnt = %d, name:%s\n",uvc_rgb_cnt,name);
    }
}


int check_uvc_video_id(void)
{
    struct uvc_function_config *fc[CAM_MAX_NUM];
    unsigned int i;
    for (i = 0; i < CAM_MAX_NUM; i++) {
        fc[i] = configfs_parse_uvc_function(i);
        if (fc[i]) {
            get_uvc_index_to_name(fc[i]->index, fc[i]->dev_name);
             uvc_ctrl[i].id = fc[i]->video;
             uvc_ctrl[i].fc = fc[i];
             LOG_DEBUG("uvc_function_config fc->video:%d,fc->streaming.num_formats:%d",fc[i]->video,uvc_ctrl[i].fc->streaming.num_formats);
        }
    }

    if (uvc_ctrl[0].id < 0 && uvc_ctrl[1].id < 0)
    {
        LOG_WARN("Please configure uvc...\n");
        return -1;
    }
    //query_uvc_streaming_intf();
    //query_uvc_streaming_maxpacket();
    return 0;
}

void add_uvc_video()
{
    if (uvc_ctrl[0].id >= 0)
    {
        uvc_video_id_add(uvc_ctrl[0].fc);
        LOG_INFO("uvc_ctrl[0].id:%d, uvc_ctrl[0].fc->video:%d\n",uvc_ctrl[0].id,uvc_ctrl[0].fc->video);
    }

    if (uvc_ctrl[1].id >= 0) {
        uvc_video_id_add(uvc_ctrl[1].fc);
    }
}

void uvc_control_init(int width, int height, int fcc, int h265, unsigned int fps, int id)
{
    pthread_mutex_lock(&lock);
    memset(&uvc_enc, 0, sizeof(uvc_enc));
    if (uvc_encode_init(&uvc_enc, width, height, fcc, h265, fps))
    {
        LOG_ERROR("%s fail!\n", __func__);
        abort();
    }
    uvc_enc.video_id = id;
    LOG_INFO("uvc_enc.video_id:%d",uvc_enc.video_id);
    uvc_encode_init_flag = true;
    pthread_mutex_unlock(&lock);
}

void uvc_control_inbuf_deinit()
{
    pthread_mutex_lock(&lock);
    if(uvc_encode_init_flag)
        uvc_encode_inbuf_deinit(&uvc_enc);
    pthread_mutex_unlock(&lock);
}

void uvc_control_exit()
{
    pthread_mutex_lock(&lock);
    if(uvc_encode_init_flag)
      uvc_encode_exit(&uvc_enc);
    memset(&uvc_enc, 0, sizeof(uvc_enc));
    uvc_encode_init_flag = false;
    pthread_mutex_unlock(&lock);
}

void uvc_read_camera_buffer(void *cam_buf, MPP_ENC_INFO_DEF *info,
                            void *extra_data, size_t extra_size)
{
#ifdef ENABLE_BUFFER_TIME_DEBUG
    struct timeval buffer_time;
    gettimeofday(&buffer_time, NULL);
    LOG_ERROR("UVC READ CAMERA BUFFER TIME:%d.%d (s)",buffer_time.tv_sec,buffer_time.tv_usec);
#endif
    pthread_mutex_lock(&lock);
    if (info->size <= uvc_enc.width * uvc_enc.height * 2)
    {
        //uvc_enc.video_id = uvc_video_id_get(0);
        uvc_enc.extra_data = extra_data;
        uvc_enc.extra_size = extra_size;
        uvc_encode_process(&uvc_enc, cam_buf, info);
    }
    else if (uvc_enc.width > 0 && uvc_enc.height > 0)
    {
        LOG_ERROR("%s: cam_size = %u, uvc_enc.width = %d, uvc_enc.height = %d\n",
                  __func__, info->size, uvc_enc.width, uvc_enc.height);
    }
    pthread_mutex_unlock(&lock);
}

static void uvc_control_wait(void)
{
    pthread_mutex_lock(&run_mutex);
    if (run_flag)
        pthread_cond_wait(&run_cond, &run_mutex);
    pthread_mutex_unlock(&run_mutex);
}

void uvc_control_signal(void)
{
    pthread_mutex_lock(&run_mutex);
    pthread_cond_signal(&run_cond);
    pthread_mutex_unlock(&run_mutex);
}

void uvc_added_signal(void)
{
    pthread_mutex_lock(&run_mutex);
    pthread_cond_signal(&video_added);
    pthread_mutex_unlock(&run_mutex);
}

static void uvc_added_wait(void)
{
    pthread_mutex_lock(&run_mutex);
    if (run_flag)
        pthread_cond_wait(&video_added, &run_mutex);
    pthread_mutex_unlock(&run_mutex);
}

static void *uvc_control_thread(void *arg)
{
    uint32_t flag = *(uint32_t *)arg;

    while (run_flag)
    {
        if (!check_uvc_video_id())
        {
            add_uvc_video();
            /* Ensure main was waiting for this signal */
            usleep(500);
            uvc_added_signal();
            if (flag & UVC_CONTROL_LOOP_ONCE)
                break;
            uvc_control_wait();
            uvc_video_id_exit_all();
        }
        else
        {
            uvc_control_wait();
        }
    }
    pthread_exit(NULL);
}

int uvc_control_loop(void)
{
    if (uvc_restart)
    {
        //uvc_video_id_exit_all();
        //add_uvc_video();
        uvc_restart = false;
        return 0;
    }
    if (uvc_ctrl[2].stop)
    {
#ifndef USE_RK_AISERVER
        if (camera_stop_callback)
            camera_stop_callback();
#endif
        uvc_ctrl[2].stop = false;
    }

    if (uvc_ctrl[2].start)
    {
        LOG_INFO("%s: video_id:%d, width:%d,height:%d,fps:%d,format:%d,eptz:%d !\n", __func__,
                 uvc_ctrl[2].id, uvc_ctrl[2].width, uvc_ctrl[2].height, uvc_ctrl[2].fps, uvc_ctrl[2].format, uvc_ctrl[2].eptz);
#ifndef USE_RK_AISERVER
        if (camera_start_callback)
        {
            LOG_INFO("%s  camera_start_callback start!\n", __func__);
            camera_start_callback(uvc_ctrl[2].id, uvc_ctrl[2].width, uvc_ctrl[2].height, uvc_ctrl[2].fps, uvc_ctrl[2].format, uvc_ctrl[2].eptz);
        }
#endif
        //camera_control_start(uvc_ctrl[2].id, uvc_ctrl[2].width, uvc_ctrl[2].height, uvc_ctrl[2].fps);
        uvc_ctrl[2].start = false;
#ifdef CAMERA_CONTROL
        // set fps later
        int set_fps = uvc_ctrl[2].fps;
        if (access("/tmp/uvc_no_set_fps", 0)){
           //sleep(1);
           if (access("/tmp/uvc_no_reduce_fps", 0) &&
               uvc_ctrl[2].format == V4L2_PIX_FMT_MJPEG && uvc_ctrl[2].height > 1440) {
               if (set_fps > 24)
                   set_fps = 24; // should >= 22
           }
           int timeout = 0;
           while(timeout < 4) {
              if(check_ispserver_work()) {
                camera_pu_control_set(UVC_PU_FPS_CONTROL, set_fps);// set camera fps
                break;
              }
              timeout++;
              if(timeout == 4){
                LOG_INFO("check ispserver is no ready,pu set fail!\n");
                break;
              }
              usleep(500000);
           }
        }
#endif
    }

    if(uvc_ctrl[2].suspend)
    {
       if (access("/tmp/uvc_no_suspend", 0)){
           sleep(5);
           if(uvc_ctrl[2].suspend){
              if (!uvc_video_get_uvc_process(uvc_ctrl[2].id))
            {
                LOG_INFO("uvc ready to suspend...\n");
                uvc_ctrl[2].suspend = 0;
                system("touch /tmp/uvc_goto_suspend");
            }
           }
       }
    }
    return 1;
}

int uvc_control_run(uint32_t flags)
{
    uvc_depth_cnt = -1;
    uvc_rgb_cnt = -1;
    uvc_flags = flags;
    if ((flags & UVC_CONTROL_CHECK_STRAIGHT) || (flags & UVC_CONTROL_CAMERA))
    {
        if (!check_uvc_video_id())
        {
            add_uvc_video();
        }
    }
    else
    {
        uevent_monitor_run(flags);
        if (pthread_create(&run_id, NULL, uvc_control_thread, &flags))
        {
            LOG_ERROR("%s: pthread_create failed!\n", __func__);
            return -1;
        }
        uvc_added_wait();
    }

    return 0;
}

void uvc_control_join(uint32_t flags)
{
    uvc_flags = flags;
    if ((flags & UVC_CONTROL_CHECK_STRAIGHT) || (flags & UVC_CONTROL_CAMERA))
    {
        uvc_video_id_exit_all();
        if (camera_stop_callback)
            camera_stop_callback();
    }
    else
    {
        run_flag = false;
        uvc_control_signal();
        pthread_join(run_id, NULL);
        if (flags & UVC_CONTROL_LOOP_ONCE)
            ;
        uvc_video_id_exit_all();
    }
}

void set_uvc_control_suspend(int suspend)
{
    LOG_INFO("suspend is %d\n",suspend);
    uvc_ctrl[2].suspend = suspend;
    if ( (suspend == 0) && !access("/tmp/uvc_goto_suspend", 0))
          system("rm /tmp/uvc_goto_suspend");
}

void set_uvc_control_start(int video_id, int width, int height, int fps, int format, int eptz)
{
    LOG_DEBUG("%s!\n", __func__);
    if (/*uvc_video_id_get(0) == video_id*/ 1)
    {
        LOG_DEBUG("%s: video_id:%d, width:%d,height:%d,fps:%d,eptz:%d!\n", __func__, video_id, width, height, fps, eptz);
        uvc_ctrl[2].id = video_id;
        uvc_ctrl[2].width = width;
        uvc_ctrl[2].height = height;
        uvc_ctrl[2].fps = fps;
        uvc_ctrl[2].format = format;
        uvc_ctrl[2].start = true;
        uvc_ctrl[2].eptz = eptz;
    }
    else
        LOG_ERROR("unexpect uvc!\n");
}

void set_uvc_control_stop(void)
{
    LOG_INFO("%s!\n", __func__);
    uvc_ctrl[2].stop = true;
}

void set_uvc_control_restart(void)
{
    if (uvc_flags & UVC_CONTROL_CAMERA)
    {
        LOG_INFO("received uvc to exit,restart to recovery now!\n");
        //system("killall -9 uvc_app &");
        uvc_restart = true;
    }
}
