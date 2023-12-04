// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_config.h"

#include <sstream>
#include <strings.h>

#include "encoder.h"
#include "key_string.h"
#include "media_type.h"
#include "utils.h"

namespace easymedia
{

    const char* rc_quality_strings[7] = {KEY_LOWEST, KEY_LOWER, KEY_LOW, KEY_MEDIUM, KEY_HIGH, KEY_HIGHER, KEY_HIGHEST};

    const char* rc_mode_strings[3] = {KEY_VBR, KEY_CBR, KEY_FIXQP};

    static const char* convert2constchar(const std::string& s, const char* array[], size_t array_len)
    {
        for (size_t i = 0; i < array_len; i++)
            if (!strcasecmp(s.c_str(), array[i])) {
                return array[i];
            }
        return nullptr;
    }

    const char* ConvertRcQuality(const std::string& s)
    {
        return convert2constchar(s, rc_quality_strings, ARRAY_ELEMS(rc_quality_strings));
    }

    const char* ConvertRcMode(const std::string& s)
    {
        return convert2constchar(s, rc_mode_strings, ARRAY_ELEMS(rc_mode_strings));
    }

    static int ParseMediaConfigFps(std::map<std::string, std::string>& params, VideoConfig& vid_cfg)
    {
        std::string value = params[KEY_FPS];
        char* num = NULL;
        char* den = NULL;

        if (value.empty()) {
            LOG("ERROR: MediaCfg: fps: KEY_FPS is null!\n");
            return -1;
        }
        num = strtok((char*)value.c_str(), "/");
        den = strtok(NULL, "/");
        if (!num || !den || (strlen(num) > 2) || (strlen(den) > 2)) {
            LOG("ERROR: MediaCfg: fps: KEY_FPS=%s is invalid!\n", value.c_str());
            return -1;
        }
        vid_cfg.frame_rate = std::atoi(num);
        vid_cfg.frame_rate_den = std::atoi(den);

        value = params[KEY_FPS_IN];
        if (value.empty()) {
            LOG("ERROR: MediaCfg: fps: KEY_FPS_IN is null!\n");
            return -1;
        }
        num = strtok((char*)value.c_str(), "/");
        den = strtok(NULL, "/");
        if (!num || !den || (strlen(num) > 2) || (strlen(den) > 2)) {
            LOG("ERROR: MediaCfg: fps: KEY_FPS_IN(%s) is null!\n", value.c_str());
            return -1;
        }

        vid_cfg.frame_in_rate = std::atoi(num);
        vid_cfg.frame_in_rate_den = std::atoi(den);

        return 0;
    }

