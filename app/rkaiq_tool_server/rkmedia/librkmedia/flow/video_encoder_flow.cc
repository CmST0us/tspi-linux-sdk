// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>

#include "encoder.h"
#include "flow.h"

#include "buffer.h"
#include "media_type.h"

#ifdef RK_MOVE_DETECTION
    #include "move_detection_flow.h"
#endif

// When the resolution is 2688x1520,
// the average encoding takes 12ms.
//
// TO DO: Dynamic calculate time cost.
#define ENC_CONST_MAX_TIME 12000 // us

namespace easymedia
{

    static bool encode(Flow* f, MediaBufferVector& input_vector);

    class VideoEncoderFlow : public Flow
    {
      public:
        VideoEncoderFlow(const char* param);
        virtual ~VideoEncoderFlow()
        {
            AutoPrintLine apl(__func__);
            StopAllThread();
        }
        static const char* GetFlowName()
        {
            return "video_enc";
        }
        int Control(unsigned long int request, ...);
        void Dump(std::string& dump_info) override;

        MediaConfig GetConfig()
        {
            MediaConfig cfg;
            memset(&cfg, 0, sizeof(cfg));
            if (enc) {
                cfg = enc->GetConfig();
            }
            return cfg;
        }

      private:
        std::shared_ptr<VideoEncoder> enc;
        bool extra_output;
        bool extra_merge;
        std::list<std::shared_ptr<MediaBuffer>> extra_buffer_list;
#ifdef RK_MOVE_DETECTION
        MoveDetectionFlow* md_flow;
#endif // RK_MOVE_DETECTION
        friend bool encode(Flow* f, MediaBufferVector& input_vector);
    };

    bool encode(Flow* f, MediaBufferVector& input_vector)
    {
        VideoEncoderFlow* vf = (VideoEncoderFlow*)f;
        std::shared_ptr<VideoEncoder> enc = vf->enc;
        std::shared_ptr<MediaBuffer>& src = input_vector[0];
        std::shared_ptr<MediaBuffer> dst, extra_dst;

        if (!src) {
            return false;
        }

        dst = std::make_shared<MediaBuffer>(); // TODO: buffer pool
        if (!dst) {
            LOG_NO_MEMORY();
            return false;
        }
        if (vf->extra_output) {
            extra_dst = std::make_shared<MediaBuffer>();
            if (!extra_dst) {
                LOG_NO_MEMORY();
                return false;
            }
        }

#ifdef RK_MOVE_DETECTION
        std::shared_ptr<MediaBuffer> md_info;
        if (vf->md_flow) {
            int smartp_enable = 0;
            enc->QueryChange(VideoEncoder::kMoveDetectionFlow, &smartp_enable, sizeof(int));
            if (!smartp_enable) {
                LOG("INFO: VEnc Flow: Wait for smartp configuration to take effect\n");
            } else {
                LOGD("VEnc Flow: LookForMdResult start!\n");

                MediaConfig mcfg = vf->GetConfig();
                int fps = mcfg.vid_cfg.frame_rate;
                if (fps <= 0) {
                    LOG("ERROR: VEnc Flow: smartp must config fps correctly!");
                    fps = 30;
                } else if (fps > 60) {
                    LOG("WARN: VEnc Flow: smartp may error in high fps:%d!", fps);
                }

                int maximum_timeout = 1000000 / fps - ENC_CONST_MAX_TIME;
                md_info = vf->md_flow->LookForMdResult(src->GetAtomicClock(), maximum_timeout);
                if (md_info) {
    #ifndef NDEBUG
                    LOGD("VEnc Flow: get md info(cnt=%d): %p, %zuBytes\n", md_info->GetValidSize() / sizeof(INFO_LIST),
                         md_info.get(), md_info->GetValidSize());
    #endif
                    if (md_info->GetSize() >= sizeof(INFO_LIST)) {
    #ifndef NDEBUG
                        INFO_LIST* info = (INFO_LIST*)md_info->GetPtr();
                        while (info->flag) {
                            LOGD("VEnc Flow: mdinfo: flag:%d, upleft:<%d, %d>, downright:<%d, "
                                 "%d>\n",
                                 info->flag, info->up_left[0], info->up_left[1], info->down_right[0],
                                 info->down_right[1]);
                            info += 1;
                        }
    #endif
                        src->SetRelatedSPtr(md_info);
                    }
                } else {
                    LOG("ERROR: VEnc Flow: fate error get null md result\n");
                }

                LOGD("VEnc Flow: LookForMdResult end!\n\n");
            }
        }
#endif // RK_MOVE_DETECTION

        if (0 != enc->Process(src, dst, extra_dst)) {
            LOG("encoder failed\n");
            return false;
        }

        bool ret = true;
        // when output fps less len input fps, enc->Proccess() may
        // return a empty mediabuff.
        if (dst->GetValidSize() > 0) {
            ret = vf->SetOutput(dst, 0);
            if (vf->extra_output) {
                ret &= vf->SetOutput(extra_dst, 1);
            }
        }

        return ret;
    }

