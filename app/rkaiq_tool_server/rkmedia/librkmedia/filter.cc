// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "filter.h"

namespace easymedia
{

    DEFINE_REFLECTOR(Filter)

    // request should equal filter_name
    DEFINE_FACTORY_COMMON_PARSE(Filter)

    DEFINE_PART_FINAL_EXPOSE_PRODUCT(Filter, Filter)

    Filter::~Filter()
    {
    }

    int Filter::Process(std::shared_ptr<MediaBuffer> input _UNUSED, std::shared_ptr<MediaBuffer>& output _UNUSED)
    {
        errno = ENOSYS;
        return -1;
    }

    int Filter::SendInput(std::shared_ptr<MediaBuffer> input _UNUSED)
    {
        errno = ENOSYS;
        return -1;
    }

    std::shared_ptr<MediaBuffer> Filter::FetchOutput()
    {
        errno = ENOSYS;
        return nullptr;
    }

} // namespace easymedia
