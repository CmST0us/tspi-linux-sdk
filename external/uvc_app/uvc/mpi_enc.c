/*
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/inotify.h>
#include "mpi_enc.h"
#include "uvc_video.h"
#include "uvc_encode.h"
#include "uvc_log.h"
#include "../cJSON/cJSON.h"
#include "mpp_osd.h"

#if MJPEG_RGA_OSD_ENABLE
#include <rga/im2d.h>
#include <rga/rga.h>
#endif

#define TEST_MPP_SEI 0

void *thread_check_mpp_enc_chenge_loop(void *user);

static int mpp_enc_cfg_set(MpiEncTestData *p, bool init);
static int check_mpp_enc_cfg_file_init(MpiEncTestData *p);
static void mpp_enc_cfg_default(MpiEncTestData *p);
static int parse_check_mpp_enc_cfg(cJSON *root, MpiEncTestData *p, bool init);
static void dump_mpp_enc_cfg(MpiEncTestData *p);
static int read_mpp_enc_cfg_modify_file(MpiEncTestData *p, bool init);
static MPP_RET mpp_enc_bps_set(MpiEncTestData *p, RK_U32 bps);
static MPP_RET mpp_set_ref_param(MpiEncTestData *p, MpiEncGopMode gop_mode);
static MPP_RET mpp_enc_set_sei(MpiEncTestData *p, MppEncSeiMode mode);
static MPP_RET mpp_enc_send_sei(MpiEncTestData *p, MppMeta meta, char *sei_data);
#if TEST_MPP_SEI
static MPP_RET mpp_test_send_user_data(MpiEncTestData *p, MppFrame frame);
#endif

#if MPP_ENC_ROI_ENABLE
static int mpp_roi_config(MpiEncTestData *p, EncROIRegion *regions, int region_cnt);
static int mpp_roi_enable_set(MpiEncTestData *p, int enable);
static int mpp_roi_enable_get(MpiEncTestData *p);
#endif

#if MPP_ENC_OSD_ENABLE
extern int mpp_enc_gen_osd_plt(MpiEncTestData *p, uint32_t *ptl_data);
extern const RK_U32 u32DftARGB8888ColorTblUser[PALETTE_TABLE_LEN];
extern const RK_U32 u32DftARGB8888ColorTbl[PALETTE_TABLE_LEN];
extern int mpp_osd_bmp_to_ayuv_image(MpiEncTestData *p, osd_data_s *draw_data);
extern void mpp_osd_region_id_enable_set(MpiEncTestData *p, int region_id, int enable);
extern mpp_osd_region_id_enable_get(MpiEncTestData *p, int region_id);
extern void mpp_osd_enable_set(MpiEncTestData *p, bool enable);
extern bool mpp_osd_enable_get(MpiEncTestData *p);
extern MPP_RET mpp_osd_default_set(MpiEncTestData *p);
void mpp_osd_run(MpiEncTestData *p, int fd, MppFrame frame);
#if MJPEG_RGA_OSD_ENABLE || YUV_RGA_OSD_ENABLE
extern void mjpeg_rga_osd_process(MpiEncTestData *p, int id, int src_fd);
#endif
#endif

#if RK_MPP_MJPEG_FPS_CONTROL
extern void *mpp_mjpeg_fps_init(bool *enable, int mode, int fps, int width, int height);
extern void mpp_mjpeg_fps_deinit(void *fps_handle);
extern bool mpp_mjpeg_encode_data_set(void *fps_handle, void *data, size_t len);
#endif

#if 0
static OptionInfo mpi_enc_cmd[] =
{
    {"i",               "input_file",           "input bitstream file"},
    {"o",               "output_file",          "output bitstream file, "},
    {"w",               "width",                "the width of input picture"},
    {"h",               "height",               "the height of input picture"},
    {"f",               "format",               "the format of input picture"},
    {"t",               "type",                 "output stream coding type"},
    {"n",               "max frame number",     "max encoding frame number"},
    {"d",               "debug",                "debug flag"},
};
#endif
static MppFrameFormat g_format = MPP_FMT_YUV420SP;

RK_S32 mpi_get_env_u32(const char *name, RK_U32 *value, RK_U32 default_value)
{
    char *ptr = getenv(name);
    if (NULL == ptr)
    {
        *value = default_value;
    }
    else
    {
        char *endptr;
        int base = (ptr[0] == '0' && ptr[1] == 'x') ? (16) : (10);
        *value = strtoul(ptr, &endptr, base);
        if (ptr == endptr)
        {
            *value = default_value;
        }
    }
    return 0;
}

static MPP_RET test_ctx_init(MpiEncTestData **data, MpiEncTestCmd *cmd)
{
    MpiEncTestData *p = NULL;
    MPP_RET ret = MPP_OK;

    if (!data || !cmd)
    {
        LOG_ERROR("invalid input data %p cmd %p\n", data, cmd);
        return MPP_ERR_NULL_PTR;
    }

    p = calloc(sizeof(MpiEncTestData), 1);
    if (!p)
    {
        LOG_ERROR("create MpiEncTestData failed\n");
        ret = MPP_ERR_MALLOC;
        goto RET;
    }

    // get paramter from cmd
    p->width        = cmd->width;
    p->height       = cmd->height;
    p->hor_stride   = cmd->width;//MPP_ALIGN(cmd->width, 16);
    p->ver_stride   = cmd->height;//MPP_ALIGN(cmd->height, 16);
    p->fmt          = cmd->format;
    p->type         = cmd->type;
    p->fps         = cmd->fps;
    if (cmd->type == MPP_VIDEO_CodingMJPEG)
        cmd->num_frames = 1;
    p->num_frames   = cmd->num_frames;
    // update resource parameter
    if (p->fmt <= MPP_FMT_YUV420SP_VU)
        p->frame_size = MPP_ALIGN(cmd->width, 16) * MPP_ALIGN(cmd->height, 16) * 2;
    else if (p->fmt <= MPP_FMT_YUV422_UYVY)
    {
        // NOTE: yuyv and uyvy need to double stride
        p->hor_stride *= 2;
        p->frame_size = p->hor_stride * p->ver_stride;
    }
    else
        p->frame_size = p->hor_stride * p->ver_stride * 4;
    p->packet_size  = p->frame_size;//p->width * p->height;

RET:
    *data = p;
    return ret;
}

static MPP_RET test_ctx_deinit(MpiEncTestData **data)
{
    MpiEncTestData *p = NULL;

    if (!data)
    {
        LOG_ERROR("invalid input data %p\n", data);
        return MPP_ERR_NULL_PTR;
    }

    p = *data;
    if (p)
    {
        if (p->fp_input)
        {
            fclose(p->fp_input);
            p->fp_input = NULL;
        }
        if (p->fp_output)
        {
            fclose(p->fp_output);
            p->fp_output = NULL;
        }
        free(p);
        *data = NULL;
    }

    return MPP_OK;
}

//abandoned interface
static MPP_RET test_mpp_setup(MpiEncTestData *p)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppEncCfg cfg;
    MppEncRcMode rc_mode = MPP_ENC_RC_MODE_CBR;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    mpi_get_env_u32("enc_version", &p->enc_version, RK_MPP_VERSION_DEFAULT);
    LOG_DEBUG("enc_version:%d,RK_MPP_USE_FULL_RANGE:%d\n",
             p->enc_version, RK_MPP_USE_FULL_RANGE);

    int need_full_range = 1;
    char *full_range = getenv("ENABLE_FULL_RANGE");
    if (full_range)
    {
        need_full_range = atoi(full_range);
        LOG_DEBUG("mpp full_range use env setting:%d \n", need_full_range);
    }

    mpi = p->mpi;
    ctx = p->ctx;
    cfg = p->cfg;

    /* setup default parameter */
    if (p->fps_in_den == 0)
        p->fps_in_den = 1;
    if (p->fps_in_num == 0)
        p->fps_in_num = 30;
    if (p->fps_out_den == 0)
        p->fps_out_den = 1;
    if (p->fps_out_num == 0)
        p->fps_out_num = 30;
    p->gop = 60;
    if (!p->bps)
        p->bps = p->width * p->height / 8 * (p->fps_out_num / p->fps_out_den);
    mpp_enc_cfg_set_s32(cfg, "prep:width", p->width);
    mpp_enc_cfg_set_s32(cfg, "prep:height", p->height);
    mpp_enc_cfg_set_s32(cfg, "prep:hor_stride", p->hor_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:ver_stride", p->ver_stride);
    mpp_enc_cfg_set_s32(cfg, "prep:format", p->fmt);

    mpp_enc_cfg_set_s32(cfg, "rc:mode", rc_mode);
    switch (rc_mode)
    {
    case MPP_ENC_RC_MODE_FIXQP :
    {
        /* do not set bps on fix qp mode */
    } break;
    case MPP_ENC_RC_MODE_CBR :
    {
        /* CBR mode has narrow bound */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_target", p->bps);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps * 15 / 16);
    }
    break;
    case MPP_ENC_RC_MODE_VBR :
    {
        /* CBR mode has wide bound */
        mpp_enc_cfg_set_s32(cfg, "rc:bps_target", p->bps);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_max", p->bps * 17 / 16);
        mpp_enc_cfg_set_s32(cfg, "rc:bps_min", p->bps * 1 / 16);
    }
    break;
    default :
    {
        LOG_ERROR("unsupport encoder rc mode %d\n", rc_mode);
    }
    break;
    }

    /* fix input / output frame rate */
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", p->fps_in_flex);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", p->fps_in_num);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", p->fps_in_den);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", p->fps_out_flex);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", p->fps_out_num);
    mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", p->fps_out_den);
    mpp_enc_cfg_set_s32(cfg, "rc:gop", p->gop);

    /* setup codec  */
    mpp_enc_cfg_set_s32(cfg, "codec:type", p->type);
    switch (p->type)
    {
    case MPP_VIDEO_CodingAVC :
    {
        /*
        * H.264 profile_idc parameter
        * 66  - Baseline profile
        * 77  - Main profile
        * 100 - High profile
        */
        mpp_enc_cfg_set_s32(cfg, "h264:profile", 100);
        /*
        * H.264 level_idc parameter
        * 10 / 11 / 12 / 13        - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
        * 20 / 21 / 22                 - cif@30fps / half-D1@@25fps / D1@12.5fps
        * 30 / 31 / 32                 - D1@25fps / 720p@30fps / 720p@60fps
        * 40 / 41 / 42                 - 1080p@30fps / 1080p@30fps / 1080p@60fps
        * 50 / 51 / 52                 - 4K@30fps
        */
        mpp_enc_cfg_set_s32(cfg, "h264:level", 40);
        mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", 1);
        mpp_enc_cfg_set_s32(cfg, "h264:cabac_idc", 0);
        mpp_enc_cfg_set_s32(cfg, "h264:trans8x8", 1);
    }
    break;
    case MPP_VIDEO_CodingMJPEG :
    {
        mpp_enc_cfg_set_s32(cfg, "jpeg:quant", 7);
    }
    break;
    case MPP_VIDEO_CodingVP8 :
    {
    } break;
    case MPP_VIDEO_CodingHEVC :
    {
        mpp_enc_cfg_set_s32(cfg, "h265:qp_init", rc_mode == MPP_ENC_RC_MODE_FIXQP ? -1 : 26);
        mpp_enc_cfg_set_s32(cfg, "h265:qp_max", 51);
        mpp_enc_cfg_set_s32(cfg, "h265:qp_min", 10);
        mpp_enc_cfg_set_s32(cfg, "h265:qp_max_i", 46);
        mpp_enc_cfg_set_s32(cfg, "h265:qp_min_i", 24);
    }
    break;
    default :
    {
        LOG_ERROR("unsupport encoder coding type %d\n", p->type);
    }
    break;
    }

    p->split_mode = MPP_ENC_SPLIT_NONE;
    p->split_arg = 0;

    if (p->split_mode)
    {
        LOG_DEBUG("split_mode %d split_arg %d\n", p->split_mode, p->split_arg);
        mpp_enc_cfg_set_s32(cfg, "split:mode", p->split_mode);
        mpp_enc_cfg_set_s32(cfg, "split:arg", p->split_arg);
    }

#if RK_MPP_USE_FULL_RANGE
    ret = mpp_enc_cfg_set_s32(cfg, "prep:range", MPP_FRAME_RANGE_JPEG);
#else
    if (need_full_range)
        ret = mpp_enc_cfg_set_s32(cfg, "prep:range", MPP_FRAME_RANGE_JPEG);
    else
        ret = mpp_enc_cfg_set_s32(cfg, "prep:range", MPP_FRAME_RANGE_UNSPECIFIED);
#endif
    if (ret)
    {
        LOG_ERROR("mpi control enc set prep:range failed ret %d\n", ret);
        goto RET;
    }

    ret = mpi->control(ctx, MPP_ENC_SET_CFG, cfg);
    if (ret)
    {
        LOG_ERROR("mpi control enc set cfg failed ret %d\n", ret);
        goto RET;
    }

    /* optional */
    p->sei_mode = MPP_ENC_SEI_MODE_DISABLE;
    ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
    if (ret)
    {
        LOG_ERROR("mpi control enc set sei cfg failed ret %d\n", ret);
        goto RET;
    }

    if (p->enc_version == 1 &&
            (p->type == MPP_VIDEO_CodingAVC ||
             p->type == MPP_VIDEO_CodingHEVC))
    {
        int header_mode = MPP_ENC_HEADER_MODE_EACH_IDR;
        ret = mpi->control(ctx, MPP_ENC_SET_HEADER_MODE, &header_mode);
        if (ret)
        {
            LOG_ERROR("mpi control enc set codec cfg failed ret %d\n", ret);
            goto RET;
        }
    }

RET:
    return ret;
}

MPP_RET mpp_mjpeg_simple_frc(MpiEncTestData *p, bool set_low)
{
    MPP_RET ret = MPP_OK;
    bool change = false;
    RK_U32 last;

    if (set_low)
    {
#if MPP_ENC_MJPEG_FRC_USE_MPP
        if (p->mjpeg_cfg.rc_mode != MPP_ENC_RC_MODE_FIXQP)
        {
            last = p->mjpeg_cfg.frc_bps;
            p->mjpeg_cfg.frc_bps = (p->mjpeg_cfg.frc_bps > MJPEG_FRC_BPS_MIN ?
                                   (p->mjpeg_cfg.frc_bps - MJPEG_FRC_BPS_PER_STEP) :
                                    p->mjpeg_cfg.frc_bps);
            if (p->mjpeg_cfg.frc_bps != last)
                change = true;
        }
#endif
        if (p->mjpeg_cfg.rc_mode == MPP_ENC_RC_MODE_FIXQP)
        {
            if (!p->mjpeg_cfg.qfactor)
            {
                last = p->mjpeg_cfg.frc_quant;
                p->mjpeg_cfg.frc_quant = (p->mjpeg_cfg.frc_quant > MJPEG_FRC_QUANT_MIN ?
                                         (p->mjpeg_cfg.frc_quant - 1) :
                                          p->mjpeg_cfg.frc_quant);
                if (p->mjpeg_cfg.frc_quant != last)
                    change = true;
            }
            else
            {
                last = p->mjpeg_cfg.frc_qfactor;
                p->mjpeg_cfg.frc_qfactor = (p->mjpeg_cfg.frc_qfactor > p->mjpeg_cfg.qfactor_frc_min ?
                                           (p->mjpeg_cfg.frc_qfactor - 1) :
                                            p->mjpeg_cfg.frc_qfactor);
                if (p->mjpeg_cfg.frc_qfactor != last)
                    change = true;
            }
        }
    }
    else
    {
#if MPP_ENC_MJPEG_FRC_USE_MPP
        if (p->mjpeg_cfg.rc_mode != MPP_ENC_RC_MODE_FIXQP)
        {
            last = p->mjpeg_cfg.frc_bps;
            p->mjpeg_cfg.frc_bps = (p->mjpeg_cfg.frc_bps < MJPEG_FRC_BPS_MAX ?
                                   (p->mjpeg_cfg.frc_bps + MJPEG_FRC_BPS_PER_STEP) :
                                    p->mjpeg_cfg.frc_bps);
            if (p->mjpeg_cfg.frc_bps != last)
                change = true;
        }
#endif
        if (p->mjpeg_cfg.rc_mode == MPP_ENC_RC_MODE_FIXQP)
        {
            if (!p->mjpeg_cfg.qfactor)
            {
                last = p->mjpeg_cfg.frc_quant;
                p->mjpeg_cfg.frc_quant = (p->mjpeg_cfg.frc_quant < p->mjpeg_cfg.quant ?
                                         (p->mjpeg_cfg.frc_quant + 1) :
                                          p->mjpeg_cfg.frc_quant);
                if (p->mjpeg_cfg.frc_quant != last)
                    change = true;
            }
            else
            {
                last = p->mjpeg_cfg.frc_qfactor;
                p->mjpeg_cfg.frc_qfactor = (p->mjpeg_cfg.frc_qfactor < p->mjpeg_cfg.qfactor ?
                                           (p->mjpeg_cfg.frc_qfactor + 1) :
                                            p->mjpeg_cfg.frc_qfactor);
                if (p->mjpeg_cfg.frc_qfactor != last)
                    change = true;
            }
        }
    }

    if (!change)
        return ret;
#if MPP_ENC_MJPEG_FRC_USE_MPP
    switch (p->mjpeg_cfg.rc_mode)
    {
        case MPP_ENC_RC_MODE_FIXQP :
        {
            /* do not set bps on fix qp mode */
        }
        break;
        case MPP_ENC_RC_MODE_CBR :
        {
            /* CBR mode has narrow bound */
            mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", p->mjpeg_cfg.frc_bps);
            mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", p->mjpeg_cfg.frc_bps * 17 / 16);
            mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", p->mjpeg_cfg.frc_bps * 15 / 16);
            LOG_INFO("frc_bps %s, change %d to %d\n", set_low ? "set down" : "set up", last, p->mjpeg_cfg.frc_bps);
        }
        break;
        case MPP_ENC_RC_MODE_VBR :
        {
            /* CBR mode has wide bound */
            mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", p->mjpeg_cfg.frc_bps);
            mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", p->mjpeg_cfg.frc_bps * 17 / 16);
            mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", p->mjpeg_cfg.frc_bps * 1 / 16);
            LOG_INFO("frc_bps %s, change %d to %d\n", set_low ? "set down" : "set up", last, p->mjpeg_cfg.frc_bps);
        }
        break;
        default :
        {
            LOG_ERROR("unsupport encoder rc mode %d\n", p->mjpeg_cfg.rc_mode);
            return MPP_NOK;
        }
        break;
    }
#endif
    if (p->mjpeg_cfg.rc_mode == MPP_ENC_RC_MODE_FIXQP)
    {
        if (!p->mjpeg_cfg.qfactor)
        {
            mpp_enc_cfg_set_s32(p->cfg, "jpeg:quant", p->mjpeg_cfg.frc_quant);
            LOG_INFO("frc_quant %s, change %d to %d\n", set_low ? "set down" : "set up", last, p->mjpeg_cfg.frc_quant);
        }
        else
        {
            mpp_enc_cfg_set_s32(p->cfg, "jpeg:q_factor", p->mjpeg_cfg.frc_qfactor);
            LOG_INFO("frc_qfactor %s, change %d to %d\n", set_low ? "set down" : "set up", last, p->mjpeg_cfg.frc_qfactor);
        }
    }
    ret = p->mpi->control(p->ctx, MPP_ENC_SET_CFG, p->cfg);
    if (ret)
    {
        LOG_ERROR("mpi control enc set cfg failed ret %d\n", ret);
    }

    return ret;
}

