// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>

#include "buffer.h"

#include "media_type.h"
#include "mpp_encoder.h"

#define ENCODER_CFG_INVALID 0x7FFFFFFF
#define ENCODER_CFG_CHECK(VALUE, MIN, MAX, DEF_VALUE, TAG)                                                             \
    if (!VALUE) {                                                                                                      \
        if (DEF_VALUE != ENCODER_CFG_INVALID) {                                                                        \
            LOG("MPP Encoder: %s use default value:%d\n", TAG, DEF_VALUE);                                             \
            VALUE = DEF_VALUE;                                                                                         \
        } else {                                                                                                       \
            LOG("ERROR: MPP Encoder: %s invalid value(%d)\n", TAG, VALUE);                                             \
            return false;                                                                                              \
        }                                                                                                              \
    } else if ((VALUE > MAX) || (VALUE < MIN)) {                                                                       \
        LOG("ERROR: MPP Encoder: %s invalid value(%d)\n", TAG, VALUE);                                                 \
        return false;                                                                                                  \
    }

namespace easymedia
{

#if 0
    static float smart_enc_mode_get_bps_factor(int bps, int w, int h) {
        float den = 1.0;
        //Reference 1080p resolution:
        //5Mb:    [bpsMax / 4,   bpsMax]
        //4Mb:    [bpsMax / 3,   bpsMax]
        //3Mb:    [bpsMax / 2,   bpsMax]
        //Others: [bpsMax * 0.8, bpsMax]
        int relatively_bps = (int)(bps * (2073600.0) / (w * h));
        if(relatively_bps >= 5242880) { //5Mb
            den = 0.25;    //1/4
        } else if(relatively_bps >= 4194304) { //4Mb
            den = 0.333;    //1/3
        } else if(relatively_bps >= 3145728) { //3Mb
            den = 0.5;    //1/2
        } else {
            den = 0.8;
        }

        return den;
    }
#endif

#define MPP_MIN_BPS_VALUE 2000      // 2Kb
#define MPP_MAX_BPS_VALUE 100000000 // 100Mb

    // According to bps_max, automatically calculate bps_target and bps_min.
    static int CalcMppBpsWithMax(MppEncRcMode rc_mode, int& bps_max, int& bps_min, int& bps_target)
    {
        if ((rc_mode != MPP_ENC_RC_MODE_FIXQP) && ((bps_max > MPP_MAX_BPS_VALUE) || (bps_max < MPP_MIN_BPS_VALUE))) {
            LOG("ERROR: MPP Encoder: bps max <%d> is not valid!\n", bps_max);
            return -1;
        }
        LOG("MPP Encoder: automatically calculate bsp with bps_max\n");
        switch (rc_mode) {
            case MPP_ENC_RC_MODE_CBR:
                // constant bitrate has very small bps range of 1/16 bps
                bps_target = bps_max * 16 / 17;
                bps_min = bps_max * 15 / 17;
                break;
            case MPP_ENC_RC_MODE_VBR:
                // variable bitrate has large bps range
                bps_target = bps_max * 16 / 17;
                bps_min = bps_max * 1 / 17;
                break;
            case MPP_ENC_RC_MODE_FIXQP:
                bps_target = bps_min = bps_max;
                return 0; // FIXQP mode bps is invalid!
            default:
                LOG("ERROR: MPP Encoder: rc_mode=%d is invalid!\n", rc_mode);
                return -1;
        }

        if (bps_min < MPP_MIN_BPS_VALUE) {
            bps_min = MPP_MIN_BPS_VALUE;
        }
        if (bps_target < bps_min) {
            bps_target = (bps_min + bps_max) / 2;
        }

        return 0;
    }

    // According to bps_target, automatically calculate bps_max and bps_min.
    static int CalcMppBpsWithTarget(MppEncRcMode rc_mode, int& bps_max, int& bps_min, int& bps_target)
    {
        if ((rc_mode != MPP_ENC_RC_MODE_FIXQP) &&
            ((bps_target > MPP_MAX_BPS_VALUE) || (bps_target < MPP_MIN_BPS_VALUE))) {
            LOG("ERROR: MPP Encoder: bps <%d> is not valid!\n", bps_target);
            return -1;
        }
        LOG("MPP Encoder: automatically calculate bsp with bps_target\n");
        switch (rc_mode) {
            case MPP_ENC_RC_MODE_CBR:
                // constant bitrate has very small bps range of 1/16 bps
                bps_max = bps_target * 17 / 16;
                bps_min = bps_target * 15 / 16;
                break;
            case MPP_ENC_RC_MODE_VBR:
                // variable bitrate has large bps range
                bps_max = bps_target * 17 / 16;
                bps_min = bps_target * 1 / 16;
                break;
            case MPP_ENC_RC_MODE_FIXQP:
                bps_target = bps_min = bps_max;
                return 0; // FIXQP mode bps is invalid!
            default:
                LOG("ERROR: MPP Encoder: rc_mode=%d is invalid!\n", rc_mode);
                return -1;
        }

        if (bps_min < MPP_MIN_BPS_VALUE) {
            bps_min = MPP_MIN_BPS_VALUE;
        }
        if (bps_max > MPP_MAX_BPS_VALUE) {
            bps_max = MPP_MAX_BPS_VALUE;
        }

        return 0;
    }

    class MPPConfig
    {
      public:
        MPPConfig();
        virtual ~MPPConfig();
        virtual bool InitConfig(MPPEncoder& mpp_enc, MediaConfig& cfg) = 0;
        virtual bool CheckConfigChange(MPPEncoder& mpp_enc, uint32_t change, std::shared_ptr<ParameterBuffer> val) = 0;
        MppEncCfg enc_cfg;
    };

    MPPConfig::MPPConfig()
    {
        enc_cfg = NULL;
        if (mpp_enc_cfg_init(&enc_cfg)) {
            LOG("ERROR: MPP Encoder: MPPConfig: cfg init failed!");
            enc_cfg = NULL;
            return;
        }
        LOG("MPP Encoder: MPPConfig: cfg init sucess!\n");
    }

    MPPConfig::~MPPConfig()
    {
        if (enc_cfg) {
            mpp_enc_cfg_deinit(enc_cfg);
            LOG("MPP Encoder: MPPConfig: cfg deinit done!\n");
        }
    }

    class MPPMJPEGConfig : public MPPConfig
    {
      public:
        MPPMJPEGConfig()
        {
        }
        ~MPPMJPEGConfig() = default;
        virtual bool InitConfig(MPPEncoder& mpp_enc, MediaConfig& cfg) override;
        virtual bool CheckConfigChange(MPPEncoder& mpp_enc, uint32_t change,
                                       std::shared_ptr<ParameterBuffer> val) override;
    };

