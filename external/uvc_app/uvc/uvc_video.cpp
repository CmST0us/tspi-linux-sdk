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

#include "uvc_video.h"
#include "uvc-gadget.h"
#include "yuv.h"
#include "mpi_enc.h"
#include "uvc_log.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <drm/drm.h>

#include <list>
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
#include "drm.h"
#endif
#if UVC_DYNAMIC_DEBUG_FPS
struct uvc_debug_info_def uvc_debug_info;
#endif

struct uvc_buffer_list
{
    std::list<struct uvc_buffer *> buffer_list;
    pthread_mutex_t mutex;
};

struct video_uvc
{
    struct uvc_buffer_list write;
    struct uvc_buffer_list read;
    struct uvc_buffer_list all;
    pthread_t id;
    bool run;
    int video_id;
};

static std::list<struct uvc_video *> lst_v;
static pthread_mutex_t mtx_v = PTHREAD_MUTEX_INITIALIZER;


static struct uvc_buffer *uvc_buffer_create(int width, int height, struct uvc_video *v)
{
    struct uvc_buffer *buffer = NULL;

    buffer = (struct uvc_buffer *)calloc(1, sizeof(struct uvc_buffer));
    if (!buffer)
        return NULL;
    buffer->width = width;
    buffer->height = height;
    buffer->size = buffer->width * buffer->height * 2;
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
    unsigned int drm_flag = 0;
    buffer->drm_buf_size = buffer->size;
    if (v->drm_fd == -1)
        v->drm_fd = drm_open();
    if (v->drm_fd < 0)
        return NULL;
    if (v->uvc_user.fcc == V4L2_PIX_FMT_YUYV || v->uvc_user.fcc == V4L2_PIX_FMT_NV12)
        drm_flag = ROCKCHIP_BO_CACHABLE;

    int ret = drm_alloc(v->drm_fd, buffer->drm_buf_size, 16, &buffer->handle, drm_flag);
    if (ret)
    {
        LOG_ERROR("drm_alloc fail\n");
        return NULL;
    }

    ret = drm_handle_to_fd(v->drm_fd, buffer->handle, &buffer->fd, DRM_CLOEXEC | DRM_RDWR);
    if (ret)
    {
        LOG_ERROR("drm_handle_to_fd fail\n");
        return NULL;
    }
    buffer->buffer = (void *)drm_map_buffer(v->drm_fd, buffer->handle, buffer->drm_buf_size);
    LOG_DEBUG("v->drm_fd=%d,buffer->handle=%d,size=%d\n", v->drm_fd, buffer->handle, buffer->drm_buf_size);
#else
    buffer->buffer = calloc(1, buffer->size);
#endif
    if (!buffer->buffer)
    {
        free(buffer);
        return NULL;
    }
    buffer->total_size = buffer->size;
    buffer->video_id = v->id;
    return buffer;
}

static void uvc_buffer_push_back(struct uvc_buffer_list *uvc_buffer,
                                 struct uvc_buffer *buffer)
{
    pthread_mutex_lock(&uvc_buffer->mutex);
    uvc_buffer->buffer_list.push_back(buffer);
    pthread_mutex_unlock(&uvc_buffer->mutex);
}

static struct uvc_buffer *uvc_buffer_pop_front(
    struct uvc_buffer_list *uvc_buffer)
{
    struct uvc_buffer *buffer = NULL;

    pthread_mutex_lock(&uvc_buffer->mutex);
    if (!uvc_buffer->buffer_list.empty())
    {
        buffer = uvc_buffer->buffer_list.front();
        uvc_buffer->buffer_list.pop_front();
    }
    pthread_mutex_unlock(&uvc_buffer->mutex);
    return buffer;
}

static struct uvc_buffer *uvc_buffer_front(struct uvc_buffer_list *uvc_buffer)
{
    struct uvc_buffer *buffer = NULL;

    pthread_mutex_lock(&uvc_buffer->mutex);
    if (!uvc_buffer->buffer_list.empty())
        buffer = uvc_buffer->buffer_list.front();
    else //{printf("buff null\n");
        buffer = NULL;
//     }
    pthread_mutex_unlock(&uvc_buffer->mutex);
    return buffer;
}

