// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image.h"

#include <assert.h>
#include <string.h>

#include "key_string.h"
#include "media_type.h"
#include "utils.h"

void GetPixFmtNumDen(const PixelFormat& fmt, int& num, int& den)
{
    num = 0;
    den = 1;
    switch (fmt) {
        case PIX_FMT_RGB332:
            num = 1;
            break;
        case PIX_FMT_YUV420P:
        case PIX_FMT_NV12:
        case PIX_FMT_NV21:
        case PIX_FMT_FBC0:
            num = 3;
            den = 2;
            break;
        case PIX_FMT_YUV422P:
        case PIX_FMT_NV16:
        case PIX_FMT_NV61:
        case PIX_FMT_YUYV422:
        case PIX_FMT_UYVY422:
        case PIX_FMT_RGB565:
        case PIX_FMT_BGR565:
        case PIX_FMT_FBC2:
            num = 2;
            break;
        case PIX_FMT_RGB888:
        case PIX_FMT_BGR888:
            num = 3;
            break;
        case PIX_FMT_ARGB8888:
        case PIX_FMT_ABGR8888:
            num = 4;
            break;
        default:
            LOG("unsupport get num/den for pixel fmt: %d\n", fmt);
    }
}

int CalPixFmtSize(const PixelFormat& fmt, const int width, const int height, int align)
{
    int num = 0;
    int den = 0;
    int extra_hdr_size = 0;
    int pix_fmt_size = 0;
    GetPixFmtNumDen(fmt, num, den);
    // fbc image: fbc hdr + fbc body.
    // fbc w,h must be 16 align, and body offset must be 4k align
    if (fmt == PIX_FMT_FBC0 || fmt == PIX_FMT_FBC2) {
        align = 16;
        extra_hdr_size = UPALIGNTO(width, align) * UPALIGNTO(height, align) / 16;
        extra_hdr_size = UPALIGNTO(extra_hdr_size, 4096);
    }
    // mpp always require buffer align by align value
    if (align > 0)
        pix_fmt_size = UPALIGNTO(width, align) * UPALIGNTO(height, align) * num / den;
    else {
        pix_fmt_size = width * height * num / den;
    }

    return (extra_hdr_size + pix_fmt_size);
}

static const struct PixFmtStringEntry {
    PixelFormat fmt;
    const char* type_str;
} pix_fmt_string_map[] = {
    {PIX_FMT_YUV420P, IMAGE_YUV420P}, {PIX_FMT_NV12, IMAGE_NV12},         {PIX_FMT_NV21, IMAGE_NV21},
    {PIX_FMT_YUV422P, IMAGE_YUV422P}, {PIX_FMT_NV16, IMAGE_NV16},         {PIX_FMT_NV61, IMAGE_NV61},
    {PIX_FMT_YUYV422, IMAGE_YUYV422}, {PIX_FMT_UYVY422, IMAGE_UYVY422},   {PIX_FMT_RGB332, IMAGE_RGB332},
    {PIX_FMT_RGB565, IMAGE_RGB565},   {PIX_FMT_BGR565, IMAGE_BGR565},     {PIX_FMT_RGB888, IMAGE_RGB888},
    {PIX_FMT_BGR888, IMAGE_BGR888},   {PIX_FMT_ARGB8888, IMAGE_ARGB8888}, {PIX_FMT_ABGR8888, IMAGE_ABGR8888},
    {PIX_FMT_FBC0, IMAGE_FBC0},       {PIX_FMT_FBC0, IMAGE_FBC2}};

PixelFormat StringToPixFmt(const char* type)
{
    if (!type) {
        return PIX_FMT_NONE;
    }
    FIND_ENTRY_TARGET_BY_STRCMP(type, pix_fmt_string_map, type_str, fmt)
    return PIX_FMT_NONE;
}

const char* PixFmtToString(PixelFormat fmt)
{
    FIND_ENTRY_TARGET(fmt, pix_fmt_string_map, fmt, type_str)
    return nullptr;
}

namespace easymedia
{
    bool ParseImageInfoFromMap(std::map<std::string, std::string>& params, ImageInfo& info, bool input)
    {
        std::string value;
        const char* type = input ? KEY_INPUTDATATYPE : KEY_OUTPUTDATATYPE;
        CHECK_EMPTY(value, params, type)
        info.pix_fmt = StringToPixFmt(value.c_str());
        if (info.pix_fmt == PIX_FMT_NONE) {
            LOG("unsupport pix fmt %s\n", value.c_str());
            return false;
        }
        CHECK_EMPTY(value, params, KEY_BUFFER_WIDTH)
        info.width = std::stoi(value);
        CHECK_EMPTY(value, params, KEY_BUFFER_HEIGHT)
        info.height = std::stoi(value);
        CHECK_EMPTY(value, params, KEY_BUFFER_VIR_WIDTH)
        info.vir_width = std::stoi(value);
        CHECK_EMPTY(value, params, KEY_BUFFER_VIR_HEIGHT)
        info.vir_height = std::stoi(value);
        return true;
    }

