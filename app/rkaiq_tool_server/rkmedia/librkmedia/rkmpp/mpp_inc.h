// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MPP_INC_H_
#define EASYMEDIA_MPP_INC_H_

#include <rk_mpi.h>

#include "buffer.h"
#include "image.h"

namespace easymedia
{
// mpp_packet_impl.h which define MPP_PACKET_FLAG_INTRA is not exposed,
// here define the same MPP_PACKET_FLAG_INTRA.
#ifndef MPP_PACKET_FLAG_INTRA
    #define MPP_PACKET_FLAG_INTRA (0x00000008)
#endif

    MppFrameFormat ConvertToMppPixFmt(const PixelFormat& fmt);
    PixelFormat ConvertToPixFmt(const MppFrameFormat& mfmt);
    const char* MppAcceptImageFmts();
    MppCodingType GetMPPCodingType(const std::string& data_type);
    MppEncRcQuality GetMPPRCQuality(const char* quality);
    MppEncRcMode GetMPPRCMode(const char* rc_mode);

    struct MPPContext {
        MPPContext();
        ~MPPContext();
        MppCtx ctx;
        MppApi* mpi;
        MppBufferGroup frame_group;
    };

    // no time-consuming, init a mppbuffer with MediaBuffer
    MPP_RET init_mpp_buffer(MppBuffer& buffer, const std::shared_ptr<MediaBuffer>& mb, size_t frame_size);
    // may time-consuming
    MPP_RET init_mpp_buffer_with_content(MppBuffer& buffer, const std::shared_ptr<MediaBuffer>& mb);

    MPP_RET mpi_enc_gen_ref_cfg(MppEncRefCfg ref, RK_S32 gop_mode = 2, RK_S32 gop_len = 0, RK_S32 vi_len = 0);

} // namespace easymedia

#endif // EASYMEDIA_MPP_INC_H_