#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
static void uvc_drm_buffer_destroy(struct uvc_video *v, struct uvc_buffer_list *uvc_buffer)
{
    struct uvc_buffer *buffer = NULL;

    pthread_mutex_lock(&uvc_buffer->mutex);
    while (!uvc_buffer->buffer_list.empty())
    {
        buffer = uvc_buffer->buffer_list.front();
        LOG_INFO("uvc_drm_buffer_destroy buffer->handle=%d size=%d\n", buffer->handle, buffer->drm_buf_size);
        drm_unmap_buffer(buffer->buffer, buffer->drm_buf_size);
        close(buffer->fd);
        drm_free(v->drm_fd, buffer->handle);
        free(buffer);
        uvc_buffer->buffer_list.pop_front();
    }
    pthread_mutex_unlock(&uvc_buffer->mutex);
    pthread_mutex_destroy(&uvc_buffer->mutex);

}
#endif
static void uvc_buffer_destroy(struct uvc_buffer_list *uvc_buffer)
{
    struct uvc_buffer *buffer = NULL;

    pthread_mutex_lock(&uvc_buffer->mutex);
    while (!uvc_buffer->buffer_list.empty())
    {
        buffer = uvc_buffer->buffer_list.front();
        free(buffer->buffer);
        free(buffer);
        uvc_buffer->buffer_list.pop_front();
    }
    pthread_mutex_unlock(&uvc_buffer->mutex);
    pthread_mutex_destroy(&uvc_buffer->mutex);
}

static void uvc_buffer_clear(struct uvc_buffer_list *uvc_buffer)
{
    pthread_mutex_lock(&uvc_buffer->mutex);
    uvc_buffer->buffer_list.clear();
    pthread_mutex_unlock(&uvc_buffer->mutex);
}

static void *uvc_gadget_pthread(void *arg)
{
    struct uvc_function_config *fc = (struct uvc_function_config *)arg;
    int *id = (int *)arg;

    prctl(PR_SET_NAME, "uvc_gadget_pthread", 0, 0, 0);

    uvc_gadget_main(fc);
    uvc_set_user_run_state(true, fc->video);

    pthread_exit(NULL);
}

int uvc_gadget_pthread_create(struct uvc_function_config *fc)
{
    pthread_t *pid = NULL;
    int id = fc->video;
    uvc_memset_uvc_user(id);
    if ((pid = uvc_video_get_uvc_pid(id)))
    {
        if (pthread_create(pid, NULL, uvc_gadget_pthread, fc))
        {
            LOG_ERROR("create uvc_gadget_pthread fail!\n");
            return -1;
        }
    }
    return 0;
}

static int _uvc_video_id_check(int id)
{
    int ret = 0;

    if (!lst_v.empty())
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                ret = -1;
                break;
            }
        }
    }

    return ret;
}