bool mjpeg_is_high_solution(MpiEncTestData *p)
{
    bool ret = false;
    if (p->type != MPP_VIDEO_CodingMJPEG)
        return ret;
    if (!p->common_cfg.frc_fps) {
        if ((p->height > 1440 && p->fps  > 15) ||
            (p->height > 1080 && p->fps  > 25) ||
            (p->height > 720 && p->fps > 30)) {
            ret = true;
        }
    } else {//if need use frc_fps to control rate,do this
        p->fps = p->common_cfg.frc_fps;
        if ((p->height > 1440 && p->fps  > 15) ||
            (p->height > 1080 && p->fps  > 25) ||
            (p->height > 720 && p->fps > 30)) {//if need use frc_fps to control rate,do change here p->fps to p->common_cfg.frc_fps
            ret = true;
        }
    }
    return ret;
}

void mpp_try_count_set(MpiEncTestData *p)
{
    if (p->common_cfg.frc_fps == 0)
        p->common_cfg.frc_fps = p->fps;
    p->common_cfg.try_count = p->common_cfg.frc_fps ?
                             (1000 / p->common_cfg.frc_fps / MPP_FRC_WAIT_TIME_MS + MPP_FRC_WAIT_COUNT_OFFSET) :
                              20 + MPP_FRC_WAIT_COUNT_OFFSET;
    p->common_cfg.try_count = p->common_cfg.try_count > (p->common_cfg.enc_time / MPP_FRC_WAIT_TIME_MS) ?
                             (p->common_cfg.try_count - p->common_cfg.enc_time / MPP_FRC_WAIT_TIME_MS) :
                              MPP_FRC_WAIT_COUNT_MIN;
    if (mjpeg_is_high_solution(p) == true)
        p->common_cfg.try_count = (p->common_cfg.try_count - 1) <
                                   MPP_MJPEG_HIGH_FPS_FRC_WAIT_COUNT_MIN ?
                                   MPP_MJPEG_HIGH_FPS_FRC_WAIT_COUNT_MIN : p->common_cfg.try_count - 1;
    else
        p->common_cfg.try_count = p->common_cfg.try_count < MPP_FRC_WAIT_COUNT_MIN ?
                                  MPP_FRC_WAIT_COUNT_MIN : p->common_cfg.try_count;
}

static MPP_RET mpp_enc_need_extra_buf(MpiEncTestData *p, MPP_ENC_INFO_DEF *info)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppBuffer buf = NULL;
    if (NULL == p)
        return MPP_ERR_NULL_PTR;
#if 1
    mpi = p->mpi;
    ctx = p->ctx;

    if (p->enc_version == 1)
    {
        //no need to get the sps/pps in addition
        if (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC)  //force set idr when begin enc
        {
            if (p->h2645_frm_count < (p->common_cfg.force_idr_count * p->common_cfg.force_idr_period))
            {
                if ((p->h2645_frm_count % p->common_cfg.force_idr_period) == 0)
                {
                    ret = mpi->control(ctx, MPP_ENC_SET_IDR_FRAME, NULL);
                    if (ret)
                    {
                        LOG_ERROR("mpi force idr frame control failed\n");
                        goto RET;
                    }
                    else
                    {
                        LOG_INFO("mpi force idr frame control ok, h2645_frm_count:%d\n", p->h2645_frm_count);
                    }
                }
                p->h2645_frm_count++;
            }
        }
    }
    else
    {
        if (p->type == MPP_VIDEO_CodingAVC)
        {
            MppPacket packet = NULL;

            ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
            if (ret)
            {
                LOG_ERROR("mpi control enc get extra info failed\n");
                goto RET;
            }

            /* get and write sps/pps for H.264 */
            if (packet)
            {
                void *ptr   = mpp_packet_get_pos(packet);
                size_t len  = mpp_packet_get_length(packet);

                if (p->fp_output)
                    fwrite(ptr, 1, len, p->fp_output);

                packet = NULL;
            }
        }
    }

    do
    {
        MppFrame frame = NULL;

        if (p->packet)
            mpp_packet_deinit(&p->packet);
        p->packet = NULL;

        ret = mpp_frame_init(&frame);
        if (ret)
        {
            LOG_ERROR("mpp_frame_init failed\n");
            goto RET;
        }

        mpp_frame_set_width(frame, p->width);
        mpp_frame_set_height(frame, p->height);
        mpp_frame_set_hor_stride(frame, p->hor_stride);
        mpp_frame_set_ver_stride(frame, p->ver_stride);
        mpp_frame_set_fmt(frame, p->fmt);
#if 0
        mpp_frame_set_buffer(frame, p->frm_buf);
#else
        MppBufferInfo inputCommit;
        memset(&inputCommit, 0, sizeof(inputCommit));
        inputCommit.type = MPP_BUFFER_TYPE_ION;
        inputCommit.size = info->size;
        inputCommit.fd = info->fd;
        ret = mpp_buffer_import(&buf, &inputCommit);
        if (ret)
        {
            LOG_ERROR("import input picture buffer failed\n");
            goto RET;
        }
        mpp_frame_set_buffer(frame, buf);
#endif
        mpp_frame_set_eos(frame, p->frm_eos);

        ret = mpi->encode_put_frame(ctx, frame);
        if (ret)
        {
            LOG_ERROR("mpp encode put frame failed\n");
            mpp_frame_deinit(&frame);
            goto RET;
        }
        mpp_frame_deinit(&frame);

        ret = mpi->encode_get_packet(ctx, &p->packet);
        if (ret)
        {
            LOG_ERROR("mpp encode get packet failed\n");
            goto RET;
        }

        if (p->packet)
        {
            // write packet to file here
            void *ptr   = mpp_packet_get_pos(p->packet);
            size_t len  = mpp_packet_get_length(p->packet);

            p->pkt_eos = mpp_packet_get_eos(p->packet);

            if (p->fp_output)
            {
                fwrite(ptr, 1, len, p->fp_output);
#if RK_MPP_DYNAMIC_DEBUG_ON
                if (access(RK_MPP_DYNAMIC_DEBUG_OUT_CHECK, 0))
                {
                    fclose(p->fp_output);
                    p->fp_output = NULL;
                    LOG_INFO("debug out file close\n");
                }
            }
            else if (!access(RK_MPP_DYNAMIC_DEBUG_OUT_CHECK, 0))
            {
                p->fp_output = fopen(p->streamout_save_dir, "w+b");
                if (p->fp_output)
                {
                    fwrite(ptr, 1, len, p->fp_output);
                    LOG_INFO("debug out file open\n");
                }
#endif
            }

            mpp_packet_deinit(&p->packet);
            p->enc_data = ptr;
            p->enc_len = len;
        }
    }
    while (0);
#endif

RET:

    if (buf)
    {
        mpp_buffer_put(buf);
        buf = NULL;
    }
    return ret;
}


static MPP_RET test_mpp_run(MpiEncTestData *p, MPP_ENC_INFO_DEF *info)
{
#if RK_MPP_USE_ZERO_COPY
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppPacket packet = NULL;
    MppFrame frame = NULL;
    RK_S32 i;
    MppBuffer buf = NULL;
    MppBuffer pkt_buf_out = NULL;
    int try_count = p->common_cfg.try_count;
    bool get_ok = false;
    int32_t use_time_us, now_time_us, last_time_us;
    struct timespec now_tm = {0, 0};
    bool in_init = false;
    bool out_init = false;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;

    mpi = p->mpi;
    ctx = p->ctx;

    ret = mpp_frame_init(&frame);
    if (ret)
    {
        printf("mpp_frame_init failed\n");
        goto RET;
    }

    mpp_frame_set_width(frame, p->width);
    mpp_frame_set_height(frame, p->height);
    mpp_frame_set_hor_stride(frame, p->hor_stride);
    mpp_frame_set_ver_stride(frame, p->ver_stride);
    mpp_frame_set_fmt(frame, p->fmt);

    MppTask task = NULL;
    RK_S32 index = i++;
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
    struct uvc_buffer *uvc_buf;

#if UVC_SEND_BUF_WHEN_ENC_READY
    if (p->frame_count >= UVC_BUFFER_NUM)
#endif
    {
        while (try_count--)
        {
            if ((!uvc_get_user_run_state(uvc_enc.video_id) || !uvc_buffer_write_enable(uvc_enc.video_id)))
            {
                // LOG_ERROR("not get write buff,read too slow.%d,%d\n",
                //      uvc_get_user_run_state(uvc_enc.video_id),uvc_buffer_write_enable(uvc_enc.video_id));
                // return ret;
                usleep(MPP_FRC_WAIT_TIME_US);
            }
            else
            {
                get_ok = true;
                break;
            }
        }
    }
#if UVC_SEND_BUF_WHEN_ENC_READY
    else
    {
        get_ok = true;
    }
#endif

    if (!get_ok)
    {
        p->loss_frm ++;
        p->continuous_frm = 0;
#if RK_MPP_DYNAMIC_DEBUG_ON
        if (access(RK_MPP_CLOSE_FRM_LOSS_DEBUG_CHECK, 0)) //no exist and output log
#endif
        {
            if (p->mjpeg_cfg.qfactor_frc_min < p->mjpeg_cfg.frc_qfactor)
                LOG_ERROR("not get write buff,read too slow %d,%d.\"touch %s \" can close debug.\n",
                           p->loss_frm,
                           p->frc_up_frm_set,
                           RK_MPP_CLOSE_FRM_LOSS_DEBUG_CHECK);
        }
        if (p->loss_frm >= 5 && p->type == MPP_VIDEO_CodingMJPEG && p->common_cfg.frc_mode)
        {
            p->loss_frm = 0;
            if (p->set_frc_low == false) // punishment mechanism
                p->frc_up_frm_set = (p->frc_up_frm_set << 1) < MPP_FRC_UP_FRM_SET_MAX ?
                                     p->frc_up_frm_set <<= 1 : MPP_FRC_UP_FRM_SET_MAX;
            else //avoid shortterm rapid decline after recovery too slow
                p->frc_up_frm_set = (p->frc_up_frm_set >> 1) > MPP_FRC_UP_FRM_SET_MIN ?
                                     p->frc_up_frm_set >>= 1 : MPP_FRC_UP_FRM_SET_INIT;
            p->set_frc_low = true;
            mpp_mjpeg_simple_frc(p, true);
        }
        return ret;
    }
    else
    {
        p->continuous_frm ++;
        if (p->continuous_frm > p->frc_up_frm_set)
        {
            p->continuous_frm = 0;
            p->loss_frm = ((p->loss_frm > 0) ? (p->loss_frm - 1) : 0);
            // done:can turn up the quant here when loss_frm = 0 or continuous_frm up to more high
            if (p->loss_frm == 0 && p->type == MPP_VIDEO_CodingMJPEG && p->common_cfg.frc_mode == FRC_BOTH_UP_LOW)
            {
                p->loss_frm = 2;
                if (p->set_frc_low == false) // reward mechanism
                    p->frc_up_frm_set = (p->frc_up_frm_set >> 1) > MPP_FRC_UP_FRM_SET_MIN ?
                                         p->frc_up_frm_set >>= 1 : MPP_FRC_UP_FRM_SET_INIT;
                p->set_frc_low = false;
                mpp_mjpeg_simple_frc(p, false);
            }
        }
    }

   // uvc_user_lock();
    uvc_buf = uvc_buffer_write_get(uvc_enc.video_id);
    if (!uvc_buf || uvc_buf->abandon)
    {
        LOG_ERROR("uvc_buffer_write_get failed(buf: %p)\n", uvc_buf);
        goto RET;
    }

    uvc_buf->pts = info->pts;
    uvc_buf->seq = info->seq;
    clock_gettime(CLOCK_MONOTONIC, &now_tm);
    now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us

#if UVC_DYNAMIC_DEBUG_USE_TIME
    if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0))
    {
        use_time_us = now_time_us - uvc_buf->pts;
#if USE_RK_AISERVER
        LOG_INFO("isp->aiserver->ipc->mpp_get_buf seq:%d latency time:%d us, %d ms\n", uvc_buf->seq, use_time_us, use_time_us / 1000);
#endif
#if USE_ROCKIT
        LOG_INFO("isp->rockit->uvc->mpp_get_buf seq:%d latency time:%d us, %d ms\n", uvc_buf->seq, use_time_us, use_time_us / 1000);
#endif
#if USE_RKMEDIA
        LOG_INFO("isp->rkmedia->uvc->mpp_get_buf seq:%d latency time:%d us, %d ms\n", uvc_buf->seq, use_time_us, use_time_us / 1000);
#endif
    }
#endif

    for (int i = 0; i < OUT_BUF_COUNT_MAX; i++) {
        if (uvc_buf->fd == p->out_buff_info[i].buf_fd) {
            out_init = true;
            packet = p->out_buff_info[i].packet;
            pkt_buf_out = p->out_buff_info[i].pkt_buf_out;
            break;
        }
    }
    if (out_init == false)
    {
        MppBufferInfo outputCommit;
        outputCommit.type = MPP_BUFFER_TYPE_DRM;
        outputCommit.size = uvc_buf->drm_buf_size;
        outputCommit.fd = uvc_buf->fd;

        ret = mpp_buffer_import(&pkt_buf_out, &outputCommit);
        if (ret)
        {
            LOG_ERROR("import output picture buffer failed\n");
            goto RET;
        }
        mpp_packet_init_with_buffer(&packet, pkt_buf_out);
        for (int i = 0; i < OUT_BUF_COUNT_MAX; i++) {
            if (p->out_buff_info[i].init == false) {
                p->out_buff_info[i].init = true;
                p->out_buff_info[i].buf_fd = uvc_buf->fd;
                p->out_buff_info[i].packet = packet;
                p->out_buff_info[i].pkt_buf_out = pkt_buf_out;
                out_init = true;
                break;
            }
        }
        //LOG_INFO("new out put fd:%d\n", uvc_buf->fd);
    }

#else
    pkt_buf_out = p->pkt_buf;
    mpp_packet_init_with_buffer(&packet, pkt_buf_out);
#endif
    //mpp_packet_set_pos(packet, NULL);
    mpp_packet_set_length(packet, 0);

#if 0
    mpp_frame_set_buffer(frame, p->frm_buf);
#else
    for (int i = 0; i < IN_BUF_COUNT_MAX; i++) {
        if (info->fd == p->in_buff_info[i].buf_fd) {
            in_init = true;
            buf = p->in_buff_info[i].buf;
            break;
        }
    }

    if (in_init == false) {
        MppBufferInfo inputCommit;
        inputCommit.type = MPP_BUFFER_TYPE_ION;
        inputCommit.size = info->size;
        inputCommit.fd = info->fd;
        ret = mpp_buffer_import(&buf, &inputCommit);
        if (ret)
        {
            LOG_ERROR("import input picture buffer failed\n");
            goto RET;
        }
        for (int i = 0; i < IN_BUF_COUNT_MAX; i++) {
            if (p->in_buff_info[i].init == false) {
                p->in_buff_info[i].init = true;
                p->in_buff_info[i].buf_fd = info->fd;
                p->in_buff_info[i].buf = buf;
                p->in_buff_info[i].pkt_buf_out = pkt_buf_out;
                in_init = true;
                break;
            }
        }
        //LOG_INFO("new in put fd:%d \n", info->fd);
    }
    mpp_frame_set_buffer(frame, buf);
#endif
    if (p->enc_version == 1)
    {
        //no need to get the sps/pps in addition
        if (p->type == MPP_VIDEO_CodingAVC || p->type == MPP_VIDEO_CodingHEVC)  //force set idr when begin enc
        {
            if (p->common_cfg.force_idr_count * p->common_cfg.force_idr_period) {
                if (p->h2645_frm_count < (p->common_cfg.force_idr_count * p->common_cfg.force_idr_period))
                {
                    if ((p->h2645_frm_count % p->common_cfg.force_idr_period) == 0)
                    {
                        LOG_INFO("mpi force idr frame control ok, h2645_frm_count:%d ,H264:%d\n",
                                  p->h2645_frm_count, (p->type == MPP_VIDEO_CodingAVC));
                    }
                    p->h2645_frm_count++;
                }
                else if (p->h2645_frm_count == p->common_cfg.force_idr_count * p->common_cfg.force_idr_period)
                {
                    if (p->type == MPP_VIDEO_CodingAVC)
                    {
                        p->h264_cfg.gop = p->h264_cfg.gop_normal;
                        mpp_enc_cfg_set_s32(p->cfg, "rc:gop", p->h264_cfg.gop);
                        mpp_enc_bps_set(p, p->h264_cfg.bps);
                        mpp_set_ref_param(p, p->h264_cfg.gop_mode);
                    }
                    else
                    {
                        p->h265_cfg.gop = p->h265_cfg.gop_normal;
                        mpp_enc_cfg_set_s32(p->cfg, "rc:gop", p->h265_cfg.gop);
                        mpp_enc_bps_set(p, p->h265_cfg.bps);
                        mpp_set_ref_param(p, p->h265_cfg.gop_mode);
                    }
                    ret = mpi->control(ctx, MPP_ENC_SET_CFG, p->cfg);
                    if (ret)
                    {
                        LOG_ERROR("mpi control enc set cfg failed ret %d\n", ret);
                        goto RET;
                    }
                    p->h2645_frm_count++;
                }
            }
        }
    }
#if MPP_ENC_ROI_ENABLE
    if (mpp_roi_enable_get(p)) {
        MppMeta meta = NULL;
        meta = mpp_frame_get_meta(frame);
        mpp_meta_set_ptr(meta, KEY_ROI_DATA, (void*)&p->roi_cfg);
    }
#endif

#if MPP_ENC_OSD_ENABLE
    mpp_osd_run(p, info->fd, frame);
#endif

#if TEST_MPP_SEI
    mpp_test_send_user_data(p, frame);    // test sei
#endif

    ret = mpi->poll(ctx, MPP_PORT_INPUT, MPP_POLL_BLOCK);
    if (ret)
    {
        LOG_ERROR("mpp task input poll failed ret %d\n", ret);
        goto RET;
    }

    ret = mpi->dequeue(ctx, MPP_PORT_INPUT, &task);
    if (ret || NULL == task)
    {
        LOG_ERROR("mpp task input dequeue failed ret %d task %p\n", ret, task);
        goto RET;
    }

    mpp_task_meta_set_frame(task, KEY_INPUT_FRAME,  frame);
    mpp_task_meta_set_packet(task, KEY_OUTPUT_PACKET, packet);
    //800us+

    ret = mpi->enqueue(ctx, MPP_PORT_INPUT, task);
    if (ret)
    {
        LOG_ERROR("mpp task input enqueue failed\n");
        goto RET;
    }
    //800us+