    bool MPPMJPEGConfig::InitConfig(MPPEncoder& mpp_enc, MediaConfig& cfg)
    {
        // for jpeg/mjpeg common.
        ImageConfig img_cfg;
        ImageInfo img_info;
        // for mjpeg.
        VideoConfig vid_cfg;
        int bps_max = 0, bps_min = 0, bps_target = 0;
        int fps_in_num = 0, fps_in_den = 0;
        int fps_out_num = 0, fps_out_den = 0;
        MppEncRcMode rc_mode = MPP_ENC_RC_MODE_FIXQP;
        int ret = 0;

        if (!enc_cfg) {
            LOG("ERROR: MPP Encoder[JPEG]: mpp enc cfg is null!\n");
            return false;
        }

        if (cfg.type == Type::Image) {
            LOG("MPP Encoder[JPEG]: config for JPEG...\n");
            img_cfg = cfg.img_cfg;
            img_info = cfg.img_cfg.image_info;
            ENCODER_CFG_CHECK(img_cfg.qfactor, 1, 99, 70, "qfactor");
        } else if (cfg.type == Type::Video) {
            LOG("MPP Encoder[JPEG]: config for MJPEG...\n");
            vid_cfg = cfg.vid_cfg;
            img_cfg = vid_cfg.image_cfg;
            img_info = cfg.img_cfg.image_info;

            // Encoder param check.
            ENCODER_CFG_CHECK(vid_cfg.frame_rate, 1, 60, ENCODER_CFG_INVALID, "fpsNum");
            ENCODER_CFG_CHECK(vid_cfg.frame_rate_den, 1, 16, 1, "fpsDen");
            ENCODER_CFG_CHECK(vid_cfg.frame_in_rate, 1, 60, ENCODER_CFG_INVALID, "fpsInNum");
            ENCODER_CFG_CHECK(vid_cfg.frame_in_rate_den, 1, 16, 1, "fpsInDen");

            if (!vid_cfg.rc_mode) {
                LOG("MPP Encoder[JPEG]: rcMode use defalut value: vbr\n");
                vid_cfg.rc_mode = KEY_VBR;
            }

            rc_mode = GetMPPRCMode(vid_cfg.rc_mode);
            if (rc_mode == MPP_ENC_RC_MODE_BUTT) {
                LOG("ERROR: MPP Encoder[JPEG]: Invalid rc mode %s\n", vid_cfg.rc_mode);
                return false;
            }
            bps_max = vid_cfg.bit_rate_max;
            bps_min = vid_cfg.bit_rate_min;
            bps_target = vid_cfg.bit_rate;
            fps_in_num = vid_cfg.frame_in_rate;
            fps_in_den = vid_cfg.frame_in_rate_den;
            fps_out_num = vid_cfg.frame_rate;
            fps_out_den = vid_cfg.frame_rate_den;

            if (CalcMppBpsWithMax(rc_mode, bps_max, bps_min, bps_target)) {
                LOG("ERROR: MPP Encoder[JPEG]: Invalid bps:[%d, %d, %d]\n", vid_cfg.bit_rate_min, vid_cfg.bit_rate,
                    vid_cfg.bit_rate_max);
                return false;
            }
        } else {
            LOG("ERROR: MPP Encoder[JPEG]: mpp enc cfg type(%d) is invalid!\n");
            return false;
        }

        ENCODER_CFG_CHECK(img_info.width, 1, 8192, ENCODER_CFG_INVALID, "width");
        ENCODER_CFG_CHECK(img_info.height, 1, 8192, ENCODER_CFG_INVALID, "height");
        ENCODER_CFG_CHECK(img_info.vir_width, 1, 8192, img_info.width, "virWidth");
        ENCODER_CFG_CHECK(img_info.vir_height, 1, 8192, img_info.height, "virHeight");
        MppFrameFormat pic_type = ConvertToMppPixFmt(img_info.pix_fmt);
        if (pic_type == -1) {
            LOG("ERROR: MPP Encoder[JPEG]: invalid pixel format!\n");
            return false;
        }

        MppPollType timeout = MPP_POLL_BLOCK;
        LOG("MPP Encoder[JPEG]: Set output block mode.\n");
        ret = mpp_enc.EncodeControl(MPP_SET_OUTPUT_TIMEOUT, &timeout);
        if (ret != 0) {
            LOG("ERROR: MPP Encoder[JPEG]: set output block failed! ret %d\n", ret);
            return false;
        }
        LOG("MPP Encoder[JPEG]: Set input block mode.\n");
        ret = mpp_enc.EncodeControl(MPP_SET_INPUT_TIMEOUT, &timeout);
        if (ret != 0) {
            LOG("ERROR: MPP Encoder[JPEG]: set input block failed! ret %d\n", ret);
            return false;
        }

        int line_size = img_info.vir_width;
        if (pic_type == MPP_FMT_YUV422_YUYV || pic_type == MPP_FMT_YUV422_UYVY) {
            line_size *= 2;
        }

        // precfg set.
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:width", img_info.width);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:height", img_info.height);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:hor_stride", line_size);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:ver_stride", img_info.vir_height);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:format", pic_type);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:mode", rc_mode); // default fixqp.
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "jpeg:q_factor", img_cfg.qfactor);
        if (cfg.type == Type::Video) {
            LOG("MPP Encoder[JPEG]: set rc cfg for MJPEG...\n");
            // mjpeg rc confg set.
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_flex", 0);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_num", fps_in_num);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_denorm", fps_in_den);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_flex", 0);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_num", fps_out_num);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_denorm", fps_out_den);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", 1); // no use, default 300.
            LOG("MPP Encoder[JPEG]: bps:[%d, %d, %d], fps_in:%d/%d, fps_out:%d/%d, "
                "gop:1\n",
                bps_min, bps_target, bps_max, fps_in_num, fps_in_den, fps_out_num, fps_out_den);
            // for mjpeg only
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "jpeg:qf_max", 99);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "jpeg:qf_min", 10);
        }

        if (ret) {
            LOG("ERROR: MPP Encoder[JPEG]: cfg set s32 failed ret %d\n", ret);
            return false;
        }

        ret = mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg);
        if (ret) {
            LOG("ERROR: MPP Encoder[JPEG]: encoder set cfg failed! ret=%d\n", ret);
            return false;
        }

        LOG("MPP Encoder[JPEG]: w x h(%d[%d] x %d[%d]), qfactor:%d\n", img_info.width, line_size, img_info.height,
            img_info.vir_height, img_cfg.qfactor);

        mpp_enc.GetConfig() = cfg;
        return true;
    }

    bool MPPMJPEGConfig::CheckConfigChange(MPPEncoder& mpp_enc, uint32_t change, std::shared_ptr<ParameterBuffer> val)
    {
        ImageConfig& iconfig = mpp_enc.GetConfig().img_cfg;
        VideoConfig& vconfig = mpp_enc.GetConfig().vid_cfg;
        int ret = 0;

        if (!enc_cfg) {
            LOG("ERROR: MPP Encoder[JPEG]: mpp enc cfg is null!\n");
            return false;
        }

        if (change & VideoEncoder::kFrameRateChange) {
            uint8_t* values = (uint8_t*)val->GetPtr();
            if (val->GetSize() < 4) {
                LOG("ERROR: MPP Encoder[JPEG]: fps should be uint8_t array[4]:"
                    "{inFpsNum, inFpsDen, outFpsNum, outFpsDen}");
                return false;
            }
            uint8_t in_fps_num = values[0];
            uint8_t in_fps_den = values[1];
            uint8_t out_fps_num = values[2];
            uint8_t out_fps_den = values[3];

            if (!out_fps_num || !out_fps_den || (out_fps_num > 60)) {
                LOG("ERROR: MPP Encoder[JPEG]: invalid out fps: [%d/%d]\n", out_fps_num, out_fps_den);
                return false;
            }

            if (in_fps_num && in_fps_den) {
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_num", in_fps_num);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_denorm", in_fps_den);
            }
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_num", out_fps_num);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_denorm", out_fps_den);
            if (ret) {
                LOG("ERROR: MPP Encoder[JPEG]: fps: cfg set s32 failed ret %d\n", ret);
                return false;
            }
            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder[JPEG]: change fps cfg failed!\n");
                return false;
            }
            if (in_fps_num && in_fps_den)
                LOG("MPP Encoder[JPEG]: new fps: [%d/%d]->[%d/%d]\n", in_fps_num, in_fps_den, out_fps_num, out_fps_den);
            else
                LOG("MPP Encoder[JPEG]: new out fps: [%d/%d]\n", out_fps_num, out_fps_den);

            vconfig.frame_rate = out_fps_num;
        } else if (change & VideoEncoder::kBitRateChange) {
            int* values = (int*)val->GetPtr();
            if (val->GetSize() < 3 * sizeof(int)) {
                LOG("ERROR: MPP Encoder[JPEG]: fps should be int array[3]:"
                    "{bpsMin, bpsTarget, bpsMax}");
                return false;
            }
            int bps_min = values[0];
            int bps_target = values[1];
            int bps_max = values[2];
            MppEncRcMode rc_mode = GetMPPRCMode(vconfig.rc_mode);
            if (rc_mode == MPP_ENC_RC_MODE_BUTT) {
                LOG("ERROR: MPP Encoder[JPEG]: bps: invalid rc mode %s\n", vconfig.rc_mode);
                return false;
            } else if (rc_mode == MPP_ENC_RC_MODE_FIXQP) {
                LOG("WARN: MPP Encoder[JPEG]: bps: FIXQP no need bps!\n");
                return true;
            }

            if (CalcMppBpsWithMax(rc_mode, bps_max, bps_min, bps_target)) {
                LOG("ERROR: MPP Encoder[JPEG]: Invalid bps:[%d, %d, %d]\n", bps_min, bps_target, bps_max);
                return false;
            }

            LOG("MPP Encoder[JPEG]: new bps:[%d, %d, %d]\n", bps_min, bps_target, bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
            if (ret) {
                LOG("ERROR: MPP Encoder[JPEG]: bps: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder[JPEG]: change bps cfg failed!\n");
                return false;
            }
            // save new value to config.
            vconfig.bit_rate = bps_target;
            vconfig.bit_rate_max = bps_max;
            vconfig.bit_rate_min = bps_min;
        } else if (change & VideoEncoder::kRcModeChange) {
            char* new_mode = (char*)val->GetPtr();
            LOG("MPP Encoder[JPEG]: new rc_mode:%s\n", new_mode);
            MppEncRcMode rc_mode = GetMPPRCMode(new_mode);
            if (rc_mode == MPP_ENC_RC_MODE_BUTT) {
                LOG("ERROR: MPP Encoder[JPEG]: rc_mode is invalid! should be cbr/vbr.\n");
                return false;
            }

            // Recalculate bps
            int bps_max = vconfig.bit_rate_max;
            int bps_min = bps_max;
            int bps_target = bps_max;
            if (CalcMppBpsWithMax(rc_mode, bps_max, bps_min, bps_target) < 0) {
                return false;
            }

            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:mode", rc_mode);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
            if (ret) {
                LOG("ERROR: MPP Encoder[JPEG]: rc mode: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder[JPEG]: change rc_mode cfg failed!\n");
                return false;
            }
            // save new value to encoder->vconfig.
            vconfig.rc_mode = ConvertRcMode(new_mode);
        } else if (change & VideoEncoder::kQPChange) {
            int qfactor = val->GetValue();
            LOG("MPP Encoder[JPEG]: new qfactor:%d\n", qfactor);
            ENCODER_CFG_CHECK(qfactor, 1, 99, ENCODER_CFG_INVALID, "JPEG:qfactor");
            ret = mpp_enc_cfg_set_s32(enc_cfg, "jpeg:q_factor", qfactor);
            if (ret) {
                LOG("ERROR: MPP Encoder[JPEG]: cfg set s32 failed! ret=%d\n", ret);
                return false;
            }

            ret = mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg);
            if (ret) {
                LOG("ERROR: MPP Encoder[JPEG]: qfactor: set cfg failed! ret=%d\n", ret);
                return false;
            }
            iconfig.qfactor = qfactor;
        } else {
            LOG("MPP Encoder[JPEG]: Unsupport request change 0x%08x!\n", change);
            return false;
        }

        return true;
    }

    class MPPCommonConfig : public MPPConfig
    {
      public:
        MPPCommonConfig(MppCodingType type) : code_type(type)
        {
        }
        ~MPPCommonConfig() = default;
        virtual bool InitConfig(MPPEncoder& mpp_enc, MediaConfig& cfg) override;
        virtual bool CheckConfigChange(MPPEncoder& mpp_enc, uint32_t change,
                                       std::shared_ptr<ParameterBuffer> val) override;

      private:
        MppCodingType code_type;
    };

    static int CalcQpWithRcQuality(const char* level, VideoEncoderQp& qps)
    {
        VideoEncoderQp qp_table[7] = {
            {-1, 6, 20, 51, 24, 51}, // Highest
            {-1, 6, 24, 51, 24, 51}, // Higher
            {-1, 6, 26, 51, 24, 51}, // High
            {-1, 6, 29, 51, 24, 51}, // Medium
            {-1, 6, 30, 51, 24, 51}, // Low
            {-1, 6, 35, 51, 24, 51}, // Lower
            {-1, 6, 38, 51, 24, 51}  // Lowest
        };

        // Read From qp cfg file.
        // To Do...

        if (!strcmp(KEY_HIGHEST, level)) {
            qps = qp_table[0];
        } else if (!strcmp(KEY_HIGHER, level)) {
            qps = qp_table[1];
        } else if (!strcmp(KEY_HIGH, level)) {
            qps = qp_table[2];
        } else if (!strcmp(KEY_MEDIUM, level)) {
            qps = qp_table[3];
        } else if (!strcmp(KEY_LOW, level)) {
            qps = qp_table[4];
        } else if (!strcmp(KEY_LOWER, level)) {
            qps = qp_table[5];
        } else if (!strcmp(KEY_LOWEST, level)) {
            qps = qp_table[6];
        } else {
            LOG("ERROR: MPP Encoder: invalid rcQualit:%s\n", level);
            return -1;
        }

        return 0;
    }

    bool MPPCommonConfig::InitConfig(MPPEncoder& mpp_enc, MediaConfig& cfg)
    {
        VideoConfig vconfig = cfg.vid_cfg;
        ImageConfig& img_cfg = vconfig.image_cfg;
        ImageInfo& image_info = cfg.img_cfg.image_info;
        MppPollType timeout = MPP_POLL_BLOCK;
        MppFrameFormat pic_type = ConvertToMppPixFmt(image_info.pix_fmt);
        int ret = 0;

        if (!enc_cfg) {
            LOG("ERROR: MPP Encoder: mpp enc cfg is null!\n");
            return false;
        }

        // In VBR mode, and the user has not set qp,
        // at this time, the qp value is obtained according to RcQuality.
        if (vconfig.rc_mode && vconfig.rc_quality && (!strcmp(vconfig.rc_mode, KEY_VBR)) &&
            (!vconfig.qp_max || !vconfig.qp_min)) {
            VideoEncoderQp qps;
            if (CalcQpWithRcQuality(vconfig.rc_quality, qps)) {
                return false;
            }
            LOG("MPP Encoder: [%s:%s] init:%d, setp:%d, min:%d, "
                "max:%d, min_i:%d, max_i:%d\n",
                vconfig.rc_mode, vconfig.rc_quality, qps.qp_init, qps.qp_step, qps.qp_min, qps.qp_max, qps.qp_min_i,
                qps.qp_max_i);
            vconfig.qp_init = qps.qp_init;
            vconfig.qp_step = qps.qp_step;
            vconfig.qp_min = qps.qp_min;
            vconfig.qp_max = qps.qp_max;
            vconfig.qp_min_i = qps.qp_min_i;
            vconfig.qp_max_i = qps.qp_max_i;
        }

        // Encoder param check.
        ENCODER_CFG_CHECK(vconfig.frame_rate, 1, 60, ENCODER_CFG_INVALID, "fpsNum");
        ENCODER_CFG_CHECK(vconfig.frame_rate_den, 1, 16, 1, "fpsDen");
        ENCODER_CFG_CHECK(vconfig.frame_in_rate, 1, 60, ENCODER_CFG_INVALID, "fpsInNum");
        ENCODER_CFG_CHECK(vconfig.frame_in_rate_den, 1, 16, 1, "fpsInDen");
        ENCODER_CFG_CHECK(vconfig.gop_size, 1, 3000, (vconfig.frame_rate > 10) ? vconfig.frame_rate : 30, "gopSize");
        ENCODER_CFG_CHECK(vconfig.qp_max, 8, 51, 48, "qpMax");
        ENCODER_CFG_CHECK(vconfig.qp_min, 1, VALUE_MIN(vconfig.qp_max, 48), VALUE_MIN(vconfig.qp_max, 8), "qpMin");
        // qp_init = -1: mpp encoder automatically generate
        // a value for qp_init.
        if (vconfig.qp_init != -1) {
            ENCODER_CFG_CHECK(vconfig.qp_init, vconfig.qp_min, vconfig.qp_max, (vconfig.qp_min + vconfig.qp_max) / 2,
                              "qpInit");
        }
        ENCODER_CFG_CHECK(vconfig.qp_step, 1, (vconfig.qp_max - vconfig.qp_min),
                          VALUE_MIN((vconfig.qp_max - vconfig.qp_min), 2), "qpStep");
        ENCODER_CFG_CHECK(img_cfg.image_info.width, 1, 8192, ENCODER_CFG_INVALID, "width");
        ENCODER_CFG_CHECK(img_cfg.image_info.height, 1, 8192, ENCODER_CFG_INVALID, "height");
        ENCODER_CFG_CHECK(img_cfg.image_info.vir_width, 1, 8192, img_cfg.image_info.width, "virWidth");
        ENCODER_CFG_CHECK(img_cfg.image_info.vir_height, 1, 8192, img_cfg.image_info.height, "virHeight");

        ENCODER_CFG_CHECK(vconfig.qp_max_i, 8, 51, 48, "qpMaxi");
        ENCODER_CFG_CHECK(vconfig.qp_min_i, 1, VALUE_MIN(vconfig.qp_max_i, 48), VALUE_MIN(vconfig.qp_max_i, 8),
                          "qpMini");

        if (pic_type == -1) {
            LOG("error input pixel format\n");
            return false;
        }

        if (!vconfig.rc_mode) {
            LOG("MPP Encoder: rcMode use defalut value: vbr\n");
            vconfig.rc_mode = KEY_VBR;
        }

        MppEncRcMode rc_mode = GetMPPRCMode(vconfig.rc_mode);
        if (rc_mode == MPP_ENC_RC_MODE_BUTT) {
            LOG("ERROR: MPP Encoder: Invalid rc mode %s\n", vconfig.rc_mode);
            return false;
        }
        int bps_max = vconfig.bit_rate_max;
        int bps_min = vconfig.bit_rate_min;
        int bps_target = vconfig.bit_rate;
        int fps_in_num = vconfig.frame_in_rate;
        int fps_in_den = vconfig.frame_in_rate_den;
        int fps_out_num = vconfig.frame_rate;
        int fps_out_den = vconfig.frame_rate_den;
        int gop = vconfig.gop_size;
        int full_range = vconfig.full_range;
        MppEncRotationCfg rotation = MPP_ENC_ROT_0;

        switch (vconfig.rotation) {
            case 0:
                rotation = MPP_ENC_ROT_0;
                LOG("MPP Encoder: rotaion = 0\n");
                break;
            case 90:
                LOG("MPP Encoder: rotaion = 90\n");
                rotation = MPP_ENC_ROT_90;
                break;
            case 180:
                LOG("MPP Encoder: rotaion = 180\n");
                rotation = MPP_ENC_ROT_180;
                break;
            case 270:
                LOG("MPP Encoder: rotaion = 270\n");
                rotation = MPP_ENC_ROT_270;
                break;
            default:
                LOG("ERROR: MPP Encoder: rotaion(%d) is invalid!\n", vconfig.rotation);
                return false;
        }

        // encoder not support fbc rotation.
        if ((image_info.pix_fmt == PIX_FMT_FBC0) || (image_info.pix_fmt == PIX_FMT_FBC2)) {
            if ((rotation == MPP_ENC_ROT_90) || (rotation == MPP_ENC_ROT_270)) {
                int tmp_value = image_info.width;
                image_info.width = image_info.height;
                image_info.height = tmp_value;
                tmp_value = image_info.vir_width;
                image_info.vir_width = image_info.vir_height;
                image_info.vir_height = tmp_value;
                LOG("MPP Encoder: rotation in fbc mode, Resolution "
                    "from %d(%d)x%d(%d) to %d(%d)x%d(%d)\n",
                    image_info.height, image_info.vir_height, image_info.width, image_info.vir_width, image_info.width,
                    image_info.vir_width, image_info.height, image_info.vir_height);
            }
            rotation = MPP_ENC_ROT_0;
        }

        // Three bit rate configuration methods:
        //  1. Straight-through mode: all three code rate values must be valid.
        //  2. Only bps_max: Generate three values based on bps_max.
        //  3. Only bps_target: Generate three values based on bps_target.
        if (bps_max && bps_min && bps_target) {
            if ((bps_max < MPP_MIN_BPS_VALUE) || (bps_max > MPP_MAX_BPS_VALUE) || (bps_min < MPP_MIN_BPS_VALUE) ||
                (bps_min > MPP_MAX_BPS_VALUE) || (bps_target < bps_min) || (bps_target > bps_max)) {
                ret = -1;
            }
        } else if (bps_max && !bps_target && !bps_min) {
            ret = CalcMppBpsWithMax(rc_mode, bps_max, bps_min, bps_target);
        } else if (bps_target && !bps_max && !bps_min) {
            ret = CalcMppBpsWithTarget(rc_mode, bps_max, bps_min, bps_target);
        } else {
            ret = -1;
        }
        if (ret < 0) {
            LOG("ERROR: MPP Encoder: Invalid bps:[%d, %d, %d]\n", vconfig.bit_rate_min, vconfig.bit_rate,
                vconfig.bit_rate_max);
            return false;
        }

        int line_size = image_info.vir_width;
        if (pic_type == MPP_FMT_YUV422_YUYV || pic_type == MPP_FMT_YUV422_UYVY) {
            line_size *= 2;
        }

        LOG("MPP Encoder: Set output block mode.\n");
        ret = mpp_enc.EncodeControl(MPP_SET_OUTPUT_TIMEOUT, &timeout);
        if (ret != 0) {
            LOG("ERROR: MPP Encoder: set output block failed ret %d\n", ret);
            return false;
        }
        LOG("MPP Encoder: Set input block mode.\n");
        ret = mpp_enc.EncodeControl(MPP_SET_INPUT_TIMEOUT, &timeout);
        if (ret != 0) {
            LOG("ERROR: MPP Encoder: set input block failed ret %d\n", ret);
            return false;
        }

        // precfg set.
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:width", image_info.width);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:height", image_info.height);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:hor_stride", line_size);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:ver_stride", image_info.vir_height);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:format", pic_type);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:rotation", rotation);
        if (full_range) {
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "prep:range", MPP_FRAME_RANGE_JPEG);
        }

        // rccfg set.
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:mode", rc_mode);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_flex", 0);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_num", fps_in_num);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_denorm", fps_in_den);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_flex", 0);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_num", fps_out_num);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_denorm", fps_out_den);
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", gop);

        vconfig.frame_rate = fps_in_num;
        LOG("MPP Encoder: bps:[%d,%d,%d] fps: [%d/%d]->[%d/%d], gop:%d\n", bps_max, bps_target, bps_min, fps_in_num,
            fps_in_den, fps_out_num, fps_out_den, gop);

        // codeccfg set.
        ret |= mpp_enc_cfg_set_s32(enc_cfg, "codec:type", code_type);
        switch (code_type) {
            case MPP_VIDEO_CodingAVC:
                // H.264 profile_idc parameter
                // 66  - Baseline profile
                // 77  - Main profile
                // 100 - High profile
                if (vconfig.profile != 66 && vconfig.profile != 77) {
                    LOG("MPP Encoder: H264 profile use defalut value: 100");
                    vconfig.profile = 100; // default PROFILE_HIGH 100
                }
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:profile", vconfig.profile);

                // H.264 level_idc parameter
                // 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
                // 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
                // 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
                // 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
                // 50 / 51 / 52         - 4K@30fps
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:level", vconfig.level);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:cabac_en", (vconfig.profile == 100) ? 1 : 0);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:cabac_idc", 0);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:trans8x8",
                                           (vconfig.trans_8x8 && (vconfig.profile == 100)) ? 1 : 0);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_init", vconfig.qp_init);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_max", vconfig.qp_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_min", vconfig.qp_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_step", vconfig.qp_step);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_max_i", vconfig.qp_max_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_min_i", vconfig.qp_min_i);
                LOG("MPP Encoder: AVC: encode profile %d level %d init_qp %d\n", vconfig.profile, vconfig.level,
                    vconfig.qp_init);
                break;
            case MPP_VIDEO_CodingHEVC:
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_init", vconfig.qp_init);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_max", vconfig.qp_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_min", vconfig.qp_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_step", vconfig.qp_step);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_max_i", vconfig.qp_max_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_min_i", vconfig.qp_min_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_delta_ip", 3);
                break;
            default:
                // will never go here, avoid gcc warning
                return false;
        }

        if (ret) {
            LOG("ERROR: MPP Encoder: cfg set s32 failed ret %d\n", ret);
            return false;
        }

        ret = mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg);
        if (ret) {
            LOG("ERROR: MPP Encoder: set cfg failed ret %d\n", ret);
            return false;
        }

        // save bps to vconfig.
        vconfig.bit_rate_max = bps_max;
        vconfig.bit_rate_min = bps_min;
        vconfig.bit_rate = bps_target;

        LOG("MPP Encoder: w x h(%d[%d] x %d[%d])\n", image_info.width, line_size, image_info.height,
            image_info.vir_height);