int uvc_video_id_check(int id)
{
    int ret = 0;

    pthread_mutex_lock(&mtx_v);
    ret = _uvc_video_id_check(id);
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

int uvc_video_id_add(struct uvc_function_config *fc)
{
    int ret = 0;
    int id = fc->video;

    LOG_INFO("add uvc video id: %d\n", id);

    pthread_mutex_lock(&mtx_v);
    if (!_uvc_video_id_check(id))
    {
        struct uvc_video *v = (struct uvc_video *)calloc(1, sizeof(struct uvc_video));
        if (v)
        {
            v->id = id;
            lst_v.push_back(v);
            pthread_mutex_unlock(&mtx_v);
            uvc_gadget_pthread_create(fc);
            pthread_mutex_lock(&mtx_v);
            pthread_mutex_init(&v->buffer_mutex, NULL);
            pthread_mutex_init(&v->user_mutex, NULL);
            ret = 0;
        }
        else
        {
            LOG_ERROR("%s: %d: memory alloc fail.\n", __func__, __LINE__);
            ret = -1;
        }
    }
    else
    {
        LOG_WARN("%s: %d: %d already exist.\n", __func__, __LINE__, id);
        ret = -1;
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

void uvc_video_id_remove(int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                pthread_mutex_destroy(&l->buffer_mutex);
                pthread_mutex_destroy(&l->user_mutex);
                free(l);
                lst_v.erase(i);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

int uvc_video_id_get(unsigned int seq)
{
    int ret = -1;

    pthread_mutex_lock(&mtx_v);
    if (!lst_v.empty())
    {
        unsigned int cnt = 0;
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            if (cnt++ == seq)
            {
                struct uvc_video *l = *i;
                ret = l->id;
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

static void uvc_gadget_pthread_exit(int id);

static int uvc_video_id_exit(int id)
{
    if (uvc_video_id_check(id))
    {
        uvc_gadget_pthread_exit(id);
        uvc_video_id_remove(id);
        return 0;
    }

    return -1;
}

static int _uvc_video_id_exit_all()
{
    int ret = -1;

    pthread_mutex_lock(&mtx_v);
    if (!lst_v.empty())
    {
        struct uvc_video *l = lst_v.front();
        pthread_mutex_unlock(&mtx_v);
        uvc_video_id_exit(l->id);
        pthread_mutex_lock(&mtx_v);
        ret = 0;
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

void uvc_video_id_exit_all()
{
    while (!_uvc_video_id_exit_all())
        continue;
}

static void _uvc_video_set_uvc_process(struct uvc_video *v, bool state)
{
    v->uvc_process = state;
}

void uvc_video_set_uvc_process(int id, bool state)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_video_set_uvc_process(l, state);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static bool _uvc_video_get_uvc_process(struct uvc_video *v)
{
    return v->uvc_process;
}

bool uvc_video_get_uvc_process(int id)
{
    bool state = false;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                state = _uvc_video_get_uvc_process(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return state;
}

pthread_t *uvc_video_get_uvc_pid(int id)
{
    pthread_t *tid = NULL;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                tid = &l->uvc_pid;
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return tid;
}

void uvc_video_join_uvc_pid(int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                if (l->uvc_pid)
                {
                    pthread_mutex_unlock(&mtx_v);
                    pthread_join(l->uvc_pid, NULL);
                    l->uvc_pid = 0;
                    pthread_mutex_lock(&mtx_v);
                }
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static void uvc_gadget_pthread_exit(int id)
{
    while (!uvc_get_user_run_state(id))
        pthread_yield();
    uvc_set_user_run_state(false, id);
    uvc_video_join_uvc_pid(id);
}

static void _uvc_get_user_resolution(struct uvc_video *v, int *width, int *height);

static int _uvc_buffer_init(struct uvc_video *v)
{
    int i = 0;
    int ret = 0;
    struct uvc_buffer *buffer = NULL;
    int width, height;

    _uvc_get_user_resolution(v, &width, &height);

    pthread_mutex_lock(&v->buffer_mutex);

    v->uvc = new video_uvc();
    if (!v->uvc)
    {
        ret = -1;
        goto exit;
    }
    v->uvc->id = 0;
    v->uvc->video_id = v->id;
    v->uvc->run = 1;
    v->buffer_s = NULL;
    pthread_mutex_init(&v->uvc->write.mutex, NULL);
    pthread_mutex_init(&v->uvc->read.mutex, NULL);

    uvc_buffer_clear(&v->uvc->write);
    uvc_buffer_clear(&v->uvc->read);
    uvc_buffer_clear(&v->uvc->all);

    LOG_DEBUG("UVC_BUFFER_NUM = %d\n", UVC_BUFFER_NUM);
    v->drm_fd = -1;
    for (i = 0; i < UVC_BUFFER_NUM; i++)
    {
        buffer = uvc_buffer_create(width, height, v);
        if (!buffer)
        {
            ret = -1;
            goto exit;
        }
#if (UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF)
        uvc_buffer_push_back(&v->uvc->all, buffer);
#endif
        uvc_buffer_push_back(&v->uvc->write, buffer);

    }
    _uvc_video_set_uvc_process(v, true);
    v->can_exit = true;

exit:
    pthread_mutex_unlock(&v->buffer_mutex);
    return ret;
}

int uvc_buffer_init(int id)
{
    int ret = -1;
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                ret = _uvc_buffer_init(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

static void _uvc_buffer_deinit(struct uvc_video *v)
{
    pthread_mutex_lock(&v->buffer_mutex);
    if (v->uvc)
    {
        v->uvc->run = 0;
        _uvc_video_set_uvc_process(v, false);
        if (v->buffer_s)
            uvc_buffer_push_back(&v->uvc->write, v->buffer_s);
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER

#if UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF
        LOG_DEBUG("_uvc_buffer_deinit all\n");
        uvc_drm_buffer_destroy(v, &v->uvc->all);
#else
        LOG_DEBUG("_uvc_buffer_deinit write\n");
        uvc_drm_buffer_destroy(v, &v->uvc->write);
        LOG_DEBUG("_uvc_buffer_deinit read\n");
        uvc_drm_buffer_destroy(v, &v->uvc->read);
#endif
        drm_close(v->drm_fd);
        LOG_DEBUG("_uvc_buffer_deinit drm_close drm_fd:%d\n", v->drm_fd);
        v->drm_fd = -1;
#else
        uvc_buffer_destroy(&v->uvc->write);
        uvc_buffer_destroy(&v->uvc->read);
#endif
        delete v->uvc;
        v->uvc = NULL;
    }
    pthread_mutex_unlock(&v->buffer_mutex);
}

void uvc_buffer_deinit(int id)
{
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
    struct uvc_video *l;
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            l = *i;
            if (id == l->id)
            {
                break;
            }
        }
    }
    int wait_cnt = 0;
    while (l->uvc_process && !l->can_exit)
    {
        wait_cnt++;
        usleep(1000);
        if (wait_cnt > 30) {
          LOG_INFO("uvc_buffer_deinit timeout 30ms,force exit!\n");
          break;
        }
    }

    pthread_mutex_lock(&mtx_v);
    _uvc_buffer_deinit(l);
    pthread_mutex_unlock(&mtx_v);
#else
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_buffer_deinit(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
#endif
}

static bool _uvc_buffer_write_enable(struct uvc_video *v)
{
    bool ret = false;

    if (pthread_mutex_trylock(&v->buffer_mutex))
        return ret;
    if (v->uvc && uvc_buffer_front(&v->uvc->write))
        ret = true;
    pthread_mutex_unlock(&v->buffer_mutex);
    return ret;
}

bool uvc_buffer_write_enable(int id)
{
    bool ret = false;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                ret = _uvc_buffer_write_enable(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

static bool _uvc_buffer_read_enable(struct uvc_video *v)
{
    bool ret = false;

    if (pthread_mutex_trylock(&v->buffer_mutex))
        return ret;
    if (v->uvc && uvc_buffer_front(&v->uvc->read))
        ret = true;
    pthread_mutex_unlock(&v->buffer_mutex);
    return ret;
}

bool uvc_buffer_read_enable(int id)
{
    bool ret = false;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                ret = _uvc_buffer_read_enable(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

static bool _uvc_mpp_buffer_write_enable(struct uvc_video *v)
{
    bool ret = false;

    //if (pthread_mutex_trylock(&v->buffer_mutex))
    //    return ret;
    if (v->uvc && uvc_buffer_front(&v->uvc->write))
        ret = true;
    // pthread_mutex_unlock(&v->buffer_mutex);
    return ret;
}

bool uvc_mpp_buffer_write_enable(int id)
{
    bool ret = false;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                ret = _uvc_buffer_write_enable(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return ret;
}

#define EX_MAX_LEN 65535
#define EX_DATA_LEN (EX_MAX_LEN - 2)
static void _uvc_buffer_write(struct uvc_video *v,
                              unsigned short stamp,
                              void *extra_data,
                              size_t extra_size,
                              void *data,
                              size_t size,
                              unsigned int fcc)
{
    const size_t cnt = extra_size / (EX_DATA_LEN + 1) + 1;
    pthread_mutex_lock(&v->buffer_mutex);
    if (v->uvc && data)
    {
        struct uvc_buffer *buffer = uvc_buffer_pop_front(&v->uvc->write);
        if (buffer && buffer->buffer)
        {
            if (buffer->total_size >= extra_size + size)
            {
                switch (fcc)
                {
                case V4L2_PIX_FMT_YUYV:
#if YUYV_AS_RAW
#ifdef USE_RK_MODULE
                    raw16_to_raw8(buffer->width, buffer->height, data, buffer->buffer);
#else
                    memcpy(buffer->buffer, data, size);
#endif
#else
                    NV12_to_YUYV(buffer->width, buffer->height, data, buffer->buffer);
#endif
                    break;
                case V4L2_PIX_FMT_MJPEG:
                    if (extra_data && buffer->total_size >= extra_size + size + 4 * cnt)
                    {
                        size_t index = 4;// FF D8 FF E0
                        size_t len = *((unsigned char *)data + index);
                        len = len * 256 + *((unsigned char *)data + index + 1);
                        index = index + len;
                        memcpy(buffer->buffer, data, index);
                        size_t ind = index;
                        for (size_t i = 1; i < cnt; i++)
                        {
                            memset((char *)buffer->buffer + ind, 0xFF, 1);
                            ind++;
                            memset((char *)buffer->buffer + ind, 0xE2, 1);
                            ind++;
                            memset((char *)buffer->buffer + ind, EX_MAX_LEN / 256, 1);
                            ind++;
                            memset((char *)buffer->buffer + ind, EX_MAX_LEN % 256, 1);
                            ind++;
                            memcpy((char *)buffer->buffer + ind,
                                   (char *)extra_data + (i - 1) * EX_DATA_LEN, EX_DATA_LEN);
                            ind += EX_DATA_LEN;
                        }
                        memset((char *)buffer->buffer + ind, 0xFF, 1);
                        ind ++;
                        memset((char *)buffer->buffer + ind, 0xE2, 1);
                        ind++;
                        memset((char *)buffer->buffer + ind,
                               (2 + extra_size - EX_DATA_LEN * (cnt - 1)) / 256, 1);
                        ind++;
                        memset((char *)buffer->buffer + ind,
                               (2 + extra_size - EX_DATA_LEN * (cnt - 1)) % 256, 1);
                        ind++;
                        memcpy((char *)buffer->buffer + ind,
                               (char *)extra_data + EX_DATA_LEN * (cnt - 1),
                               extra_size - EX_DATA_LEN * (cnt - 1));
                        ind += extra_size - EX_DATA_LEN * (cnt - 1);
                        memcpy((char *)buffer->buffer + ind,
                               (char *)data + index, size - index);
                        extra_size += 4 * cnt;
                    }
                    else
                    {
                        memcpy(buffer->buffer, data, size);
                    }
                    //memcpy((char*)buffer->buffer + size, &stamp, sizeof(stamp));
                    //size += sizeof(stamp);
                    break;
                case V4L2_PIX_FMT_H264:
                case V4L2_PIX_FMT_H265:
                    if (extra_data && extra_size > 0)
                        memcpy(buffer->buffer, extra_data, extra_size);
                    if (extra_size >= 0)
                        memcpy((char *)buffer->buffer + extra_size, data, size);
                    break;
                }
                buffer->size = extra_size + size;
                uvc_buffer_push_back(&v->uvc->read, buffer);
            }
            else
            {
                uvc_buffer_push_back(&v->uvc->write, buffer);
            }
        }
    }
    pthread_mutex_unlock(&v->buffer_mutex);
}

void uvc_buffer_write(unsigned short stamp,
                      void *extra_data,
                      size_t extra_size,
                      void *data,
                      size_t size,
                      unsigned int fcc,
                      int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_buffer_write(l, stamp, extra_data, extra_size, data, size, fcc);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
struct uvc_buffer *uvc_buffer_write_get(int id)
{
    struct uvc_buffer *buffer = NULL;
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                l->can_exit = false;
                pthread_mutex_lock(&l->buffer_mutex);
                buffer = uvc_buffer_pop_front(&l->uvc->write);
                pthread_mutex_unlock(&l->buffer_mutex);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
    return buffer;
}

struct uvc_buffer *uvc_buffer_write_get_nolock(int id)
{
    struct uvc_buffer *buffer = NULL;
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                l->can_exit = false;
                pthread_mutex_lock(&l->buffer_mutex);
                buffer = uvc_buffer_pop_front(&l->uvc->write);
                //if (l->)
                pthread_mutex_unlock(&l->buffer_mutex);
                break;
            }
        }
    }
    return buffer;
}

void uvc_buffer_read_set_nolock(int id, struct uvc_buffer *buf)
{
#if 1
    //uvc_ipc_test_write();

#endif
    if (buf->abandon)
    {
        LOG_INFO("uvc_buffer_read_set_nolock abandon\n");
    }
    else if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                l->can_exit = true;
                pthread_mutex_lock(&l->buffer_mutex);
                uvc_buffer_push_back(&l->uvc->read, buf);
                pthread_mutex_unlock(&l->buffer_mutex);
//                l->shm_control->sendUVCBuffer();
                break;
            }
        }
    }
}

void uvc_buffer_read_set(int id, struct uvc_buffer *buf)
{
    pthread_mutex_lock(&mtx_v);
    if (buf->abandon)
    {
        LOG_INFO("uvc_buffer_read_set abandon\n");
    }
    else if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                l->can_exit = true;
                pthread_mutex_lock(&l->buffer_mutex);
                uvc_buffer_push_back(&l->uvc->read, buf);
                pthread_mutex_unlock(&l->buffer_mutex);
                break;
            }
        }
    }

    pthread_mutex_unlock(&mtx_v);
}

void uvc_buffer_write_set(int id, struct uvc_buffer *buf)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                // l->can_exit = true;
                pthread_mutex_lock(&l->buffer_mutex);
                uvc_buffer_push_back(&l->uvc->write, buf);
                pthread_mutex_unlock(&l->buffer_mutex);
                break;
            }
        }
    }

    pthread_mutex_unlock(&mtx_v);
}

void uvc_buffer_all_set(int id, struct uvc_buffer *buf)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                // l->can_exit = true;
                pthread_mutex_lock(&l->buffer_mutex);
                uvc_buffer_push_back(&l->uvc->all, buf);
                pthread_mutex_unlock(&l->buffer_mutex);
                break;
            }
        }
    }

    pthread_mutex_unlock(&mtx_v);
}

#endif
static void _uvc_set_user_resolution(struct uvc_video *v, int width, int height)
{
    pthread_mutex_lock(&v->user_mutex);
    v->uvc_user.width = width;
    v->uvc_user.height = height;
    LOG_DEBUG("uvc_user.width = %u, uvc_user.height = %u\n", v->uvc_user.width,
             v->uvc_user.height);
    pthread_mutex_unlock(&v->user_mutex);
}

void uvc_set_user_resolution(int width, int height, int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_set_user_resolution(l, width, height);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static void _uvc_get_user_resolution(struct uvc_video *v, int *width, int *height)
{
    pthread_mutex_lock(&v->user_mutex);
    *width = v->uvc_user.width;
    *height = v->uvc_user.height;
    pthread_mutex_unlock(&v->user_mutex);
}

void uvc_get_user_resolution(int *width, int *height, int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_get_user_resolution(l , width, height);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static bool _uvc_get_user_run_state(struct uvc_video *v)
{
    bool ret;

    pthread_mutex_lock(&v->user_mutex);
    ret = v->uvc_user.run;
    pthread_mutex_unlock(&v->user_mutex);

    return ret;
}

bool uvc_get_user_run_state(int id)
{
    bool state = false;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                state = _uvc_get_user_run_state(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return state;
}

#if UVC_SEND_BUF_WHEN_ENC_READY
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
int uvc_video_qbuf_index(struct uvc_device *dev, struct uvc_buffer *send_buf, int index, int len)
{
#if 0
    LOG_INFO("uvc_video_qbuf_index enter send_buf=%p,index:%d len=%d,dev->nbufs=%d\n",
                     send_buf, index, len, dev->nbufs);
#endif
    unsigned int i;
    int ret;
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof (req));

    req.count = dev->nbufs;
    req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    req.memory = V4L2_MEMORY_DMABUF;
    if (index == 0) {
        uvc_video_reqbufs(dev, dev->nbufs);
        uvc_video_stream(dev, 1);
        dev->vbuf_info = (struct v4l2_buffer_info *)calloc(dev->nbufs, sizeof(struct v4l2_buffer_info));
    }

    /* UVC standalone setup. */
    if (dev->run_standalone)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = req.type;
        buf.index = index;
        buf.memory = req.memory;

        struct uvc_buffer *uvc_buf = send_buf;//uvc_buffer_write_get(dev->video_id);
        buf.m.fd = uvc_buf->fd;
        buf.length = uvc_buf->total_size;
        buf.bytesused = len;
        dev->ubuf.bytesused = len;
        dev->vbuf_info[index].fd = buf.m.fd;
        dev->vbuf_info[index].uvc_buf = uvc_buf;
        dev->vbuf_info[index].index = index;

        if (ioctl(dev->uvc_fd, VIDIOC_QBUF, &buf) < 0)
        {
            LOG_ERROR("%s ioctl(VIDIOC_QBUF): %m,buf.m.fd=%d,buf.length=%d\n", dev, buf.m.fd,  buf.length);
            return -1;
        }
        else
        {
            LOG_INFO("V4L2: %u buffers allocated index:%d\n", dev->nbufs, index);
        }
    }

    return 0;
}
#endif
#endif
static void _uvc_set_user_run_state(struct uvc_video *v, bool state)
{
    pthread_mutex_lock(&v->user_mutex);
    v->uvc_user.run = state;
    pthread_mutex_unlock(&v->user_mutex);
}

void uvc_set_user_run_state(bool state, int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_set_user_run_state(l, state);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static void _uvc_set_user_fcc(struct uvc_video *v, unsigned int fcc)
{
    v->uvc_user.fcc = fcc;
}

void uvc_set_user_fcc(unsigned int fcc, int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_set_user_fcc(l, fcc);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static unsigned int _uvc_get_user_fcc(struct uvc_video *v)
{
    return v->uvc_user.fcc;
}

unsigned int uvc_get_user_fcc(int id)
{
    unsigned int fcc = 0;

    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                fcc = _uvc_get_user_fcc(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);

    return fcc;
}

static void _uvc_memset_uvc_user(struct uvc_video *v)
{
    memset(&v->uvc_user, 0, sizeof(struct uvc_user));
}

void uvc_memset_uvc_user(int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_memset_uvc_user(l);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static bool _uvc_buffer_check(struct uvc_video *v, struct uvc_buffer *buffer)
{
    int width = 0, height = 0;

    _uvc_get_user_resolution(v, &width, &height);
    if (buffer->width == width && buffer->height == height)
        return true;
    else
        return false;
}

static void uvc_delay_time_calcu_before_get(struct uvc_device *dev, struct uvc_video *v)
{
#if UVC_DYNAMIC_DEBUG_USE_TIME
    if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0))
    {
        int32_t use_time_us, now_time_us;
        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us
        use_time_us = now_time_us - v->last_pts;   //usb send ok and calcu the latency time use last pts
        LOG_INFO("isp->mpp->usb_ready->usb_send_ok seq:%d latency time:%d us, %d ms\n", v->last_seq, use_time_us, use_time_us / 1000);
    }

    if (dev->usb_state == USB_STATE_FIRST_GET_READY)
    {
        int32_t use_time_us;
        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        dev->usb_state = USB_STATE_FIRST_GET_OK;
        dev->first_usb_get_ready_pts = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000;
        use_time_us = dev->first_usb_get_ready_pts - dev->stream_on_pts;
        LOG_INFO("steamon->get_ready latency time:%d us, %d ms\n", use_time_us, use_time_us / 1000);
    }
    else if (dev->usb_state == USB_STATE_FIRST_SEND_OK)
    {
        struct timespec now_tm = {0, 0};
        int32_t use_time_us;
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        dev->first_usb_send_ok_pts = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000;
        dev->usb_state = USB_STATE_NORMAL_RUN;
        use_time_us = dev->first_usb_send_ok_pts - dev->stream_on_pts;
        LOG_INFO("steamon->get_ready->get_ok->send_ok latency time:%d us, %d ms\n", use_time_us, use_time_us / 1000);
        use_time_us = dev->first_usb_send_ok_pts - v->last_pts;
        LOG_INFO("isp->mpp->usb_ready->usb_send_ok seq:%d latency time:%d us, %d ms\n", v->last_seq, use_time_us, use_time_us / 1000);
    }
#endif

}

static void uvc_delay_time_calcu_after_get(struct uvc_device *dev, struct uvc_video *v, struct uvc_buffer *buffer)
{
#if UVC_DYNAMIC_DEBUG_USE_TIME
    if (dev->usb_state == USB_STATE_FIRST_GET_OK)
    {
        struct timespec now_tm = {0, 0};
        int32_t use_time_us;
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        dev->first_usb_get_ok_pts = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000;
        dev->usb_state = USB_STATE_FIRST_SEND_OK;
        use_time_us = dev->first_usb_get_ok_pts - dev->stream_on_pts;
        LOG_INFO("steamon->get_ready->get_ok seq:%d time:%d us, %d ms\n", buffer->seq, use_time_us, use_time_us / 1000);
    }

    v->last_pts = buffer->pts;
    v->now_pts = buffer->pts;
    v->last_seq = buffer->seq;
    if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0))
    {
        int32_t use_time_us, now_time_us;
        struct timespec now_tm = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &now_tm);
        now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us
        use_time_us = now_time_us - v->now_pts;
        LOG_INFO("isp->mpp->usb_ready seq:%d latency time:%d us, %d ms\n", buffer->seq, use_time_us, use_time_us / 1000);
    }
#endif
}
static int drop_frame_cnt = 0;
struct uvc_buffer *uvc_get_enc_data(struct uvc_device *dev, struct uvc_video *v, bool init)
{
    struct uvc_buffer *buffer = NULL;
    v->idle_cnt = 0;
    int time_out = 60;
    if (init)
        time_out = 30;
    while (!(buffer = uvc_buffer_front(&v->uvc->read)) && _uvc_get_user_run_state(v))
    {
        pthread_mutex_unlock(&mtx_v);
        usleep(1000);
        v->idle_cnt++;
        pthread_mutex_lock(&mtx_v);
        if (v->idle_cnt > time_out)
        {
            if(!(buffer = uvc_buffer_front(&v->uvc->read)) && _uvc_get_user_run_state(v)) // onece more check it.
            {
                if (init == false) {
                    if(drop_frame_cnt == 0) {
                      LOG_DEBUG("fill buf timeout %d ms, abandon this write buf %d\n",time_out, v->buffer_s->fd);
                    }
                    //v->buffer_s->abandon = true;
                    //uvc_buffer_pop_front(&v->uvc->write);
                } else {
                    if(drop_frame_cnt == 0) {
                       LOG_DEBUG("init:%d,fill buf timeout %d ms\n", init, time_out);
                    }
                }
                drop_frame_cnt++;
                if(drop_frame_cnt > 200) {
                   if (access("/tmp/uvc_camera_no_buf", 0)) {
                     LOG_INFO("it's already 200 frames buf no to get from CAMERA,tell watchdog now\n");
                     system("touch /tmp/uvc_camera_no_buf &");
                   }
                   drop_frame_cnt = 0;
                }
            }
            break;
        }
    }
   // LOG_INFO("init:%d,uvc_get_enc_data: %p\n", init, buffer);
#if UVC_SEND_BUF_WHEN_ENC_READY
    if (init && buffer) {
        uvc_video_qbuf_index(dev, buffer, dev->get_buf_count, buffer->size);
    }
#endif