#if 1
    ret = mpi->poll(ctx, MPP_PORT_OUTPUT, MPP_POLL_BLOCK);
    if (ret)
    {
        LOG_ERROR("mpp task output poll failed ret %d\n", ret);
        goto RET;
    }//9028 us for 1080p
#else
    do
    {
        ret = mpi->poll(ctx, MPP_PORT_OUTPUT, MPP_POLL_NON_BLOCK);
        usleep(100);
    }
    while (ret);//8882 us for 1080p
#endif

    ret = mpi->dequeue(ctx, MPP_PORT_OUTPUT, &task);
    if (ret || NULL == task)
    {
        LOG_ERROR("mpp task output dequeue failed ret %d task %p\n", ret, task);
        goto RET;
    }

    if (task)
    {
        MppFrame packet_out = NULL;
        mpp_task_meta_get_packet(task, KEY_OUTPUT_PACKET, &packet_out);
        assert(packet_out == packet);
        if (packet)
        {
            // write packet to file here
            size_t len = mpp_packet_get_length(packet);
#if RK_MPP_DYNAMIC_DEBUG_ON
            if (!access(RK_MPP_OUT_LEN_DEBUG_CHECK, 0))
                LOG_INFO("out_len=%d\n", len);
#endif

#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
            uvc_buf->size = len;
            uvc_buf->frame_count = p->frame_count;
            if (p->fps_ctr_enable == true) {
#if (RK_MPP_MJPEG_FPS_CONTROL && MJPEG_FPS_CONTROL_V2)
                if (!mpp_mjpeg_encode_data_set(p->fps_handle, (void *)uvc_buf, len)) {
                    uvc_buffer_write_set(uvc_enc.video_id, uvc_buf);
                    LOG_WARN("mpp_mjpeg_encode_data_set fail, giveback the uvc_buf:%d\n", uvc_buf->fd);
                    //uvc_buffer_read_set(uvc_enc.video_id, uvc_buf);
                }
#endif
            } else {
                uvc_buffer_read_set(uvc_enc.video_id, uvc_buf);
            }
#else
            void *ptr = mpp_packet_get_pos(packet);
#endif
            if (p->fp_output)
            {
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
                fwrite(uvc_buf->buffer, 1, len, p->fp_output);
#else
                fwrite(ptr, 1, len, p->fp_output);
#endif
#if RK_MPP_DYNAMIC_DEBUG_ON
                if (access(RK_MPP_DYNAMIC_DEBUG_OUT_CHECK, 0))
                {
                    fclose(p->fp_output);
                    p->fp_output = NULL;
                    LOG_INFO("debug out file close\n");
                }
            }
            else if (!access(RK_MPP_DYNAMIC_DEBUG_OUT_CHECK, 0))
            {
                p->fp_output = fopen(p->streamout_save_dir, "w+b");
                if (p->fp_output)
                {
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
                    fwrite(uvc_buf->buffer, 1, len, p->fp_output);
#else
                    fwrite(ptr, 1, len, p->fp_output);
#endif
                    LOG_INFO("debug out file open\n");
                }
#endif
            }
#ifndef RK_MPP_USE_UVC_VIDEO_BUFFER
            p->enc_data = ptr;
            p->enc_len = len;
#endif
           // mpp_packet_deinit(&packet);
            clock_gettime(CLOCK_MONOTONIC, &now_tm);
            last_time_us = now_time_us;
            now_time_us = now_tm.tv_sec * 1000000LL + now_tm.tv_nsec / 1000; // us
            use_time_us = now_time_us - uvc_buf->pts;

            p->common_cfg.enc_time = (now_time_us - last_time_us) / 1000 +
                                     (p->common_cfg.try_count - try_count) * MPP_FRC_WAIT_TIME_MS;
            p->common_cfg.enc_time = p->common_cfg.enc_time > 80 ? 80 : p->common_cfg.enc_time < 2 ? 2 : p->common_cfg.enc_time;
            mpp_try_count_set(p);
#if UVC_DYNAMIC_DEBUG_USE_TIME
            if (!access(UVC_DYNAMIC_DEBUG_USE_TIME_CHECK, 0))
            {
                LOG_INFO("mpp_enc+getbuf time:%d ms, try_count:%d \n",
                          p->common_cfg.enc_time, p->common_cfg.try_count);
#if USE_RK_AISERVER
                LOG_INFO("isp->aiserver->ipc->mpp_enc_ok latency time:%d us, %d ms\n",
                          use_time_us, use_time_us / 1000);
#endif
#if USE_ROCKIT
                LOG_INFO("isp->rockit->uvc->mpp_enc_ok latency time:%d us, %d ms\n",
                          use_time_us, use_time_us / 1000);
#endif
#if USE_RKMEDIA
                LOG_INFO("isp->rkmedia->uvc->mpp_enc_ok latency time:%d us, %d ms\n",
                          use_time_us, use_time_us / 1000);
#endif
            }
#endif
        }
        //9175 us for 1080p
        p->frame_count++;
    }
    /*
            if(p->frame_count % 100 == 0) {
              LOG_INFO("outlen=%d\n", uvc_buf->size);
            }
    */
    ret = mpi->enqueue(ctx, MPP_PORT_OUTPUT, task);
    if (ret)
    {
        LOG_ERROR("mpp task output enqueue failed\n");
        goto RET;
    }//9195 us for 1080p

RET:
  //  uvc_user_unlock();

    if (frame)
    {
        mpp_frame_deinit(&frame);
        frame = NULL;
    }
    if (in_init == false)
    {
        if (buf)
        {
            mpp_buffer_put(buf);
            buf = NULL;
        }
    }
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
    if (out_init == false)
    {
        if (pkt_buf_out)
        {
            mpp_buffer_put(pkt_buf_out);
            pkt_buf_out = NULL;
        }
        if (packet)
            mpp_packet_deinit(&packet);
    }
#endif

    return ret;
#else
     mpp_enc_need_extra_buf(p, info);
#endif
}

MPP_RET mpi_enc_test_init(MpiEncTestCmd *cmd, MpiEncTestData **data)
{
    MPP_RET ret = MPP_OK;
    MpiEncTestData *p = NULL;

    LOG_DEBUG("mpi_enc_test start\n");

    ret = test_ctx_init(&p, cmd);
    if (ret)
    {
        LOG_ERROR("test data init failed ret %d\n", ret);
        return ret;
    }
    *data = p;

#if 0
    ret = mpp_buffer_get(NULL, &p->frm_buf, p->frame_size);
    if (ret)
    {
        LOG_INFO("failed to get buffer for input frame ret %d\n", ret);
        return ret;
    }
#endif

    LOG_INFO("mpi_enc_test encoder test start w %d h %d type %d\n",
             p->width, p->height, p->type);
    if (p->type != 0)
    {
        // encoder demo
        ret = mpp_create(&p->ctx, &p->mpi);
        if (ret)
        {
            LOG_ERROR("mpp_create failed ret %d\n", ret);
            return ret;
        }

        ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
        if (ret)
        {
            LOG_ERROR("mpp_init failed ret %d\n", ret);
            return ret;
        }
        ret = mpp_enc_cfg_init(&p->cfg);
        if (ret)
        {
            LOG_ERROR("mpp_enc_cfg_init failed ret %d\n", ret);
            return ret;
        }
#if RK_MPP_USE_ZERO_COPY
#ifndef RK_MPP_USE_UVC_VIDEO_BUFFER
        ret = mpp_buffer_group_get_internal(&p->pkt_grp, MPP_BUFFER_TYPE_DRM);
        if (ret)
        {
            LOG_ERROR("failed to get buffer group for output packet ret %d\n", ret);
            return ret;
        }
        p->packet_size = p->width * p->height;

        ret = mpp_buffer_get(p->pkt_grp, &p->pkt_buf, p->packet_size);
        if (ret)
        {
            LOG_ERROR("failed to get buffer for pkt_buf ret %d\n", ret);
            return ret;
        }
#else
#if MPP_ENC_OSD_ENABLE
        ret = mpp_buffer_group_get_internal(&p->pkt_grp, MPP_BUFFER_TYPE_DRM);
        if (ret)
        {
            LOG_ERROR("failed to get buffer group for output packet ret %d\n", ret);
            return ret;
        }
#endif
#endif
#endif
    }
    mpp_enc_cfg_default(p);

    if (!check_mpp_enc_cfg_file_init(p))
        LOG_DEBUG("check_mpp_enc_cfg_file ok\n");
    ret = mpp_enc_cfg_set(p, true);
    dump_mpp_enc_cfg(p);
    if (ret)
    {
        LOG_ERROR("mpp_enc_cfg_set failed ret %d\n", ret);
        return ret;
    }
    pthread_create(&p->check_cfg_change_hd, NULL, thread_check_mpp_enc_chenge_loop, p);
#if RK_MPP_MJPEG_FPS_CONTROL
    if (mjpeg_is_high_solution(p) == true || p->mjpeg_cfg.enc_mode >= 2)
    {
        if (p->mjpeg_cfg.enc_mode == 0)
            p->fps_ctr_enable = true;
        p->fps_handle = mpp_mjpeg_fps_init(&p->fps_ctr_enable, p->mjpeg_cfg.enc_mode, p->fps, p->width, p->height);
    }
#endif

    mpi_get_env_u32("uvc_enc_out", &cmd->have_output, 0);

    if (cmd->have_output || !access(RK_MPP_DYNAMIC_DEBUG_OUT_CHECK, 0))
    {
        p->fp_output = fopen(p->streamout_save_dir, "w+b");
        if (NULL == p->fp_output)
        {
            LOG_ERROR("failed to open output file %s\n", p->streamout_save_dir);
        }
        LOG_DEBUG("debug out file open\n");
    }

    if (!access(RK_MPP_DYNAMIC_DEBUG_IN_CHECK, 0))
    {
        p->fp_input = fopen(p->streamin_save_dir, "w+b");
        if (NULL == p->fp_input)
        {
            LOG_ERROR("failed to open in file %s\n", p->streamin_save_dir);
        }
        LOG_WARN("warnning:debug in file open, open it will lower the fps\n");
    }

    for (int i = 0; i < OUT_BUF_COUNT_MAX; i++) {
        p->out_buff_info[i].init = false;
        p->out_buff_info[i].buf_fd = -1;
    }
    for (int i = 0; i < IN_BUF_COUNT_MAX; i++) {
        p->in_buff_info[i].init = false;
        p->in_buff_info[i].buf_fd = -1;
    }

    p->yuv_rotation_drm_fd = -1;
    p->yuv_rotation_fd = -1;
    if ((p->type == 0 && p->common_cfg.rotation) || (p->type == MPP_VIDEO_CodingMJPEG && p->common_cfg.rotation == 2)) {
        p->yuv_rotation_drm_fd = drm_open();
        if (p->yuv_rotation_drm_fd < 0) {
            LOG_ERROR("yuv_rotation_drm_fd open fail\n");
            return MPP_NOK;
        }
        p->yuv_rotation_drm_size = p->width * p->height * 2;
        ret = drm_alloc(p->yuv_rotation_drm_fd,  p->yuv_rotation_drm_size, 16, &p->yuv_rotation_handle, 0);
        if (ret)
        {
            LOG_ERROR("yuv_rotation drm_alloc fail\n");
            return MPP_NOK;
        }
        ret = drm_handle_to_fd(p->yuv_rotation_drm_fd, p->yuv_rotation_handle, &p->yuv_rotation_fd, 0);
        if (ret)
        {
            LOG_ERROR("yuv_rotation drm_handle_to_fd fail\n");
            return MPP_NOK;
        }
    }

    LOG_INFO("fps_ctr_enable:%d,roi:%d,enc_osd:%d,rga_osd:%d\n",
              p->fps_ctr_enable, MPP_ENC_ROI_ENABLE,
              MPP_ENC_OSD_ENABLE, MJPEG_RGA_OSD_ENABLE);

    return ret;
}

MPP_RET mpi_enc_test_run(MpiEncTestData **data, MPP_ENC_INFO_DEF *info)
{
    MPP_RET ret = MPP_OK;
    MpiEncTestData *p = *data;

    ret = test_mpp_run(p, info);
    if (ret)
        LOG_ERROR("test mpp run failed ret %d\n", ret);
    return ret;
}

MPP_RET mpi_enc_inbuf_deinit(MpiEncTestData *data)
{
    MpiEncTestData *p = data;
    for (int i = 0; i < IN_BUF_COUNT_MAX; i++) {
        if (p->in_buff_info[i].init) {
            if (p->in_buff_info[i].buf)
            {
                mpp_buffer_put(p->in_buff_info[i].buf);
                p->in_buff_info[i].buf = NULL;
                p->in_buff_info[i].buf_fd = -1;
            }
        }
        p->in_buff_info[i].init = false;
    }
}

MPP_RET mpi_enc_test_deinit(MpiEncTestData **data)
{
    MPP_RET ret = MPP_OK;
    MpiEncTestData *p = *data;

#if RK_MPP_MJPEG_FPS_CONTROL
    mpp_mjpeg_fps_deinit(p->fps_handle);
#endif
    if (p->yuv_rotation_drm_fd >= 0) {
        if (p->yuv_rotation_fd >= 0)
            close(p->yuv_rotation_fd);
        drm_free(p->yuv_rotation_drm_fd, p->yuv_rotation_handle);
        drm_close(p->yuv_rotation_drm_fd);
    }

    for (int i = 0; i < OUT_BUF_COUNT_MAX; i++) {
        if (p->out_buff_info[i].init) {
            if (p->out_buff_info[i].packet)
                mpp_packet_deinit(&p->out_buff_info[i].packet);
#ifdef RK_MPP_USE_UVC_VIDEO_BUFFER
            if (p->out_buff_info[i].pkt_buf_out)
            {
                mpp_buffer_put(p->out_buff_info[i].pkt_buf_out);
                p->out_buff_info[i].pkt_buf_out = NULL;
            }
#endif
            p->out_buff_info[i].init = false;
        }
    }
    mpi_enc_inbuf_deinit(p);

    if (p->check_cfg_change_hd) {
        pthread_cancel(p->check_cfg_change_hd);
        pthread_join(p->check_cfg_change_hd, NULL);
    }
    if (p->cfg_notify_fd)
    {
        inotify_rm_watch(p->cfg_notify_fd, p->cfg_notify_wd);
        close(p->cfg_notify_fd);
    }
    if (p->packet)
        mpp_packet_deinit(&p->packet);
    if (p->ctx) {
       ret = p->mpi->reset(p->ctx);
       if (ret)
       {
          LOG_ERROR("mpi->reset failed\n");
       }
    }
    if (p->ctx)
    {
        mpp_destroy(p->ctx);
        p->ctx = NULL;
    }
    if (p->cfg) {
        mpp_enc_cfg_deinit(p->cfg);
        p->cfg = NULL;
    }

#if MPP_ENC_OSD_ENABLE
    if (p->osd_idx_buf) {
        mpp_buffer_put(p->osd_idx_buf);
        p->osd_idx_buf = NULL;
    }

    if (p->osd_data.buf) {
        LOG_DEBUG("MPP Encoder: free osd buff\n");
        mpp_buffer_put(p->osd_data.buf);
        p->osd_data.buf = NULL;
    }
#if MJPEG_RGA_OSD_ENABLE || YUV_RGA_OSD_ENABLE
    else if (p->rga_osd_drm_fd > 0 &&
             ((p->type == MPP_VIDEO_CodingMJPEG && MJPEG_RGA_OSD_ENABLE) ||
              (p->type == 0 && YUV_RGA_OSD_ENABLE))) {
        for (int i = 0; i < p->osd_count; i++) {
           if (p->osd_cfg[i].set_ok) {
               drm_unmap_buffer(p->osd_cfg[i].buffer, p->osd_cfg[i].drm_size);
               if (p->osd_cfg[i].rga_osd_fd)
                   close(p->osd_cfg[i].rga_osd_fd);
               if (p->rga_osd_drm_fd && p->osd_cfg[i].handle)
                   drm_free(p->rga_osd_drm_fd, p->osd_cfg[i].handle);
           }
        }
        drm_close(p->rga_osd_drm_fd);
    }
#endif
#endif

#if MPP_ENC_ROI_ENABLE
    if (p->roi_cfg.regions) {
        free(p->roi_cfg.regions);
        p->roi_cfg.regions = NULL;
    }
#endif
#if 0
    if (p->frm_buf)
    {
        mpp_buffer_put(p->frm_buf);
        p->frm_buf = NULL;
    }
#endif

#if RK_MPP_USE_ZERO_COPY
#ifndef RK_MPP_USE_UVC_VIDEO_BUFFER
    if (p->pkt_buf)
    {
        mpp_buffer_put(p->pkt_buf);
        p->pkt_buf = NULL;
    }

    if (p->pkt_grp)
    {
        mpp_buffer_group_put(p->pkt_grp);
        p->pkt_grp = NULL;
    }
#endif
#else
#if MPP_ENC_OSD_ENABLE
    if (p->pkt_grp)
    {
        mpp_buffer_group_put(p->pkt_grp);
        p->pkt_grp = NULL;
    }
#endif
#endif

    if (MPP_OK == ret)
    {
        LOG_INFO("mpi_enc_test success \n");
        //LOG_INFO("mpi_enc_test success total frame %d bps %lld\n",
        //        p->frame_count, (RK_U64)((p->stream_size * 8 * p->fps) / p->frame_count));
    }
    else
    {
        LOG_ERROR("mpi_enc_test failed ret %d\n", ret);
    }
    test_ctx_deinit(&p);

    return ret;
}

void mpi_enc_cmd_config(MpiEncTestCmd *cmd, int width, int height, int fcc, int h265, unsigned int fps)
{
    memset((void *)cmd, 0, sizeof(*cmd));
    cmd->width = width;
    cmd->height = height;
    cmd->format = g_format;
    cmd->fps = fps;

    char *env_h265 = getenv("ENABLE_UVC_H265");
    if (env_h265)
    {
        h265 = atoi(env_h265);
        LOG_INFO("V4L2_PIX_FMT_H264 force use h265 ?:%d \n", h265);
    }
    if(!access(RK_MPP_ENABLE_UVC_H265, 0)) {
        h265 = 1;
        LOG_INFO("tmp: V4L2_PIX_FMT_H264 force use h265!!\n");
    }

    switch (fcc)
    {
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_NV12:
        LOG_DEBUG("%s: yuyv not need mpp encodec: %d\n", __func__, fcc);
        break;
    case V4L2_PIX_FMT_MJPEG:
        cmd->type = MPP_VIDEO_CodingMJPEG;
        break;
    case V4L2_PIX_FMT_H264:
    {
        if (h265)
            cmd->type = MPP_VIDEO_CodingHEVC;
        else
            cmd->type = MPP_VIDEO_CodingAVC;//MPP_VIDEO_CodingAVC;//MPP_VIDEO_CodingHEVC
        break;
    }
    case V4L2_PIX_FMT_H265:
        cmd->type = MPP_VIDEO_CodingHEVC;
        break;
    default:
        LOG_WARN("%s: not support fcc: %d\n", __func__, fcc);
        break;
    }

}