    bool ParseMediaConfigFromMap(std::map<std::string, std::string>& params, MediaConfig& mc)
    {
        std::string value = params[KEY_OUTPUTDATATYPE];
        if (value.empty()) {
            LOG("miss %s\n", KEY_OUTPUTDATATYPE);
            return false;
        }
        bool image_in = string_start_withs(value, IMAGE_PREFIX);
        bool video_in = string_start_withs(value, VIDEO_PREFIX);
        bool audio_in = string_start_withs(value, AUDIO_PREFIX);
        if (!image_in && !video_in && !audio_in) {
            LOG("unsupport outtype %s\n", value.c_str());
            return false;
        }
        ImageInfo info;
        CodecType codec_type = StringToCodecType(value.c_str());
        if (codec_type == CODEC_TYPE_NONE) {
            LOG("ERROR: unsupport outtype %s\n", value.c_str());
            return false;
        }

        if (image_in || video_in) {
            if (!ParseImageInfoFromMap(params, info)) {
                return false;
            }
        } else {
            // audio
            AudioConfig& aud_cfg = mc.aud_cfg;
            if (!ParseSampleInfoFromMap(params, aud_cfg.sample_info)) {
                return false;
            }
            CHECK_EMPTY(value, params, KEY_COMPRESS_BITRATE)
            aud_cfg.bit_rate = std::stoi(value);
            CHECK_EMPTY(value, params, KEY_FLOAT_QUALITY)
            aud_cfg.quality = std::stof(value);
            // CHECK_EMPTY(value, params, KEY_CODECTYPE)
            aud_cfg.codec_type = codec_type;
            mc.type = Type::Audio;
            return true;
        }
        if (image_in) {
            ImageConfig& img_cfg = mc.img_cfg;
            img_cfg.image_info = info;
            GET_STRING_TO_INT(img_cfg.qfactor, params, KEY_JPEG_QFACTOR, 0)
            img_cfg.codec_type = codec_type;
            mc.type = Type::Image;
        } else if (video_in) {
            VideoConfig& vid_cfg = mc.vid_cfg;
            ImageConfig& img_cfg = vid_cfg.image_cfg;
            img_cfg.image_info = info;
            img_cfg.codec_type = codec_type;
            GET_STRING_TO_INT(img_cfg.qfactor, params, KEY_JPEG_QFACTOR, 0)
            GET_STRING_TO_INT(vid_cfg.qp_init, params, KEY_COMPRESS_QP_INIT, 0)
            GET_STRING_TO_INT(vid_cfg.qp_step, params, KEY_COMPRESS_QP_STEP, 0)
            GET_STRING_TO_INT(vid_cfg.qp_min, params, KEY_COMPRESS_QP_MIN, 0)
            GET_STRING_TO_INT(vid_cfg.qp_max, params, KEY_COMPRESS_QP_MAX, 0)
            GET_STRING_TO_INT(vid_cfg.bit_rate, params, KEY_COMPRESS_BITRATE, 0)
            GET_STRING_TO_INT(vid_cfg.bit_rate_min, params, KEY_COMPRESS_BITRATE_MIN, 0)
            GET_STRING_TO_INT(vid_cfg.bit_rate_max, params, KEY_COMPRESS_BITRATE_MAX, 0)
            GET_STRING_TO_INT(vid_cfg.qp_max_i, params, KEY_COMPRESS_QP_MAX_I, 0)
            GET_STRING_TO_INT(vid_cfg.qp_min_i, params, KEY_COMPRESS_QP_MIN_I, 0)
            GET_STRING_TO_INT(vid_cfg.trans_8x8, params, KEY_H264_TRANS_8x8, 0)
            GET_STRING_TO_INT(vid_cfg.level, params, KEY_LEVEL, 0)
            GET_STRING_TO_INT(vid_cfg.gop_size, params, KEY_VIDEO_GOP, 0)
            GET_STRING_TO_INT(vid_cfg.profile, params, KEY_PROFILE, 0)
            GET_STRING_TO_INT(vid_cfg.full_range, params, KEY_FULL_RANGE, 0)
            GET_STRING_TO_INT(vid_cfg.ref_frm_cfg, params, KEY_REF_FRM_CFG, 0)
            GET_STRING_TO_INT(vid_cfg.rotation, params, KEY_ROTATION, 0)

            if (ParseMediaConfigFps(params, vid_cfg) < 0) {
                return false;
            }

            const std::string rc_q = params[KEY_COMPRESS_RC_QUALITY];
            if (rc_q.empty()) {
                vid_cfg.rc_quality = NULL;
            } else {
                vid_cfg.rc_quality = ConvertRcQuality(rc_q);
            }

            const std::string rc_m = params[KEY_COMPRESS_RC_MODE];
            if (rc_m.empty()) {
                vid_cfg.rc_mode = NULL;
            } else {
                vid_cfg.rc_mode = ConvertRcMode(rc_m);
            }

            mc.type = Type::Video;
        }
        return true;
    }

