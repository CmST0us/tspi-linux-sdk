// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_ENCODER_H_
#define EASYMEDIA_ENCODER_H_

#include <mutex>

#ifdef __cplusplus

    #include "codec.h"
    #include "media_reflector.h"

namespace easymedia
{

    DECLARE_FACTORY(Encoder)

    // usage: REFLECTOR(Encoder)::Create<T>(codecname, param)
    // T must be the final class type exposed to user
    DECLARE_REFLECTOR(Encoder)

    #define DEFINE_ENCODER_FACTORY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT)                                                 \
        DEFINE_MEDIA_CHILD_FACTORY(REAL_PRODUCT, REAL_PRODUCT::GetCodecName(), FINAL_EXPOSE_PRODUCT, Encoder)          \
        DEFINE_MEDIA_CHILD_FACTORY_EXTRA(REAL_PRODUCT)                                                                 \
        DEFINE_MEDIA_NEW_PRODUCT_BY(REAL_PRODUCT, Encoder, Init() != true)

    class Encoder : public Codec
    {
      public:
        virtual ~Encoder() = default;
        virtual bool InitConfig(const MediaConfig& cfg);
    };

    // self define by user
    class ParameterBuffer
    {
      public:
        ParameterBuffer(size_t st = sizeof(int)) : size(st), ptr(nullptr)
        {
            if (sizeof(int) != st && st != 0) {
                ptr = malloc(st);
                if (!ptr) {
                    size = 0;
                }
            }
        }
        ~ParameterBuffer()
        {
            if (ptr) {
                free(ptr);
            }
        }
        size_t GetSize()
        {
            return size;
        }
        int GetValue()
        {
            return value;
        }
        void SetValue(int v)
        {
            value = v;
        }
        void* GetPtr()
        {
            return ptr;
        }
        void SetPtr(void* data, size_t data_len)
        {
            if (ptr && ptr != data) {
                free(ptr);
            }
            ptr = data;
            size = data_len;
        }

      private:
        size_t size;
        int value;
        void* ptr;
    };

    #define DEFINE_VIDEO_ENCODER_FACTORY(REAL_PRODUCT) DEFINE_ENCODER_FACTORY(REAL_PRODUCT, VideoEncoder)

    class _API VideoEncoder : public Encoder
    {
      public:
        // changes
        static const uint32_t kQPChange = (1 << 0);
        static const uint32_t kFrameRateChange = (1 << 1);
        static const uint32_t kBitRateChange = (1 << 2);
        static const uint32_t kForceIdrFrame = (1 << 3);
        static const uint32_t kOSDDataChange = (1 << 4);
        static const uint32_t kOSDPltChange = (1 << 5);
        static const uint32_t kMoveDetectionFlow = (1 << 6);
        static const uint32_t kROICfgChange = (1 << 7);
        static const uint32_t kRcModeChange = (1 << 8);
        static const uint32_t kRcQualityChange = (1 << 9);
        static const uint32_t kSplitChange = (1 << 10);
        static const uint32_t kGopChange = (1 << 11);
        static const uint32_t kGopModeChange = (1 << 12);
        static const uint32_t kProfileChange = (1 << 13);
        static const uint32_t kUserDataChange = (1 << 14);
        // enable fps/bps statistics.
        static const uint32_t kEnableStatistics = (1 << 31);

        VideoEncoder() : codec_type(CODEC_TYPE_NONE)
        {
        }
        virtual ~VideoEncoder() = default;
        void RequestChange(uint32_t change, std::shared_ptr<ParameterBuffer> value);
        virtual void QueryChange(uint32_t change, void* value, int32_t size);

      protected:
        bool HasChangeReq()
        {
            return !change_list.empty();
        }
        std::pair<uint32_t, std::shared_ptr<ParameterBuffer>> PeekChange();

        CodecType codec_type;

      private:
        std::mutex change_mtx;
        std::list<std::pair<uint32_t, std::shared_ptr<ParameterBuffer>>> change_list;

        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Encoder)
    };

    #define DEFINE_AUDIO_ENCODER_FACTORY(REAL_PRODUCT) DEFINE_ENCODER_FACTORY(REAL_PRODUCT, AudioEncoder)

    class _API AudioEncoder : public Encoder
    {
      public:
        AudioEncoder() : codec_type(CODEC_TYPE_NONE)
        {
        }
        virtual ~AudioEncoder() = default;
        virtual int GetNbSamples()
        {
            return 0;
        }

      protected:
        CodecType codec_type;

        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Encoder)
    };

} // namespace easymedia

#endif

#endif // EASYMEDIA_ENCODER_H_
