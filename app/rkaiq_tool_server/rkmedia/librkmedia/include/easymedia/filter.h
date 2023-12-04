// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_FILTER_H_
#define EASYMEDIA_FILTER_H_

#include <stdarg.h>

#include <memory>

#include "control.h"
#include "media_reflector.h"

namespace easymedia
{

    DECLARE_FACTORY(Filter)

    // usage: REFLECTOR(Filter)::Create<T>(filtername, param)
    // T must be the final class type exposed to user
    DECLARE_REFLECTOR(Filter)

#define DEFINE_FILTER_FACTORY(REAL_PRODUCT, FINAL_EXPOSE_PRODUCT)                                                      \
    DEFINE_MEDIA_CHILD_FACTORY(REAL_PRODUCT, REAL_PRODUCT::GetFilterName(), FINAL_EXPOSE_PRODUCT, Filter)              \
    DEFINE_MEDIA_CHILD_FACTORY_EXTRA(REAL_PRODUCT)                                                                     \
    DEFINE_MEDIA_NEW_PRODUCT_BY(REAL_PRODUCT, Filter, GetError() < 0)

#define DEFINE_COMMON_FILTER_FACTORY(REAL_PRODUCT) DEFINE_FILTER_FACTORY(REAL_PRODUCT, Filter)

    class MediaBuffer;
    class _API Filter
    {
      public:
        virtual ~Filter() = 0;
        static const char* GetFilterName()
        {
            return nullptr;
        }
        // sync call, input and output must be valid
        virtual int Process(std::shared_ptr<MediaBuffer> input, std::shared_ptr<MediaBuffer>& output);
        // some filter may output many buffers with one input.
        // sync or async safe call, depends on specific filter.
        virtual int SendInput(std::shared_ptr<MediaBuffer> input);
        virtual std::shared_ptr<MediaBuffer> FetchOutput();

        virtual int IoCtrl(unsigned long int request _UNUSED, ...)
        {
            return -1;
        }

        DEFINE_ERR_GETSET()
        DECLARE_PART_FINAL_EXPOSE_PRODUCT(Filter)
    };

} // namespace easymedia

#endif // #ifndef EASYMEDIA_FILTER_H_
