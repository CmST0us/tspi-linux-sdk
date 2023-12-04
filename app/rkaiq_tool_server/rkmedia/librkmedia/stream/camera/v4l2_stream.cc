// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_stream.h"

#include <cstring>
#include <fcntl.h>

#include "control.h"

ispp_t rkispp_hw_info;
isp_t rkisp_hw_info;

namespace easymedia
{

    V4L2Context::V4L2Context(enum v4l2_buf_type cap_type, v4l2_io io_func, const std::string device)
        : fd(-1), capture_type(cap_type), vio(io_func), started(false)
#ifndef NDEBUG
          ,
          path(device)
#endif
    {
        const char* dev = device.c_str();
        fd = v4l2_open(dev, O_RDWR | O_CLOEXEC, 0);
        if (fd < 0) {
            LOG("open %s failed %m\n", dev);
        }
        LOG("open %s, fd %d\n", dev, fd);
    }

    V4L2Context::~V4L2Context()
    {
        if (fd >= 0) {
            SetStarted(false);
            v4l2_close(fd);
            LOGD("close %s, fd %d\n", path.c_str(), fd);
        }
    }

    bool V4L2Context::SetStarted(bool val)
    {
        std::lock_guard<std::mutex> _lk(mtx);
        if (started == val) {
            return true;
        }
        enum v4l2_buf_type cap_type = capture_type;
        unsigned int request = val ? VIDIOC_STREAMON : VIDIOC_STREAMOFF;
        if (IoCtrl(request, &cap_type) < 0) {
            LOG("ioctl(%d): %m\n", (int)request);
            return false;
        }
        started = val;
        return true;
    }

    int V4L2Context::IoCtrl(unsigned long int request, void* arg)
    {
        if (fd < 0) {
            errno = EINVAL;
            return -1;
        }
        return V4L2IoCtl(&vio, fd, request, arg);
    }

    V4L2MediaCtl::V4L2MediaCtl()
    {
    }

    V4L2MediaCtl::~V4L2MediaCtl()
    {
    }

    int get_ispp_subdevs(struct media_device* device, const char* devpath, ispp_t* ispp_info)
    {
        media_entity* entity = NULL;
        const char* entity_name = NULL;

        if (!device || !ispp_info || !devpath) {
            return -1;
        }

        strncpy(ispp_info->media_dev_path, devpath, sizeof(ispp_info->media_dev_path));

        entity = media_get_entity_by_name(device, "rkispp_m_bypass", strlen("rkispp_m_bypass"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(ispp_info->ispp_m_bypass_path, entity_name, sizeof(ispp_info->ispp_m_bypass_path));
            }
        }
        entity = media_get_entity_by_name(device, "rkispp_scale0", strlen("rkispp_scale0"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(ispp_info->ispp_scale0_path, entity_name, sizeof(ispp_info->ispp_scale0_path));
            }
        }
        entity = media_get_entity_by_name(device, "rkispp_scale1", strlen("rkispp_scale1"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(ispp_info->ispp_scale1_path, entity_name, sizeof(ispp_info->ispp_scale1_path));
            }
        }
        entity = media_get_entity_by_name(device, "rkispp_scale2", strlen("rkispp_scale2"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(ispp_info->ispp_scale2_path, entity_name, sizeof(ispp_info->ispp_scale2_path));
            }
        }

        return 0;
    }

    int get_isp_subdevs(struct media_device* device, const char* devpath, isp_t* isp_info)
    {
        media_entity* entity = NULL;
        const char* entity_name = NULL;

        if (!device || !isp_info || !devpath) {
            return -1;
        }

        strncpy(isp_info->media_dev_path, (char*)devpath, sizeof(isp_info->media_dev_path));
        entity = media_get_entity_by_name(device, "rkisp_mainpath", strlen("rkisp_mainpath"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(isp_info->isp_main_path, (char*)entity_name, sizeof(isp_info->isp_main_path));
            }
        }
        entity = media_get_entity_by_name(device, "rkisp_selfpath", strlen("rkisp_selfpath"));
        if (entity) {
            entity_name = media_entity_get_devname(entity);
            if (entity_name) {
                strncpy(isp_info->isp_self_path, (char*)entity_name, sizeof(isp_info->isp_self_path));
            }
        }

        return 0;
    }

