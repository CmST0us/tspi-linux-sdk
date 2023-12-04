// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MEDIA_CONFIG_H_
#define EASYMEDIA_MEDIA_CONFIG_H_

#include "flow.h"
#include "image.h"
#include "media_type.h"
#include "sound.h"

typedef struct {
    ImageInfo image_info;
    CodecType codec_type;
    // for jpeg only
    int qfactor; // 1-99: higher value => higher quality
} ImageConfig;

typedef struct {
    ImageConfig image_cfg;
    int qp_init;
    int qp_step;
    int qp_min;
    int qp_max;
    int qp_max_i;
    int qp_min_i;
    int bit_rate;
    int bit_rate_max;
    int bit_rate_min;
    // The frame rate may be a fraction:
    //   @frame_rate: numerator of the output fps
    //   @frame_rate_den: denominator of the output fps.
    //   @frame_in_rate: numerator of the input fps.
    //   @frame_in_rate_den: denominator of the input fps.
    int frame_rate;
    int frame_rate_den;
    int frame_in_rate;
    int frame_in_rate_den;
    int trans_8x8; // h264 encoder
    int level;     // h264 encoder
    int gop_size;
    int profile; // h264 encoder
    // encoder work in fullrage mode.
    int full_range;
    // reference frame config.
    int ref_frm_cfg;
    // rotation:0, 90, 180, 270;
    int rotation;
    // quality - quality parameter
    //    (extra CQP level means special constant-qp (CQP) mode)
    //    (extra AQ_ONLY means special aq only mode)
    // "worst", "worse", "medium", "better", "best", "cqp", "aq_only"
    const char* rc_quality;
    // rc_mode - rate control mode
    // "vbr", "cbr", "fixqp"
    const char* rc_mode;
} VideoConfig;

typedef struct {
    SampleInfo sample_info;
    CodecType codec_type;
    // uint64_t channel_layout;
    int bit_rate;
    float quality; // vorbis: 0.0 ~ 1.0;
} AudioConfig;

typedef struct {
    union {
        VideoConfig vid_cfg;
        ImageConfig img_cfg;
        AudioConfig aud_cfg;
    };
    Type type;
} MediaConfig;

#define OSD_REGIONS_CNT 8

typedef struct {
    uint8_t* buffer; // Content: ID of palette
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t width;
    uint32_t height;
    uint32_t inverse;
    uint32_t region_id; // max = 8.
    uint8_t enable;
} OsdRegionData;

typedef struct {
    uint16_t x;           /**< horizontal position of top left corner */
    uint16_t y;           /**< vertical position of top left corner */
    uint16_t w;           /**< width of ROI rectangle */
    uint16_t h;           /**< height of ROI rectangle */
    uint16_t intra;       /**< flag of forced intra macroblock */
    int16_t quality;      /**<  qp of macroblock */
    uint16_t qp_area_idx; /**< qp min max area select*/
    uint8_t area_map_en;  /**< enable area map */
    uint8_t abs_qp_en;    /**< absolute qp enable flag*/
} EncROIRegion;

typedef struct {
    char* type;
    uint32_t max_bps;
    // KEY_WORST/KEY_WORSE/KEY_MEDIUM/KEY_BETTER/KEY_BEST
    const char* rc_quality;
    // KEY_VBR/KEY_CBR
    const char* rc_mode;
    uint16_t fps;
    uint16_t gop;
    // For AVC
    uint8_t profile;
    uint8_t enc_levle;
} VideoEncoderCfg;

typedef struct {
    int qp_init;
    int qp_step;
    int qp_min; // 0~48
    int qp_max; // 8-51
    int qp_min_i;
    int qp_max_i;
} VideoEncoderQp;

typedef enum
{
    GOP_MODE_NORMALP = 0, // normal p mode
    GOP_MODE_TSVC2,       // tsvc: 2 layer
    GOP_MODE_TSVC3,       // tsvc: 3 layer
    GOP_MODE_TSVC4,       // tsvc: 4 layer
    GOP_MODE_SMARTP,      // smart p mode
} EncGopMode;

typedef struct {
    EncGopMode mode;
    int gop_size;
    int ip_qp_delta; // qp delta between I frame and P frame.
    int interval;    // interval for smartp
    int vi_qp_delta; // virtual I frame qp delta for smartp.
} EncGopModeParam;

