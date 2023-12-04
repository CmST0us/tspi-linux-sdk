// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_DECODER_H_
#define EASYMEDIA_DECODER_H_

#include "codec.h"
#include "media_reflector.h"

namespace easymedia
{

    DECLARE_FACTORY(Decoder)

    // usage: REFLECTOR(Decoder)::Create<T>(codecname, param)
    // T must be the final class type exposed to user
    DECLARE_REFLECTOR(Decoder)

#define DEFINE_DECODER_FACTORY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT)                                                     \
    DEFINE_MEDIA_CHILD_FACTORY(REAL_PRODUCT, REAL_PRODUCT::GetCodecName(), FINAL_EXPOSE_PRODUCT, Decoder)              \
    DEFINE_MEDIA_CHILD_FACTORY_EXTRA(REAL_PRODUCT)                                                                     \
    DEFINE_MEDIA_NEW_PRODUCT_BY(REAL_PRODUCT, Decoder, Init() != true)

#define DEFINE_AUDIO_DECODER_FACTORY(REAL_PRODUCT) DEFINE_DECODER_FACTORY(REAL_PRODUCT, AudioDecoder)

    class Decoder : public Codec
    {
      public:
        virtual ~Decoder() = default;
        virtual bool InitConfig(const MediaConfig& cfg);
    };

    class _API AudioDecoder : public Decoder
    {
      public:
        virtual ~AudioDecoder() = default;
        virtual int GetNbSamples()
        {
            return 0;
        }

      protected:
        CodecType codec_type;

        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Decoder)
    };

#define DEFINE_VIDEO_DECODER_FACTORY(REAL_PRODUCT) DEFINE_DECODER_FACTORY(REAL_PRODUCT, VideoDecoder)

    class _API VideoDecoder : public Decoder
    {
      public:
        virtual ~VideoDecoder() = default;

        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Decoder)
    };

} // namespace easymedia

#endif // #ifndef EASYMEDIA_DECODER_H_
