// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MEDIA_TYPE_H_
#define EASYMEDIA_MEDIA_TYPE_H_

enum class Type
{
    None = -1,
    Audio = 0,
    Image,
    Video,
    Text
};

// My fixed convention:
//  definition = "=", value separator = ",", definition separator = "\n"
//  for example,
//    "input_data_type=image:nv12,image:uyvy422\noutput_data_type=video:h264"

// My fixed convention:
//  type internal separator = ":", type near separator = "\n"
#define TYPENEAR(type) type "\n"

#define TYPE_NOTHING nullptr
#define TYPE_ANYTHING ""

#define IMAGE_PREFIX "image:"
#define IMAGE_GRAY8 "imamge:gray8"
#define IMAGE_GRAY16 "imamge:gray16"
#define IMAGE_YUV420P "image:yuv420p"
#define IMAGE_NV12 "image:nv12"
#define IMAGE_NV21 "image:nv21"
#define IMAGE_YV12 "image:yv12"
#define IMAGE_FBC2 "image:fbc2"
#define IMAGE_FBC0 "image:fbc0"
#define IMAGE_YUV422P "image:yuv422p"
#define IMAGE_NV16 "image:nv16"
#define IMAGE_NV61 "image:nv61"
#define IMAGE_YV16 "image:yv16"
#define IMAGE_YUYV422 "image:yuyv422"
#define IMAGE_UYVY422 "image:uyvy422"
#define IMAGE_RGB332 "image:rgb332"
#define IMAGE_RGB565 "image:rgb565"
#define IMAGE_BGR565 "image:bgr565"
#define IMAGE_RGB888 "image:rgb888"
#define IMAGE_BGR888 "image:bgr888"
#define IMAGE_ARGB8888 "image:argb8888"
#define IMAGE_ABGR8888 "image:abgr8888"

#define IMAGE_JPEG "image:jpeg"

#define VIDEO_PREFIX "video:"
#define VIDEO_H264 "video:h264"
#define VIDEO_H265 "video:h265"
#define VIDEO_MJPEG "video:mjpeg"

#define AUDIO_PREFIX "audio:"

#define AUDIO_PCM_U8 "audio:pcm_u8"
#define AUDIO_PCM_S16 "audio:pcm_s16"
#define AUDIO_PCM_S32 "audio:pcm_s32"
#define AUDIO_PCM_FLT "audio:pcm_flt"
#define AUDIO_PCM_U8P "audio:pcm_u8p"
#define AUDIO_PCM_S16P "audio:pcm_s16p"
#define AUDIO_PCM_S32P "audio:pcm_s32p"
#define AUDIO_PCM_FLTP "audio:pcm_fltp"

#define AUDIO_PCM                                                                                                      \
    TYPENEAR(AUDIO_PCM_U8)                                                                                             \
    TYPENEAR(AUDIO_PCM_S16)                                                                                            \
    TYPENEAR(AUDIO_PCM_S32)                                                                                            \
    TYPENEAR(AUDIO_PCM_FLT)                                                                                            \
    TYPENEAR(AUDIO_PCM_U8P)                                                                                            \
    TYPENEAR(AUDIO_PCM_S16P)                                                                                           \
    TYPENEAR(AUDIO_PCM_S32P)                                                                                           \
    TYPENEAR(AUDIO_PCM_FLTP)

#define AUDIO_AAC "audio:aac"
#define AUDIO_MP2 "audio:mp2"
#define AUDIO_VORBIS "audio:vorbis"
#define AUDIO_G711A "audio:g711a"
#define AUDIO_G711U "audio:g711U"
#define AUDIO_G726 "audio:g726"

#define TEXT_PREFIX "text:"

#define STREAM_OGG "stream:ogg"

#define STREAM_FILE "stream:file"

#define NN_MODEL_PREFIX "nn_model:"
#define NN_FLOAT32 "nn:float32"
#define NN_FLOAT16 "nn:float16"
#define NN_INT8 "nn:int8"
#define NN_UINT8 "nn:uint8"
#define NN_INT16 "nn:int16"

#define MUXER_MP4 "mp4"
#define MUXER_FLV "flv"
#define MUXER_F4V "f4v"
#define MUXER_MKV "mkv"
#define MUXER_AVI "avi"
#define MUXER_MPEG_TS "mpegts"
#define MUXER_MPEG_PS "mpeg"

typedef enum
{
    CODEC_TYPE_NONE = -1,
    // Audio
    CODEC_TYPE_AAC,
    CODEC_TYPE_MP2,
    CODEC_TYPE_VORBIS,
    CODEC_TYPE_G711A,
    CODEC_TYPE_G711U,
    CODEC_TYPE_G726,
    // Video
    CODEC_TYPE_H264,
    CODEC_TYPE_H265,
    CODEC_TYPE_JPEG,
    CODEC_TYPE_NB
} CodecType;

__attribute__((visibility("default"))) const char* CodecTypeToString(CodecType fmt);
__attribute__((visibility("default"))) CodecType StringToCodecType(const char* fmt_str);

#include <string>

namespace easymedia
{

    Type StringToDataType(const char* data_type);

    class SupportMediaTypes
    {
      public:
        std::string types;
    };

} // namespace easymedia

#endif // #ifndef EASYMEDIA_MEDIA_TYPE_H_