    // roi_regions:(x,x,x,x,x,x,x,x,x)(x,x,x,x,x,x,x,x,x)...
    std::vector<EncROIRegion> StringToRoiRegions(const std::string& str_regions)
    {
        std::vector<EncROIRegion> ret;
        const char* start = nullptr;
        if (str_regions.empty()) {
            return std::move(ret);
        }

        start = str_regions.c_str();
        while (start) {
            EncROIRegion region = {0, 0, 0, 0, 0, 0, 0, 0, 0};
            start = strstr(start, "(");
            if (!start) {
                break;
            }
            const char* end = strstr(start, ")");
            if (!end) {
                LOG("ERROR: RoiRegions string is invalid! end error! Value:%s\n", str_regions.c_str());
                break;
            }

            int commas_cnt = 0;
            const char* commas_str = start;
            while (commas_str && (commas_str < end)) {
                commas_str = strstr(commas_str, ",");
                if (!commas_str) {
                    break;
                } else if (commas_str < end) {
                    commas_cnt++;
                }
                commas_str++;
            }

            if (commas_cnt != 8) {
                LOG("ERROR: RoiRegions string is invalid! Value:%s\n", str_regions.c_str());
                break;
            }
            int x, y, w, h, intra, quality, qp_area_idx, area_map_en, abs_qp_en;
            int r = sscanf(start, "(%d,%d,%d,%d,%d,%d,%d,%d,%d)", &x, &y, &w, &h, &intra, &quality, &qp_area_idx,
                           &area_map_en, &abs_qp_en);
            if (r != 9) {
                LOG("ERROR: Fail to sscanf(ret=%d) : %m\n", r);
                ret.clear();
                return std::move(ret);
            }
            region.x = (uint16_t)x;
            region.y = (uint16_t)y;
            region.w = (uint16_t)w;
            region.h = (uint16_t)h;
            region.intra = (uint16_t)intra;
            region.quality = (int16_t)quality;
            region.qp_area_idx = (uint16_t)qp_area_idx;
            region.area_map_en = (uint8_t)area_map_en;
            region.abs_qp_en = (uint8_t)abs_qp_en;
            ret.push_back(std::move(region));
            start = end;
        }

        return std::move(ret);
    }

    std::string to_param_string(const ImageConfig& img_cfg)
    {
        std::string ret = to_param_string(img_cfg.image_info);
        PARAM_STRING_APPEND_TO(ret, KEY_JPEG_QFACTOR, img_cfg.qfactor);
        PARAM_STRING_APPEND_TO(ret, KEY_CODECTYPE, img_cfg.codec_type);
        return ret;
    }

