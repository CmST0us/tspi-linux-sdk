// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "demuxer.h"

#include "buffer.h"
#include "utils.h"

namespace easymedia
{

    DEFINE_REFLECTOR(Demuxer)

    // request should equal demuxer_name
    DEFINE_FACTORY_COMMON_PARSE(Demuxer)

    Demuxer::Demuxer(const char* param) : total_time(0.0f)
    {
        std::map<std::string, std::string> params;
        if (!parse_media_param_map(param, params)) {
            return;
        }
        for (auto& p : params) {
            const std::string& key = p.first;
            if (key == KEY_PATH) {
                path = p.second;
            }
        }
    }

    DEFINE_PART_FINAL_EXPOSE_PRODUCT(Demuxer, Demuxer)

} // namespace easymedia
