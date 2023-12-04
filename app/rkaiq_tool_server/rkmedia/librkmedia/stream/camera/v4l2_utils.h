// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_V4L2_UTILS_H_
#define EASYMEDIA_V4L2_UTILS_H_

#include <linux/videodev2.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include "key_string.h"
#include "media_type.h"
#include "rkaiq/common/mediactl/mediactl.h"
#include "utils.h"

#ifndef V4L2_PIX_FMT_FBC2
    #define V4L2_PIX_FMT_FBC2 v4l2_fourcc('F', 'B', 'C', '2')
    #define V4L2_PIX_FMT_FBC0 v4l2_fourcc('F', 'B', 'C', '0')
#endif

#define DEV_PATH_LEN 64
#define MAX_MEDIA_INDEX 20

#define DRIVER_NAME "rkispp"
#define MB_ENTITY_NAME DRIVER_NAME "_m_bypass"
#define S0_ENTITY_NAME DRIVER_NAME "_scale0"
#define S1_ENTITY_NAME DRIVER_NAME "_scale1"
#define S2_ENTITY_NAME DRIVER_NAME "_scale2"

typedef struct {
    char media_dev_path[DEV_PATH_LEN];
    char ispp_m_bypass_path[DEV_PATH_LEN];
    char ispp_scale0_path[DEV_PATH_LEN];
    char ispp_scale1_path[DEV_PATH_LEN];
    char ispp_scale2_path[DEV_PATH_LEN];
} ispp_t;

typedef struct {
    char media_dev_path[DEV_PATH_LEN];
    char isp_main_path[DEV_PATH_LEN];
    char isp_self_path[DEV_PATH_LEN];
} isp_t;

namespace easymedia
{

    __u32 GetV4L2Type(const char* v4l2type);
    __u32 GetV4L2FmtByString(const char* type);
    __u32 GetV4L2ColorSpaceByString(const char* type);
    const std::string& GetStringOfV4L2Fmts();

    typedef struct {
        int (*open_f)(const char* file, int oflag, ...);
        int (*close_f)(int fd);
        int (*dup_f)(int fd);
        int (*ioctl_f)(int fd, unsigned long int request, ...);
        ssize_t (*read_f)(int fd, void* buffer, size_t n);
        void* (*mmap_f)(void* start, size_t length, int prot, int flags, int fd, int64_t offset);
        int (*munmap_f)(void* _start, size_t length);
    } v4l2_io;

    bool SetV4L2IoFunction(v4l2_io* vio, bool use_libv4l2 = false);
    int V4L2IoCtl(v4l2_io* vio, int fd, unsigned long int request, void* arg);

} // namespace easymedia

#endif // #ifndef EASYMEDIA_V4L2_UTILS_H_