    VideoEncoderFlow::VideoEncoderFlow(const char* param)
        : extra_output(false), extra_merge(false)
#ifdef RK_MOVE_DETECTION
          ,
          md_flow(nullptr)
#endif
    {
        std::list<std::string> separate_list;
        std::map<std::string, std::string> params;

        LOG("VEnc Flow: dump param:%s\n", param);
        if (!ParseWrapFlowParams(param, params, separate_list)) {
            SetError(-EINVAL);
            return;
        }
        std::string& codec_name = params[KEY_NAME];
        if (codec_name.empty()) {
            LOG("missing codec name\n");
            SetError(-EINVAL);
            return;
        }

        std::string& extra_merge_value = params[KEY_NEED_EXTRA_MERGE];
        if (!extra_merge_value.empty()) {
            extra_merge = !!std::stoi(extra_merge_value);
        }

        const char* ccodec_name = codec_name.c_str();
        // check input/output type
        std::string&& rule = gen_datatype_rule(params);
        if (rule.empty()) {
            SetError(-EINVAL);
            return;
        }
        if (!REFLECTOR(Encoder)::IsMatch(ccodec_name, rule.c_str())) {
            LOG("Unsupport for video encoder %s : [%s]\n", ccodec_name, rule.c_str());
            SetError(-EINVAL);
            return;
        }

        std::string& enc_param_str = separate_list.back();
        std::map<std::string, std::string> enc_params;

        if (!parse_media_param_map(enc_param_str.c_str(), enc_params)) {
            SetError(-EINVAL);
            return;
        }
        // copy data type to enc params.
        std::string::size_type idx;
        idx = enc_param_str.find(KEY_OUTPUTDATATYPE);
        if (idx == enc_param_str.npos)
            PARAM_STRING_APPEND(enc_param_str, KEY_OUTPUTDATATYPE, params[KEY_OUTPUTDATATYPE]);
        if (enc_params[KEY_INPUTDATATYPE].empty()) {
            enc_params[KEY_INPUTDATATYPE] = params[KEY_INPUTDATATYPE];
        }
        if (enc_params[KEY_OUTPUTDATATYPE].empty()) {
            enc_params[KEY_OUTPUTDATATYPE] = params[KEY_OUTPUTDATATYPE];
        }

        MediaConfig mc;
        if (!ParseMediaConfigFromMap(enc_params, mc)) {
            SetError(-EINVAL);
            return;
        }

        auto encoder = REFLECTOR(Encoder)::Create<VideoEncoder>(ccodec_name, enc_param_str.c_str());
        if (!encoder) {
            LOG("Fail to create video encoder %s<%s>\n", ccodec_name, enc_param_str.c_str());
            SetError(-EINVAL);
            return;
        }

        if (!encoder->InitConfig(mc)) {
            LOG("Fail to init config, %s\n", ccodec_name);
            SetError(-EINVAL);
            return;
        }

        std::string roi_region_str = enc_params[KEY_ROI_REGIONS];
        if (!roi_region_str.empty()) {
            int roi_regions_cnt = 0;
            std::vector<EncROIRegion> roi_regions;
            roi_regions = StringToRoiRegions(roi_region_str);
            roi_regions_cnt = roi_regions.size();
            if (roi_regions_cnt) {
                EncROIRegion* regions = (EncROIRegion*)malloc(roi_regions_cnt * sizeof(EncROIRegion));
                for (int i = 0; i < roi_regions_cnt; i++) {
                    (regions + i)->x = roi_regions[i].x;
                    (regions + i)->y = roi_regions[i].y;
                    (regions + i)->w = roi_regions[i].w;
                    (regions + i)->h = roi_regions[i].h;
                    (regions + i)->intra = roi_regions[i].intra;
                    (regions + i)->quality = roi_regions[i].quality;
                    (regions + i)->qp_area_idx = roi_regions[i].qp_area_idx;
                    (regions + i)->area_map_en = roi_regions[i].area_map_en;
                    (regions + i)->abs_qp_en = roi_regions[i].abs_qp_en;
                    LOG("VEnc Flow: Roi Regions[%d]: (%d,%d,%d,%d,%d,%d,%d,%d,%d)\n", i, roi_regions[i].x,
                        roi_regions[i].y, roi_regions[i].w, roi_regions[i].h, roi_regions[i].intra,
                        roi_regions[i].quality, roi_regions[i].qp_area_idx, roi_regions[i].area_map_en,
                        roi_regions[i].abs_qp_en);
                }

                auto pbuff = std::make_shared<ParameterBuffer>(0);
                pbuff->SetPtr(regions, roi_regions_cnt * sizeof(EncROIRegion));
                encoder->RequestChange(VideoEncoder::kROICfgChange, pbuff);
            }
        }

        void* extra_data = nullptr;
        size_t extra_data_size = 0;
        encoder->GetExtraData(&extra_data, &extra_data_size);
        // TODO: if not h264
        const std::string& output_dt = enc_params[KEY_OUTPUTDATATYPE];

        enc = encoder;

        SlotMap sm;
        sm.input_slots.push_back(0);
        sm.output_slots.push_back(0);
        if (params[KEY_NEED_EXTRA_OUTPUT] == "y") {
            extra_output = true;
            sm.output_slots.push_back(1);
        }
        sm.process = encode;
        sm.thread_model = Model::ASYNCCOMMON;
        sm.mode_when_full = InputMode::DROPFRONT;
        sm.input_maxcachenum.push_back(3);
        if (!InstallSlotMap(sm, "VideoEncoderFlow", 40)) {
            LOG("Fail to InstallSlotMap, %s\n", ccodec_name);
            SetError(-EINVAL);
            return;
        }
        SetFlowTag("VideoEncoderFlow");

        if (extra_data && extra_data_size > 0 && (output_dt == VIDEO_H264 || output_dt == VIDEO_H265)) {

            if (extra_merge) {
                std::shared_ptr<MediaBuffer> extra_buf = std::make_shared<MediaBuffer>();
                extra_buf->SetPtr(extra_data);
                extra_buf->SetValidSize(extra_data_size);
                extra_buf->SetUserFlag(MediaBuffer::kExtraIntra);
                SetOutput(extra_buf, 0);
            } else {
                if (output_dt == VIDEO_H264)
                    extra_buffer_list =
                        split_h264_separate((const uint8_t*)extra_data, extra_data_size, gettimeofday());
                else
                    extra_buffer_list =
                        split_h265_separate((const uint8_t*)extra_data, extra_data_size, gettimeofday());
                for (auto& extra_buffer : extra_buffer_list) {
                    assert(extra_buffer->GetUserFlag() & MediaBuffer::kExtraIntra);
                    SetOutput(extra_buffer, 0);
                }
            }

            if (extra_output) {
                std::shared_ptr<MediaBuffer> nullbuffer;
                SetOutput(nullbuffer, 1);
            }
        }
    }

