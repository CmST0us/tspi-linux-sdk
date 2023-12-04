// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_RKNN_UTILS_H_
#define EASYMEDIA_RKNN_UTILS_H_

#ifdef USE_ROCKFACE
    #include <rockface/rockface.h>
#endif

#ifdef USE_ROCKX
    #include <rockx/rockx.h>
#endif

namespace easymedia
{

#ifdef USE_ROCKFACE
    rockface_pixel_format StrToRockFacePixelFMT(const char* fmt_str);
#endif

#ifdef USE_ROCKX
    rockx_pixel_format StrToRockxPixelFMT(const char* fmt_str);
#endif

} // namespace easymedia

#endif // #ifndef EASYMEDIA_RKNN_UTILS_H_