void mpi_enc_cmd_config_mjpg(MpiEncTestCmd *cmd, int width, int height)
{
    memset((void *)cmd, 0, sizeof(*cmd));
    cmd->width = width;
    cmd->height = height;
    cmd->format = g_format;
    cmd->type = MPP_VIDEO_CodingMJPEG;
}

void mpi_enc_cmd_config_h264(MpiEncTestCmd *cmd, int width, int height)
{
    memset((void *)cmd, 0, sizeof(*cmd));
    cmd->width = width;
    cmd->height = height;
    cmd->format = g_format;
    cmd->type = MPP_VIDEO_CodingAVC;
}
void mpi_enc_set_format(MppFrameFormat format)
{
    g_format = format;
}
int mpi_enc_get_h264_extra(MpiEncTestData *p, void *buffer, size_t *size)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    if (NULL == p)
    {
        *size = 0;
        return -1;
    }
    mpi = p->mpi;
    ctx = p->ctx;
    MppPacket packet = NULL;
    ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
    if (ret)
    {
        LOG_ERROR("mpi control enc get extra info failed\n");
        *size = 0;
        return -1;
    }
    if (packet)
    {
        void *ptr   = mpp_packet_get_pos(packet);
        size_t len  = mpp_packet_get_length(packet);
        LOG_INFO("%s: len = %d\n", __func__, len);
        if (*size >= len)
        {
            memcpy(buffer, ptr, len);
            *size = len;
        }
        else
        {
            LOG_INFO("%s: input buffer size = %d\n", __func__, *size);
            *size = 0;
        }
        packet = NULL;
    }
    return 0;
}

static unsigned long get_file_size(const char *filename)
{
    struct stat buf;
    if (stat(filename, &buf) < 0)
        return 0;
    return (unsigned long)buf.st_size;
}

static void mpp_enc_cfg_default(MpiEncTestData *p)
{
    if (NULL == p)
        return;
#if MPP_ENC_ROI_ENABLE
    p->roi_enable = 0;
#endif
    //common set
    p->common_cfg.fbc = false;
    p->common_cfg.split_mode = 0;
    p->common_cfg.split_arg = 0;
    p->common_cfg.force_idr_count = RK_MPP_H264_FORCE_IDR_COUNT;
    p->common_cfg.force_idr_period = RK_MPP_H264_FORCE_IDR_PERIOD;
    p->common_cfg.frc_fps = p->fps; // use frame rate control
    p->common_cfg.frc_mode = FRC_ONLY_LOW;
    p->common_cfg.rotation = 0;
    if (p->type == MPP_VIDEO_CodingMJPEG)
        p->common_cfg.enc_time = p->width * p->height / 110000;
    else
        p->common_cfg.enc_time = p->width * p->height / 330000;
    p->common_cfg.enc_time = p->common_cfg.enc_time > 80 ? 80 : p->common_cfg.enc_time < 2 ? 2 : p->common_cfg.enc_time;

    //mjpeg set
    p->mjpeg_cfg.quant = 7;
    p->mjpeg_cfg.qfactor = 80; //default set this
    p->mjpeg_cfg.range = MPP_FRAME_RANGE_JPEG; //default for full(only full)
    p->mjpeg_cfg.qfactor_min = 1;
    p->mjpeg_cfg.qfactor_max = 99;
    p->mjpeg_cfg.framerate = 0; // use host set
    p->mjpeg_cfg.gop = 30;
    p->mjpeg_cfg.sei = MPP_ENC_SEI_MODE_DISABLE;
#if MPP_ENC_MJPEG_FRC_USE_MPP
    p->mjpeg_cfg.rc_mode = MPP_ENC_RC_MODE_CBR;
#else
    p->mjpeg_cfg.rc_mode = MPP_ENC_RC_MODE_FIXQP; // not use mpp frc here must be fixqp, do not modify this setting.
#endif
    p->mjpeg_cfg.bps = p->width * p->height / 8 * p->mjpeg_cfg.framerate * 3 / 2; // 1080p 11Mbps
    p->mjpeg_cfg.frc_qfactor = p->mjpeg_cfg.qfactor;
    p->mjpeg_cfg.qfactor_frc_min = MJPEG_FRC_QFACTOR_MIN;

    //h264 set
    p->h264_cfg.gop = 60;
    p->h264_cfg.rc_mode = MPP_ENC_RC_MODE_CBR;
    p->h264_cfg.framerate = 0; // use host set
    p->h264_cfg.range = MPP_FRAME_RANGE_JPEG; //default for full
    p->h264_cfg.head_each_idr = true;
    p->h264_cfg.sei = MPP_ENC_SEI_MODE_DISABLE;
    p->h264_cfg.qp.init = 26;
    p->h264_cfg.qp.max = 48;
    p->h264_cfg.qp.min = 8;
    p->h264_cfg.qp.step = 8;
    p->h264_cfg.profile = MPP_ENC_CFG_H264_DEFAULT_PROFILE;
    p->h264_cfg.cabac_en = 1;
    p->h264_cfg.cabac_idc = 0;
    p->h264_cfg.trans_8x8 = 1;
    p->h264_cfg.level = MPP_ENC_CFG_H264_DEFAULT_LEVEL;
    p->h264_cfg.bps = p->width * p->height / 8 * p->h264_cfg.framerate / 2;
    p->h264_cfg.idr_bps = p->width * p->height / 8 * p->h264_cfg.framerate;
    p->h264_cfg.vi_len = p->h264_cfg.gop / 2;
    p->h264_cfg.gop_mode = GOP_MODE_NORMALP;

    //h265 set
    p->h265_cfg.gop = 60;
    p->h265_cfg.rc_mode = MPP_ENC_RC_MODE_CBR;
    p->h265_cfg.framerate = 0; // use host set
    p->h265_cfg.range = MPP_FRAME_RANGE_JPEG; //default for full
    p->h265_cfg.head_each_idr = true;
    p->h265_cfg.sei = MPP_ENC_SEI_MODE_DISABLE;
    p->h265_cfg.qp.init = 24;
    p->h265_cfg.qp.max = 51;
    p->h265_cfg.qp.min = 10;
    p->h265_cfg.qp.step = 4;
    p->h265_cfg.qp.max_i_qp = 46;
    p->h265_cfg.qp.min_i_qp = 24;
    p->h265_cfg.bps = p->width * p->height / 8 * p->h265_cfg.framerate / 2;
    p->h265_cfg.idr_bps = p->width * p->height / 8 * p->h265_cfg.framerate;
    p->h265_cfg.vi_len = p->h265_cfg.gop / 2;
    p->h265_cfg.gop_mode = GOP_MODE_NORMALP;

#if MPP_ENC_OSD_ENABLE
    p->osd_count = 0;
    p->osd_plt_user = false;
    p->osd_enable = false;
    for (int i = 0; i < OSD_REGIONS_CNT; i++) {
        p->osd_cfg[i].set_ok = false;
        p->osd_cfg[i].enable = false;
        p->osd_cfg[i].start_x = 0;
        p->osd_cfg[i].start_y = 0;
    }
#endif
}

static void dump_mpp_enc_cfg(MpiEncTestData *p)
{

#if MPP_ENC_ROI_ENABLE
    LOG_DEBUG("### dump_mpp_enc_cfg for roi cfg:\n");
    LOG_DEBUG("roi_enable=%d\n", p->roi_enable);
#endif

    LOG_DEBUG("### dump_mpp_enc_cfg for common cfg:\n");
    LOG_DEBUG("fbc=%d,split_mode=%d,split_arg=%d,force_idr_count=%d,force_idr_period=%d,frc_fps=%d\n"
             "frc_mode=%d,enc_time=%d,try_count=%d,frc_use_mpp=%d,fps:%d\n"
             "p->streamin_save_dir=%s,p->streamout_save_dir=%s,rotation=%d\n",
             p->common_cfg.fbc, p->common_cfg.split_mode, p->common_cfg.split_arg,
             p->common_cfg.force_idr_count, p->common_cfg.force_idr_period,
             p->common_cfg.frc_fps, p->common_cfg.frc_mode,
             p->common_cfg.enc_time, p->common_cfg.try_count,
             MPP_ENC_MJPEG_FRC_USE_MPP, p->fps,
             p->streamin_save_dir, p->streamout_save_dir,
             p->common_cfg.rotation*90);

    LOG_DEBUG("###dump_mpp_enc_cfg for mjpeg cfg:\n");
    LOG_DEBUG("quant=%d,q_fator=%d,range=%d,q_min=%d,q_max=%d,gop=%d,rc_mode=%d \n"
             "bps=%d,framerate=%d,enc_mode:%d sei:%d\n",
             p->mjpeg_cfg.quant, p->mjpeg_cfg.qfactor, p->mjpeg_cfg.range,
             p->mjpeg_cfg.qfactor_min, p->mjpeg_cfg.qfactor_max,
             p->mjpeg_cfg.gop, p->mjpeg_cfg.rc_mode, p->mjpeg_cfg.bps,
             p->mjpeg_cfg.framerate, p->mjpeg_cfg.enc_mode, p->mjpeg_cfg.sei);

    LOG_DEBUG("### dump_mpp_enc_cfg for h264 cfg:\n");
    LOG_DEBUG("gop=%d,rc_mode=%d,framerate=%d,range=%d,head_each_idr=%d \n"
             "sei=%d,qp.init=%d,qp.max=%d,qp.min=%d,qp.step=%d,profile=%d \n"
             "cabac_en=%d,cabac_idc=%d,trans_8x8=%d,level=%d,bps=%d idr_bps=%d \n"
             "vi_len=%d,gop_mode=%d\n",
             p->h264_cfg.gop, p->h264_cfg.rc_mode, p->h264_cfg.framerate,
             p->h264_cfg.range, p->h264_cfg.head_each_idr,
             p->h264_cfg.sei, p->h264_cfg.qp.init,
             p->h264_cfg.qp.max, p->h264_cfg.qp.min,
             p->h264_cfg.qp.step, p->h264_cfg.profile,
             p->h264_cfg.cabac_en, p->h264_cfg.cabac_idc,
             p->h264_cfg.trans_8x8, p->h264_cfg.level,
             p->h264_cfg.bps, p->h264_cfg.idr_bps,
             p->h264_cfg.vi_len, p->h264_cfg.gop_mode);

    LOG_DEBUG("### dump_mpp_enc_cfg for h265 cfg:\n");
    LOG_DEBUG("gop=%d,rc_mode=%d,framerate=%d,range=%d,head_each_idr=%d \n"
             "sei=%d,qp.init=%d,qp.max=%d,qp.min=%d,qp.step=%d,max_i_qp=%d \n"
             "min_i_qp=%d,bps=%d idr_bps=%d vi_len=%d,gop_mode=%d\n",
             p->h265_cfg.gop, p->h265_cfg.rc_mode, p->h265_cfg.framerate,
             p->h265_cfg.range, p->h265_cfg.head_each_idr,
             p->h265_cfg.sei, p->h265_cfg.qp.init,
             p->h265_cfg.qp.max, p->h265_cfg.qp.min,
             p->h265_cfg.qp.step, p->h265_cfg.qp.max_i_qp,
             p->h265_cfg.qp.min_i_qp, p->h265_cfg.bps, p->h265_cfg.idr_bps,
             p->h265_cfg.vi_len, p->h265_cfg.gop_mode);

#if MPP_ENC_OSD_ENABLE
    LOG_DEBUG("### dump_mpp_enc_cfg for osd cfg:\n");
    LOG_DEBUG("osd_enable=%d, count=%d, osd_plt_user=%d\n",
               p->osd_enable, p->osd_count, p->osd_plt_user);
    for (int i = 0; i < OSD_REGIONS_CNT; i++) {
        LOG_DEBUG("enable=%d, set_ok=%d, start_x=%d, start_y=%d, path=%s\n",
                   p->osd_cfg[i].enable, p->osd_cfg[i].set_ok,
                   p->osd_cfg[i].start_x, p->osd_cfg[i].start_y,
                   p->osd_cfg[i].image_path);
    }
#endif

}