#if 0
        MppPacket packet = nullptr;
        ret = mpp_enc.EncodeControl(MPP_ENC_GET_EXTRA_INFO, &packet);
        if(ret) {
            LOG("ERROR: MPP Encoder: get extra info failed\n");
            return false;
        }

        // Get and write sps/pps for H.264/5
        if(packet) {
            void* ptr = mpp_packet_get_pos(packet);
            size_t len = mpp_packet_get_length(packet);
            if(!mpp_enc.SetExtraData(ptr, len)) {
                LOG("ERROR: MPP Encoder: SetExtraData failed\n");
                return false;
            }
            mpp_enc.GetExtraData()->SetUserFlag(MediaBuffer::kExtraIntra);
            packet = NULL;
        }
#endif

        if (vconfig.ref_frm_cfg) {
            MppEncRefCfg ref = NULL;

            LOG("MPP Encoder: enable tsvc mode...\n");
            if (mpp_enc_ref_cfg_init(&ref)) {
                LOG("ERROR: MPP Encoder: ref cfg init failed!\n");
                return false;
            }
            if (mpi_enc_gen_ref_cfg(ref)) {
                LOG("ERROR: MPP Encoder: ref cfg gen failed!\n");
                mpp_enc_ref_cfg_deinit(&ref);
                return false;
            }
            ret = mpp_enc.EncodeControl(MPP_ENC_SET_REF_CFG, ref);
            mpp_enc_ref_cfg_deinit(&ref);
        }

        int header_mode = MPP_ENC_HEADER_MODE_EACH_IDR;
        ret = mpp_enc.EncodeControl(MPP_ENC_SET_HEADER_MODE, &header_mode);
        if (ret) {
            LOG("ERROR: MPP Encoder: set heder mode failed ret %d\n", ret);
            return false;
        }

        mpp_enc.GetConfig().vid_cfg = vconfig;
        mpp_enc.GetConfig().type = Type::Video;
        return true;
    }

    bool MPPCommonConfig::CheckConfigChange(MPPEncoder& mpp_enc, uint32_t change, std::shared_ptr<ParameterBuffer> val)
    {
        VideoConfig& vconfig = mpp_enc.GetConfig().vid_cfg;
        int ret = 0;

        if (!enc_cfg) {
            LOG("ERROR: MPP Encoder: mpp enc cfg is null!\n");
            return false;
        }

        if (change & VideoEncoder::kFrameRateChange) {
            uint8_t* values = (uint8_t*)val->GetPtr();
            if (val->GetSize() < 4) {
                LOG("ERROR: MPP Encoder: fps should be uint8_t array[4]:"
                    "{inFpsNum, inFpsDen, outFpsNum, outFpsDen}");
                return false;
            }
            uint8_t in_fps_num = values[0];
            uint8_t in_fps_den = values[1];
            uint8_t out_fps_num = values[2];
            uint8_t out_fps_den = values[3];

            if (!out_fps_num || !out_fps_den || (out_fps_num > 60)) {
                LOG("ERROR: MPP Encoder: invalid out fps: [%d/%d]\n", out_fps_num, out_fps_den);
                return false;
            }

            if (in_fps_num && in_fps_den) {
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_num", in_fps_num);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_in_denorm", in_fps_den);
            }
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_num", out_fps_num);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:fps_out_denorm", out_fps_den);
            if (ret) {
                LOG("ERROR: MPP Encoder: fps: cfg set s32 failed ret %d\n", ret);
                return false;
            }
            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change fps cfg failed!\n");
                return false;
            }
            if (in_fps_num && in_fps_den) {
                LOG("MPP Encoder: new fps: [%d/%d]->[%d/%d]\n", in_fps_num, in_fps_den, out_fps_num, out_fps_den);
            } else {
                LOG("MPP Encoder: new out fps: [%d/%d]\n", out_fps_num, out_fps_den);
            }

            vconfig.frame_rate = out_fps_num;
        } else if (change & VideoEncoder::kBitRateChange) {
            int* values = (int*)val->GetPtr();
            if (val->GetSize() < 3 * sizeof(int)) {
                LOG("ERROR: MPP Encoder: fps should be int array[3]:"
                    "{bpsMin, bpsTarget, bpsMax}");
                return false;
            }
            int bps_min = values[0];
            int bps_target = values[1];
            int bps_max = values[2];
            MppEncRcMode rc_mode = GetMPPRCMode(vconfig.rc_mode);
            if (rc_mode == MPP_ENC_RC_MODE_BUTT) {
                LOG("ERROR: MPP Encoder: bps: invalid rc mode %s\n", vconfig.rc_mode);
                return false;
            }

            // Three bit rate configuration methods:
            //  1. Straight-through mode: all three code rate values must be valid.
            //  2. Only bps_max: Generate three values based on bps_max.
            //  3. Only bps_target: Generate three values based on bps_target.
            if (bps_max && bps_min && bps_target) {
                if ((bps_max > MPP_MAX_BPS_VALUE) || (bps_max < MPP_MIN_BPS_VALUE) || (bps_min > MPP_MAX_BPS_VALUE) ||
                    (bps_min < MPP_MIN_BPS_VALUE) || (bps_target > bps_max) || (bps_target < bps_min)) {
                    ret = -1;
                }
            } else if (bps_max && !bps_target && !bps_min) {
                ret = CalcMppBpsWithMax(rc_mode, bps_max, bps_min, bps_target);
            } else if (bps_target && !bps_max && !bps_min) {
                ret = CalcMppBpsWithTarget(rc_mode, bps_max, bps_min, bps_target);
            } else {
                ret = -1;
            }
            if (ret < 0) {
                LOG("ERROR: MPP Encoder: Invalid bps:[%d, %d, %d]\n", bps_min, bps_target, bps_max);
                return false;
            }

            ret = 0;
            LOG("MPP Encoder: new bps:[%d, %d, %d]\n", bps_min, bps_target, bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
            if (ret) {
                LOG("ERROR: MPP Encoder: bps: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change bps cfg failed!\n");
                return false;
            }
            // save new value to config.
            vconfig.bit_rate = bps_target;
            vconfig.bit_rate_max = bps_max;
            vconfig.bit_rate_min = bps_min;
        } else if (change & VideoEncoder::kRcModeChange) {
            char* new_mode = (char*)val->GetPtr();
            LOG("MPP Encoder: new rc_mode:%s\n", new_mode);
            MppEncRcMode rc_mode = GetMPPRCMode(new_mode);
            if (rc_mode == MPP_ENC_RC_MODE_BUTT) {
                LOG("ERROR: MPP Encoder: rc_mode is invalid! should be cbr/vbr.\n");
                return false;
            }

            // Recalculate bps
            int bps_max = vconfig.bit_rate_max;
            int bps_min = bps_max;
            int bps_target = bps_max;
            if (CalcMppBpsWithMax(rc_mode, bps_max, bps_min, bps_target) < 0) {
                return false;
            }

            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:mode", rc_mode);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
            if (ret) {
                LOG("ERROR: MPP Encoder: rc mode: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change rc_mode cfg failed!\n");
                return false;
            }
            // save new value to encoder->vconfig.
            vconfig.rc_mode = ConvertRcMode(new_mode);
            ;
        } else if (change & VideoEncoder::kProfileChange) {
            if (val->GetSize() < 2 * sizeof(int)) {
                LOG("ERROR: MPP Encoder: fps should be int array[2]:"
                    "{profile_idc, level}");
                return false;
            }
            int profile_idc = *((int*)val->GetPtr());
            int level = *((int*)val->GetPtr() + 1);
            LOG("MPP Encoder: new profile_idc:%d, level:%d\n", profile_idc, level);

            if (vconfig.image_cfg.codec_type != CODEC_TYPE_H264) {
                LOG("ERROR: MPP Encoder: Current codec:%d not support Profile change.\n", vconfig.image_cfg.codec_type);
                return false;
            }

            // H.264 profile_idc parameter
            // 66  - Baseline profile
            // 77  - Main profile
            // 100 - High profile
            if ((profile_idc != 66) && (profile_idc != 77) && (profile_idc != 100)) {
                LOG("MPP Encoder: Invalid H264 profile(%d)\n", profile_idc);
                return false;
            }
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:profile", profile_idc);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:cabac_en", (profile_idc == 100) ? 1 : 0);

            // H.264 level_idc parameter
            // 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
            // 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
            // 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
            // 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
            // 50 / 51 / 52         - 4K@30fps
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:level", level);

            if (ret) {
                LOG("ERROR: MPP Encoder: profile: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change rc_mode cfg failed!\n");
                return false;
            }
            // save new value to encoder->vconfig.
            vconfig.profile = profile_idc;
            vconfig.level = level;
        } else if (change & VideoEncoder::kRcQualityChange) {
            char* rc_quality = (char*)val->GetPtr();
            VideoEncoderQp qps;

            if (strcmp(vconfig.rc_mode, KEY_VBR)) {
                LOG("ERROR: MPP Encoder: only vbr mode support rcQuality changes!\n");
                return false;
            }

            if (CalcQpWithRcQuality(rc_quality, qps)) {
                return false;
            }

            LOG("MPP Encoder: [%s:%s->%s] init:%d, setp:%d, min:%d, "
                "max:%d, min_i:%d, max_i:%d\n",
                vconfig.rc_mode, vconfig.rc_quality, rc_quality, qps.qp_init, qps.qp_step, qps.qp_min, qps.qp_max,
                qps.qp_min_i, qps.qp_max_i);

            if (code_type == MPP_VIDEO_CodingAVC) {
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_init", qps.qp_init);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_max", qps.qp_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_min", qps.qp_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_step", qps.qp_step);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_max_i", qps.qp_max_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_min_i", qps.qp_min_i);
            } else if (code_type == MPP_VIDEO_CodingHEVC) {
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_init", qps.qp_init);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_max", qps.qp_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_min", qps.qp_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_step", qps.qp_step);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_max_i", qps.qp_max_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_min_i", qps.qp_min_i);
                // when qp changes qp_delta_ip will reset 0,
                // so we should set default 3 again.
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_delta_ip", 3);
            }
            if (ret) {
                LOG("ERROR: MPP Encoder: qp: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change qp cfg failed!\n");
                return false;
            }
            vconfig.qp_init = qps.qp_init;
            vconfig.qp_min = qps.qp_min;
            vconfig.qp_max = qps.qp_max;
            vconfig.qp_step = qps.qp_step;
            vconfig.qp_max_i = qps.qp_max_i;
            vconfig.qp_min_i = qps.qp_min_i;
            vconfig.rc_quality = ConvertRcQuality(rc_quality);
        } else if (change & VideoEncoder::kGopChange) {
            int new_gop_size = val->GetValue();
            if (new_gop_size < 0) {
                LOG("ERROR: MPP Encoder: gop size invalid!\n");
                return false;
            }
            LOG("MPP Encoder: gop change frome %d to %d\n", vconfig.gop_size, new_gop_size);
            ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", new_gop_size);
            if (ret) {
                LOG("ERROR: MPP Encoder: gop: cfg set s32 failed ret %d\n", ret);
                return false;
            }
            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change gop cfg failed!\n");
                return false;
            }
            // save to vconfig
            vconfig.gop_size = new_gop_size;
            // gop change restart userata status
            mpp_enc.RestartUserData();
        } else if (change & VideoEncoder::kQPChange) {
            VideoEncoderQp* qps = (VideoEncoderQp*)val->GetPtr();
            if (val->GetSize() < sizeof(VideoEncoderQp)) {
                LOG("ERROR: MPP Encoder: Incomplete VideoEncoderQp information\n");
                return false;
            }
            LOG("MPP Encoder: new qp:[%d, %d, %d, %d, %d, %d]\n", qps->qp_init, qps->qp_step, qps->qp_min, qps->qp_max,
                qps->qp_min_i, qps->qp_max_i);

            if (code_type == MPP_VIDEO_CodingAVC) {
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_init", qps->qp_init);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_max", qps->qp_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_min", qps->qp_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_step", qps->qp_step);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_max_i", qps->qp_max_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_min_i", qps->qp_min_i);
            } else if (code_type == MPP_VIDEO_CodingHEVC) {
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_init", qps->qp_init);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_max", qps->qp_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_min", qps->qp_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_step", qps->qp_step);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_max_i", qps->qp_max_i);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_min_i", qps->qp_min_i);
                // when qp changes qp_delta_ip will reset 0,
                // so we should set default 3 again.
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_delta_ip", 3);
            }
            if (ret) {
                LOG("ERROR: MPP Encoder: qp: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: change qp cfg failed!\n");
                return false;
            }
            vconfig.qp_init = qps->qp_init;
            vconfig.qp_min = qps->qp_min;
            vconfig.qp_max = qps->qp_max;
            vconfig.qp_step = qps->qp_step;
            vconfig.qp_max_i = qps->qp_max_i;
            vconfig.qp_min_i = qps->qp_min_i;
        } else if (change & VideoEncoder::kROICfgChange) {
            EncROIRegion* regions = (EncROIRegion*)val->GetPtr();
            if (val->GetSize() && (val->GetSize() < sizeof(EncROIRegion))) {
                LOG("ERROR: MPP Encoder: ParameterBuffer size is invalid!\n");
                return false;
            }
            int region_cnt = val->GetSize() / sizeof(EncROIRegion);
            mpp_enc.RoiUpdateRegions(regions, region_cnt);
        } else if (change & VideoEncoder::kForceIdrFrame) {
            LOG("MPP Encoder: force idr frame...\n");
            if (mpp_enc.EncodeControl(MPP_ENC_SET_IDR_FRAME, nullptr) != 0) {
                LOG("ERROR: MPP Encoder: force idr frame control failed!\n");
                return false;
            }
            // force idr frame, restart userata status
            mpp_enc.RestartUserData();
        } else if (change & VideoEncoder::kSplitChange) {
            if (val->GetSize() < (2 * sizeof(int))) {
                LOG("ERROR: MPP Encoder: Incomplete split information\n");
                return false;
            }
            RK_U32 split_mode = *((unsigned int*)val->GetPtr());
            RK_U32 split_arg = *((unsigned int*)val->GetPtr() + 1);

            LOG("MPP Encoder: split_mode:%u, split_arg:%u\n", split_mode, split_arg);
            ret |= mpp_enc_cfg_set_u32(enc_cfg, "split:mode", split_mode);
            ret |= mpp_enc_cfg_set_u32(enc_cfg, "split:arg", split_arg);
            if (ret) {
                LOG("ERROR: MPP Encoder: split: cfg set s32 failed ret %d\n", ret);
                return false;
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                LOG("ERROR: MPP Encoder: set split mode failed!\n");
                return false;
            }
        } else if (change & VideoEncoder::kGopModeChange) {
            if (val->GetSize() < sizeof(EncGopModeParam)) {
                LOG("ERROR: MPP Encoder: Incomplete gop mode params\n");
                return false;
            }
            EncGopModeParam* gop_param = (EncGopModeParam*)val->GetPtr();
            EncGopMode gop_mode = gop_param->mode;
            MppEncRefCfg ref = NULL;

            switch (gop_mode) {
                case GOP_MODE_TSVC2:
                case GOP_MODE_TSVC3:
                case GOP_MODE_TSVC4: {
                    LOG("MPP Encoder: Set GopMode to \"TSVC\" mode...\n");
                    if (mpp_enc_ref_cfg_init(&ref)) {
                        LOG("ERROR: MPP Encoder: ref cfg init failed!\n");
                        return false;
                    }
                    if (mpi_enc_gen_ref_cfg(ref, gop_mode)) {
                        LOG("ERROR: MPP Encoder: ref cfg gen failed!\n");
                        mpp_enc_ref_cfg_deinit(&ref);
                        return false;
                    }
                    ret = mpp_enc.EncodeControl(MPP_ENC_SET_REF_CFG, ref);
                    mpp_enc_ref_cfg_deinit(&ref);
                } break;
                case GOP_MODE_SMARTP: {
                    /*********************************************************
                     * Set Gop Mode
                     * *******************************************************/
                    // virtual intra frame gap.
                    int smartp_vi_len = gop_param->gop_size;
                    int smartp_gop_len = gop_param->interval;
                    int smartp_qp_delta = gop_param->ip_qp_delta;

                    LOG("MPP Encoder: Set GopMode to \"SMARTP\" mode. "
                        "gop_size:%d, interval:%d, ip_qp_delta:%d...\n",
                        gop_param->gop_size, gop_param->interval, gop_param->ip_qp_delta);

                    if (mpp_enc_ref_cfg_init(&ref)) {
                        LOG("ERROR: MPP Encoder: ref cfg init failed!\n");
                        return false;
                    }
                    if (mpi_enc_gen_ref_cfg(ref, gop_mode, smartp_gop_len, smartp_vi_len)) {
                        LOG("ERROR: MPP Encoder: ref cfg gen failed!\n");
                        mpp_enc_ref_cfg_deinit(&ref);
                        return false;
                    }
                    ret = mpp_enc.EncodeControl(MPP_ENC_SET_REF_CFG, ref);
                    mpp_enc_ref_cfg_deinit(&ref);

                    /*********************************************************
                     * Set Gop size
                     * *******************************************************/
                    ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", smartp_gop_len);
                    if (ret) {
                        LOG("ERROR: MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
                        return false;
                    }

                    /*********************************************************
                     * Set qp delta
                     * *******************************************************/
                    if (vconfig.image_cfg.codec_type == CODEC_TYPE_H265) {
                        ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_delta_ip", smartp_qp_delta);
                        if (ret) {
                            LOG("ERROR: MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
                            return false;
                        }
                    } else if (vconfig.image_cfg.codec_type == CODEC_TYPE_H264) {
                        ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_delta_ip", smartp_qp_delta);
                        if (ret) {
                            LOG("ERROR: MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
                            return false;
                        }
                    }

                    /*********************************************************
                     * Set vi gop size
                     * *******************************************************/
                    // ToDo...

                    if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                        LOG("ERROR: MPP Encoder: change gop cfg failed!\n");
                        return false;
                    }
                } break;
                case GOP_MODE_NORMALP: {
                    LOG("MPP Encoder: Set GopMode to \"NORMALP\" mode...\n");
                    ret = mpp_enc.EncodeControl(MPP_ENC_SET_REF_CFG, NULL);

                    /*********************************************************
                     * Reset Gop size
                     * *******************************************************/
                    ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", vconfig.gop_size);
                    if (ret) {
                        LOG("ERROR: MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
                        return false;
                    }
                    /*********************************************************
                     * Set qp delta
                     * *******************************************************/
                    int normal_qp_delta = gop_param->ip_qp_delta;
                    if (vconfig.image_cfg.codec_type == CODEC_TYPE_H265) {
                        ret |= mpp_enc_cfg_set_s32(enc_cfg, "h265:qp_delta_ip", normal_qp_delta);
                        if (ret) {
                            LOG("ERROR: MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
                            return false;
                        }
                    } else if (vconfig.image_cfg.codec_type == CODEC_TYPE_H264) {
                        ret |= mpp_enc_cfg_set_s32(enc_cfg, "h264:qp_delta_ip", normal_qp_delta);
                        if (ret) {
                            LOG("ERROR: MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
                            return false;
                        }
                    }

                    if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                        LOG("ERROR: MPP Encoder: change gop cfg failed!\n");
                        return false;
                    }
                } break;
                default:
                    LOG("ERROR: MPP Encoder: gop mode: unsupport mode value(%d)!\n", gop_mode);
                    return false;
            }

            if (ret) {
                LOG("ERROR: MPP Encoder: set gop mode failed!\n");
                return false;
            }
        }
#ifdef MPP_SUPPORT_HW_OSD
        else if (change & VideoEncoder::kOSDDataChange) {
            // type: OsdRegionData*
            LOGD("MPP Encoder: config osd regions\n");
            if (val->GetSize() < sizeof(OsdRegionData)) {
                LOG("ERROR: MPP Encoder: palette buff should be OsdRegionData type\n");
                return false;
            }
            OsdRegionData* param = (OsdRegionData*)val->GetPtr();
            if (mpp_enc.OsdRegionSet(param)) {
                LOG("ERROR: MPP Encoder: set osd regions error!\n");
                return false;
            }
        } else if (change & VideoEncoder::kOSDPltChange) {
            // type: 265 * U32 array.
            LOG("MPP Encoder: config osd palette\n");
            if (val->GetSize() < (sizeof(int) * 4)) {
                LOG("ERROR: MPP Encoder: palette buff should be U32 * 256\n");
                return false;
            }
            uint32_t* param = (uint32_t*)val->GetPtr();
            if (mpp_enc.OsdPaletteSet(param)) {
                LOG("ERROR: MPP Encoder: set Palette error!\n");
                return false;
            }
        }
#endif
        else if (change & VideoEncoder::kUserDataChange) {
            // type: OsdRegionData*
            LOGD("MPP Encoder: config userdata\n");
            if (val->GetSize() <= 0) {
                LOG("ERROR: MPP Encoder: invalid userdata size\n");
                return false;
            }
            uint8_t enable_all_frames = *(uint8_t*)val->GetPtr();
            mpp_enc.EnableUserDataAllFrame(enable_all_frames ? true : false);

            const char* data = (char*)val->GetPtr() + 1;
            uint16_t data_len = val->GetSize() - 1;
            if (mpp_enc.SetUserData(data, data_len)) {
                LOG("ERROR: MPP Encoder: set userdata error!\n");
                return false;
            }
        } else if (change & VideoEncoder::kMoveDetectionFlow) {
            RcApiBrief brief;

            if (val->GetPtr()) {
#if 0
                int bps_max = vconfig.bit_rate_max;
                int bps_min = vconfig.bit_rate_min;
                int bps_target = vconfig.bit_rate;
                int w = vconfig.image_cfg.image_info.vir_width;
                int h = vconfig.image_cfg.image_info.vir_height;
                float bps_factor = smart_enc_mode_get_bps_factor(vconfig.bit_rate_max, w, h);
                bps_min = (int)(vconfig.bit_rate_max * bps_factor);

                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:mode", MPP_ENC_RC_MODE_VBR);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_min", bps_min);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_max", bps_max);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:bps_target", bps_target);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", 300);

                LOG("MPP Encoder: smart enc mode: factor:%f, bps:[%d,%d,%d] gop:%d\n",
                    bps_factor, bps_max, bps_target, bps_min, 300);
                if(mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                    LOG("ERROR: MPP Encoder: rc control for smart enc failed!\n");
                    return false;
                }

                //save to vconfig
                vconfig.gop_size = 300;
                vconfig.rc_mode = KEY_VBR;
                vconfig.bit_rate_min = bps_min;
#endif
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:mode", MPP_ENC_RC_MODE_VBR);
                ret |= mpp_enc_cfg_set_s32(enc_cfg, "rc:gop", 300);
                if (mpp_enc.EncodeControl(MPP_ENC_SET_CFG, enc_cfg) != 0) {
                    LOG("ERROR: MPP Encoder: rc control for smart enc failed!\n");
                    return false;
                }
                // save to vconfig
                vconfig.gop_size = 300;
                vconfig.rc_mode = KEY_VBR;
                // gop change restart userata status
                mpp_enc.RestartUserData();

                // Enable smart mode.
                brief.name = "smart";
                brief.type = code_type;
                LOG("MPP Encoder: enable smart enc mode...\n");
            } else {
                brief.name = "defalut";
                brief.type = code_type;
                LOG("MPP Encoder: disable smart enc mode...\n");
            }

            if (mpp_enc.EncodeControl(MPP_ENC_SET_RC_API_CURRENT, &brief) != 0) {
                LOG("ERROR: MPP Encoder: enable smart enc control failed!\n");
                return false;
            }
            mpp_enc.rc_api_brief_name = brief.name;
        } else {
            LOG("Unsupport request change 0x%08x!\n", change);
            return false;
        }

        return true;
    }

    class MPPFinalEncoder : public MPPEncoder
    {
      public:
        MPPFinalEncoder(const char* param);
        virtual ~MPPFinalEncoder()
        {
            if (mpp_config) {
                delete mpp_config;
            }
        }

        static const char* GetCodecName()
        {
            return "rkmpp";
        }
        virtual bool InitConfig(const MediaConfig& cfg) override;

      protected:
        // Change configs which are not contained in sps/pps.
        virtual bool CheckConfigChange(std::pair<uint32_t, std::shared_ptr<ParameterBuffer>>) override;

        MPPConfig* mpp_config;
    };

    MPPFinalEncoder::MPPFinalEncoder(const char* param) : mpp_config(nullptr)
    {
        std::string output_data_type = get_media_value_by_key(param, KEY_OUTPUTDATATYPE);
        SetMppCodeingType(output_data_type.empty() ? MPP_VIDEO_CodingUnused : GetMPPCodingType(output_data_type));
    }

    bool MPPFinalEncoder::InitConfig(const MediaConfig& cfg)
    {
        assert(!mpp_config);
        MediaConfig new_cfg = cfg;
        switch (coding_type) {
            case MPP_VIDEO_CodingMJPEG:
                mpp_config = new MPPMJPEGConfig();
                new_cfg.img_cfg.codec_type = codec_type;
                break;
            case MPP_VIDEO_CodingAVC:
            case MPP_VIDEO_CodingHEVC:
                new_cfg.vid_cfg.image_cfg.codec_type = codec_type;
                mpp_config = new MPPCommonConfig(coding_type);
                break;
            default:
                LOG("Unsupport mpp encode type: %d\n", coding_type);
                return false;
        }
        if (!mpp_config) {
            LOG_NO_MEMORY();
            return false;
        }
        return mpp_config->InitConfig(*this, new_cfg);
    }

    bool MPPFinalEncoder::CheckConfigChange(std::pair<uint32_t, std::shared_ptr<ParameterBuffer>> change_pair)
    {
        // common ConfigChange process
        if (change_pair.first & VideoEncoder::kEnableStatistics) {
            bool value = (change_pair.second->GetValue()) ? true : false;
            set_statistics_switch(value);
            return true;
        }

        assert(mpp_config);
        if (!mpp_config) {
            return false;
        }
        return mpp_config->CheckConfigChange(*this, change_pair.first, change_pair.second);
    }

    DEFINE_VIDEO_ENCODER_FACTORY(MPPFinalEncoder)
    const char* FACTORY(MPPFinalEncoder)::ExpectedInputDataType()
    {
        return MppAcceptImageFmts();
    }

#define VIDEO_ENC_OUTPUT                                                                                               \
    TYPENEAR(VIDEO_H264)                                                                                               \
    TYPENEAR(VIDEO_H265) TYPENEAR(VIDEO_MJPEG) TYPENEAR(IMAGE_JPEG)

    const char* FACTORY(MPPFinalEncoder)::OutPutDataType()
    {
        return VIDEO_ENC_OUTPUT;
    }

} // namespace easymedia
