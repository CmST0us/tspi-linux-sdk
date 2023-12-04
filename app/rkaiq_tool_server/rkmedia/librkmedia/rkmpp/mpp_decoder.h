// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MPP_DECODER_H
#define EASYMEDIA_MPP_DECODER_H

#include "decoder.h"
#include "mpp_inc.h"

namespace easymedia
{

    // A hw video decoder which call the mpp interface directly.
    class MPPDecoder : public VideoDecoder
    {
      public:
        MPPDecoder(const char* param);
        virtual ~MPPDecoder() = default;
        static const char* GetCodecName()
        {
            return "rkmpp";
        }

        virtual bool Init() override;
        virtual int Process(const std::shared_ptr<MediaBuffer>& input, std::shared_ptr<MediaBuffer>& output,
                            std::shared_ptr<MediaBuffer> extra_output) override;
        virtual int SendInput(const std::shared_ptr<MediaBuffer>& input) override;
        virtual std::shared_ptr<MediaBuffer> FetchOutput() override;

      private:
        PixelFormat output_format;
        RK_S32 fg_limit_num;
        RK_U32 need_split;
        MppPollType timeout;
        MppCodingType coding_type;
        std::shared_ptr<MPPContext> mpp_ctx;
        bool support_sync;
        bool support_async;
        static const RK_S32 kFRAMEGROUP_MAX_FRAMES = 16;
    };

} // namespace easymedia

#endif // EASYMEDIA_MPP_DECODER_H