static int parse_check_mpp_enc_cfg(cJSON *root, MpiEncTestData *p, bool init)
{
    int ret = 0;
    RK_U32 fps;

    if (NULL == p)
        return -1;

    LOG_DEBUG("parse_mpp_enc_cfg type: %d init:%d\n", p->type, init);
    if (!init)
    {
        p->common_cfg.change = 0;
        p->mjpeg_cfg.change = 0;
        p->h264_cfg.change = 0;
        p->h265_cfg.change = 0;
    }
    cJSON *child = cJSON_GetObjectItem(root, "mpp_enc_cfg");
    if (!child)
    {
        LOG_ERROR("parse_mpp_enc_cfg mpp_enc_cfg err\n");
        return -1;
    }
    cJSON *child_version = cJSON_GetObjectItem(child, "version");
    if (child_version)
    {
        LOG_DEBUG("parse_mpp_enc_cfg version:%s\n", child_version->valuestring);
    }

    cJSON *child_common = cJSON_GetObjectItem(child, "common");
    if (!child_common)
    {
        LOG_ERROR("parse_mpp_enc_cfg common err\n");
        return -1;
    }
    else
    {
        cJSON *child_common_param = NULL;
        if (init)
            child_common_param = cJSON_GetObjectItem(child_common, "param_init");
        else
            child_common_param = cJSON_GetObjectItem(child_common, "param_change");
        if (child_common_param)
        {
            cJSON *child_common_fbc = cJSON_GetObjectItem(child_common_param, "fbc");
            if (child_common_fbc)
            {
                p->common_cfg.fbc = strstr(child_common_fbc->valuestring, "off") ? 0 : MPP_FRAME_FBC_AFBC_V1;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(0);
            }
            cJSON *child_common_split_mode = cJSON_GetObjectItem(child_common_param, "split_mode");
            if (child_common_split_mode)
            {
                p->common_cfg.split_mode = strstr(child_common_split_mode->valuestring, "none") ? MPP_ENC_SPLIT_NONE :
                                           strstr(child_common_split_mode->valuestring, "byte") ? MPP_ENC_SPLIT_BY_BYTE :
                                           strstr(child_common_split_mode->valuestring, "ctu") ? MPP_ENC_SPLIT_BY_CTU : MPP_ENC_SPLIT_NONE;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(1);
            }
            cJSON *child_common_split_arg = cJSON_GetObjectItem(child_common_param, "split_arg");
            if (child_common_split_arg)
            {
                p->common_cfg.split_arg = child_common_split_arg->valueint;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(2);
            }
            cJSON *child_common_idr_count = cJSON_GetObjectItem(child_common_param, "force_idr_count");
            if (child_common_idr_count)
            {
                p->common_cfg.force_idr_count = child_common_idr_count->valueint;
                p->common_cfg.force_idr_count = p->common_cfg.force_idr_count < 0 ? 0 :
                                                p->common_cfg.force_idr_count > 100 ? 100 :
                                                p->common_cfg.force_idr_count;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(3);
            }
            cJSON *child_common_idr_period = cJSON_GetObjectItem(child_common_param, "force_idr_period");
            if (child_common_idr_period)
            {
                p->common_cfg.force_idr_period = child_common_idr_period->valueint;
                p->common_cfg.force_idr_period = p->common_cfg.force_idr_period < 1 ? 1 :
                                                 p->common_cfg.force_idr_period > 100 ? 100 :
                                                 p->common_cfg.force_idr_period;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(4);
            }
            cJSON *child_common_frc_fps = cJSON_GetObjectItem(child_common_param, "frc_fps");
            if (child_common_frc_fps)
            {
                p->common_cfg.frc_fps = child_common_frc_fps->valueint;
                p->common_cfg.frc_fps = p->common_cfg.frc_fps < 0 ? 0 :
                p->common_cfg.frc_fps > 100 ? 100 : p->common_cfg.frc_fps;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(5);
            }
            cJSON *child_common_frc_mode = cJSON_GetObjectItem(child_common_param, "frc_mode");
            if (child_common_frc_mode)
            {
                p->common_cfg.frc_mode = child_common_frc_mode->valueint;
                p->common_cfg.frc_mode = p->common_cfg.frc_mode < FRC_OFF ? FRC_OFF :
                p->common_cfg.frc_mode > FRC_BOTH_UP_LOW ? FRC_BOTH_UP_LOW : p->common_cfg.frc_mode;
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(6);
            }
            cJSON *child_common_rotation = cJSON_GetObjectItem(child_common_param, "rotation");
            if (child_common_rotation)
            {
                switch (child_common_rotation->valueint) {
                    case 0:
                        p->common_cfg.rotation = 0;
                    break;
                    case 90:
                        p->common_cfg.rotation = 1;
                    break;
                    case 180:
                        p->common_cfg.rotation = 2;
                    break;
                    case 270:
                        p->common_cfg.rotation = 3;
                    break;
                    default:
                        LOG_WARN("rotation:%d not support\n", child_common_rotation->valueint);
                        p->common_cfg.rotation = 0;
                    break;
                }
                p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(7); //not support change after enc
            }
            cJSON *child_common_stream_save_dir = cJSON_GetObjectItem(child_common_param, "stream_save_dir");
            if (child_common_stream_save_dir)
            {
                if (strlen(child_common_stream_save_dir->valuestring) >
                    (MPP_STREAM_SAVE_DIR_LEN - strlen(RK_MPP_DEBUG_OUT_FILE) - 1))
                {
                    LOG_ERROR("stream_save_dir:%s, the name too long\n",
                               child_common_stream_save_dir->valuestring);
                    sprintf(p->streamin_save_dir,"/data%s",
                            RK_MPP_DEBUG_IN_FILE);
                    sprintf(p->streamout_save_dir,"/data%s",
                            RK_MPP_DEBUG_OUT_FILE);
                }
                else
                {
                    sprintf(p->streamin_save_dir,"/%s%s",
                            child_common_stream_save_dir->valuestring,
                            RK_MPP_DEBUG_IN_FILE);
                    sprintf(p->streamout_save_dir,"/%s%s",
                            child_common_stream_save_dir->valuestring,
                            RK_MPP_DEBUG_OUT_FILE);
                }
                //p->common_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(7); //no need
            }
            else
            {
                    sprintf(p->streamin_save_dir,"/data%s",
                            RK_MPP_DEBUG_IN_FILE);
                    sprintf(p->streamout_save_dir,"/data%s",
                            RK_MPP_DEBUG_OUT_FILE);
            }

            LOG_INFO("common_cfg.change:0x%x\n", p->common_cfg.change);
        }
        else
        {
            LOG_WARN("no find common param_init or param_change\n");
        }
    }

    fps = (p->common_cfg.frc_fps ? p->common_cfg.frc_fps : p->fps);
    switch (p->type)
    {
    case MPP_VIDEO_CodingMJPEG :
    {
        cJSON *child_mjpeg = cJSON_GetObjectItem(child, "mjpeg");
        if (!child_mjpeg)
        {
            LOG_ERROR("parse_mpp_enc_cfg mjpeg err\n");
            return -1;
        }
        else
        {
            cJSON *child_mjpeg_param = NULL;
            if (init)
                child_mjpeg_param = cJSON_GetObjectItem(child_mjpeg, "param_init");
            else
                child_mjpeg_param = cJSON_GetObjectItem(child_mjpeg, "param_change");
            if (child_mjpeg_param)
            {
                cJSON *child_mjpeg_quant = cJSON_GetObjectItem(child_mjpeg_param, "quant");
                if (child_mjpeg_quant)
                {
                    p->mjpeg_cfg.quant = child_mjpeg_quant->valueint;
                    p->mjpeg_cfg.quant = p->mjpeg_cfg.quant < 1 ? 1 :
                                         p->mjpeg_cfg.quant > 10 ? 10 :
                                         p->mjpeg_cfg.quant;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(0);
                }
                cJSON *child_mjpeg_range = cJSON_GetObjectItem(child_mjpeg_param, "range");
                if (child_mjpeg_range)
                {
                    p->mjpeg_cfg.range = strstr(child_mjpeg_range->valuestring, "limit") ?
                                         MPP_FRAME_RANGE_MPEG : MPP_FRAME_RANGE_JPEG;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(1);
                }

                char resolution_name[15] = "";
                sprintf(resolution_name, "%d*%dp%d",p->width, p->height, fps);
                cJSON *child_mjpeg_resolution_name = cJSON_GetObjectItem(child_mjpeg_param, resolution_name);
                cJSON *child_mjpeg_qfactor = NULL;
                cJSON *child_mjpeg_qfactor_frc_min =  NULL;
                if (child_mjpeg_resolution_name) {
                    child_mjpeg_qfactor = cJSON_GetObjectItem(child_mjpeg_resolution_name, "qfactor");
                    child_mjpeg_qfactor_frc_min = cJSON_GetObjectItem(child_mjpeg_resolution_name, "qfactor_frc_min");
                } else {
                    child_mjpeg_qfactor = cJSON_GetObjectItem(child_mjpeg_param, "qfactor");
                    child_mjpeg_qfactor_frc_min = cJSON_GetObjectItem(child_mjpeg_param, "qfactor_frc_min");
                }
                if (child_mjpeg_qfactor)
                {
                    p->mjpeg_cfg.qfactor = child_mjpeg_qfactor->valueint;
                    p->mjpeg_cfg.qfactor = p->mjpeg_cfg.qfactor < 0 ? 0 :
                                           p->mjpeg_cfg.qfactor > 99 ? 99 :
                                           p->mjpeg_cfg.qfactor;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(2);
                }
                if (child_mjpeg_qfactor_frc_min)
                {
                    p->mjpeg_cfg.qfactor_frc_min = child_mjpeg_qfactor_frc_min->valueint;
                    p->mjpeg_cfg.qfactor_frc_min = p->mjpeg_cfg.qfactor_frc_min < 0 ? 0 :
                                                   p->mjpeg_cfg.qfactor_frc_min > p->mjpeg_cfg.qfactor ? p->mjpeg_cfg.qfactor :
                                                   p->mjpeg_cfg.qfactor_frc_min;
                }

                cJSON *child_mjpeg_qfactor_min = cJSON_GetObjectItem(child_mjpeg_param, "qfactor_min");
                if (child_mjpeg_qfactor_min)
                {
                    p->mjpeg_cfg.qfactor_min = child_mjpeg_qfactor_min->valueint;
                    p->mjpeg_cfg.qfactor_min = p->mjpeg_cfg.qfactor_min < 0 ? 0 :
                                               p->mjpeg_cfg.qfactor_min > p->mjpeg_cfg.qfactor ? p->mjpeg_cfg.qfactor :
                                               p->mjpeg_cfg.qfactor_min;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(3);
                }
                cJSON *child_mjpeg_qfactor_max = cJSON_GetObjectItem(child_mjpeg_param, "qfactor_max");
                if (child_mjpeg_qfactor_max)
                {
                    p->mjpeg_cfg.qfactor_max = child_mjpeg_qfactor_max->valueint;
                    p->mjpeg_cfg.qfactor_max = p->mjpeg_cfg.qfactor_max < p->mjpeg_cfg.qfactor ? p->mjpeg_cfg.qfactor :
                                               p->mjpeg_cfg.qfactor_max > 99 ? 99 :
                                               p->mjpeg_cfg.qfactor_max;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(4);
                }
                cJSON *child_mjpeg_gop = cJSON_GetObjectItem(child_mjpeg_param, "gop");
                if (child_mjpeg_gop)
                {
                    p->mjpeg_cfg.gop = child_mjpeg_gop->valueint;
                    p->mjpeg_cfg.gop = p->mjpeg_cfg.gop < 1 ? 1 :
                                       p->mjpeg_cfg.gop > 100 ? 100 :
                                       p->mjpeg_cfg.gop;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(5);
                }
#if MPP_ENC_MJPEG_FRC_USE_MPP
                cJSON *child_mjpeg_rc_mode = cJSON_GetObjectItem(child_mjpeg_param, "rc_mode");
                if (child_mjpeg_rc_mode)
                {
                    p->mjpeg_cfg.rc_mode = strstr(child_mjpeg_rc_mode->valuestring, "cbr") ? MPP_ENC_RC_MODE_CBR :
                                           strstr(child_mjpeg_rc_mode->valuestring, "vbr") ? MPP_ENC_RC_MODE_VBR :
                                           strstr(child_mjpeg_rc_mode->valuestring, "fixqp") ? MPP_ENC_RC_MODE_FIXQP : MPP_ENC_RC_MODE_CBR;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(6);
                }
#endif
                cJSON *child_mjpeg_bps = cJSON_GetObjectItem(child_mjpeg_param, "bps");
                if (child_mjpeg_bps)
                {
                    p->mjpeg_cfg.bps = child_mjpeg_bps->valueint;
                    p->mjpeg_cfg.bps = p->mjpeg_cfg.bps < MPP_ENC_CFG_MIN_BPS ? MPP_ENC_CFG_MIN_BPS :
                                       p->mjpeg_cfg.bps > MPP_ENC_MJPEG_CFG_MAX_BPS ? MPP_ENC_MJPEG_CFG_MAX_BPS :
                                       p->mjpeg_cfg.bps;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(7);
                }
                cJSON *child_mjpeg_framerate = cJSON_GetObjectItem(child_mjpeg_param, "framerate");
                if (child_mjpeg_framerate)
                {
                    p->mjpeg_cfg.framerate = child_mjpeg_framerate->valueint;
                    p->mjpeg_cfg.framerate = p->mjpeg_cfg.framerate < MPP_ENC_CFG_MIN_FPS ? MPP_ENC_CFG_MIN_FPS :
                                             p->mjpeg_cfg.framerate > MPP_ENC_CFG_MAX_FPS ? MPP_ENC_CFG_MAX_FPS :
                                             p->mjpeg_cfg.framerate;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(8);
                }
                cJSON *child_mjpeg_enc_mode = cJSON_GetObjectItem(child_mjpeg_param, "enc_mode");
                if (child_mjpeg_enc_mode)
                {
                    p->mjpeg_cfg.enc_mode = child_mjpeg_enc_mode->valueint;
                    p->mjpeg_cfg.enc_mode = p->mjpeg_cfg.enc_mode < 0 ? 0 :
                                             p->mjpeg_cfg.enc_mode > 3 ? 3 :
                                             p->mjpeg_cfg.enc_mode;
                   // p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(9); //not need.
                }
                cJSON *child_mjpeg_sei = cJSON_GetObjectItem(child_mjpeg_param, "sei");
                if (child_mjpeg_sei)
                {
                    p->mjpeg_cfg.sei = strstr(child_mjpeg_sei->valuestring, "SEQ") ? MPP_ENC_SEI_MODE_ONE_SEQ :
                                       strstr(child_mjpeg_sei->valuestring, "FRAME") ? MPP_ENC_SEI_MODE_ONE_FRAME :
                                       MPP_ENC_SEI_MODE_DISABLE;
                    p->mjpeg_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(11);
                }

                LOG_INFO("mjpeg_cfg.change:0x%x\n", p->mjpeg_cfg.change);
            }
            else
            {
                LOG_WARN("no find mjpeg param_init or param_change\n");
            }
        }
    }
    break;
    case MPP_VIDEO_CodingAVC :
    {
        cJSON *child_h264 = cJSON_GetObjectItem(child, "h264");
        if (!child_h264)
        {
            LOG_ERROR("parse_mpp_enc_cfg h264 err\n");
            return -1;
        }
        else
        {
            cJSON *child_h264_param = NULL;
            if (init)
                child_h264_param = cJSON_GetObjectItem(child_h264, "param_init");
            else
                child_h264_param = cJSON_GetObjectItem(child_h264, "param_change");
            if (child_h264_param)
            {
                cJSON *child_h264_gop = cJSON_GetObjectItem(child_h264_param, "gop");
                if (child_h264_gop)
                {
                    p->h264_cfg.gop = child_h264_gop->valueint;
                    p->h264_cfg.gop = p->h264_cfg.gop < 1 ? 1 :
                                      p->h264_cfg.gop > 1000 ? 1000 :
                                      p->h264_cfg.gop;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(0);
                }
                cJSON *child_h264_rc_mode = cJSON_GetObjectItem(child_h264_param, "rc_mode");
                if (child_h264_rc_mode)
                {
                    p->h264_cfg.rc_mode = strstr(child_h264_rc_mode->valuestring, "cbr") ? MPP_ENC_RC_MODE_CBR :
                                          strstr(child_h264_rc_mode->valuestring, "avbr") ? MPP_ENC_RC_MODE_AVBR :
                                          strstr(child_h264_rc_mode->valuestring, "vbr") ? MPP_ENC_RC_MODE_VBR :
                                          strstr(child_h264_rc_mode->valuestring, "fixqp") ? MPP_ENC_RC_MODE_FIXQP : MPP_ENC_RC_MODE_CBR;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(1);
                }
                cJSON *child_h264_framerate = cJSON_GetObjectItem(child_h264_param, "framerate");
                if (child_h264_framerate)
                {
                    p->h264_cfg.framerate = child_h264_framerate->valueint;
                    p->h264_cfg.framerate = p->h264_cfg.framerate < MPP_ENC_CFG_MIN_FPS ? MPP_ENC_CFG_MIN_FPS :
                                            p->h264_cfg.framerate > MPP_ENC_CFG_MAX_FPS ? MPP_ENC_CFG_MAX_FPS :
                                            p->h264_cfg.framerate;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(2);
                }
                cJSON *child_h264_range = cJSON_GetObjectItem(child_h264_param, "range");
                if (child_h264_range)
                {
                    p->h264_cfg.range = strstr(child_h264_range->valuestring, "limit") ?
                                        MPP_FRAME_RANGE_MPEG : MPP_FRAME_RANGE_JPEG;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(3);
                }
                cJSON *child_h264_head_each_idr = cJSON_GetObjectItem(child_h264_param, "head_each_idr");
                if (child_h264_head_each_idr)
                {
                    p->h264_cfg.head_each_idr = strstr(child_h264_head_each_idr->valuestring, "on") ?
                                                MPP_ENC_HEADER_MODE_EACH_IDR : MPP_ENC_HEADER_MODE_DEFAULT;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(4);
                }
                cJSON *child_h264_sei = cJSON_GetObjectItem(child_h264_param, "sei");
                if (child_h264_sei)
                {
                    p->h264_cfg.sei = strstr(child_h264_sei->valuestring, "SEQ") ? MPP_ENC_SEI_MODE_ONE_SEQ :
                                      strstr(child_h264_sei->valuestring, "FRAME") ? MPP_ENC_SEI_MODE_ONE_FRAME :
                                      MPP_ENC_SEI_MODE_DISABLE;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(5);
                }
                cJSON *child_h264_qp_init = cJSON_GetObjectItem(child_h264_param, "qp_init");
                if (child_h264_qp_init)
                {
                    p->h264_cfg.qp.init = child_h264_qp_init->valueint;
                    p->h264_cfg.qp.init = p->h264_cfg.qp.init < 1 ? 1 :
                                          p->h264_cfg.qp.init > 51 ? 51 :
                                          p->h264_cfg.qp.init;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(6);
                }
                cJSON *child_h264_qp_max = cJSON_GetObjectItem(child_h264_param, "qp_max");
                if (child_h264_qp_max)
                {
                    p->h264_cfg.qp.max = child_h264_qp_max->valueint;
                    p->h264_cfg.qp.max = p->h264_cfg.qp.max < 8 ? 8 :
                                         p->h264_cfg.qp.max > 51 ? 51 :
                                         p->h264_cfg.qp.max;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(7);
                }
                cJSON *child_h264_qp_min = cJSON_GetObjectItem(child_h264_param, "qp_min");
                if (child_h264_qp_min)
                {
                    p->h264_cfg.qp.min = child_h264_qp_min->valueint;
                    p->h264_cfg.qp.min = p->h264_cfg.qp.min < 0 ? 0 :
                                         p->h264_cfg.qp.min > 48 ? 48 :
                                         p->h264_cfg.qp.min;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(8);
                }
                cJSON *child_h264_qp_step = cJSON_GetObjectItem(child_h264_param, "qp_step");
                if (child_h264_qp_step)
                {
                    p->h264_cfg.qp.step = child_h264_qp_step->valueint;
                    p->h264_cfg.qp.step = p->h264_cfg.qp.step < 1 ? 1 :
                                          p->h264_cfg.qp.step > 51 ? 51 :
                                          p->h264_cfg.qp.step;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(9);
                }
                cJSON *child_h264_profile = cJSON_GetObjectItem(child_h264_param, "profile");
                if (child_h264_profile)
                {
                    p->h264_cfg.profile = child_h264_profile->valueint;
                    if (p->h264_cfg.profile != 66 && p->h264_cfg.profile != 77 && p->h264_cfg.profile != 100)
                    {
                        LOG_WARN("set h264_cfg.profile err %d, set default to %d\n",
                                 p->h264_cfg.profile, MPP_ENC_CFG_H264_DEFAULT_PROFILE);
                        p->h264_cfg.profile = MPP_ENC_CFG_H264_DEFAULT_PROFILE;
                    }
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(10);
                }
                cJSON *child_h264_cabac_en = cJSON_GetObjectItem(child_h264_param, "cabac_en");
                if (child_h264_cabac_en)
                {
                    p->h264_cfg.cabac_en = child_h264_cabac_en->valueint;
                    p->h264_cfg.cabac_en = p->h264_cfg.cabac_en < 0 ? 0 :
                                           p->h264_cfg.cabac_en > 1 ? 1 :
                                           p->h264_cfg.cabac_en;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(11);
                }
                cJSON *child_h264_cabac_idc = cJSON_GetObjectItem(child_h264_param, "cabac_idc");
                if (child_h264_cabac_idc)
                {
                    p->h264_cfg.cabac_idc = child_h264_cabac_idc->valueint;
                    p->h264_cfg.cabac_idc = p->h264_cfg.cabac_idc < 0 ? 0 :
                                            p->h264_cfg.cabac_idc > 1 ? 1 :
                                            p->h264_cfg.cabac_idc;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(12);
                }
                cJSON *child_h264_trans_8x8 = cJSON_GetObjectItem(child_h264_param, "trans_8x8");
                if (child_h264_trans_8x8)
                {
                    p->h264_cfg.trans_8x8 = child_h264_trans_8x8->valueint;
                    p->h264_cfg.trans_8x8 = p->h264_cfg.trans_8x8 < 0 ? 0 :
                                            p->h264_cfg.trans_8x8 > 1 ? 1 :
                                            p->h264_cfg.trans_8x8;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(13);
                }
                cJSON *child_h264_level = cJSON_GetObjectItem(child_h264_param, "level");
                if (child_h264_level)
                {
                    p->h264_cfg.level = child_h264_level->valueint;
                    if (!((p->h264_cfg.level >= 10 && p->h264_cfg.level <= 13) ||
                            (p->h264_cfg.level >= 20 && p->h264_cfg.level <= 22) ||
                            (p->h264_cfg.level >= 30 && p->h264_cfg.level <= 32) ||
                            (p->h264_cfg.level >= 40 && p->h264_cfg.level <= 42) ||
                            (p->h264_cfg.level >= 50 && p->h264_cfg.level <= 52)))
                    {
                        LOG_WARN("set h264_cfg.level err %d, set default to %d\n",
                                 p->h264_cfg.level, MPP_ENC_CFG_H264_DEFAULT_LEVEL);
                        p->h264_cfg.level = MPP_ENC_CFG_H264_DEFAULT_LEVEL;
                    }
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(14);
                }
                char resolution_name[15] = "";
                sprintf(resolution_name, "%d*%dp%d",p->width, p->height, fps);
                cJSON *child_h264_bps = NULL;
                cJSON *child_h264_force_idr_bps = NULL;
                cJSON *child_h264_resolution_name = cJSON_GetObjectItem(child_h264_param, resolution_name);
                if (child_h264_resolution_name) {
                    child_h264_bps = cJSON_GetObjectItem(child_h264_resolution_name, "bps");
                    child_h264_force_idr_bps = cJSON_GetObjectItem(child_h264_resolution_name, "force_idr_bps");
                } else {
                    child_h264_bps = cJSON_GetObjectItem(child_h264_param, "bps");
                    child_h264_force_idr_bps = cJSON_GetObjectItem(child_h264_param, "force_idr_bps");
                }

                if (child_h264_bps)
                {
                    p->h264_cfg.bps = child_h264_bps->valueint;
                    p->h264_cfg.bps = p->h264_cfg.bps < MPP_ENC_CFG_MIN_BPS ? MPP_ENC_CFG_MIN_BPS :
                                      p->h264_cfg.bps > MPP_ENC_CFG_MAX_BPS ? MPP_ENC_CFG_MAX_BPS :
                                      p->h264_cfg.bps;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(15);
                }
                if (child_h264_force_idr_bps)
                {
                    p->h264_cfg.idr_bps = child_h264_force_idr_bps->valueint;
                    p->h264_cfg.idr_bps = p->h264_cfg.idr_bps < MPP_ENC_CFG_MIN_BPS ? MPP_ENC_CFG_MIN_BPS :
                                          p->h264_cfg.idr_bps > MPP_ENC_CFG_MAX_BPS ? MPP_ENC_CFG_MAX_BPS :
                                          p->h264_cfg.idr_bps;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(16);
                }
                cJSON *child_h264_vi_eln = cJSON_GetObjectItem(child_h264_param, "vi_len");
                if (child_h264_vi_eln)
                {
                    p->h264_cfg.vi_len = child_h264_vi_eln->valueint;
                    p->h264_cfg.vi_len = p->h264_cfg.vi_len < 0 ? 0 :
                                         p->h264_cfg.vi_len;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(17);
                }
                cJSON *child_h264_gop_mode = cJSON_GetObjectItem(child_h264_param, "gop_mode");
                if (child_h264_gop_mode)
                {
                    p->h264_cfg.gop_mode = child_h264_gop_mode->valueint;
                    p->h264_cfg.gop_mode = p->h264_cfg.gop_mode < GOP_MODE_NORMALP ? GOP_MODE_NORMALP :
                                           p->h264_cfg.gop_mode > GOP_MODE_SMARTP ? GOP_MODE_SMARTP :
                                           p->h264_cfg.gop_mode;
                    p->h264_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(18);
                }

                LOG_INFO("h264_cfg.change:0x%x\n", p->h264_cfg.change);
            }
            else
            {
                LOG_WARN("no find h264 param_init or param_change\n");
            }
        }
    }
    break;
    case MPP_VIDEO_CodingVP8 :
    {
        ret = -1;
    }
    break;
    case MPP_VIDEO_CodingHEVC :
    {
        cJSON *child_h265 = cJSON_GetObjectItem(child, "h265");
        if (!child_h265)
        {
            LOG_ERROR("parse_mpp_enc_cfg h265 err\n");
            return -1;
        }
        else
        {
            cJSON *child_h265_param = NULL;
            if (init)
                child_h265_param = cJSON_GetObjectItem(child_h265, "param_init");
            else
                child_h265_param = cJSON_GetObjectItem(child_h265, "param_change");
            if (child_h265_param)
            {
                cJSON *child_h265_gop = cJSON_GetObjectItem(child_h265_param, "gop");
                if (child_h265_gop)
                {
                    p->h265_cfg.gop = child_h265_gop->valueint;
                    p->h265_cfg.gop = p->h265_cfg.gop < 1 ? 1 :
                                      p->h265_cfg.gop > 1000 ? 1000 :
                                      p->h265_cfg.gop;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(0);
                }
                cJSON *child_h265_rc_mode = cJSON_GetObjectItem(child_h265_param, "rc_mode");
                if (child_h265_rc_mode)
                {
                    p->h265_cfg.rc_mode = strstr(child_h265_rc_mode->valuestring, "cbr") ? MPP_ENC_RC_MODE_CBR :
                                          strstr(child_h265_rc_mode->valuestring, "avbr") ? MPP_ENC_RC_MODE_AVBR :
                                          strstr(child_h265_rc_mode->valuestring, "vbr") ? MPP_ENC_RC_MODE_VBR :
                                          strstr(child_h265_rc_mode->valuestring, "fixqp") ? MPP_ENC_RC_MODE_FIXQP : MPP_ENC_RC_MODE_CBR;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(1);
                }
                cJSON *child_h265_framerate = cJSON_GetObjectItem(child_h265_param, "framerate");
                if (child_h265_framerate)
                {
                    p->h265_cfg.framerate = child_h265_framerate->valueint;
                    p->h265_cfg.framerate = p->h265_cfg.framerate < MPP_ENC_CFG_MIN_FPS ? MPP_ENC_CFG_MIN_FPS :
                                            p->h265_cfg.framerate > MPP_ENC_CFG_MAX_FPS ? MPP_ENC_CFG_MAX_FPS :
                                            p->h265_cfg.framerate;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(2);
                }
                cJSON *child_h265_range = cJSON_GetObjectItem(child_h265_param, "range");
                if (child_h265_range)
                {
                    p->h265_cfg.range = strstr(child_h265_range->valuestring, "limit") ?
                                        MPP_FRAME_RANGE_MPEG : MPP_FRAME_RANGE_JPEG;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(3);
                }
                cJSON *child_h265_head_each_idr = cJSON_GetObjectItem(child_h265_param, "head_each_idr");
                if (child_h265_head_each_idr)
                {
                    p->h265_cfg.head_each_idr = strstr(child_h265_head_each_idr->valuestring, "on") ?
                                                MPP_ENC_HEADER_MODE_EACH_IDR : MPP_ENC_HEADER_MODE_DEFAULT;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(4);
                }
                cJSON *child_h265_sei = cJSON_GetObjectItem(child_h265_param, "sei");
                if (child_h265_sei)
                {
                    p->h265_cfg.sei = strstr(child_h265_sei->valuestring, "SEQ") ? MPP_ENC_SEI_MODE_ONE_SEQ :
                                      strstr(child_h265_sei->valuestring, "FRAME") ? MPP_ENC_SEI_MODE_ONE_FRAME :
                                      MPP_ENC_SEI_MODE_DISABLE;;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(5);
                }
                cJSON *child_h265_qp_init = cJSON_GetObjectItem(child_h265_param, "qp_init");
                if (child_h265_qp_init)
                {
                    p->h265_cfg.qp.init = child_h265_qp_init->valueint;
                    p->h265_cfg.qp.init = p->h265_cfg.qp.init < 1 ? 1 :
                                          p->h265_cfg.qp.init > 51 ? 51 :
                                          p->h265_cfg.qp.init;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(6);
                }
                cJSON *child_h265_qp_max = cJSON_GetObjectItem(child_h265_param, "qp_max");
                if (child_h265_qp_max)
                {
                    p->h265_cfg.qp.max = child_h265_qp_max->valueint;
                    p->h265_cfg.qp.max = p->h265_cfg.qp.max < 8 ? 8 :
                                         p->h265_cfg.qp.max > 51 ? 51 :
                                         p->h265_cfg.qp.max;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(7);
                }
                cJSON *child_h265_qp_min = cJSON_GetObjectItem(child_h265_param, "qp_min");
                if (child_h265_qp_min)
                {
                    p->h265_cfg.qp.min = child_h265_qp_min->valueint;
                    p->h265_cfg.qp.min = p->h265_cfg.qp.min < 0 ? 0 :
                                         p->h265_cfg.qp.min > 48 ? 48 :
                                         p->h265_cfg.qp.min;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(8);
                }
                cJSON *child_h265_qp_step = cJSON_GetObjectItem(child_h265_param, "qp_step");
                if (child_h265_qp_step)
                {
                    p->h265_cfg.qp.step = child_h265_qp_step->valueint;
                    p->h265_cfg.qp.step = p->h265_cfg.qp.step < 1 ? 1 :
                                          p->h265_cfg.qp.step > 51 ? 51 :
                                          p->h265_cfg.qp.step;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(9);
                }
                cJSON *child_h265_qp_max_i = cJSON_GetObjectItem(child_h265_param, "max_i_qp");
                if (child_h265_qp_max_i)
                {
                    p->h265_cfg.qp.max_i_qp = child_h265_qp_max_i->valueint;
                    p->h265_cfg.qp.max_i_qp = p->h265_cfg.qp.max_i_qp < 8 ? 8 :
                                              p->h265_cfg.qp.max_i_qp > 51 ? 51 :
                                              p->h265_cfg.qp.max_i_qp;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(10);
                }
                cJSON *child_h265_qp_min_i = cJSON_GetObjectItem(child_h265_param, "min_i_qp");
                if (child_h265_qp_min_i)
                {
                    p->h265_cfg.qp.min_i_qp = child_h265_qp_min_i->valueint;
                    p->h265_cfg.qp.min_i_qp = p->h265_cfg.qp.min_i_qp < 0 ? 0 :
                                              p->h265_cfg.qp.min_i_qp > 48 ? 48 :
                                              p->h265_cfg.qp.min_i_qp;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(11);
                }
                char resolution_name[15] = "";
                sprintf(resolution_name, "%d*%dp%d",p->width, p->height, fps);
                cJSON *child_h265_bps = NULL;
                cJSON *child_h265_force_idr_bps = NULL;
                cJSON *child_h265_resolution_name = cJSON_GetObjectItem(child_h265_param, resolution_name);
                if (child_h265_resolution_name) {
                    child_h265_bps = cJSON_GetObjectItem(child_h265_resolution_name, "bps");
                    child_h265_force_idr_bps = cJSON_GetObjectItem(child_h265_resolution_name, "force_idr_bps");
                } else {
                    child_h265_bps = cJSON_GetObjectItem(child_h265_param, "bps");
                    child_h265_force_idr_bps = cJSON_GetObjectItem(child_h265_param, "force_idr_bps");
                }

                if (child_h265_bps)
                {
                    p->h265_cfg.bps = child_h265_bps->valueint;
                    p->h265_cfg.bps = p->h265_cfg.bps < MPP_ENC_CFG_MIN_BPS ? MPP_ENC_CFG_MIN_BPS :
                                      p->h265_cfg.bps > MPP_ENC_CFG_MAX_BPS ? MPP_ENC_CFG_MAX_BPS :
                                      p->h265_cfg.bps;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(12);
                }
                if (child_h265_force_idr_bps)
                {
                    p->h265_cfg.idr_bps = child_h265_force_idr_bps->valueint;
                    p->h265_cfg.idr_bps = p->h265_cfg.idr_bps < MPP_ENC_CFG_MIN_BPS ? MPP_ENC_CFG_MIN_BPS :
                                          p->h265_cfg.idr_bps > MPP_ENC_CFG_MAX_BPS ? MPP_ENC_CFG_MAX_BPS :
                                          p->h265_cfg.idr_bps;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(13);
                }
                cJSON *child_h265_vi_eln = cJSON_GetObjectItem(child_h265_param, "vi_len");
                if (child_h265_vi_eln)
                {
                    p->h265_cfg.vi_len = child_h265_vi_eln->valueint;
                    p->h265_cfg.vi_len = p->h265_cfg.vi_len < 0 ? 0 :
                                         p->h265_cfg.vi_len;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(14);
                }
                cJSON *child_h265_gop_mode = cJSON_GetObjectItem(child_h265_param, "gop_mode");
                if (child_h265_gop_mode)
                {
                    p->h265_cfg.gop_mode = child_h265_gop_mode->valueint;
                    p->h265_cfg.gop_mode = p->h265_cfg.gop_mode < GOP_MODE_NORMALP ? GOP_MODE_NORMALP :
                                           p->h265_cfg.gop_mode > GOP_MODE_SMARTP ? GOP_MODE_SMARTP :
                                           p->h265_cfg.gop_mode;
                    p->h265_cfg.change |= MPP_ENC_CFG_CHANGE_BIT(15);
                }

                LOG_INFO("h265_cfg.change:0x%x\n", p->h265_cfg.change);
            }
            else
            {
                LOG_WARN("no find h265 param_init or param_change\n");
            }
        }
    }
    break;
    case 0:
    break;
    default :
    {
        LOG_ERROR("unsupport encoder coding type %d\n", p->type);
        ret = -1;
    }
    break;
    }


#if MPP_ENC_OSD_ENABLE
    cJSON *child_osd = cJSON_GetObjectItem(child, "osd");
    if (!child_osd)
    {
        LOG_INFO("no osd info\n");
    }
    else
    {
        cJSON *child_osd_enable = cJSON_GetObjectItem(child_osd, "enable");
        if (child_osd_enable)
        {
            p->osd_enable = strstr(child_osd_enable->valuestring, "on") ?
                                   true : false;
        }
        cJSON *child_osd_count = cJSON_GetObjectItem(child_osd, "count");
        if (child_osd_count)
        {
            p->osd_count = child_osd_count->valueint;
            p->osd_count = p->osd_count < 0 ? 0 :
                           p->osd_count > OSD_REGIONS_CNT ? OSD_REGIONS_CNT :
                           p->osd_count;
        }
        cJSON *child_osd_plt_user = cJSON_GetObjectItem(child_osd, "plt_user");
        if (child_osd_plt_user)
        {
            p->osd_plt_user = child_osd_plt_user->valueint;
        }

        char osd_name[6] = "osd_0";
        for (int i = 0; i < p->osd_count; i ++)
        {
            osd_name[4] = i + 48;
            cJSON *child_osd_index = cJSON_GetObjectItem(child_osd, osd_name);
            if (child_osd_index)
            {
                p->osd_cfg[i].set_ok = true;
                cJSON *child_osd_index_type = cJSON_GetObjectItem(child_osd_index, "type");;
                if (child_osd_index_type)
                {
                     p->osd_cfg[i].type = strstr(child_osd_index_type->valuestring, "picture") ?
                                                 OSD_REGION_TYPE_PICTURE : OSD_REGION_TYPE_PICTURE;
                }
                cJSON *child_osd_index_enable = cJSON_GetObjectItem(child_osd_index, "enable");
                if (child_osd_index_enable)
                {
                     p->osd_cfg[i].enable = strstr(child_osd_index_enable->valuestring, "on") ?
                                                 true : false;
                }

                char resolution_name[10] = "";
                sprintf(resolution_name, "%d*%d",p->width, p->height);
                cJSON *child_osd_resolution_name = cJSON_GetObjectItem(child_osd_index, resolution_name);
                if (!child_osd_resolution_name) {
                    child_osd_resolution_name = cJSON_GetObjectItem(child_osd_index, "common");
                }
                if (child_osd_resolution_name)
                {
                    cJSON *child_osd_index_path = cJSON_GetObjectItem(child_osd_resolution_name, "path");
                    if (child_osd_index_path)
                    {
                        if (strlen(child_osd_index_path->valuestring) >
                            (MPP_ENC_OSD_IMAGE_PATH_LEN - 1))
                        {
                            LOG_ERROR("osd img path:%s, the name too long\n",
                                       child_osd_index_path->valuestring);
                            p->osd_cfg[i].enable = false;
                            p->osd_cfg[i].set_ok = false;
                        }
                        else
                        {
                            sprintf(p->osd_cfg[i].image_path, "%s",
                                    child_osd_index_path->valuestring);
                        }
                    }
                    cJSON *child_osd_index_start_x = cJSON_GetObjectItem(child_osd_resolution_name, "start_x");
                    if (child_osd_index_start_x)
                    {
                         p->osd_cfg[i].start_x = UPALIGNTO16((int)(child_osd_index_start_x->valuedouble * p->width));
                    }
                    cJSON *child_osd_index_start_y = cJSON_GetObjectItem(child_osd_resolution_name, "start_y");
                    if (child_osd_index_start_y)
                    {
                         p->osd_cfg[i].start_y = UPALIGNTO16((int)(child_osd_index_start_y->valuedouble * p->height));
                    }
                }
                else
                {
                    LOG_INFO("no such resolution_name:%s && common path\n", resolution_name);
                }
            }
            else
            {
                LOG_INFO("no such osd %s\n", osd_name);
            }
        }
    }
#endif

    return ret;
}