    std::string to_param_string(const VideoConfig& vid_cfg)
    {
        const ImageConfig& img_cfg = vid_cfg.image_cfg;
        std::string ret = to_param_string(img_cfg);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_QP_INIT, vid_cfg.qp_init);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_QP_STEP, vid_cfg.qp_step);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_QP_MIN, vid_cfg.qp_min);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_QP_MAX, vid_cfg.qp_max);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_BITRATE, vid_cfg.bit_rate);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_BITRATE_MAX, vid_cfg.bit_rate_max);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_BITRATE_MIN, vid_cfg.bit_rate_min);
        std::stringstream str_stream;
        std::string fps;
        str_stream << vid_cfg.frame_rate;
        str_stream << "/";
        str_stream << vid_cfg.frame_rate_den;
        str_stream >> fps;
        PARAM_STRING_APPEND(ret, KEY_FPS, fps);
        str_stream.clear();
        str_stream << vid_cfg.frame_in_rate;
        str_stream << "/";
        str_stream << vid_cfg.frame_in_rate_den;
        str_stream >> fps;
        PARAM_STRING_APPEND(ret, KEY_FPS_IN, fps);
        PARAM_STRING_APPEND_TO(ret, KEY_LEVEL, vid_cfg.level);
        PARAM_STRING_APPEND_TO(ret, KEY_VIDEO_GOP, vid_cfg.gop_size);
        PARAM_STRING_APPEND_TO(ret, KEY_PROFILE, vid_cfg.profile);
        if (vid_cfg.rc_quality) {
            PARAM_STRING_APPEND(ret, KEY_COMPRESS_RC_QUALITY, vid_cfg.rc_quality);
        }
        if (vid_cfg.rc_mode) {
            PARAM_STRING_APPEND(ret, KEY_COMPRESS_RC_MODE, vid_cfg.rc_mode);
        }
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_QP_MAX_I, vid_cfg.qp_max_i);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_QP_MIN_I, vid_cfg.qp_min_i);
        PARAM_STRING_APPEND_TO(ret, KEY_H264_TRANS_8x8, vid_cfg.trans_8x8);
        PARAM_STRING_APPEND_TO(ret, KEY_FULL_RANGE, vid_cfg.full_range);
        PARAM_STRING_APPEND_TO(ret, KEY_REF_FRM_CFG, vid_cfg.ref_frm_cfg);
        PARAM_STRING_APPEND_TO(ret, KEY_ROTATION, vid_cfg.rotation);
        return ret;
    }

    std::string to_param_string(const AudioConfig& aud_cfg)
    {
        std::string ret = to_param_string(aud_cfg.sample_info);
        PARAM_STRING_APPEND_TO(ret, KEY_COMPRESS_BITRATE, aud_cfg.bit_rate);
        PARAM_STRING_APPEND_TO(ret, KEY_FLOAT_QUALITY, aud_cfg.quality);
        PARAM_STRING_APPEND_TO(ret, KEY_CODECTYPE, aud_cfg.codec_type);
        return ret;
    }

    std::string to_param_string(const MediaConfig& mc, const std::string& out_type)
    {
        std::string ret;
        MediaConfig mc_temp = mc;
        bool image_in = string_start_withs(out_type, IMAGE_PREFIX);
        bool video_in = string_start_withs(out_type, VIDEO_PREFIX);
        bool audio_in = string_start_withs(out_type, AUDIO_PREFIX);
        if (!image_in && !video_in && !audio_in) {
            LOG("unsupport outtype %s\n", out_type.c_str());
            return ret;
        }

        PARAM_STRING_APPEND(ret, KEY_OUTPUTDATATYPE, out_type);
        if (image_in) {
            mc_temp.img_cfg.codec_type = StringToCodecType(out_type.c_str());
            ret.append(to_param_string(mc_temp.img_cfg));
        }

        if (video_in) {
            mc_temp.vid_cfg.image_cfg.codec_type = StringToCodecType(out_type.c_str());
            ret.append(to_param_string(mc_temp.vid_cfg));
        }

        if (audio_in) {
            mc_temp.aud_cfg.codec_type = StringToCodecType(out_type.c_str());
            ret.append(to_param_string(mc_temp.aud_cfg));
        }

        return ret;
    }

    std::string get_video_encoder_config_string(const ImageInfo& info, const VideoEncoderCfg& cfg)
    {
        if (!info.width || !info.height || (info.pix_fmt >= PIX_FMT_NB) || (info.pix_fmt <= PIX_FMT_NONE)) {
            LOG("ERROR: %s image info is wrong!\n", __func__);
            return NULL;
        }

        if (StringToCodecType(cfg.type) < 0) {
            LOG("ERROR: %s not support enc type:%s!\n", __func__, cfg.type);
            return NULL;
        }

        if (cfg.rc_quality && strcmp(cfg.rc_quality, KEY_HIGHEST) && strcmp(cfg.rc_quality, KEY_HIGHER) &&
            strcmp(cfg.rc_quality, KEY_HIGH) && strcmp(cfg.rc_quality, KEY_MEDIUM) && strcmp(cfg.rc_quality, KEY_LOW) &&
            strcmp(cfg.rc_quality, KEY_LOWER) && strcmp(cfg.rc_quality, KEY_LOWEST)) {
            LOG("ERROR: %s rc_quality is invalid!"
                "should be [KEY_LOWEST, KEY_HIGHEST]\n",
                __func__);
            return NULL;
        }

        if (cfg.rc_mode && strcmp(cfg.rc_mode, KEY_VBR) && strcmp(cfg.rc_mode, KEY_CBR)) {
            LOG("ERROR: %s rc_mode is invalid! should be KEY_VBR/KEY_VBR\n", __func__);
            return NULL;
        }

        MediaConfig enc_config;
        memset(&enc_config, 0, sizeof(enc_config));
        VideoConfig& vid_cfg = enc_config.vid_cfg;
        ImageConfig& img_cfg = vid_cfg.image_cfg;
        img_cfg.image_info.pix_fmt = info.pix_fmt;
        img_cfg.image_info.width = info.width;
        img_cfg.image_info.height = info.height;
        img_cfg.image_info.vir_width = info.vir_width;
        img_cfg.image_info.vir_height = info.vir_height;
        img_cfg.codec_type = StringToCodecType(cfg.type);

        if (cfg.fps) {
            vid_cfg.frame_rate = vid_cfg.frame_in_rate = cfg.fps;
        } else {
            vid_cfg.frame_rate = vid_cfg.frame_in_rate = 30;
            LOG("INFO: VideoEnc: frame rate use defalut value:30\n");
        }

        vid_cfg.gop_size = cfg.gop;

        if (cfg.max_bps) {
            vid_cfg.bit_rate_max = cfg.max_bps;
        } else {
            int den, num;
            GetPixFmtNumDen(info.pix_fmt, num, den);
            int wh_product = info.width * info.height;
            if (wh_product > 2073600) {
                vid_cfg.bit_rate_max = wh_product * vid_cfg.frame_rate * num / den / 20;
            } else if (wh_product > 921600) {
                vid_cfg.bit_rate_max = wh_product * vid_cfg.frame_rate * num / den / 17;
            } else if (wh_product > 101376) {
                vid_cfg.bit_rate_max = wh_product * vid_cfg.frame_rate * num / den / 15;
            } else {
                vid_cfg.bit_rate_max = wh_product * vid_cfg.frame_rate * num / den / 8;
            }
            LOG("INFO: VideoEnc: maxbps use defalut value:%d\n", vid_cfg.bit_rate);
        }

        vid_cfg.rc_quality = cfg.rc_quality;
        vid_cfg.rc_mode = cfg.rc_mode;

        std::string enc_param = "";
        enc_param.append(easymedia::to_param_string(enc_config, cfg.type));
        return enc_param;
    }

    int video_encoder_set_bps(std::shared_ptr<Flow>& enc_flow, unsigned int target, unsigned int min, unsigned int max)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        int* bps_array = (int*)malloc(3 * sizeof(int));
        bps_array[0] = min;
        bps_array[1] = target;
        bps_array[2] = max;

        pbuff->SetPtr(bps_array, 3 * sizeof(int));
        enc_flow->Control(VideoEncoder::kBitRateChange, pbuff);

        return 0;
    }

    int video_encoder_set_rc_quality(std::shared_ptr<Flow>& enc_flow, const char* rc_quality)
    {
        if (!enc_flow || !rc_quality) {
            return -EINVAL;
        }

        if (strcmp(rc_quality, KEY_HIGHEST) && strcmp(rc_quality, KEY_HIGHER) && strcmp(rc_quality, KEY_HIGH) &&
            strcmp(rc_quality, KEY_MEDIUM) && strcmp(rc_quality, KEY_LOW) && strcmp(rc_quality, KEY_LOWER) &&
            strcmp(rc_quality, KEY_LOWEST)) {
            LOG("ERROR: %s rc_quality:%s is invalid! "
                "should be [KEY_LOWEST, KEY_HIGHEST]\n",
                __func__, rc_quality);
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        int str_len = strlen(rc_quality);
        char* quality = (char*)malloc(str_len + 1);
        memcpy(quality, rc_quality, strlen(rc_quality));
        quality[str_len] = '\0';
        pbuff->SetPtr(quality, strlen(rc_quality));
        enc_flow->Control(VideoEncoder::kRcQualityChange, pbuff);

        return 0;
    }

    int video_encoder_set_rc_mode(std::shared_ptr<Flow>& enc_flow, const char* rc_mode)
    {
        if (!enc_flow || !rc_mode) {
            return -EINVAL;
        }

        if (strcmp(rc_mode, KEY_VBR) && strcmp(rc_mode, KEY_CBR)) {
            LOG("ERROR: %s rc_mode is invalid! should be KEY_VBR/KEY_VBR\n", __func__);
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        int str_len = strlen(rc_mode);
        char* mode = (char*)malloc(str_len + 1);
        memcpy(mode, rc_mode, strlen(rc_mode));
        mode[str_len] = '\0';
        pbuff->SetPtr(mode, strlen(rc_mode));
        enc_flow->Control(VideoEncoder::kRcModeChange, pbuff);

        return 0;
    }

    int video_encoder_set_qp(std::shared_ptr<Flow>& enc_flow, VideoEncoderQp& qps)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        // qp_max       - 8 ~ 51
        // qp_min       - 0 ~ 48
        if ((qps.qp_max && ((qps.qp_max > 51) || (qps.qp_max < 8))) ||
            (qps.qp_max_i && ((qps.qp_max_i > 51) || (qps.qp_max_i < 8))) || (qps.qp_min < 0) || (qps.qp_min > 48) ||
            (qps.qp_min_i < 0) || (qps.qp_min_i > 48) || (qps.qp_min > qps.qp_max) || (qps.qp_min_i > qps.qp_max_i)) {
            LOG("ERROR: qp range error. qp_min:[0, 48]; qp_max:[8, 51]\n");
            return -EINVAL;
        }

        if ((qps.qp_init > qps.qp_max) || (qps.qp_init < qps.qp_min)) {
            LOG("ERROR: qp_init should be within [qp_min, qp_max]\n");
            return -EINVAL;
        }

        if (!qps.qp_step || (qps.qp_step > (qps.qp_max - qps.qp_min))) {
            LOG("ERROR: qp_step should be within (0, qp_max - qp_min]\n");
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        VideoEncoderQp* qp_struct = (VideoEncoderQp*)malloc(sizeof(VideoEncoderQp));
        memcpy(qp_struct, &qps, sizeof(VideoEncoderQp));
        pbuff->SetPtr(qp_struct, sizeof(VideoEncoderQp));
        enc_flow->Control(VideoEncoder::kQPChange, pbuff);

        return 0;
    }

    int jpeg_encoder_set_qfactor(std::shared_ptr<Flow>& enc_flow, int qfactor)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        if ((qfactor > 99) || (qfactor < 1)) {
            LOG("ERROR: %s: qfactor should be within [1, 99]\n", __func__);
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetValue(qfactor);
        enc_flow->Control(VideoEncoder::kQPChange, pbuff);

        return 0;
    }

    int video_encoder_force_idr(std::shared_ptr<Flow>& enc_flow)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        enc_flow->Control(VideoEncoder::kForceIdrFrame, pbuff);

        return 0;
    }

    int video_encoder_set_fps(std::shared_ptr<Flow>& enc_flow, uint8_t out_num, uint8_t out_den, uint8_t in_num,
                              uint8_t in_den)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        if (!out_den || !out_num || (out_den > 16) || (out_num > 120) || (in_den > 16) || (in_num > 120)) {
            LOG("ERROR: fps(%d/%d) is invalid! num:[1,120], den:[1, 16].\n", out_num, out_den);
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        uint8_t* fps_array = (uint8_t*)malloc(4 * sizeof(uint8_t));
        fps_array[0] = in_num;
        fps_array[1] = in_den;
        fps_array[2] = out_num;
        fps_array[3] = out_den;

        pbuff->SetPtr(fps_array, 4);
        enc_flow->Control(VideoEncoder::kFrameRateChange, pbuff);

        return 0;
    }

    // input palette should be yuva formate.
    int video_encoder_set_osd_plt(std::shared_ptr<Flow>& enc_flow, const uint32_t* yuv_plt)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        uint32_t* plt = (uint32_t*)malloc(256 * sizeof(uint32_t));
        memcpy(plt, yuv_plt, 256 * sizeof(uint32_t));

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(plt, 256 * sizeof(uint32_t));
        enc_flow->Control(VideoEncoder::kOSDPltChange, pbuff);

        return 0;
    }

    int video_encoder_set_gop_size(std::shared_ptr<Flow>& enc_flow, int gop)
    {
        if (!enc_flow || (gop < 0)) {
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetValue(gop);
        enc_flow->Control(VideoEncoder::kGopChange, pbuff);

        return 0;
    }

    int video_encoder_set_osd_region(std::shared_ptr<Flow>& enc_flow, OsdRegionData* region_data)
    {
        if (!enc_flow || !region_data) {
            return -EINVAL;
        }

        if (region_data->enable && ((region_data->width % 16) || (region_data->height % 16))) {
            LOG("ERROR: osd region size must be a multiple of 16x16.");
            return -EINVAL;
        }

        int buffer_size = region_data->width * region_data->height;
        OsdRegionData* rdata = (OsdRegionData*)malloc(sizeof(OsdRegionData) + buffer_size);
        memcpy((void*)rdata, (void*)region_data, sizeof(OsdRegionData));
        if (buffer_size) {
            rdata->buffer = (uint8_t*)rdata + sizeof(OsdRegionData);
            memcpy(rdata->buffer, region_data->buffer, buffer_size);
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(rdata, sizeof(OsdRegionData) + buffer_size);
        enc_flow->Control(VideoEncoder::kOSDDataChange, pbuff);

        return 0;
    }

    int video_encoder_set_move_detection(std::shared_ptr<Flow>& enc_flow, std::shared_ptr<Flow>& md_flow)
    {
        int ret = 0;
        void** rdata = (void**)malloc(sizeof(void*));
        *rdata = md_flow.get();

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(rdata, sizeof(sizeof(void*)));
        ret = enc_flow->Control(easymedia::VideoEncoder::kMoveDetectionFlow, pbuff);

        return ret;
    }

    int video_encoder_set_roi_regions(std::shared_ptr<Flow>& enc_flow, EncROIRegion* regions, int region_cnt)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        int rsize = 0;
        void* rdata = NULL;
        if (regions && region_cnt) {
            rsize = sizeof(EncROIRegion) * region_cnt;
            rdata = (void*)malloc(rsize);
            memcpy(rdata, (void*)regions, rsize);
        }
        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(rdata, rsize);
        enc_flow->Control(VideoEncoder::kROICfgChange, pbuff);
        return 0;
    }

    int video_encoder_set_roi_regions(std::shared_ptr<Flow>& enc_flow, std::string roi_param)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        auto regions = StringToRoiRegions(roi_param);
        int region_cnt = regions.size();
        if (!region_cnt) {
            return -EINVAL;
        }

        EncROIRegion* rdata = NULL;
        int rsize = sizeof(EncROIRegion) * region_cnt;
        rdata = (EncROIRegion*)malloc(rsize);
        if (!rdata) {
            return -EINVAL;
        }

        int i = 0;
        for (auto iter : regions) {
            memcpy((void*)&rdata[i++], (void*)&iter, sizeof(EncROIRegion));
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(rdata, rsize);
        enc_flow->Control(VideoEncoder::kROICfgChange, pbuff);
        return 0;
    }

    int video_move_detect_set_rects(std::shared_ptr<Flow>& md_flow, ImageRect* rects, int rect_cnt)
    {
        if (!md_flow || !rects || !rect_cnt) {
            return -EINVAL;
        }

        return md_flow->Control(easymedia::S_MD_ROI_RECTS, rects, rect_cnt);
    }

    int video_move_detect_set_rects(std::shared_ptr<Flow>& md_flow, std::string rects_param)
    {
        if (!md_flow) {
            return -EINVAL;
        }

        std::vector<ImageRect> rect_vector;
        rect_vector = StringToImageRect(rects_param);
        if (rect_vector.empty()) {
            return -EINVAL;
        }

        int rect_cnt = (int)rect_vector.size();
        ImageRect* rects = (ImageRect*)malloc(rect_cnt * sizeof(ImageRect));
        if (!rects) {
            return -ENOSPC;
        }

        int i = 0;
        for (auto iter : rect_vector) {
            memcpy((void*)&rects[i++], (void*)&iter, sizeof(ImageRect));
        }

        int ret = md_flow->Control(easymedia::S_MD_ROI_RECTS, rects, rect_cnt);
        free(rects);
        return ret;
    }

    int video_encoder_set_split(std::shared_ptr<Flow>& enc_flow, unsigned int mode, unsigned int size)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        uint32_t* param = (uint32_t*)malloc(2 * sizeof(uint32_t));
        *param = mode;
        *(param + 1) = size;
        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(param, 2 * sizeof(uint32_t));
        enc_flow->Control(VideoEncoder::kSplitChange, pbuff);

        return 0;
    }

    int video_encoder_set_gop_mode(std::shared_ptr<Flow>& enc_flow, EncGopModeParam* mode_params)
    {
        if (!enc_flow || !mode_params) {
            return -EINVAL;
        }

        uint8_t* param = (uint8_t*)malloc(sizeof(EncGopModeParam));
        if (!param) {
            return -ENOSPC;
        }

        memcpy(param, mode_params, sizeof(EncGopModeParam));
        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(param, sizeof(EncGopModeParam));
        enc_flow->Control(VideoEncoder::kGopModeChange, pbuff);

        return 0;
    }

    int video_encoder_set_avc_profile(std::shared_ptr<Flow>& enc_flow, int profile_idc, int level)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        if ((profile_idc != 66) && (profile_idc != 77) && (profile_idc != 100)) {
            LOG("ERROR: %s profile_idc:%d is invalid!"
                "Only supprot: 66:Baseline, 77:Main Profile, 100: High Profile\n",
                __func__, profile_idc);
            return -EINVAL;
        }

        int* param = (int*)malloc(2 * sizeof(int));
        *param = profile_idc;
        *(param + 1) = level;
        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(param, 2 * sizeof(int));
        enc_flow->Control(VideoEncoder::kProfileChange, pbuff);

        return 0;
    }

    int video_encoder_set_userdata(std::shared_ptr<Flow>& enc_flow, void* data, int len, int all_frames)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        if (!data && len) {
            LOG("ERROR: invalid userdata size!\n");
            return -EINVAL;
        }

        // Param formate: allFrameEnableFlag(8bit) + dataPoint
        uint8_t* param = (uint8_t*)malloc(len + 1);
        *param = all_frames ? 1 : 0;
        if (len) {
            memcpy(param + 1, data, len);
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetPtr(param, len + 1);
        enc_flow->Control(VideoEncoder::kUserDataChange, pbuff);

        return 0;
    }

    int video_encoder_enable_statistics(std::shared_ptr<Flow>& enc_flow, int enable)
    {
        if (!enc_flow) {
            return -EINVAL;
        }

        auto pbuff = std::make_shared<ParameterBuffer>(0);
        pbuff->SetValue(enable);
        enc_flow->Control(VideoEncoder::kEnableStatistics, pbuff);

        return 0;
    }

} // namespace easymedia
