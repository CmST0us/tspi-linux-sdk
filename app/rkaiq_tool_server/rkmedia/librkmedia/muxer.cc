// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "muxer.h"

namespace easymedia
{

    DEFINE_REFLECTOR(Muxer)

    // request should equal muxer_name
    DEFINE_FACTORY_COMMON_PARSE(Muxer)

    Muxer::Muxer(const char* param _UNUSED) : io_output(nullptr), m_handler(nullptr), m_write_callback_func(nullptr)
    {
    }

    DEFINE_PART_FINAL_EXPOSE_PRODUCT(Muxer, Muxer)

} // namespace easymedia