static MPP_RET mpp_enc_bps_set(MpiEncTestData *p, RK_U32 bps)
{
    MPP_RET ret = MPP_OK;

    switch (p->type)
    {
        case MPP_VIDEO_CodingMJPEG:
        {
            switch (p->mjpeg_cfg.rc_mode)
            {
                case MPP_ENC_RC_MODE_FIXQP :
                {
                    /* do not set bps on fix qp mode */
                } break;
                case MPP_ENC_RC_MODE_CBR :
                {
                    /* CBR mode has narrow bound */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 15 / 16);
                    p->mjpeg_cfg.frc_bps = bps;
                }
                break;
                case MPP_ENC_RC_MODE_VBR :
                {
                    /* CBR mode has wide bound */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 1 / 16);
                    p->mjpeg_cfg.frc_bps = bps;
                }
                break;
                default :
                {
                    LOG_ERROR("unsupport encoder rc mode %d\n", p->mjpeg_cfg.rc_mode);
                }
                break;
            }
        }
        break;
        case MPP_VIDEO_CodingAVC:
        {
            switch (p->h264_cfg.rc_mode)
            {
                case MPP_ENC_RC_MODE_FIXQP :
                {
                    /* do not set bps on fix qp mode */
                } break;
                case MPP_ENC_RC_MODE_CBR :
                {
                    /* CBR mode has narrow bound */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 15 / 16);
                }
                break;
                case MPP_ENC_RC_MODE_VBR :
                {
                    /* VBR mode has wide bound */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 1 / 16);
                }
                break;
                case MPP_ENC_RC_MODE_AVBR :
                {
                    /* AVBR mode min is mean the silence bps, do not too low */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 1 / 4);
                }
                break;
                default :
                {
                    LOG_ERROR("unsupport encoder rc mode %d\n", p->h264_cfg.rc_mode);
                }
                break;
            }
        }
        break;
        case MPP_VIDEO_CodingHEVC:
        {
            switch (p->h265_cfg.rc_mode)
            {
                case MPP_ENC_RC_MODE_FIXQP :
                {
                    /* do not set bps on fix qp mode */
                } break;
                case MPP_ENC_RC_MODE_CBR :
                {
                    /* CBR mode has narrow bound */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 15 / 16);
                }
                break;
                case MPP_ENC_RC_MODE_VBR :
                {
                    /* CBR mode has wide bound */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 1 / 16);
                }
                break;
                case MPP_ENC_RC_MODE_AVBR :
                {
                    /* AVBR mode min is mean the silence bps, do not too low */
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_target", bps);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_max", bps * 17 / 16);
                    mpp_enc_cfg_set_s32(p->cfg, "rc:bps_min", bps * 1 / 4);
                }
                break;
                default :
                {
                    LOG_ERROR("unsupport encoder rc mode %d\n", p->h265_cfg.rc_mode);
                }
                break;
            }
        }
        break;
    }
    return ret;
}

