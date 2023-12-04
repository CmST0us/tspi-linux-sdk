// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>

#include "media_type.h"
#include "utils.h"

static const struct CodecTypeEntry {
    CodecType fmt;
    const char* fmt_str;
} codec_type_string_map[] = {
    {CODEC_TYPE_AAC, AUDIO_AAC},     {CODEC_TYPE_MP2, AUDIO_MP2},     {CODEC_TYPE_VORBIS, AUDIO_VORBIS},
    {CODEC_TYPE_G711A, AUDIO_G711A}, {CODEC_TYPE_G711U, AUDIO_G711U}, {CODEC_TYPE_G726, AUDIO_G726},
    {CODEC_TYPE_H264, VIDEO_H264},   {CODEC_TYPE_H265, VIDEO_H265},   {CODEC_TYPE_JPEG, VIDEO_MJPEG},
    {CODEC_TYPE_JPEG, IMAGE_JPEG},
};

const char* CodecTypeToString(CodecType fmt)
{
    FIND_ENTRY_TARGET(fmt, codec_type_string_map, fmt, fmt_str)
    return nullptr;
}

CodecType StringToCodecType(const char* fmt_str)
{
    FIND_ENTRY_TARGET_BY_STRCMP(fmt_str, codec_type_string_map, fmt_str, fmt)
    return CODEC_TYPE_NONE;
}

namespace easymedia
{

    Type StringToDataType(const char* data_type)
    {
        if (string_start_withs(data_type, AUDIO_PREFIX)) {
            return Type::Audio;
        } else if (string_start_withs(data_type, IMAGE_PREFIX)) {
            return Type::Image;
        } else if (string_start_withs(data_type, VIDEO_PREFIX)) {
            return Type::Video;
        } else if (string_start_withs(data_type, TEXT_PREFIX)) {
            return Type::Text;
        }
        return Type::None;
    }

} // namespace easymedia