    if (buffer)
        uvc_delay_time_calcu_after_get(dev, v, buffer);
    return buffer;
}

#if UVC_SEND_BUF_WHEN_ENC_READY
struct uvc_buffer *uvc_user_fill_buffer_init(struct uvc_device *dev)
{
    pthread_mutex_lock(&mtx_v);
    struct uvc_buffer *buffer = NULL;
    if (_uvc_video_id_check(dev->video_id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (dev->video_id == l->id)
            {
                uvc_delay_time_calcu_before_get(dev, l);
                if(uvc_get_enc_data(dev, l, true)) {
                    buffer = uvc_buffer_pop_front(&l->uvc->read);
                }
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
    return buffer;
}
#endif

static void _uvc_user_fill_buffer(struct uvc_video *v, struct uvc_device *dev, struct v4l2_buffer *buf)
{
    struct uvc_buffer *buffer = NULL;
    bool get_ok = false;
    uvc_delay_time_calcu_before_get(dev, v);

#if 0//UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF
    for (int i = 0; i < dev->nbufs; i++)
    {
        if (dev->vbuf_info[i].fd == buf->m.fd)
        {
            v->buffer_s = dev->vbuf_info[i].uvc_buf;
            v->buffer_s->abandon = false;
            break;
        }
    }
    uvc_buffer_push_back(&v->uvc->write, v->buffer_s);
#endif

    buffer = uvc_get_enc_data(dev, v, false);
    if (buffer)
    {
        for (int i = 0; i < dev->nbufs; i++)
        {
            if (buffer == dev->vbuf_info[i].uvc_buf)
            {
                buf->index = dev->vbuf_info[i].index;
                buf->sequence = buffer->seq;
                buf->m.fd = dev->vbuf_info[i].fd;
                get_ok = true;
                break;
            }
        }

        if (get_ok == false || !_uvc_buffer_check(v, buffer)) {
            LOG_ERROR("fail:%d\n", get_ok);
            return;
        }
        if (_uvc_get_user_run_state(v) && _uvc_video_get_uvc_process(v))
        {
            if (buf->length >= buffer->size && buffer->buffer)
            {
                // buf->bytesused = 550*1024;//for test
                buf->bytesused = buffer->size;
#if UVC_IO_METHOD == UVC_IO_METHOD_MMAP
                memcpy(dev->mem[buf->index].start, buffer->buffer, buffer->size);
#elif UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF

#else
                dev->mem[buf->index].start = buffer->buffer;
#endif
                drop_frame_cnt = 0;
            }
        }
        else
        {
            LOG_WARN("_uvc_user_fill_buffer no go here \n");
#if UVC_IO_METHOD == UVC_IO_METHOD_MMAP
            buf->bytesused = buf->length;
#elif UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF
            buf->bytesused = 0;
#else
            buf->bytesused = buf->length;
#endif
        }

        buffer = uvc_buffer_pop_front(&v->uvc->read);
#if UVC_IO_METHOD == UVC_IO_METHOD_MMAP
        if (!v->buffer_s)
        {
            v->buffer_s = buffer;
        }
        else
        {
            uvc_buffer_push_back(&v->uvc->write, v->buffer_s);
            v->buffer_s = buffer;
        }
#elif UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF

#else

#endif

    }
    else if (v->buffer_s)
    {
        if (!_uvc_buffer_check(v, v->buffer_s))
            return;
        if (_uvc_get_user_run_state(v) && _uvc_video_get_uvc_process(v))
        {
            if (buf->length >= v->buffer_s->size && v->buffer_s->buffer)
            {
                buf->bytesused = v->buffer_s->size;
#if UVC_IO_METHOD == UVC_IO_METHOD_MMAP
                memcpy(dev->mem[buf->index].start, v->buffer_s->buffer, v->buffer_s->size);
#elif UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF
                buf->bytesused = 0;
                buf->m.fd = 0;
#else
                dev->mem[buf->index].start = v->buffer_s->buffer;
#endif
            }
        }
    }
    else
    {
        buf->bytesused = 0;
        buf->m.fd = 0;
#if UVC_IO_METHOD == UVC_IO_METHOD_MMAP
        memset(dev->mem[buf->index].start, 0, buf->length);
#elif UVC_IO_METHOD == UVC_IO_METHOD_DMA_BUFF

#else

#endif
    }

}

void uvc_user_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf, int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_user_fill_buffer(l, dev, buf);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

static void _uvc_user_set_write_buffer(struct uvc_video *v, struct uvc_device *dev, struct v4l2_buffer *buf)
{
    struct uvc_buffer *buffer = NULL;
    bool get_ok = false;
 //   uvc_delay_time_calcu_before_get(dev, v);
    for (int i = 0; i < dev->nbufs; i++)
    {
        if (dev->vbuf_info[i].fd == buf->m.fd)
        {
            v->buffer_s = dev->vbuf_info[i].uvc_buf;
            v->buffer_s->abandon = false;
            get_ok = true;
            break;
        }
    }
    if (get_ok)
        uvc_buffer_push_back(&v->uvc->write, v->buffer_s);
    else
        LOG_ERROR("fail:%d\n", buf->m.fd);
}

void uvc_user_set_write_buffer(struct uvc_device *dev, struct v4l2_buffer *buf, int id)
{
    pthread_mutex_lock(&mtx_v);
    if (_uvc_video_id_check(id))
    {
        for (std::list<struct uvc_video *>::iterator i = lst_v.begin(); i != lst_v.end(); ++i)
        {
            struct uvc_video *l = *i;
            if (id == l->id)
            {
                _uvc_user_set_write_buffer(l, dev, buf);
                break;
            }
        }
    }
    pthread_mutex_unlock(&mtx_v);
}

void uvc_user_lock()
{
    pthread_mutex_lock(&mtx_v);
}

void uvc_user_unlock()
{
    pthread_mutex_unlock(&mtx_v);
}

