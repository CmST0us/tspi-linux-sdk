// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoder.h"

namespace easymedia
{

    DEFINE_REFLECTOR(Decoder)

    // request should equal codec_name
    DEFINE_FACTORY_COMMON_PARSE(Decoder)

    bool Decoder::InitConfig(const MediaConfig& cfg)
    {
        UNUSED(cfg);
        return true;
    }

    DEFINE_PART_FINAL_EXPOSE_PRODUCT(AudioDecoder, Decoder)
    DEFINE_PART_FINAL_EXPOSE_PRODUCT(VideoDecoder, Decoder)

} // namespace easymedia