    int V4L2MediaCtl::InitHwInfos()
    {
        char sys_path[64];
        FILE* fp = NULL;
        struct media_device* device = NULL;
        uint32_t i = 0;

        while (i < MAX_MEDIA_INDEX) {
            snprintf(sys_path, 64, "/dev/media%d", i++);
            fp = fopen(sys_path, "r");
            if (!fp) {
                continue;
            }
            fclose(fp);
            device = media_device_new(sys_path);

            /* Enumerate entities, pads and links. */
            media_device_enumerate(device);

            struct media_entity* entity = nullptr;
            entity = media_get_entity_by_name(device, RKISP_SUBDEV_NAME, strlen(RKISP_SUBDEV_NAME));
            if (entity != NULL) {
                LOG("find %s: %s\n", sys_path, RKISP_SUBDEV_NAME);
                get_isp_subdevs(device, sys_path, &rkisp_hw_info);
                goto media_unref;
            }

            entity = media_get_entity_by_name(device, RKIISPP_SUBDEV_NAME, strlen(RKIISPP_SUBDEV_NAME));
            if (entity != NULL) {
                LOG("find %s: %s\n", sys_path, RKIISPP_SUBDEV_NAME);
                get_ispp_subdevs(device, sys_path, &rkispp_hw_info);
                goto media_unref;
            }

        media_unref:
            media_device_unref(device);
        }
        return 0;
    }

    V4L2Stream::V4L2Stream(const char* param) : use_libv4l2(false), fd(-1), capture_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        memset(&vio, 0, sizeof(vio));
        std::map<std::string, std::string> params;
        std::list<std::pair<const std::string, std::string&>> req_list;
        std::string str_libv4l2;
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_USE_LIBV4L2, str_libv4l2));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_DEVICE, device));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_SUB_DEVICE, sub_device));
        std::string cap_type;
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_V4L2_CAP_TYPE, cap_type));
        int ret = parse_media_param_match(param, params, req_list);
        if (ret == 0) {
            return;
        }
        if (!str_libv4l2.empty()) {
            use_libv4l2 = !!std::stoi(str_libv4l2);
        }
        if (!cap_type.empty())
            capture_type = static_cast<enum v4l2_buf_type>(GetV4L2Type(cap_type.c_str()));
        v4l2_medctl = std::make_shared<V4L2MediaCtl>();
        if (v4l2_medctl) {
            ret = v4l2_medctl->InitHwInfos();
            if (ret) {
                return;
            }
        }
    }

    int V4L2Stream::Open()
    {
        if (!SetV4L2IoFunction(&vio, use_libv4l2)) {
            return -EINVAL;
        }
        if (!sub_device.empty()) {
            // TODO:
        }

        if (!strcmp(device.c_str(), MB_ENTITY_NAME)) {
            devname = std::string(rkispp_hw_info.ispp_m_bypass_path);
        } else if (!strcmp(device.c_str(), S0_ENTITY_NAME)) {
            devname = std::string(rkispp_hw_info.ispp_scale0_path);
        } else if (!strcmp(device.c_str(), S1_ENTITY_NAME)) {
            devname = std::string(rkispp_hw_info.ispp_scale1_path);
        } else if (!strcmp(device.c_str(), S2_ENTITY_NAME)) {
            devname = std::string(rkispp_hw_info.ispp_scale2_path);
        } else if (!strcmp(device.c_str(), "rkisp_mainpath")) {
            devname = std::string(rkisp_hw_info.isp_main_path);
        } else if (!strcmp(device.c_str(), "rkisp_selfpath")) {
            devname = std::string(rkisp_hw_info.isp_self_path);
        } else {
            devname = device;
        }
        v4l2_ctx = std::make_shared<V4L2Context>(capture_type, vio, devname);
        if (!v4l2_ctx) {
            return -ENOMEM;
        }
        fd = v4l2_ctx->GetDeviceFd();
        if (fd < 0) {
            v4l2_ctx = nullptr;
            return -1;
        }
        return 0;
    }

    int V4L2Stream::Close()
    {
        if (v4l2_ctx) {
            v4l2_ctx->SetStarted(false);
            v4l2_ctx = nullptr; // release reference
        }
        fd = -1;
        return 0;
    }

    int V4L2Stream::IoCtrl(unsigned long int request, ...)
    {
        va_list vl;
        va_start(vl, request);
        void* arg = va_arg(vl, void*);
        va_end(vl);
        switch (request) {
            case S_SUB_REQUEST: {
                auto sub = (SubRequest*)arg;
                return V4L2IoCtl(&vio, fd, sub->sub_request, sub->arg);
            }
            case S_STREAM_OFF: {
                return v4l2_ctx->SetStarted(false) ? 0 : -1;
            }
        }
        return -1;
    }

} // namespace easymedia