static MPP_RET mpi_enc_gen_ref_cfg(MppEncRefCfg ref, MpiEncGopMode gop_mode)
{
    MppEncRefLtFrmCfg lt_ref[4];
    MppEncRefStFrmCfg st_ref[16];
    RK_S32 lt_cnt = 0;
    RK_S32 st_cnt = 0;
    MPP_RET ret = MPP_OK;

    memset(&lt_ref, 0, sizeof(lt_ref));
    memset(&st_ref, 0, sizeof(st_ref));

    switch (gop_mode) {
    case GOP_MODE_TSVC4: {
        // tsvc4
        //      /-> P1      /-> P3        /-> P5      /-> P7
        //     /           /             /           /
        //    //--------> P2            //--------> P6
        //   //                        //
        //  ///---------------------> P4
        // ///
        // P0 ------------------------------------------------> P8
        lt_cnt = 1;

        /* set 8 frame lt-ref gap */
        lt_ref[0].lt_idx        = 0;
        lt_ref[0].temporal_id   = 0;
        lt_ref[0].ref_mode      = REF_TO_PREV_LT_REF;
        lt_ref[0].lt_gap        = 8;
        lt_ref[0].lt_delay      = 0;

        st_cnt = 9;
        /* set tsvc4 st-ref struct */
        /* st 0 layer 0 - ref */
        st_ref[0].is_non_ref    = 0;
        st_ref[0].temporal_id   = 0;
        st_ref[0].ref_mode      = REF_TO_TEMPORAL_LAYER;
        st_ref[0].ref_arg       = 0;
        st_ref[0].repeat        = 0;
        /* st 1 layer 3 - non-ref */
        st_ref[1].is_non_ref    = 1;
        st_ref[1].temporal_id   = 3;
        st_ref[1].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[1].ref_arg       = 0;
        st_ref[1].repeat        = 0;
        /* st 2 layer 2 - ref */
        st_ref[2].is_non_ref    = 0;
        st_ref[2].temporal_id   = 2;
        st_ref[2].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[2].ref_arg       = 0;
        st_ref[2].repeat        = 0;
        /* st 3 layer 3 - non-ref */
        st_ref[3].is_non_ref    = 1;
        st_ref[3].temporal_id   = 3;
        st_ref[3].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[3].ref_arg       = 0;
        st_ref[3].repeat        = 0;
        /* st 4 layer 1 - ref */
        st_ref[4].is_non_ref    = 0;
        st_ref[4].temporal_id   = 1;
        st_ref[4].ref_mode      = REF_TO_PREV_LT_REF;
        st_ref[4].ref_arg       = 0;
        st_ref[4].repeat        = 0;
        /* st 5 layer 3 - non-ref */
        st_ref[5].is_non_ref    = 1;
        st_ref[5].temporal_id   = 3;
        st_ref[5].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[5].ref_arg       = 0;
        st_ref[5].repeat        = 0;
        /* st 6 layer 2 - ref */
        st_ref[6].is_non_ref    = 0;
        st_ref[6].temporal_id   = 2;
        st_ref[6].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[6].ref_arg       = 0;
        st_ref[6].repeat        = 0;
        /* st 7 layer 3 - non-ref */
        st_ref[7].is_non_ref    = 1;
        st_ref[7].temporal_id   = 3;
        st_ref[7].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[7].ref_arg       = 0;
        st_ref[7].repeat        = 0;
        /* st 8 layer 0 - ref */
        st_ref[8].is_non_ref    = 0;
        st_ref[8].temporal_id   = 0;
        st_ref[8].ref_mode      = REF_TO_TEMPORAL_LAYER;
        st_ref[8].ref_arg       = 0;
        st_ref[8].repeat        = 0;
    } break;
    case GOP_MODE_TSVC3: {
        // tsvc3
        //     /-> P1      /-> P3
        //    /           /
        //   //--------> P2
        //  //
        // P0/---------------------> P4
        lt_cnt = 0;

        st_cnt = 5;
        /* set tsvc4 st-ref struct */
        /* st 0 layer 0 - ref */
        st_ref[0].is_non_ref    = 0;
        st_ref[0].temporal_id   = 0;
        st_ref[0].ref_mode      = REF_TO_TEMPORAL_LAYER;
        st_ref[0].ref_arg       = 0;
        st_ref[0].repeat        = 0;
        /* st 1 layer 2 - non-ref */
        st_ref[1].is_non_ref    = 1;
        st_ref[1].temporal_id   = 2;
        st_ref[1].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[1].ref_arg       = 0;
        st_ref[1].repeat        = 0;
        /* st 2 layer 1 - ref */
        st_ref[2].is_non_ref    = 0;
        st_ref[2].temporal_id   = 1;
        st_ref[2].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[2].ref_arg       = 0;
        st_ref[2].repeat        = 0;
        /* st 3 layer 2 - non-ref */
        st_ref[3].is_non_ref    = 1;
        st_ref[3].temporal_id   = 2;
        st_ref[3].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[3].ref_arg       = 0;
        st_ref[3].repeat        = 0;
        /* st 4 layer 0 - ref */
        st_ref[4].is_non_ref    = 0;
        st_ref[4].temporal_id   = 0;
        st_ref[4].ref_mode      = REF_TO_TEMPORAL_LAYER;
        st_ref[4].ref_arg       = 0;
        st_ref[4].repeat        = 0;
    } break;
    case GOP_MODE_TSVC2: {
        // tsvc2
        //   /-> P1
        //  /
        // P0--------> P2
        lt_cnt = 0;

        st_cnt = 3;
        /* set tsvc4 st-ref struct */
        /* st 0 layer 0 - ref */
        st_ref[0].is_non_ref    = 0;
        st_ref[0].temporal_id   = 0;
        st_ref[0].ref_mode      = REF_TO_TEMPORAL_LAYER;
        st_ref[0].ref_arg       = 0;
        st_ref[0].repeat        = 0;
        /* st 1 layer 2 - non-ref */
        st_ref[1].is_non_ref    = 1;
        st_ref[1].temporal_id   = 1;
        st_ref[1].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[1].ref_arg       = 0;
        st_ref[1].repeat        = 0;
        /* st 2 layer 1 - ref */
        st_ref[2].is_non_ref    = 0;
        st_ref[2].temporal_id   = 0;
        st_ref[2].ref_mode      = REF_TO_PREV_REF_FRM;
        st_ref[2].ref_arg       = 0;
        st_ref[2].repeat        = 0;
    } break;
    default : {
        LOG_ERROR("unsupport gop mode %d\n", gop_mode);
        return MPP_NOK;
    } break;
    }

    if (lt_cnt || st_cnt) {
        ret = mpp_enc_ref_cfg_set_cfg_cnt(ref, lt_cnt, st_cnt);

        if (lt_cnt)
            ret = mpp_enc_ref_cfg_add_lt_cfg(ref, lt_cnt, lt_ref);

        if (st_cnt)
            ret = mpp_enc_ref_cfg_add_st_cfg(ref, st_cnt, st_ref);

        /* check and get dpb size */
        ret = mpp_enc_ref_cfg_check(ref);
    }

    return ret;
}

static MPP_RET mpi_enc_gen_smart_gop_ref_cfg(MppEncRefCfg ref, RK_S32 gop_len, RK_S32 vi_len)
{
    MppEncRefLtFrmCfg lt_ref[4];
    MppEncRefStFrmCfg st_ref[16];
    RK_S32 lt_cnt = 1;
    RK_S32 st_cnt = 8;
    RK_S32 pos = 0;
    MPP_RET ret = MPP_OK;

    memset(&lt_ref, 0, sizeof(lt_ref));
    memset(&st_ref, 0, sizeof(st_ref));

    ret = mpp_enc_ref_cfg_set_cfg_cnt(ref, lt_cnt, st_cnt);

    /* set 8 frame lt-ref gap */
    lt_ref[0].lt_idx        = 0;
    lt_ref[0].temporal_id   = 0;
    lt_ref[0].ref_mode      = REF_TO_PREV_LT_REF;
    lt_ref[0].lt_gap        = gop_len;
    lt_ref[0].lt_delay      = 0;

    ret = mpp_enc_ref_cfg_add_lt_cfg(ref, 1, lt_ref);

    /* st 0 layer 0 - ref */
    st_ref[pos].is_non_ref  = 0;
    st_ref[pos].temporal_id = 0;
    st_ref[pos].ref_mode    = REF_TO_PREV_INTRA;
    st_ref[pos].ref_arg     = 0;
    st_ref[pos].repeat      = 0;
    pos++;

    /* st 1 layer 1 - non-ref */
    if (vi_len > 1) {
        st_ref[pos].is_non_ref  = 0;
        st_ref[pos].temporal_id = 1;
        st_ref[pos].ref_mode    = REF_TO_PREV_REF_FRM;
        st_ref[pos].ref_arg     = 0;
        st_ref[pos].repeat      = vi_len - 2;
        //LOG_ERROR("pos:%d vi_len= %d gop_len=%d st_ref[pos].repeat=%d\n", pos, vi_len, gop_len, st_ref[pos].repeat);
        pos++;
    }

    st_ref[pos].is_non_ref  = 0;
    st_ref[pos].temporal_id = 0;
    st_ref[pos].ref_mode    = REF_TO_PREV_INTRA;
    st_ref[pos].ref_arg     = 0;
    st_ref[pos].repeat      = 0;
    pos++;

    ret = mpp_enc_ref_cfg_add_st_cfg(ref, pos, st_ref);

    /* check and get dpb size */
    ret = mpp_enc_ref_cfg_check(ref);

    return ret;
}

static MPP_RET mpp_set_ref_param(MpiEncTestData *p, MpiEncGopMode gop_mode)
{
    MPP_RET ret = MPP_OK;
    MppEncRefCfg ref = NULL;
    MppApi *mpi = p->mpi;
    MppCtx ctx = p->ctx;

    if (p->type != MPP_VIDEO_CodingAVC && p->type != MPP_VIDEO_CodingHEVC) {
        LOG_ERROR("MPP Encoder: encode:%d not support set ref param\n", p->type);
        return ret;
    }

    RK_U32 gop_len = (p->type == MPP_VIDEO_CodingAVC ? p->h264_cfg.gop : p->h265_cfg.gop);
    RK_U32 vi_len = (p->type == MPP_VIDEO_CodingAVC ? p->h264_cfg.vi_len : p->h265_cfg.vi_len);

    if (gop_mode <= GOP_MODE_SMARTP && gop_mode > GOP_MODE_NORMALP) {
        if (mpp_enc_ref_cfg_init(&ref)) {
            LOG_ERROR("MPP Encoder: ref cfg init failed!\n");
            goto RET;
        }
        if (gop_mode == GOP_MODE_SMARTP) {
            if (mpi_enc_gen_smart_gop_ref_cfg(ref, gop_len, vi_len)) {
                LOG_ERROR("MPP Encoder: ref cfg gen smart gop failed!\n");
                goto RET;
            }
        } else {  // tsvc for h264
            if (p->type != MPP_VIDEO_CodingAVC) {
                LOG_ERROR("MPP Encoder: encode:%d not support gop_mode:%d\n", p->type, gop_mode);
                return ret;
            }

            if (mpi_enc_gen_ref_cfg(ref, gop_mode)) {
                LOG_ERROR("MPP Encoder: ref cfg gen failed!\n");
                goto RET;
            }

        }
        ret = mpi->control(ctx, MPP_ENC_SET_REF_CFG, ref);
        if (ret) {
            LOG_ERROR("mpi control enc set ref cfg failed ret %d\n", ret);
            goto RET;
        }
    }

    if (gop_mode == GOP_MODE_NORMALP || gop_mode == GOP_MODE_SMARTP) {
        if (gop_mode == GOP_MODE_NORMALP)
            ret = mpi->control(ctx, MPP_ENC_SET_REF_CFG, NULL);
        ret |= mpp_enc_cfg_set_s32(p->cfg, "rc:gop", gop_len);
        if (ret) {
            LOG_ERROR("MPP Encoder: gop mode: cfg set s32 failed ret %d\n", ret);
            goto RET;
        }
        if (mpi->control(ctx, MPP_ENC_SET_CFG, p->cfg) != 0) {
            LOG_ERROR("MPP Encoder: change gop cfg failed!\n");
            goto RET;
        }
    }

RET:
    if (ref)
        mpp_enc_ref_cfg_deinit(&ref);
    return ret;
}

//cfg sei/change the sei cfg
static MPP_RET mpp_enc_set_sei(MpiEncTestData *p, MppEncSeiMode mode)
{
    MPP_RET ret = MPP_NOK;

    if (p->type == MPP_VIDEO_CodingAVC) {
        p->h264_cfg.sei = mode;
    } else if (p->type == MPP_VIDEO_CodingHEVC) {
        p->h265_cfg.sei = mode;
    } else if (p->type == MPP_VIDEO_CodingMJPEG) {
        p->mjpeg_cfg.sei = mode;
    } else {
        LOG_ERROR("this type:%d not suppor set sei!\n", p->type);
        return ret;
    }
    ret = p->mpi->control(p->ctx, MPP_ENC_SET_SEI_CFG, &mode);
    if (ret) {
        LOG_ERROR("mpi control enc set sei cfg failed ret %d\n", ret);
        return ret;
    }
    LOG_ERROR("this type:%d set sei:%d!\n", p->type, mode);

    return ret;
}

//send sei data, need call this func every frame
static MPP_RET mpp_enc_send_sei(MpiEncTestData *p, MppMeta meta, char *sei_data)
{
    MPP_RET ret = MPP_NOK;

    if ((p->type == MPP_VIDEO_CodingAVC && p->h264_cfg.sei) ||
        (p->type == MPP_VIDEO_CodingHEVC && p->h265_cfg.sei) ||
        (p->type == MPP_VIDEO_CodingMJPEG && p->mjpeg_cfg.sei)) {
        p->user_data.pdata = sei_data;
        p->user_data.len = strlen(sei_data) + 1;
        LOG_ERROR("this type:%d set sei:%s!\n", p->type, p->user_data.pdata);
        return mpp_meta_set_ptr(meta, KEY_USER_DATA, &p->user_data);
    }

    return ret;
}

#if TEST_MPP_SEI
static MPP_RET mpp_test_send_user_data(MpiEncTestData *p, MppFrame frame)
{
    MppMeta meta = NULL;
    meta = mpp_frame_get_meta(frame);
    if (!access("/tmp/sei1", 0)) {
        system("rm /tmp/sei1");
        mpp_enc_set_sei(p, MPP_ENC_SEI_MODE_ONE_SEQ);
        mpp_enc_send_sei(p, meta, "lxh-test-seq-123456");
    } else if (!access("/tmp/sei2", 0)) {
        //system("rm /tmp/sei2");
        mpp_enc_set_sei(p, MPP_ENC_SEI_MODE_ONE_FRAME);
        mpp_enc_send_sei(p, meta, "lxh-test-frame-123456");
    }
    if (!access("/tmp/sei3", 0)) {
        system("rm /tmp/sei2");
        system("rm /tmp/sei3");
        mpp_enc_set_sei(p, MPP_ENC_SEI_MODE_DISABLE);
    }
}
#endif

