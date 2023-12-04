// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "v4l2_utils.h"

#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifdef HAVE_LIBV4L2
    #include <libv4l2.h>
#endif

namespace easymedia
{

    __u32 GetV4L2Type(const char* v4l2type)
    {
        if (!v4l2type) {
            return 0;
        }
        if (!strcmp(v4l2type, KEY_V4L2_C_TYPE(VIDEO_CAPTURE))) {
            return V4L2_BUF_TYPE_VIDEO_CAPTURE;
        }
        if (!strcmp(v4l2type, KEY_V4L2_C_TYPE(VIDEO_CAPTURE_MPLANE))) {
            return V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        }
        if (!strcmp(v4l2type, KEY_V4L2_C_TYPE(VIDEO_OUTPUT))) {
            return V4L2_BUF_TYPE_VIDEO_OUTPUT;
        }
        if (!strcmp(v4l2type, KEY_V4L2_C_TYPE(VIDEO_OUTPUT_MPLANE))) {
            return V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        }

        if (!strcmp(v4l2type, KEY_V4L2_M_TYPE(MEMORY_MMAP))) {
            return V4L2_MEMORY_MMAP;
        }
        if (!strcmp(v4l2type, KEY_V4L2_M_TYPE(MEMORY_USERPTR))) {
            return V4L2_MEMORY_USERPTR;
        }
        if (!strcmp(v4l2type, KEY_V4L2_M_TYPE(MEMORY_OVERLAY))) {
            return V4L2_MEMORY_OVERLAY;
        }
        if (!strcmp(v4l2type, KEY_V4L2_M_TYPE(MEMORY_DMABUF))) {
            return V4L2_MEMORY_DMABUF;
        }

        return 0;
    }

    static const struct V4L2FmtStringEntry {
        __u32 fmt;
        const char* type_str;
    } v4l2_fmt_string_map[] = {{V4L2_PIX_FMT_YUV420, IMAGE_YUV420P},  {V4L2_PIX_FMT_NV12, IMAGE_NV12},
                               {V4L2_PIX_FMT_NV21, IMAGE_NV21},       {V4L2_PIX_FMT_FBC2, IMAGE_FBC2},
                               {V4L2_PIX_FMT_FBC0, IMAGE_FBC0},       {V4L2_PIX_FMT_YUV422P, IMAGE_YUV422P},
                               {V4L2_PIX_FMT_NV16, IMAGE_NV16},       {V4L2_PIX_FMT_NV61, IMAGE_NV61},
                               {V4L2_PIX_FMT_YUYV, IMAGE_YUYV422},    {V4L2_PIX_FMT_UYVY, IMAGE_UYVY422},
                               {V4L2_PIX_FMT_SRGGB8, IMAGE_RGB332},   {V4L2_PIX_FMT_RGB565, IMAGE_RGB565},
                               {V4L2_PIX_FMT_BGR24, IMAGE_RGB888},    {V4L2_PIX_FMT_RGB24, IMAGE_BGR888},
                               {V4L2_PIX_FMT_ABGR32, IMAGE_ARGB8888}, {V4L2_PIX_FMT_ARGB32, IMAGE_ABGR8888},
                               {V4L2_PIX_FMT_MJPEG, IMAGE_JPEG},      {V4L2_PIX_FMT_H264, VIDEO_H264}};

    __u32 GetV4L2FmtByString(const char* type)
    {
        FIND_ENTRY_TARGET_BY_STRCMP(type, v4l2_fmt_string_map, type_str, fmt)
        return 0;
    }

    static const struct V4L2ColorSpaceStringEntry {
        __u32 colorspace;
        const char* type_str;
    } v4l2_cs_string_map[] = {{V4L2_COLORSPACE_DEFAULT, KEY_V4L2_CS(DEFAULT)},
                              {V4L2_COLORSPACE_SMPTE170M, KEY_V4L2_CS(SMPTE170M)},
                              {V4L2_COLORSPACE_SMPTE240M, KEY_V4L2_CS(SMPTE240M)},
                              {V4L2_COLORSPACE_REC709, KEY_V4L2_CS(REC709)},
                              {V4L2_COLORSPACE_BT878, KEY_V4L2_CS(BT878)},
                              {V4L2_COLORSPACE_470_SYSTEM_M, KEY_V4L2_CS(470_SYSTEM_M)},
                              {V4L2_COLORSPACE_470_SYSTEM_BG, KEY_V4L2_CS(470_SYSTEM_BG)},
                              {V4L2_COLORSPACE_JPEG, KEY_V4L2_CS(JPEG)},
                              {V4L2_COLORSPACE_SRGB, KEY_V4L2_CS(SRGB)},
                              {V4L2_COLORSPACE_ADOBERGB, KEY_V4L2_CS(ADOBERGB)},
                              {V4L2_COLORSPACE_BT2020, KEY_V4L2_CS(BT2020)},
                              {V4L2_COLORSPACE_RAW, KEY_V4L2_CS(RAW)},
                              {V4L2_COLORSPACE_DCI_P3, KEY_V4L2_CS(DCI_P3)}};

    __u32 GetV4L2ColorSpaceByString(const char* type)
    {
        FIND_ENTRY_TARGET_BY_STRCMP(type, v4l2_cs_string_map, type_str, colorspace)
        return 0;
    }

    class _V4L2_SUPPORT_TYPES : public SupportMediaTypes
    {
      public:
        _V4L2_SUPPORT_TYPES()
        {
            for (size_t i = 0; i < ARRAY_ELEMS(v4l2_fmt_string_map) - 1; i++) {
                types.append(v4l2_fmt_string_map[i].type_str);
            }
        }
    };
    static _V4L2_SUPPORT_TYPES priv_types;
    const std::string& GetStringOfV4L2Fmts()
    {
        return priv_types.types;
    }

    bool SetV4L2IoFunction(v4l2_io* vio, bool use_libv4l2)
    {
#define SET_WRAPPERS(prefix)                                                                                           \
    do {                                                                                                               \
        vio->open_f = prefix##open;                                                                                    \
        vio->close_f = prefix##close;                                                                                  \
        vio->dup_f = prefix##dup;                                                                                      \
        vio->ioctl_f = prefix##ioctl;                                                                                  \
        vio->read_f = prefix##read;                                                                                    \
        vio->mmap_f = prefix##mmap;                                                                                    \
        vio->munmap_f = prefix##munmap;                                                                                \
    } while (0)

        if (use_libv4l2) {
#ifdef HAVE_LIBV4L2
            SET_WRAPPERS(v4l2_);
#else
            LOG("libv4l2 is not configured.\n");
            return false;
#endif
        } else {
            SET_WRAPPERS();
        }
        return true;
    }

    int V4L2IoCtl(v4l2_io* vio, int fd, unsigned long int request, void* arg)
    {
        assert(vio->ioctl_f);
        return xioctl(vio->ioctl_f, fd, request, arg);
    }

} // namespace easymedia