#include <map>

namespace easymedia
{
    extern const char* rc_quality_strings[7];
    extern const char* rc_mode_strings[3];
    const char* ConvertRcQuality(const std::string& s);
    const char* ConvertRcMode(const std::string& s);
    bool ParseMediaConfigFromMap(std::map<std::string, std::string>& params, MediaConfig& mc);
    _API std::vector<EncROIRegion> StringToRoiRegions(const std::string& str_regions);
    _API std::string to_param_string(const ImageConfig& img_cfg);
    _API std::string to_param_string(const VideoConfig& vid_cfg);
    _API std::string to_param_string(const AudioConfig& aud_cfg);
    _API std::string to_param_string(const MediaConfig& mc, const std::string& out_type);
    _API std::string get_video_encoder_config_string(const ImageInfo& info, const VideoEncoderCfg& cfg);
    _API int video_encoder_set_bps(std::shared_ptr<Flow>& enc_flow, unsigned int target, unsigned int min = 0,
                                   unsigned int max = 0);
    // rc_quality Ranges:
    //   KEY_WORST/KEY_WORSE/KEY_MEDIUM/KEY_BETTER/KEY_BEST
    _API int video_encoder_set_rc_quality(std::shared_ptr<Flow>& enc_flow, const char* rc_quality);
    // rc_mode Ranges:KEY_VBR/KEY_CBR
    _API int video_encoder_set_rc_mode(std::shared_ptr<Flow>& enc_flow, const char* rc_mode);
    _API int video_encoder_set_qp(std::shared_ptr<Flow>& enc_flow, VideoEncoderQp& qps);
    _API int video_encoder_force_idr(std::shared_ptr<Flow>& enc_flow);
    _API int video_encoder_set_fps(std::shared_ptr<Flow>& enc_flow, uint8_t out_num, uint8_t out_den,
                                   uint8_t in_num = 0, uint8_t in_den = 0);
    _API int video_encoder_set_osd_plt(std::shared_ptr<Flow>& enc_flow, const uint32_t* yuv_plt);
    _API int video_encoder_set_osd_region(std::shared_ptr<Flow>& enc_flow, OsdRegionData* region_data);
    _API int video_encoder_set_move_detection(std::shared_ptr<Flow>& enc_flow, std::shared_ptr<Flow>& md_flow);
    _API int video_encoder_set_roi_regions(std::shared_ptr<Flow>& enc_flow, EncROIRegion* regions, int region_cnt);
    _API int video_encoder_set_roi_regions(std::shared_ptr<Flow>& enc_flow, std::string roi_param);
    _API int video_encoder_set_gop_size(std::shared_ptr<Flow>& enc_flow, int gop);
    _API int video_move_detect_set_rects(std::shared_ptr<Flow>& md_flow, ImageRect* rects, int rect_cnt);
    _API int video_move_detect_set_rects(std::shared_ptr<Flow>& md_flow, std::string rects_param);
    _API int video_encoder_set_avc_profile(std::shared_ptr<Flow>& enc_flow, int profile_idc, int level = 0);

    // mode: slice split mode
    // 0 - No slice is split
    // 1 - Slice is split by byte number
    // 2 - Slice is split by macroblock / ctu number
    //
    // szie: slice split size parameter
    // When split by byte number this value is the max byte number for each slice.
    // When split by macroblock / ctu number this value is the MB/CTU number
    // for each slice.
    _API int video_encoder_set_split(std::shared_ptr<Flow>& enc_flow, unsigned int mode, unsigned int size);
    _API int video_encoder_set_gop_mode(std::shared_ptr<Flow>& enc_flow, EncGopModeParam* params);
    _API int video_encoder_set_userdata(std::shared_ptr<Flow>& enc_flow, void* data, int len, int all_frames = 0);
    _API int video_encoder_enable_statistics(std::shared_ptr<Flow>& enc_flow, int enable);
    // Set jpeg encoder qfactor, value frome 1 to 99.
    _API int jpeg_encoder_set_qfactor(std::shared_ptr<Flow>& enc_flow, int qfactor);
} // namespace easymedia

#endif // #ifndef EASYMEDIA_MEDIA_CONFIG_H_
