#if CONFIG_DBSERVER

#ifndef __ISP_FUNC_H
#define __ISP_FUNC_H

#include <stdbool.h>
#include "rkaiq/common/rk_aiq_comm.h"
#include "rkaiq/common/rk_aiq_types.h"
#include "rkaiq/uAPI/rk_aiq_user_api_imgproc.h"
// #include "rk_aiq_uapi_ae_int_types.h"
// #include "rk_aiq_types_awb_algo_int.h"

#define DEFAULT_SPATIAL_DENOIZE_LEVEL 50
#define DEFAULT_TEMPORAL_DENOIZE_LEVEL 50
#define LED_OFF 0
#define LED_ON 1
#define HDR_MODE_OFF "close"
#define HDR_MODE_HDR2 "HDR2"
#define HDR_MODE_HDR3 "HDR3"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum work_mode_0_e {
  WM0_INVALID_MODE = -1,
  WM0_MANAUL_MODE = 0,
  WM0_AUTO_MODE
} work_mode_0_t;

typedef enum wk_mode_1_e {
  WM1_INVALID_MODE = -1,
  WM1_CLOSE_MODE = 0,
  WM1_OPEN_MODE
} work_mode_1_t;

typedef enum work_mode_2_e {
  WM2_INVALID_MODE = -1,
  WM2_CLOSE_MODE = 0,
  WM2_OPEN_MODE,
  WM2_AUTO_MODE
} work_mode_2_t;

// typedef enum enum_hdr_level {

// } blc_hdr_level;

typedef enum night2day_mode_e {
  ND_AUTO_MODE = 0,
  ND_DAY_MODE,
  ND_NIGHT_MODE,
  ND_SCHEDULE_MODE,
  ND_INVALID_MODE
} night2day_mode_t;

typedef enum white_balance_mode_e {
  WB_INVALID = -1,
  WB_AUTO = 0,
  WB_MANUAL,
  WB_LOCK,
  WB_FLUORESCENT_LAMP,
  WB_INCANDESCENT,
  WB_WARM_LIGHT,
  WB_NATURE_LIGHT
} white_balance_mode_t;

typedef enum noise_reduce_mode_e {
  NR_INVAILD = -1,
  NR_DEFAULT = 0,
  NR_2D,
  NR_3D,
  NR_MIX
} nr_mode_t;

typedef enum gray_scale_mode_e {
  GS_INVALID = -1,
  GS_0_255 = 0,
  GS_16_235
} gs_mode_t;

typedef enum distortion_correction_mode_e {
  DC_INVALID = -1,
  DC_CLOSE = 0,
  DC_LDCH,
  DC_FEC
} dc_mode_t;

typedef enum flip_mode_e {
  FM_INVALID = -1,
  FM_CLOSE = 0,
  FM_FLIP,
  FM_MIRROR,
  FM_CENTER
} flip_mode_t;

typedef int (*ispserver_status_signal_send)(int status);

int get_led_state();
void send_stream_on_signal();
void set_stream_on();
void set_stream_off();
int check_stream_status();
void reset_flow();

void brightness_set(int level);
void contrast_set(int level);
void saturation_set(int level);
void sharpness_set(int level);
void hue_set(int level);

int manual_white_balance_set(int r_level, int g_level, int b_level);
int manual_white_balance_level_set(int r_level, int g_level, int b_level);
int white_balance_style_set(white_balance_mode_t style);
int white_balance_gain_get(rk_aiq_wb_gain_t *gain);

// set fix fps, set -1 to set fps auto
int isp_fix_fps_set(int rate);
int frequency_mode_set(expPwrLineFreq_t mode);

int hdr_global_value_set(rk_aiq_working_mode_t hdr_mode);
rk_aiq_working_mode_t hdr_global_value_get();
int hdr2_normal_set(rk_aiq_working_mode_t hdr_mode);
void hdr_mode_set(rk_aiq_working_mode_t mode, int ledIsOn);
void hdr_mode_set4db(rk_aiq_working_mode_t mode);
int blc_hdr_level_set(int level);
int blc_hdr_level_enum_set(unsigned int level);

int32_t str2time_sec(char *time_str);
void gc_cpsl_cfg_fill_light_mode_set(rk_aiq_cpsls_t mode);
int rk_smart_get_scene_param(double *pdLumaDay, double *pdLumaNight,
                             double *pdRGratio, double *pdBGratio,
                             unsigned int *p_u32_exposure, double *pdExpAgain);
int night_to_day_para_set(rk_aiq_cpsl_cfg_t compensate_light_cfg);
int fill_light_brightness_set(int strength);
int night_to_day_auto_mode_set(int filter_level, int filter_time);
int set_gray_open_led(int fill_light_brightness);
int set_color_close_led();
int set_night_mode(int fill_light_brightness);
int set_day_mode(int hdr_mode);

int bypass_stream_rotation_set(int rotation);
int mirror_mode_set(flip_mode_t mode);

int nr_level_set(int spatial_level, int temporal_level);
int nr_para_set(nr_mode_t mode, int spatial_level, int temporal_level);

void gc_dc_mode_set(dc_mode_t mode);
int fec_level_set(int fec_level);
int ldch_level_set(int level);
int dc_para_set(dc_mode_t mode, int ldch_level, int fec_level);

int dehaze_strength_set(int level);
int dehaze_para_set(work_mode_2_t mode, int level);

float exposure_time_str2float(char *time);
int exposure_weight_str2array(char *weight,int32_t* g_weight);
int exposure_time_set(char *time);
int exposure_gain_set(int gain);
int auto_exposure_set();
int exposure_info_get(Uapi_ExpQueryInfo_t *stExpInfo, rk_aiq_wb_cct_t *stCCT);
int manual_exposure_manual_gain_set_char(char *time, int gain);
int manual_exposure_manual_gain_set_float(float expTime, int gain);
int manual_exposure_auto_gain_set_char(char *time);
int manual_exposure_auto_gain_set_float(float expTime);
int manual_exposure_grid_weight_set(int32_t* weight,int evbias);

int blc_region_para_set(work_mode_1_t mode, int strength);
int blc_region_strength_set(int strength);
int blc_hlc_para_set(work_mode_1_t mode, int hlc_level, int dark_level);
int blc_hlc_level_set(int hlc_level, int dark_level);

int isp_status_sender_register(ispserver_status_signal_send send_func);

#ifdef __cplusplus
}
#endif

#endif

#endif
