// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_DEMUXER_H_
#define EASYMEDIA_DEMUXER_H_

#include <list>

#include "media_config.h"
#include "media_reflector.h"
#include "stream.h"

namespace easymedia
{

    DECLARE_FACTORY(Demuxer)

    // usage: REFLECTOR(Demuxer)::Create<T>(demuxname, param)
    // T must be the final class type exposed to user
    DECLARE_REFLECTOR(Demuxer)

#define DEFINE_DEMUXER_FACTORY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT)                                                     \
    DEFINE_MEDIA_CHILD_FACTORY(REAL_PRODUCT, REAL_PRODUCT::GetDemuxName(), FINAL_EXPOSE_PRODUCT, Demuxer)              \
    DEFINE_MEDIA_CHILD_FACTORY_EXTRA(REAL_PRODUCT)                                                                     \
    DEFINE_MEDIA_NEW_PRODUCT_BY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT, GetError() < 0)

    class MediaBuffer;
    class _API Demuxer
    {
      public:
        Demuxer(const char* param);
        virtual ~Demuxer() = default;
        static const char* GetDemuxName()
        {
            return nullptr;
        }

        // Indicate whether the demuxer do internally decoding
        virtual bool IncludeDecoder()
        {
            return false;
        }
        // Demuxer set the value of MediaConfig
        virtual bool Init(std::shared_ptr<Stream> input, MediaConfig* out_cfg) = 0;
        virtual char** GetComment()
        {
            return nullptr;
        }
        virtual std::shared_ptr<MediaBuffer> Read(size_t request_size = 0) = 0;

      public:
        double total_time; // seconds

      protected:
        std::string path;

        DEFINE_ERR_GETSET()
        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Demuxer)
    };

} // namespace easymedia

#endif // #ifndef EASYMEDIA_DEMUXER_H_
