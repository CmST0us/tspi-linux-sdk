// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_RGA_H_
#define EASYMEDIA_RGA_H_

#include <rga/RockchipRga.h>

#include "buffer.h"
#include "filter.h"
#include "image.h"

namespace easymedia
{

    class ImageBuffer;

    class RgaFilter : public Filter
    {
      public:
        RgaFilter(const char* param);
        virtual ~RgaFilter() = default;
        static const char* GetFilterName()
        {
            return "rkrga";
        }
        virtual int Process(std::shared_ptr<MediaBuffer> input, std::shared_ptr<MediaBuffer>& output) override;

        void SetRects(std::vector<ImageRect> vec_rect);
        static RockchipRga gRkRga;

      private:
        std::vector<ImageRect> vec_rect;
        int rotate;
    };

    int rga_blit(std::shared_ptr<ImageBuffer> src, std::shared_ptr<ImageBuffer> dst, ImageRect* src_rect = nullptr,
                 ImageRect* dst_rect = nullptr, int rotate = 0);

} // namespace easymedia

#endif // #ifndef EASYMEDIA_RGA_H_