static MPP_RET mpp_enc_cfg_set(MpiEncTestData *p, bool init)
{
    MPP_RET ret;
    MppApi *mpi;
    MppCtx ctx;
    MppEncCfg cfg;

    if (NULL == p)
        return MPP_ERR_NULL_PTR;
    if (p->type == 0) {
#if MPP_ENC_OSD_ENABLE
        mpp_osd_default_set(p);
#endif
        return MPP_OK;
    }

    mpi_get_env_u32("enc_version", &p->enc_version, RK_MPP_VERSION_DEFAULT);

    char *full_range = getenv("ENABLE_FULL_RANGE");
    if (full_range)
    {
        int need_full_range = atoi(full_range);
        p->h264_cfg.range = need_full_range ? MPP_FRAME_RANGE_JPEG : MPP_FRAME_RANGE_MPEG;
        p->h265_cfg.range = need_full_range ? MPP_FRAME_RANGE_JPEG : MPP_FRAME_RANGE_MPEG;
        LOG_DEBUG("mpp full_range use env setting:%d \n", need_full_range);
    }

    mpi = p->mpi;
    ctx = p->ctx;
    cfg = p->cfg;

    if (init)
    {
        mpp_enc_cfg_set_s32(cfg, "prep:width", p->width);
        mpp_enc_cfg_set_s32(cfg, "prep:height", p->height);
        mpp_enc_cfg_set_s32(cfg, "prep:hor_stride", p->hor_stride);
        mpp_enc_cfg_set_s32(cfg, "prep:ver_stride", p->ver_stride);
        if (!(p->type == MPP_VIDEO_CodingMJPEG && p->common_cfg.rotation == 2))
            mpp_enc_cfg_set_s32(cfg, "prep:rotation",  p->common_cfg.rotation);
    }
    if (init || (p->common_cfg.change & BIT(0)))
    {
        p->fmt |= p->common_cfg.fbc;
        mpp_enc_cfg_set_s32(cfg, "prep:format", p->fmt);
    }

    if (p->common_cfg.split_mode && (init || (p->common_cfg.change & BIT(1))))
    {
        mpp_enc_cfg_set_u32(cfg, "split:mode", p->common_cfg.split_mode);
    }

    if (p->common_cfg.split_mode && (init || (p->common_cfg.change & BIT(2))))
    {
        mpp_enc_cfg_set_u32(cfg, "split:arg", p->common_cfg.split_arg);
    }
    if (init || (p->common_cfg.change & BIT(5)))
    {
        mpp_try_count_set(p);
    }

    p->frc_up_frm_set = MPP_FRC_UP_FRM_SET_INIT; // when continues_frm up to this num which can turn up the frame rate

    mpp_enc_cfg_set_s32(cfg, "codec:type", p->type);
    switch (p->type)
    {
    case MPP_VIDEO_CodingMJPEG :
    {
        if (!p->mjpeg_cfg.qfactor && (init || (p->mjpeg_cfg.change & BIT(0))))
        {
            mpp_enc_cfg_set_s32(cfg, "jpeg:quant", p->mjpeg_cfg.quant);
            p->mjpeg_cfg.frc_quant = p->mjpeg_cfg.quant;
        }
        if (p->mjpeg_cfg.qfactor && (init || (p->mjpeg_cfg.change & BIT(2))))
        {
            mpp_enc_cfg_set_s32(cfg, "jpeg:q_factor", p->mjpeg_cfg.qfactor);
            p->mjpeg_cfg.frc_qfactor = p->mjpeg_cfg.qfactor;
        }
        if (p->mjpeg_cfg.qfactor && (init || (p->mjpeg_cfg.change & BIT(3))))
            mpp_enc_cfg_set_s32(cfg, "jpeg:qf_min", p->mjpeg_cfg.qfactor_min);
        if (p->mjpeg_cfg.qfactor && (init || (p->mjpeg_cfg.change & BIT(4))))
            mpp_enc_cfg_set_s32(cfg, "jpeg:qf_max", p->mjpeg_cfg.qfactor_max);
        if (init || (p->mjpeg_cfg.change & BIT(5)))
            mpp_enc_cfg_set_s32(cfg, "rc:gop", p->mjpeg_cfg.gop);
        if (init || (p->mjpeg_cfg.change & BIT(6)))
            mpp_enc_cfg_set_s32(cfg, "rc:mode", p->mjpeg_cfg.rc_mode);
        if (init || (p->mjpeg_cfg.change & BIT(7)))
        {
            mpp_enc_bps_set(p, p->mjpeg_cfg.bps);
        }
        if (init || (p->mjpeg_cfg.change & BIT(8)))
        {
            /* setup default parameter */
            if (p->mjpeg_cfg.framerate)
            {
                LOG_WARN("warnning!!!mjpeg_cfg fps set %d to %d, if not want to change fps "
                          "do not set framerate at file of mpp_enc_cfg.conf \n",
                           p->fps, p->mjpeg_cfg.framerate);
            }
            else
            {
                p->mjpeg_cfg.framerate = p->fps;
            }
            p->fps_in_den = 1;
            p->fps_in_num = p->fps;
            p->fps_out_den = 1;
            p->fps_out_num = p->mjpeg_cfg.framerate;
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", p->fps_in_flex);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", p->fps_in_num);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", p->fps_in_den);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", p->fps_out_flex);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", p->fps_out_num);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", p->fps_out_den);
            p->fps = p->mjpeg_cfg.framerate;
        }
        if (init || (p->mjpeg_cfg.change & BIT(11)))
        {
            ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->mjpeg_cfg.sei);
            if (ret)
            {
                LOG_ERROR("mpi control enc set sei cfg failed ret %d\n", ret);
                goto RET;
            }
        }

    }
    break;
    case MPP_VIDEO_CodingAVC :
    {
        if (init || (p->h264_cfg.change & BIT(0)))
        {
            p->h264_cfg.gop_normal = p->h264_cfg.gop;
            if (init && p->common_cfg.force_idr_count * p->common_cfg.force_idr_period)
            {
                mpp_enc_cfg_set_s32(cfg, "rc:gop", p->common_cfg.force_idr_period);
                p->h264_cfg.gop = p->common_cfg.force_idr_period;
            }
            else
            {
                mpp_enc_cfg_set_s32(cfg, "rc:gop", p->h264_cfg.gop);
            }
        }
        if (init || (p->h264_cfg.change & BIT(1)))
            mpp_enc_cfg_set_s32(cfg, "rc:mode", p->h264_cfg.rc_mode);
        if (init || (p->h264_cfg.change & BIT(2)))
        {
            if (p->h264_cfg.framerate)
            {
                LOG_WARN("warnning!!!h264_cfg fps set %d to %d, if not want to change fps "
                          "do not set framerate at file of mpp_enc_cfg.conf \n",
                           p->fps, p->h264_cfg.framerate);
            }
            else
            {
                p->h264_cfg.framerate = p->fps;
            }
            /* setup default parameter */
            p->fps_in_den = 1;
            p->fps_in_num = p->fps;
            p->fps_out_den = 1;
            p->fps_out_num = p->h264_cfg.framerate;
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", p->fps_in_flex);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", p->fps_in_num);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", p->fps_in_den);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", p->fps_out_flex);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", p->fps_out_num);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", p->fps_out_den);
            p->fps = p->h264_cfg.framerate;
        }
        if (init || (p->h264_cfg.change & BIT(3)))
        {
            ret = mpp_enc_cfg_set_s32(cfg, "prep:range", p->h264_cfg.range);
            if (ret)
            {
                LOG_ERROR("mpi control enc set prep:range failed ret %d\n", ret);
                goto RET;
            }
        }

        if (init || (p->h264_cfg.change & BIT(4)))
        {
            ret = mpi->control(ctx, MPP_ENC_SET_HEADER_MODE, &p->h264_cfg.head_each_idr);
            if (ret)
            {
                LOG_ERROR("mpi control enc set codec cfg failed ret %d\n", ret);
                goto RET;
            }
        }
        if (init || (p->h264_cfg.change & BIT(5)))
        {
            ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->h264_cfg.sei);
            if (ret)
            {
                LOG_ERROR("mpi control enc set sei cfg failed ret %d\n", ret);
                goto RET;
            }
        }
        if (init || (p->h264_cfg.change & BIT(6)))
            mpp_enc_cfg_set_s32(cfg, "h264:qp_init", p->h264_cfg.qp.init);
        if (init || (p->h264_cfg.change & BIT(7)))
            mpp_enc_cfg_set_s32(cfg, "h264:qp_max", p->h264_cfg.qp.max);
        if (init || (p->h264_cfg.change & BIT(8)))
            mpp_enc_cfg_set_s32(cfg, "h264:qp_min", p->h264_cfg.qp.min);
        if (init || (p->h264_cfg.change & BIT(9)))
            mpp_enc_cfg_set_s32(cfg, "h264:qp_step", p->h264_cfg.qp.step);
        if (init || (p->h264_cfg.change & BIT(10)))
            mpp_enc_cfg_set_s32(cfg, "h264:profile", p->h264_cfg.profile);
        if (init || (p->h264_cfg.change & BIT(11)))
            mpp_enc_cfg_set_s32(cfg, "h264:cabac_en", p->h264_cfg.cabac_en);
        if (init || (p->h264_cfg.change & BIT(12)))
            mpp_enc_cfg_set_s32(cfg, "h264:cabac_idc", p->h264_cfg.cabac_idc);
        if (init || (p->h264_cfg.change & BIT(13)))
            mpp_enc_cfg_set_s32(cfg, "h264:trans8x8", p->h264_cfg.trans_8x8);
        if (init || (p->h264_cfg.change & BIT(14)))
            mpp_enc_cfg_set_s32(cfg, "h264:level", p->h264_cfg.level);

        if (init || (p->h264_cfg.change & BIT(15)))
        {
            if (init && p->h264_cfg.idr_bps)
            {
                mpp_enc_bps_set(p, p->h264_cfg.idr_bps);
            }
            else
            {
                mpp_enc_bps_set(p, p->h264_cfg.bps);
            }
        }
        if (init || (p->h264_cfg.change & BIT(18)))
        {
            mpp_set_ref_param(p, p->h264_cfg.gop_mode);
        }

    }
    break;
    case MPP_VIDEO_CodingVP8 :
    {
    } break;
    case MPP_VIDEO_CodingHEVC :
    {
        if (init || (p->h265_cfg.change & BIT(0)))
        {
            p->h265_cfg.gop_normal = p->h265_cfg.gop;
            if (init && p->common_cfg.force_idr_count * p->common_cfg.force_idr_period)
            {
                mpp_enc_cfg_set_s32(cfg, "rc:gop", p->common_cfg.force_idr_period);
                p->h265_cfg.gop = p->common_cfg.force_idr_period;
            }
            else
            {
                mpp_enc_cfg_set_s32(cfg, "rc:gop", p->h265_cfg.gop);
            }
        }
        if (init || (p->h265_cfg.change & BIT(1)))
            mpp_enc_cfg_set_s32(cfg, "rc:mode", p->h265_cfg.rc_mode);
        if (init || (p->h265_cfg.change & BIT(2)))
        {
            if (p->h265_cfg.framerate)
            {
                LOG_WARN("warnning!!!h265_cfg fps set %d to %d, if not want to change fps "
                          "do not set framerate at file of mpp_enc_cfg.conf \n",
                           p->fps, p->h265_cfg.framerate);
            }
            else
            {
                p->h265_cfg.framerate = p->fps;
            }
            /* setup default parameter */
            p->fps_in_den = 1;
            p->fps_in_num = p->fps;
            p->fps_out_den = 1;
            p->fps_out_num = p->h265_cfg.framerate;
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_flex", p->fps_in_flex);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_num", p->fps_in_num);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_in_denorm", p->fps_in_den);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_flex", p->fps_out_flex);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_num", p->fps_out_num);
            mpp_enc_cfg_set_s32(cfg, "rc:fps_out_denorm", p->fps_out_den);
            p->fps = p->h265_cfg.framerate;
        }
        if (init || (p->h265_cfg.change & BIT(3)))
        {
            ret = mpp_enc_cfg_set_s32(cfg, "prep:range", p->h265_cfg.range);
            if (ret)
            {
                LOG_ERROR("mpi control enc set prep:range failed ret %d\n", ret);
                goto RET;
            }
        }
        if (init || (p->h265_cfg.change & BIT(4)))
        {
            ret = mpi->control(ctx, MPP_ENC_SET_HEADER_MODE, &p->h265_cfg.head_each_idr);
            if (ret)
            {
                LOG_ERROR("mpi control enc set codec cfg failed ret %d\n", ret);
                goto RET;
            }
        }
        if (init || (p->h265_cfg.change & BIT(5)))
        {
            ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->h265_cfg.sei);
            if (ret)
            {
                LOG_ERROR("mpi control enc set sei cfg failed ret %d\n", ret);
                goto RET;
            }
        }
        if (init || (p->h265_cfg.change & BIT(6)))
            mpp_enc_cfg_set_s32(cfg, "h265:qp_init", p->h265_cfg.qp.init);
        if (init || (p->h265_cfg.change & BIT(7)))
            mpp_enc_cfg_set_s32(cfg, "h265:qp_max", p->h265_cfg.qp.max);
        if (init || (p->h265_cfg.change & BIT(8)))
            mpp_enc_cfg_set_s32(cfg, "h265:qp_min", p->h265_cfg.qp.min);
        if (init || (p->h265_cfg.change & BIT(9)))
            mpp_enc_cfg_set_s32(cfg, "h265:qp_step", p->h265_cfg.qp.step);
        if (init || (p->h265_cfg.change & BIT(10)))
            mpp_enc_cfg_set_s32(cfg, "h265:qp_max_i", p->h265_cfg.qp.max_i_qp);
        if (init || (p->h265_cfg.change & BIT(11)))
            mpp_enc_cfg_set_s32(cfg, "h265:qp_min_i", p->h265_cfg.qp.min_i_qp);
        if (init || (p->h265_cfg.change & BIT(12)))
        {
            if (init && p->h265_cfg.idr_bps)
            {
                mpp_enc_bps_set(p, p->h265_cfg.idr_bps);
            }
            else
            {
                mpp_enc_bps_set(p, p->h265_cfg.bps);
            }
        }
        if (init || (p->h265_cfg.change & BIT(15)))
        {
            mpp_set_ref_param(p, p->h265_cfg.gop_mode);
        }

    }
    break;
    default :
    {
        LOG_ERROR("unsupport encoder coding type %d\n", p->type);
    }
    break;
    }
    ret = mpi->control(ctx, MPP_ENC_SET_CFG, cfg);
    if (ret)
    {
        LOG_ERROR("mpi control enc set cfg failed ret %d\n", ret);
        goto RET;
    }
    //mpp_set_ref_param(p, GOP_MODE_TSVC3);  // for test tsvc
    //mpp_set_ref_param(p, GOP_MODE_SMARTP);   // for test smartp

#if MPP_ENC_ROI_ENABLE
    mpp_env_get_u32("roi_enable", &p->roi_enable, 0);
#if 0 // test //
    EncROIRegion *region = (EncROIRegion *)malloc(2 * sizeof(EncROIRegion));
    /* calculated in pixels */
    region[0].x = 0;
    region[0].y = 0;
    region[0].w = 640;
    region[0].h = 360;
    region[0].intra = 0;      /* flag of forced intra macroblock */
    region[0].quality = 1;   /* qp of macroblock */
    region[0].abs_qp_en = 1; /*absolute qp set*/
    region[0].area_map_en = 1;
    region[0].qp_area_idx = 0; /*the value more large  and the priority more high*/

    region[1].x = 1000;
    region[1].y = 600;
    region[1].w = 200;
    region[1].h = 100;
    region[1].intra = 0;      /* flag of forced intra macroblock */
    region[1].quality = 1;   /* qp of macroblock */
    region[1].abs_qp_en = 1;
    region[1].area_map_en = 1;
    region[1].qp_area_idx = 0;

    mpp_roi_config(p, region, 2);
    free(region);
#endif
#endif
#if MPP_ENC_OSD_ENABLE
    ret = mpp_osd_default_set(p);
#endif
RET:
    return ret;
}

static int read_mpp_enc_cfg_modify_file(MpiEncTestData *p, bool init)
{
    int ret = -1;
    unsigned long read_size = 0;

    FILE *modify_fd = fopen(RK_MPP_ENC_CFG_MODIFY_PATH, "rb");
    unsigned long size = get_file_size(RK_MPP_ENC_CFG_MODIFY_PATH);
    LOG_DEBUG("get cfg size=%ld\n", size);
    char *cfg = (char *)malloc(size);
    while (read_size != size)
    {
        read_size += fread(cfg, 1, size - read_size, modify_fd);
    }
    //LOG_INFO("get cfg =%s read_size=%ld\n", cfg, read_size);
    cJSON *root = cJSON_Parse(cfg);
    if (root == NULL)
    {
        LOG_ERROR("the %s is broken\n", RK_MPP_ENC_CFG_MODIFY_PATH);
    }
    else
    {
        ret = parse_check_mpp_enc_cfg(root, p, init);
    }
    if (modify_fd)
        fclose(modify_fd);
    if (cfg)
        free(cfg);
    if (root)
        cJSON_Delete(root);

    return ret;
}

static int check_mpp_enc_cfg_file_init(MpiEncTestData *p)
{
    int ret = -1;
    char cmd[128] = {0};
    if (NULL == p)
        return -1;

    if (!access(RK_MPP_ENC_CFG_MODIFY_PATH, F_OK))
    {
        ret = read_mpp_enc_cfg_modify_file(p, true);
    }

    if (ret)
    {
        if (!access(RK_MPP_ENC_CFG_ORIGINAL_PATH, F_OK))
        {
            sprintf(cmd, "cp %s %s", RK_MPP_ENC_CFG_ORIGINAL_PATH, RK_MPP_ENC_CFG_MODIFY_PATH);
            system(cmd);
            LOG_DEBUG("copy enc cfg file...\n");
            ret = read_mpp_enc_cfg_modify_file(p, true);
        }
        else
        {
            LOG_ERROR("file :%s not exit!\n", RK_MPP_ENC_CFG_ORIGINAL_PATH);
            ret = -1;
        }
    }
    else
    {

    }

    p->cfg_notify_fd = inotify_init();
    return ret;
}

void *thread_check_mpp_enc_chenge_loop(void *user)
{
    int ret = 0;
    MPP_RET mpp_ret;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, PTHREAD_CREATE_JOINABLE);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    MpiEncTestData *p = (MpiEncTestData *)user;
    int last_wd = 0;
    unsigned char buf[1024] = {0};
    struct inotify_event *event = NULL;
    bool modify = false;
    while (1)
    {
        last_wd = p->cfg_notify_wd;
        p->cfg_notify_wd = inotify_add_watch(p->cfg_notify_fd, RK_MPP_ENC_CFG_MODIFY_PATH, IN_MODIFY | IN_DELETE_SELF);//IN_ALL_EVENTS);
        modify = false;
        if (p->cfg_notify_wd != -1)
        {
            if ((last_wd == -1 && p->cfg_notify_wd == 1))
            {
                modify = true;
            }
            else
            {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(p->cfg_notify_fd, &fds);

                if (select(p->cfg_notify_fd + 1, &fds, NULL, NULL, NULL) > 0)
                {
                    int len, index = 0;
                    memset(buf, 0, sizeof(buf));
                    len = read(p->cfg_notify_fd, &buf, sizeof(buf));

                    while (len > 0 && index < len)
                    {
                        event = (struct inotify_event *)(buf + index);
                        if (event->mask == IN_MODIFY)
                        {
                            modify = true;
                        }
                        else if (event->mask == IN_DELETE_SELF)
                        {
                            p->cfg_notify_wd = -1;
                        }

                        index += sizeof(struct inotify_event) + event->len;
                    }

                }

            }
        }
        sleep(1);
        if (modify)
        {
            LOG_INFO("the enc cfg file change or creat, do update.wd=%d,last_wd=%d\n", p->cfg_notify_wd, last_wd);
            ret = read_mpp_enc_cfg_modify_file(p, false);
            if (ret)
                LOG_ERROR("error: the enc cfg file is broken.please check.\n");
            else
            {
                mpp_ret = mpp_enc_cfg_set(p, false);
                dump_mpp_enc_cfg(p);
                if (mpp_ret)
                {
                    LOG_ERROR("mpp_enc_cfg_set failed ret %d\n", mpp_ret);
                }
            }
        }

        inotify_rm_watch(p->cfg_notify_fd, p->cfg_notify_wd);
        close(p->cfg_notify_fd);
        p->cfg_notify_fd = inotify_init();
#if 0
        if (p->cfg_notify_wd == IN_MODIFY || (last_wd == -1 && p->cfg_notify_wd == 1))
        {
            LOG_INFO("the enc cfg file change or creat, do update.wd=%d,last_wd=%d\n", p->cfg_notify_wd, last_wd);
            ret = read_mpp_enc_cfg_modify_file(p, false);
            if (ret)
                LOG_ERROR("error: the enc cfg file is broken.please check.\n");
            else
            {
                dump_mpp_enc_cfg(p);
                mpp_ret = mpp_enc_cfg_set(p, false);
                if (mpp_ret)
                {
                    LOG_ERROR("mpp_enc_cfg_set failed ret %d\n", mpp_ret);
                }
            }
            inotify_rm_watch(p->cfg_notify_fd, p->cfg_notify_wd);
            close(p->cfg_notify_fd);
            p->cfg_notify_fd = inotify_init();
        }
        sleep(1);
#endif
    }
}

#if MPP_ENC_ROI_ENABLE
static int mpp_roi_enable_set(MpiEncTestData *p, int enable)
{
    p->roi_enable = enable;
}

static int mpp_roi_enable_get(MpiEncTestData *p)
{
    return p->roi_enable;
}

static int mpp_roi_config(MpiEncTestData *p, EncROIRegion *regions, int region_cnt)
{
    if (!regions || region_cnt == 0) {
        p->roi_number = 0;
        if (p->roi_cfg.regions) {
            free(p->roi_cfg.regions);
            p->roi_cfg.regions = NULL;
        }
        LOG_INFO("disable roi function\n");
        return 0;
    }
    int msize = region_cnt * sizeof(MppEncROIRegion);
    MppEncROIRegion *region = (MppEncROIRegion *)malloc(msize);
    if (!region) {
        return -1;
    }
    for (int i = 0; i < region_cnt; i++) {
      if ((regions[i].x % 16) || (regions[i].y % 16) ||
          (regions[i].w % 16) || (regions[i].h % 16)) {
          LOG_WARN("WARN: MPP Encoder: region parameter should be an integer multiple of 16\n");
          LOG_WARN("WARN: MPP Encoder: reset region[%d] frome <%d,%d,%d,%d> to <%d,%d,%d,%d>\n",
                    i, regions[i].x, regions[i].y, regions[i].w, regions[i].h,
                    UPALIGNTO16(regions[i].x), UPALIGNTO16(regions[i].y),
                    UPALIGNTO16(regions[i].w), UPALIGNTO16(regions[i].h));
          regions[i].x = UPALIGNTO16(regions[i].x);
          regions[i].y = UPALIGNTO16(regions[i].y);
          regions[i].w = UPALIGNTO16(regions[i].w);
          regions[i].h = UPALIGNTO16(regions[i].h);
      }
      LOG_DEBUG("MPP Encoder: roi region[%d]:<%d,%d,%d,%d>\n",
                 i, regions[i].x, regions[i].y, regions[i].w, regions[i].h);
      LOG_DEBUG("MPP Encoder: roi region[%d].intra=%d,\n", i, regions[i].intra);
      LOG_DEBUG("MPP Encoder: roi region[%d].quality=%d,\n", i, regions[i].quality);
      LOG_DEBUG("MPP Encoder: roi region[%d].abs_qp_en=%d,\n", i, regions[i].abs_qp_en);
      LOG_DEBUG("MPP Encoder: roi region[%d].qp_area_idx=%d,\n", i, regions[i].qp_area_idx);
      LOG_DEBUG("MPP Encoder: roi region[%d].area_map_en=%d,\n", i, regions[i].area_map_en);
      assert(regions[i].x < 8192);
      assert(regions[i].y < 8192);
      assert(regions[i].w < 8192);
      assert(regions[i].h < 8192);
      assert(regions[i].x < 8192);
      assert(regions[i].intra <= 1);
      assert(regions[i].abs_qp_en <= 1);
      assert(regions[i].qp_area_idx <= 7);
      assert(regions[i].area_map_en <= 1);
      VALUE_SCOPE_CHECK(regions[i].quality, -48, 51);
      region[i].x = regions[i].x;
      region[i].y = regions[i].y;
      region[i].w = regions[i].w;
      region[i].h = regions[i].h;
      region[i].intra = regions[i].intra;
      region[i].quality = regions[i].quality;
      region[i].abs_qp_en = regions[i].abs_qp_en;
      region[i].qp_area_idx = regions[i].qp_area_idx;
      region[i].area_map_en = regions[i].area_map_en;
    }
    p->roi_cfg.number = region_cnt;
    if (p->roi_cfg.regions)
        free(p->roi_cfg.regions);
    p->roi_cfg.regions = region;
    //mpp_meta_set_ptr(meta, KEY_ROI_DATA, (void*)&p->roi_cfg);
    return 0;
}
#endif