    std::string to_param_string(const ImageInfo& info, bool input)
    {
        std::string s;
        const char* fmt = PixFmtToString(info.pix_fmt);
        if (!fmt) {
            return s;
        }
        if (input) {
            PARAM_STRING_APPEND(s, KEY_INPUTDATATYPE, fmt);
        } else {
            PARAM_STRING_APPEND(s, KEY_OUTPUTDATATYPE, fmt);
        }
        PARAM_STRING_APPEND_TO(s, KEY_BUFFER_WIDTH, info.width);
        PARAM_STRING_APPEND_TO(s, KEY_BUFFER_HEIGHT, info.height);
        PARAM_STRING_APPEND_TO(s, KEY_BUFFER_VIR_WIDTH, info.vir_width);
        PARAM_STRING_APPEND_TO(s, KEY_BUFFER_VIR_HEIGHT, info.vir_height);
        return s;
    }

    std::string TwoImageRectToString(const std::vector<ImageRect>& src_dst)
    {
        char r[128] = {0};
        assert(src_dst[0].x < 10000 && src_dst[0].y < 10000);
        assert(src_dst[0].w < 10000 && src_dst[0].h < 10000);
        assert(src_dst[1].x < 10000 && src_dst[1].y < 10000);
        assert(src_dst[1].w < 10000 && src_dst[1].h < 10000);
        snprintf(r, sizeof(r), "(%d,%d,%d,%d)%s(%d,%d,%d,%d)", src_dst[0].x, src_dst[0].y, src_dst[0].w, src_dst[0].h,
                 KEY_RIGHT_DIRECTION, src_dst[1].x, src_dst[1].y, src_dst[1].w, src_dst[1].h);
        return r;
    }

    std::vector<ImageRect> StringToTwoImageRect(const std::string& str_rect)
    {
        std::vector<ImageRect> ret;
        const char* s = nullptr;
        if (str_rect.empty() || !(s = strstr(str_rect.c_str(), KEY_RIGHT_DIRECTION))) {
            return std::move(ret);
        }
        const char* args[2] = {str_rect.c_str(), s + sizeof(KEY_RIGHT_DIRECTION) - 1};
        for (int i = 0; i < 2; i++) {
            ImageRect rect = {0, 0, 0, 0};
            int r = sscanf(args[i], "(%d,%d,%d,%d)", &rect.x, &rect.y, &rect.w, &rect.h);
            if (r != 4) {
                LOG("Fail to sscanf(ret=%d) : %m\n", r);
                ret.clear();
                return std::move(ret);
            }
            ret.push_back(std::move(rect));
        }

        return std::move(ret);
    }

    std::string ImageRectToString(const ImageRect& src_dst)
    {
        char r[64] = {0};
        assert(src_dst.x >= 0 && src_dst.y >= 0);
        assert(src_dst.x < 10000 && src_dst.y < 10000);
        assert(src_dst.w < 10000 && src_dst.h < 10000);
        snprintf(r, sizeof(r), "(%d,%d,%d,%d)", src_dst.x, src_dst.y, src_dst.w, src_dst.h);
        return r;
    }

    std::vector<ImageRect> StringToImageRect(const std::string& str_rect)
    {
        std::vector<ImageRect> ret;
        const char* start = nullptr;
        const char* delimiter = nullptr;

        if (str_rect.empty()) {
            return std::move(ret);
        }

        start = str_rect.c_str();
        while (start) {
            delimiter = start;
            // (x,y,w,h) format check.
            for (int i = 0; i < 4; i++) {
                if (i == 3) {
                    delimiter = strstr(delimiter, ")");
                } else {
                    delimiter = strstr(delimiter, ",");
                }
                if (!delimiter) {
                    return std::move(ret);
                }
                delimiter += 1;
            }

            ImageRect rect = {0, 0, 0, 0};
            int r = sscanf(start, "(%d,%d,%d,%d)", &rect.x, &rect.y, &rect.w, &rect.h);
            if (r != 4) {
                LOG("Fail to sscanf(ret=%d) : %m\n", r);
                return std::move(ret);
            }

            ret.push_back(std::move(rect));
            start = strstr(delimiter, "(");
        }

        return std::move(ret);
    }

} // namespace easymedia
