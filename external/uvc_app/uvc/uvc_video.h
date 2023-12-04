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

#ifndef __UVC_VIDEO_H__
#define __UVC_VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include "uvc-gadget.h"

#define V4L2_PIX_FMT_H265     v4l2_fourcc('H', '2', '6', '5') /* H265 with start codes */

#define UVC_BUFFER_NUM 3 //use mjpeg_fps can set 4
#define YUYV_AS_RAW 0

#define UVC_DYNAMIC_DEBUG_USE_TIME 1 //release version can set to 0
#define UVC_DYNAMIC_DEBUG_USE_TIME_CHECK "/tmp/uvc_use_time"

#define UVC_DYNAMIC_DEBUG_FPS 1 //release version can set to 0
#define UVC_DYNAMIC_DEBUG_ISP_FPS_CHECK "/tmp/uvc_isp_fps"
#define UVC_DYNAMIC_DEBUG_IPC_FPS_CHECK "/tmp/uvc_ipc_fps"

#define UVC_SEND_BUF_WHEN_ENC_READY 1


#if UVC_DYNAMIC_DEBUG_FPS
struct uvc_debug_info_def
{
    bool first_frm;
    bool debug_isp_fps;
    bool debug_ipc_fps;
    float fps;
    int ipc_frm;
    int isp_frm;
    struct timeval enter_time;
    unsigned int stream_on_pts;
};

extern struct uvc_debug_info_def uvc_debug_info;
#endif

struct uvc_device;

struct uvc_buffer
{
    void *buffer;
    size_t size; // encode out size
    size_t total_size;
    int width;
    int height;
    int video_id;

    size_t drm_buf_size;
    int fd; // for packet buf fd
    unsigned int handle; // for drm handle

    int v4l2_fd;
    bool abandon;
    unsigned int pts;
    int frame_count;
    unsigned int seq;
};

struct uvc_user
{
    unsigned int width;
    unsigned int height;
    bool run;
    unsigned int fcc;
};

struct uvc_video
{
    int id;
    bool uvc_process;
    pthread_t uvc_pid;
    struct video_uvc *uvc;
    pthread_mutex_t buffer_mutex;
    pthread_mutex_t user_mutex;
    struct uvc_user uvc_user;
    int idle_cnt;
    struct uvc_buffer *buffer_s;
    int drm_fd; // drm fd
    bool can_exit;
    unsigned int last_pts;
    unsigned int now_pts;
    unsigned int last_seq;
    struct uvc_device *dev;
};

int uvc_gadget_pthread_create(struct uvc_function_config *fc);

int uvc_video_id_check(int id);
int uvc_video_id_add(struct uvc_function_config *fc);
void uvc_video_id_remove(int id);
void uvc_video_id_exit_all();
int uvc_video_id_get(unsigned int seq);

void uvc_video_set_uvc_process(int id, bool state);
bool uvc_video_get_uvc_process(int id);

int uvc_buffer_init(int id);
void uvc_buffer_deinit(int id);
bool uvc_buffer_write_enable(int id);
bool uvc_mpp_buffer_write_enable(int id);

void uvc_buffer_write(unsigned short stamp,
                      void *extra_data,
                      size_t extra_size,
                      void *data,
                      size_t size,
                      unsigned int fcc,
                      int id);
void uvc_set_user_resolution(int width, int height, int id);
void uvc_get_user_resolution(int *width, int *height, int id);
bool uvc_get_user_run_state(int id);
void uvc_set_user_run_state(bool state, int id);
void uvc_set_user_fcc(unsigned int fcc, int id);
unsigned int uvc_get_user_fcc(int id);
void uvc_memset_uvc_user(int id);
pthread_t *uvc_video_get_uvc_pid(int id);
void uvc_user_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf, int id);
void uvc_buffer_write_set(int id, struct uvc_buffer *buf);
void uvc_buffer_all_set(int id, struct uvc_buffer *buf);
struct uvc_buffer *uvc_get_enc_data(struct uvc_device *dev, struct uvc_video *v, bool init);
#if UVC_SEND_BUF_WHEN_ENC_READY
int uvc_video_qbuf_index(struct uvc_device *dev, struct uvc_buffer *send_buf, int index, int len);
struct uvc_buffer *uvc_user_fill_buffer_init(struct uvc_device *dev);
#endif
void uvc_user_set_write_buffer(struct uvc_device *dev, struct v4l2_buffer *buf, int id);
bool uvc_buffer_read_enable(int id);

#ifdef __cplusplus
}
#endif

#endif
