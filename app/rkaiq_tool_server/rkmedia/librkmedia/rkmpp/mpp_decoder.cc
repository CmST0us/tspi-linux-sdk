// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mpp_decoder.h"

#include <assert.h>
#include <unistd.h>

#include "buffer.h"

namespace easymedia
{

    MPPDecoder::MPPDecoder(const char* param)
        : output_format(PIX_FMT_NONE), fg_limit_num(kFRAMEGROUP_MAX_FRAMES), need_split(1), timeout(MPP_POLL_NON_BLOCK),
          coding_type(MPP_VIDEO_CodingUnused), support_sync(true), support_async(true)
    {
        MediaConfig& cfg = GetConfig();
        ImageInfo& img_info = cfg.img_cfg.image_info;
        img_info.pix_fmt = PIX_FMT_NONE;
        cfg.type = Type::None;
        std::map<std::string, std::string> params;
        std::list<std::pair<const std::string, std::string&>> req_list;
        std::string input_data_type;
        std::string output_data_type;
        std::string limit_max_frame_num;
        std::string split_mode;
        std::string stimeout;
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_INPUTDATATYPE, input_data_type));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_OUTPUTDATATYPE, output_data_type));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_MPP_GROUP_MAX_FRAMES, limit_max_frame_num));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_MPP_SPLIT_MODE, split_mode));
        req_list.push_back(std::pair<const std::string, std::string&>(KEY_OUTPUT_TIMEOUT, stimeout));

        int ret = parse_media_param_match(param, params, req_list);
        if (ret == 0 || input_data_type.empty()) {
            LOG("missing %s\n", KEY_INPUTDATATYPE);
            return;
        }
        coding_type = GetMPPCodingType(input_data_type);
        if (!output_data_type.empty()) {
            output_format = StringToPixFmt(output_data_type.c_str());
            if (output_format == PIX_FMT_NONE) {
                LOG("invalid output format %s\n", output_data_type.c_str());
                return;
            }
        }
        if (!limit_max_frame_num.empty()) {
            fg_limit_num = std::stoi(limit_max_frame_num);
            if (fg_limit_num > 0 && fg_limit_num <= 3) {
                LOG("invalid framegroup limit frame num: %d\n", fg_limit_num);
                return;
            }
        }
        if (!split_mode.empty()) {
            need_split = std::stoi(split_mode);
        }
        if (!stimeout.empty()) {
            int tout = std::stoi(stimeout);
            if (tout == 0) {
                timeout = MPP_POLL_NON_BLOCK;
            } else if (tout < 0) {
                timeout = MPP_POLL_BLOCK;
            } else {
                timeout = (MppPollType)tout;
            }
        }
    }

    bool MPPDecoder::Init()
    {
        if (coding_type == MPP_VIDEO_CodingUnused) {
            return false;
        }
        if (coding_type == MPP_VIDEO_CodingMJPEG) {
            // now mpp jpegd can not be async
            support_async = false;
        }
        mpp_ctx = std::make_shared<MPPContext>();
        if (!mpp_ctx) {
            return false;
        }
        MppCtx ctx = NULL;
        MppApi* mpi = NULL;
        MPP_RET ret = mpp_create(&ctx, &mpi);
        if (MPP_OK != ret) {
            LOG("mpp_create failed\n");
            return false;
        }
        mpp_ctx->ctx = ctx;
        mpp_ctx->mpi = mpi;
        assert(ctx);
        assert(mpi);

        MppParam param = NULL;

        if (need_split) {
            param = &need_split;
            ret = mpi->control(ctx, MPP_DEC_SET_PARSER_SPLIT_MODE, param);
            LOG("mpi control MPP_DEC_SET_PARSER_SPLIT_MODE ret = %d\n", ret);
            if (MPP_OK != ret) {
                return false;
            }
        }

        ret = mpp_init(ctx, MPP_CTX_DEC, coding_type);
        if (ret != MPP_OK) {
            LOG("mpp_init dec failed with type %d\n", coding_type);
            return false;
        }

        if (timeout != MPP_POLL_NON_BLOCK) {
            // if timeout is MPP_POLL_BLOCK, some codec need a whole key frame with
            // extradata, such as, h264 need spspps+I frame as once input
            RK_U32 to = timeout;
            param = &to;
            ret = mpi->control(ctx, MPP_SET_OUTPUT_TIMEOUT, param);
            LOG("mpi set output timeout = %d, ret = %d\n", timeout, ret);
            if (MPP_OK != ret) {
                return false;
            }
        }

        if (fg_limit_num > 0) {
            ret = mpp_buffer_group_get_internal(&mpp_ctx->frame_group, MPP_BUFFER_TYPE_ION);
            if (ret != MPP_OK) {
                LOG("Failed to retrieve buffer group (ret = %d)\n", ret);
                return false;
            }
            ret = mpi->control(ctx, MPP_DEC_SET_EXT_BUF_GROUP, mpp_ctx->frame_group);
            if (ret != MPP_OK) {
                LOG("Failed to assign buffer group (ret = %d)\n", ret);
                return false;
            }
            ret = mpp_buffer_group_limit_config(mpp_ctx->frame_group, 0, fg_limit_num);
            if (ret != MPP_OK) {
                LOG("Failed to set buffer group limit (ret = %d)\n", ret);
                return false;
            }
            LOG("mpi set group limit = %d\n", fg_limit_num);
        }

        if (coding_type == MPP_VIDEO_CodingMJPEG && output_format != PIX_FMT_NONE) {
            auto oformat = ConvertToMppPixFmt(output_format);
            if (oformat < 0) {
                LOG("unsupport set output format=%s\n", PixFmtToString(output_format));
                return false;
            }
            ret = mpi->control(ctx, MPP_DEC_SET_OUTPUT_FORMAT, &oformat);
            if (ret != MPP_OK) {
                LOG("Failed to set output format (ret = %d)\n", ret);
                return false;
            }
        }

        return true;
    }

    class MPPFrameContext
    {
      public:
        MPPFrameContext(std::shared_ptr<MPPContext> ctx, MppFrame f) : mctx(ctx), frame(f)
        {
        }
        ~MPPFrameContext()
        {
            if (frame) {
                mpp_frame_deinit(&frame);
            }
        }

      private:
        std::shared_ptr<MPPContext> mctx;
        MppFrame frame;
    };

    static int __free_mppframecontext(void* p)
    {
        assert(p);
        delete (MPPFrameContext*)p;
        return 0;
    }

    // frame may be deinit here or depends on ImageBuffer
    static int SetImageBufferWithMppFrame(std::shared_ptr<ImageBuffer> ib, std::shared_ptr<MPPContext> mctx,
                                          MppFrame& frame)
    {
        const MppBuffer buffer = mpp_frame_get_buffer(frame);
        if (!buffer || mpp_buffer_get_size(buffer) == 0) {
            LOG("Failed to retrieve the frame buffer\n");
            return -EFAULT;
        }
        ImageInfo& info = ib->GetImageInfo();
        info.pix_fmt = ConvertToPixFmt(mpp_frame_get_fmt(frame));
        info.width = mpp_frame_get_width(frame);
        info.height = mpp_frame_get_height(frame);
        info.vir_width = mpp_frame_get_hor_stride(frame);
        info.vir_height = mpp_frame_get_ver_stride(frame);
        size_t size = CalPixFmtSize(info);
        auto pts = mpp_frame_get_pts(frame);
        bool eos = mpp_frame_get_eos(frame) ? true : false;
        if (!ib->IsValid()) {
            MPPFrameContext* ctx = new MPPFrameContext(mctx, frame);
            if (!ctx) {
                LOG_NO_MEMORY();
                return -ENOMEM;
            }
            ib->SetFD(mpp_buffer_get_fd(buffer));
            ib->SetPtr(mpp_buffer_get_ptr(buffer));
            assert(size <= mpp_buffer_get_size(buffer));
            ib->SetSize(mpp_buffer_get_size(buffer));
            ib->SetUserData(ctx, __free_mppframecontext);
        } else {
            assert(ib->GetSize() >= size);
            if (!ib->IsHwBuffer()) {
                void* ptr = ib->GetPtr();
                assert(ptr);
                LOGD("extra time-consuming memcpy to cpu!\n");
                memcpy(ptr, mpp_buffer_get_ptr(buffer), size);
                // sync to cpu?
            }
            mpp_frame_deinit(&frame);
        }
        ib->SetValidSize(size);
        ib->SetUSTimeStamp(pts);
        ib->SetEOF(eos);

        return 0;
    }

    int MPPDecoder::Process(const std::shared_ptr<MediaBuffer>& input, std::shared_ptr<MediaBuffer>& output,
                            std::shared_ptr<MediaBuffer> extra_output _UNUSED)
    {
        if (!support_sync) {
            errno = ENOSYS;
            return -ENOSYS;
        }

        if (!input || !input->IsValid()) {
            return 0;
        }
        if (!output) {
            return -EINVAL;
        }
        if (output->GetType() != Type::Image) {
            LOG("mpp decoder output must be image buffer\n");
            return -EINVAL;
        }

        MPP_RET ret;
        MppPacket packet = NULL;
        MppBuffer mpp_buf = NULL;
        MppFrame frame = NULL;
        MppTask task = NULL;
        MppFrame frame_out = NULL;
        MppCtx ctx = mpp_ctx->ctx;
        MppApi* mpi = mpp_ctx->mpi;
        size_t output_buffer_size = 0;

        ret = init_mpp_buffer_with_content(mpp_buf, input);
        if (ret || !mpp_buf) {
            LOG("Failed to init MPP buffer with content (ret = %d)\n", ret);
            return -EFAULT;
        }
        ret = mpp_packet_init_with_buffer(&packet, mpp_buf);
        mpp_buffer_put(mpp_buf);
        mpp_buf = NULL;
        if (ret != MPP_OK) {
            LOG("Failed to init MPP packet with buffer (ret = %d)\n", ret);
            goto out;
        }
        assert(packet);
        mpp_packet_set_length(packet, input->GetValidSize());
        mpp_packet_set_pts(packet, input->GetUSTimeStamp());
        if (input->IsEOF()) {
            LOG("send eos packet to MPP\n");
            mpp_packet_set_eos(packet);
        }
        output_buffer_size = output->GetSize();
        if (output_buffer_size == 0 || !output->IsHwBuffer()) {
            uint8_t* buffer = static_cast<uint8_t*>(input->GetPtr());
            if (coding_type == MPP_VIDEO_CodingMJPEG && buffer) {
                // parse width and height
                int a;
                size_t buffer_size = input->GetValidSize();
                size_t pos = 0;
                int w = 0, h = 0;
                if (buffer_size < 4) {
                    goto out;
                }
                a = (int)buffer[pos++];
                if (a != 0xFF || buffer[pos++] != 0xD8) {
                    LOG("input is not jpeg\n");
                    goto out;
                }
                for (;;) {
                    int marker = 0;
                    uint16_t itemlen;
                    uint16_t ll, lh;
                    if (pos > buffer_size - 1) {
                        break;
                    }
                    for (a = 0; a <= 16 && pos < buffer_size - 1; a++) {
                        marker = (int)buffer[pos++];
                        if (marker != 0xff) {
                            break;
                        }
                        if (a >= 16) {
                            LOG("too many padding bytes\n");
                            goto out;
                        }
                    }
                    if (marker == 0xD9 || marker == 0xDA) {
                        break;
                    }
                    if (pos > buffer_size - 2) {
                        break;
                    }
                    lh = (uint16_t)buffer[pos++];
                    ll = (uint16_t)buffer[pos++];
                    itemlen = (lh << 8) | ll;
                    if (itemlen < 2) {
                        LOG("invalid marker\n");
                        goto out;
                    }
                    if (pos + itemlen - 2 > buffer_size) {
                        LOG("Premature end of jpeg?\n");
                        goto out;
                    }
                    if (((marker >> 4) ^ 0xC) == 0 && marker != 0xC4 && marker != 0xC8 && marker != 0xCC) {
                        // LOGD("got marker 0x%02X\n", marker);
                        if (pos + 5 > buffer_size) {
                            LOG("Invalid Section Marker 0x%02X\n", marker);
                            goto out;
                        }
                        lh = (uint16_t)buffer[pos + 1];
                        ll = (uint16_t)buffer[pos + 2];
                        h = (lh << 8) | ll;
                        lh = (uint16_t)buffer[pos + 3];
                        ll = (uint16_t)buffer[pos + 4];
                        w = (lh << 8) | ll;
                        // LOGD("input w/h: %d, %d\n", w, h);
                        break;
                    }
                    pos += itemlen - 2;
                }
                if (w == 0 || h == 0) {
                    LOG("can not get width and height of jpeg\n");
                    goto out;
                }
                auto fmt = output_format;
                if (fmt == PIX_FMT_NONE) {
                    fmt = PIX_FMT_ARGB8888;
                }
                output_buffer_size =
                    CalPixFmtSize(fmt, UPALIGNTO16(w), UPALIGNTO16(h)) + UPALIGNTO16(w) * UPALIGNTO16(h) / 2;
            } else {
                LOG("TODO: mpp need external output buffer now\n");
                goto out;
            }
        }
        ret = init_mpp_buffer(mpp_buf, output, output_buffer_size);
        if (ret || !mpp_buf) {
            LOG("Failed to init MPP buffer (ret = %d)\n", ret);
            goto out;
        }
        if (mpp_buf) {
            ret = mpp_frame_init(&frame);
            if (MPP_OK != ret) {
                LOG("Failed mpp_frame_init (ret = %d)\n", ret);
                goto out;
            }
            mpp_frame_set_buffer(frame, mpp_buf);
            mpp_buffer_put(mpp_buf);
            mpp_buf = NULL;
        }
        ret = mpi->poll(ctx, MPP_PORT_INPUT, MPP_POLL_MAX);
        if (ret) {
            LOG("mpp input poll failed (ret = %d)\n", ret);
            goto out;
        }
        ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);
        if (ret) {
            LOG("mpp task input dequeue failed (ret = %d)\n", ret);
            goto out;
        }
        assert(task);
        mpp_task_meta_set_packet(task, KEY_INPUT_PACKET, packet);
        if (frame) {
            mpp_task_meta_set_frame(task, KEY_OUTPUT_FRAME, frame);
        }
        ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);
        if (ret) {
            LOG("mpp task input enqueue failed (ret = %d)\n", ret);
            goto out;
        }
        ret = mpi->poll(ctx, MPP_PORT_OUTPUT, timeout != MPP_POLL_NON_BLOCK ? (MppPollType)timeout : MPP_POLL_MAX);
        if (ret) {
            LOG("mpp output poll failed (ret = %d)\n", ret);
            goto out;
        }
        ret = mpi->dequeue(ctx, MPP_PORT_OUTPUT, &task);
        if (ret) {
            LOG("mpp task output dequeue failed (ret = %d)\n", ret);
            goto out;
        }
        mpp_packet_deinit(&packet);
        if (!task) {
            goto out;
        }
        mpp_task_meta_get_frame(task, KEY_OUTPUT_FRAME, &frame_out);
        assert(frame_out == frame); // one in, one out
        ret = mpi->enqueue(ctx, MPP_PORT_OUTPUT, task);
        if (ret) {
            LOG("mpp task output enqueue failed\n");
        }
        if (mpp_frame_get_errinfo(frame_out)) {
            LOG("Received a errinfo frame.\n");
            goto out;
        }
        if (SetImageBufferWithMppFrame(std::static_pointer_cast<ImageBuffer>(output), mpp_ctx, frame)) {
            goto out;
        }

        return 0;

    out:
        if (mpp_buf) {
            mpp_buffer_put(mpp_buf);
        }
        if (packet) {
            mpp_packet_deinit(&packet);
        }
        if (frame) {
            mpp_frame_deinit(&frame);
        }
        return -EFAULT;
    }

    int MPPDecoder::SendInput(const std::shared_ptr<MediaBuffer>& input)
    {
        if (!support_async) {
            errno = ENOSYS;
            return -ENOSYS;
        }
        if (!input) {
            return 0;
        }
        int fret = 0;
        MPP_RET ret;
        MppPacket packet = NULL;
        void* buffer = input->GetPtr();
        size_t size = input->GetValidSize();

        ret = mpp_packet_init(&packet, buffer, size);
        if (ret != MPP_OK) {
            LOG("Failed to init MPP packet (ret = %d)\n", ret);
            return -EFAULT;
        }
        mpp_packet_set_pts(packet, input->GetUSTimeStamp());
        if (input->IsEOF()) {
            LOG("send eos packet to MPP\n");
            mpp_packet_set_eos(packet);
        }
        ret = mpp_ctx->mpi->decode_put_packet(mpp_ctx->ctx, packet);
        if (ret != MPP_OK) {
            if (ret == MPP_ERR_BUFFER_FULL) {
                // LOG("Buffer full writing %d bytes to decoder\n", size);
                fret = -EAGAIN;
            } else {
                LOG("Failed to put a packet to MPP (ret = %d)\n", ret);
                fret = -EFAULT;
            }
        }
        mpp_packet_deinit(&packet);

        return fret;
    }

    std::shared_ptr<MediaBuffer> MPPDecoder::FetchOutput()
    {
        MppFrame mppframe = NULL;
        MppCtx ctx = mpp_ctx->ctx;
        MppApi* mpi = mpp_ctx->mpi;
    RETRY_GET_FRAME:
        MPP_RET ret = mpi->decode_get_frame(ctx, &mppframe);
        errno = 0;
        LOGD("decode_get_frame ret = %d, mpp frame is %p\n", ret, mppframe);
        if (ret != MPP_OK) {
            if (ret != MPP_ERR_TIMEOUT) {
                LOG("Failed to get a frame from MPP (ret = %d)\n", ret);
            }
            return nullptr;
        }
        if (!mppframe) {
            // LOG("mppframe is NULL\n", mppframe);
            return nullptr;
        }
        if (mpp_frame_get_info_change(mppframe)) {
            MediaConfig& cfg = GetConfig();
            ImageInfo& img_info = cfg.img_cfg.image_info;
            img_info.pix_fmt = ConvertToPixFmt(mpp_frame_get_fmt(mppframe));
            img_info.width = mpp_frame_get_width(mppframe);
            img_info.height = mpp_frame_get_height(mppframe);
            img_info.vir_width = mpp_frame_get_hor_stride(mppframe);
            img_info.vir_height = mpp_frame_get_ver_stride(mppframe);
            mpp_frame_deinit(&mppframe);
            mppframe = NULL;
            cfg.type = Type::Image;
            ret = mpi->control(ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
            if (ret != MPP_OK) {
                LOG("info change ready failed ret = %d\n", ret);
            }
            LOG("MppDec Info change get, %dx%d in (%dx%d)\n", img_info.width, img_info.height, img_info.vir_width,
                img_info.vir_height);

            // Decoder in Blocking mode, and need_split = 0.
            // This means that the decoder is in single-frame input and single-frame
            // output mode, and it cannot output an empty buffer in this mode.
            if ((timeout == MPP_POLL_BLOCK) && (need_split == 0)) {
                goto RETRY_GET_FRAME;
            }

            // return a zero size buffer, but contain image info
            auto mb = std::make_shared<ImageBuffer>();
            if (!mb) {
                errno = ENOMEM;
                goto out;
            }
            mb->GetImageInfo() = img_info;
            return mb;
        } else if (mpp_frame_get_eos(mppframe)) {
            LOG("Received EOS frame.\n");
            auto mb = std::make_shared<ImageBuffer>();
            if (!mb) {
                errno = ENOMEM;
                goto out;
            }
            mb->SetUSTimeStamp(mpp_frame_get_pts(mppframe));
            mb->SetEOF(true);
            mpp_frame_deinit(&mppframe);
            return mb;
        } else if (mpp_frame_get_discard(mppframe)) {
            LOG("Received a discard frame.\n");
            goto out;
        } else if (mpp_frame_get_errinfo(mppframe)) {
            LOG("Received a errinfo frame.\n");
            goto out;
        } else {
            auto mb = std::make_shared<ImageBuffer>();
            if (!mb) {
                errno = ENOMEM;
                goto out;
            }
            if (SetImageBufferWithMppFrame(mb, mpp_ctx, mppframe)) {
                goto out;
            }

            return mb;
        }

    out:
        mpp_frame_deinit(&mppframe);
        return nullptr;
    }

    DEFINE_VIDEO_DECODER_FACTORY(MPPDecoder)
    const char* FACTORY(MPPDecoder)::ExpectedInputDataType()
    {
        return TYPENEAR(IMAGE_JPEG) TYPENEAR(VIDEO_H264) TYPENEAR(VIDEO_H265);
    }
    const char* FACTORY(MPPDecoder)::OutPutDataType()
    {
        return IMAGE_NV12;
    }

} // namespace easymedia