    int VideoEncoderFlow::Control(unsigned long int request, ...)
    {
        va_list ap;
        va_start(ap, request);
        auto value = va_arg(ap, std::shared_ptr<ParameterBuffer>);
        va_end(ap);
        assert(value);

#ifdef RK_MOVE_DETECTION
        if (request == VideoEncoder::kMoveDetectionFlow) {
            if (value->GetSize() != sizeof(void**)) {
                LOG("ERROR: VEnc Flow: move detect config falied!\n");
                return -1;
            }
            md_flow = *((MoveDetectionFlow**)value->GetPtr());
            LOGD("VEnc Flow: md_flow:%p\n", md_flow);
        }
#endif // RK_MOVE_DETECTION

        enc->RequestChange(request, value);
        return 0;
    }

    void VideoEncoderFlow::Dump(std::string& dump_info)
    {
        const MediaConfig mc = GetConfig();
        char str_line[1024] = {0};

        DumpBase(dump_info);
        sprintf(str_line, "#Dump Flow(%s) advanced info:\r\n", GetFlowTag());
        dump_info.append(str_line);
        memset(str_line, 0, sizeof(str_line));
        sprintf(str_line, "  Name:%s\r\n", GetFlowName());
        dump_info.append(str_line);

        memset(str_line, 0, sizeof(str_line));
        if (mc.type == Type::Image) {
            dump_info.append("  CodecType: JPEG\r\n");
            sprintf(str_line, "  Input: %d(%d)x%d(%d) fmt:%s\r\n", mc.img_cfg.image_info.width,
                    mc.img_cfg.image_info.vir_width, mc.img_cfg.image_info.height, mc.img_cfg.image_info.vir_height,
                    PixFmtToString(mc.img_cfg.image_info.pix_fmt));
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  Qfactor:%d\n", mc.img_cfg.qfactor);
            dump_info.append(str_line);
        } else if (mc.type == Type::Video) {
            const VideoConfig& vcfg = mc.vid_cfg;
            const ImageConfig& imgcfg = vcfg.image_cfg;

            if (imgcfg.codec_type == CODEC_TYPE_H264) {
                dump_info.append("  CodecType: H264\r\n");
            } else if (imgcfg.codec_type == CODEC_TYPE_H265) {
                dump_info.append("  CodecType: H265\r\n");
            } else {
                LOG("ERROR: VEnc Flow: config fatal error!\n");
                return;
            }
            sprintf(str_line, "  Input: %d(%d)x%d(%d) fmt:%s\r\n", imgcfg.image_info.width, imgcfg.image_info.vir_width,
                    imgcfg.image_info.height, imgcfg.image_info.vir_height, PixFmtToString(imgcfg.image_info.pix_fmt));
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  QpArray: init:%d min:%d, max:%d, step:%d, min_i:%d, max_i:%d\r\n", vcfg.qp_init,
                    vcfg.qp_min, vcfg.qp_max, vcfg.qp_step, vcfg.qp_min_i, vcfg.qp_max_i);
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  BitRate: target:%d, min:%d, max:%d\r\n", vcfg.bit_rate, vcfg.bit_rate_min,
                    vcfg.bit_rate_max);
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  FrameRate: in:%d/%d, out:%d/%d\r\n", vcfg.frame_in_rate, vcfg.frame_in_rate_den,
                    vcfg.frame_rate, vcfg.frame_rate_den);
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  GopSize: %d\r\n", vcfg.gop_size);
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  RcQuality: %s\r\n", vcfg.rc_quality);
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  RcMode: %s\r\n", vcfg.rc_mode);
            dump_info.append(str_line);
            memset(str_line, 0, sizeof(str_line));
            sprintf(str_line, "  FullRange: %s\r\n", vcfg.full_range ? "Enable" : "Disable");
            dump_info.append(str_line);

            if (imgcfg.codec_type == CODEC_TYPE_H264) {
                memset(str_line, 0, sizeof(str_line));
                sprintf(str_line, "  Trans8x8: %s\r\n", vcfg.trans_8x8 ? "Enable" : "Disable");
                dump_info.append(str_line);
                memset(str_line, 0, sizeof(str_line));
                sprintf(str_line, "  H264Level: %d\r\n", vcfg.level);
                dump_info.append(str_line);
                memset(str_line, 0, sizeof(str_line));
                sprintf(str_line, "  H264Profile: %d\r\n", vcfg.profile);
                dump_info.append(str_line);
            }
        } else {
            LOG("ERROR: VEnc Flow: Dump: to do...!\n");
            return;
        }

        return;
    }

    DEFINE_FLOW_FACTORY(VideoEncoderFlow, Flow)
    // type depends on encoder
    const char* FACTORY(VideoEncoderFlow)::ExpectedInputDataType()
    {
        return "";
    }
    const char* FACTORY(VideoEncoderFlow)::OutPutDataType()
    {
        return "";
    }

} // namespace easymedia
