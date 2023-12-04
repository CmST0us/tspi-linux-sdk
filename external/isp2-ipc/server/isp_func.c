#if CONFIG_DBSERVER
#include <errno.h>
#include <gdbus.h>
#include <glib.h>
#include <linux/kernel.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/log.h"
#include "config.h"
#include "isp_func.h"
#include "mediactl/mediactl.h"
#include "rkaiq/common/rk_aiq_comm.h"
#include "rkaiq/uAPI/rk_aiq_user_api_ae.h"
#include "rkaiq/uAPI/rk_aiq_user_api_sysctl.h"

#if ENABLE_MEDIASERVER
#include "mediaserver.h"
#endif

extern rk_aiq_sys_ctx_t *db_aiq_ctx;

rk_aiq_wb_gain_t gs_wb_gain = {2.083900, 1.000000, 1.000000, 2.018500};
static pthread_mutex_t db_aiq_ctx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cpsl_cfg_mutex = PTHREAD_MUTEX_INITIALIZER;

rk_aiq_working_mode_t gc_hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
static rk_aiq_cpsl_cfg_t gc_cpsl_cfg;
static int gc_cpsl_cfg_init_flag = 0;
static expPwrLineFreq_t gc_frequency_mode = EXP_PWR_LINE_FREQ_60HZ;
static int gc_led_mode = LED_OFF;
static work_mode_2_t gc_dehaze_mode = WM2_INVALID_MODE;
static dc_mode_t gc_dc_mode = DC_INVALID;
static int g_stream_on = 0;
static int g_fix_fps = -1;
static ispserver_status_signal_send g_send_func = NULL;
float exposure_uvc_time[10] = {0.0005f, 0.001f,  0.002f,  0.0039f, 0.0078f,
                               0.0156f, 0.0312f, 0.0625f, 0.1250f, 0.2500f};
float exposure_isp_time[10] = {0.0005f, 0.001f,  0.002f,  0.0039f, 0.0078f,
                               0.0100f, 0.0150f, 0.0200f, 0.0250f, 0.0300f};

static void set_led_state(int status) { gc_led_mode = status; }

int get_led_state() { return gc_led_mode; }

void send_stream_on_signal() {
  if (g_send_func) {
    g_send_func(1);
  }
}

void set_stream_on() {
  if (!g_stream_on && g_send_func) {
    g_send_func(1);
  }
  g_stream_on = 1;
}

void set_stream_off() {
  if (g_stream_on && g_send_func) {
    g_send_func(0);
  }
  g_stream_on = 0;
}

int check_stream_status() {
  return g_stream_on;
}

void reset_flow() {
#if ENABLE_MEDIASERVER
  set_stream_off();
  mediaserver_stop_flow();
  while (!g_stream_on) {
    usleep(100);
  }
  mediaserver_restart_flow();
  LOG_INFO("reset flow end!\n");
#else
  LOG_ERROR("no mediaserver, reset flow fail, please overwrite reset_flow in "
            "isp_func.c\n");
#endif
}

void brightness_set(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_uapi_setBrightness(db_aiq_ctx,
                            (int)(level * 2.55)); // [0, 100]->[0, 255]
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
}

void contrast_set(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_uapi_setContrast(db_aiq_ctx,
                          (int)(level * 2.55)); // [0, 100]->[0, 255]
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
}

void saturation_set(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_uapi_setSaturation(db_aiq_ctx,
                            (int)(level * 2.55)); // [0, 100]->[0, 255]
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
}

void sharpness_set(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_uapi_setSharpness(db_aiq_ctx, level); // [0, 100]
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
}

void hue_set(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_uapi_setHue(db_aiq_ctx, (int)(level * 2.55)); // [0, 100]->[0, 255]
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
}

int manual_white_balance_level_set(int r_level, int g_level, int b_level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  rk_aiq_wb_gain_t gain;
  float ratio_r, ratio_g, ratio_b;
  r_level = (r_level == 0) ? 1 : r_level;
  g_level = (g_level == 0) ? 1 : g_level;
  b_level = (b_level == 0) ? 1 : b_level;

  gain.rgain = r_level / 50.0f * gs_wb_gain.rgain;   // [0, 100]->[1.0, 4.0]
  gain.grgain = g_level / 50.0f * gs_wb_gain.grgain; // [0, 100]->[1.0, 4.0]
  gain.gbgain = g_level / 50.0f * gs_wb_gain.gbgain; // [0, 100]->[1.0, 4.0]
  gain.bgain = b_level / 50.0f * gs_wb_gain.bgain;   // [0, 100]->[1.0, 4.0]
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  ret = rk_aiq_uapi_setMWBGain(db_aiq_ctx, &gain);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO("r_level is %d, g_level is %d, b_level is %d, set ret is %d\n",
           r_level, g_level, b_level, ret);
  return ret;
}

int manual_white_balance_set(int r_level, int g_level, int b_level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  ret = rk_aiq_uapi_setWBMode(db_aiq_ctx, OP_MANUAL);
  LOG_DEBUG("set white balance mode manual, ret is %d\n", ret);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  ret = manual_white_balance_level_set(r_level, g_level, b_level);
  return ret;
}

int white_balance_style_set(white_balance_mode_t style) {
  LOG_DEBUG("white balance style is %d\n", style);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  if (style != WB_LOCK) {
    ret = rk_aiq_uapi_unlockAWB(db_aiq_ctx);
    LOG_DEBUG("set unlock white balance, ret is %d\n", ret);
  }
  switch (style) {
  case WB_INVALID: {
    LOG_WARN("set invalid white balance mode\n");
    break;
  }
  case WB_AUTO: {
    ret = rk_aiq_uapi_setWBMode(db_aiq_ctx, OP_AUTO);
    LOG_INFO("set autoWhiteBalance, ret is %d\n", ret);
    break;
  }
  case WB_MANUAL: {
    ret = rk_aiq_uapi_setWBMode(db_aiq_ctx, OP_MANUAL);
    LOG_INFO("set manualWhiteBalance, without level, ret is %d\n", ret);
    break;
  }
  case WB_LOCK: {
    ret = rk_aiq_uapi_lockAWB(db_aiq_ctx);
    LOG_INFO("set lock whilte balance, ret is %d\n", ret);
    break;
  }
  case WB_FLUORESCENT_LAMP: {
    ret = rk_aiq_uapi_setMWBScene(db_aiq_ctx, RK_AIQ_WBCT_DAYLIGHT);
    LOG_INFO("set fluorescentLamp, ret is %d\n", ret);
    break;
  }
  case WB_INCANDESCENT: {
    ret = rk_aiq_uapi_setMWBScene(db_aiq_ctx, RK_AIQ_WBCT_INCANDESCENT);
    LOG_INFO("set incandescent, ret is %d\n", ret);
    break;
  }
  case WB_WARM_LIGHT: {
    ret = rk_aiq_uapi_setMWBScene(db_aiq_ctx, RK_AIQ_WBCT_WARM_FLUORESCENT);
    LOG_INFO("set warmLight, ret is %d\n", ret);
    break;
  }
  case WB_NATURE_LIGHT: {
    rk_aiq_uapi_setMWBCT(db_aiq_ctx, 5500);
    LOG_INFO("set naturalLight, ret is %d\n", ret);
    break;
  }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

static int isp_output_fps_set_no_mutex(int rate) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  if (g_fix_fps > 0)
    rate = g_fix_fps;
  int ret = 0;
  frameRateInfo_t info;
  memset(&info, 0, sizeof(info));
  rk_aiq_uapi_getFrameRate(db_aiq_ctx, &info);
  if (info.fps != rate) {
    info.fps = rate;
    info.mode = OP_MANUAL;
    ret = rk_aiq_uapi_setFrameRate(db_aiq_ctx, info);
  }
  LOG_INFO("set output fps: %d, ret is %d\n", rate, ret);
  return ret;
}

static int isp_output_fps_set(int rate) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = isp_output_fps_set_no_mutex(rate);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

static int isp_output_fps_get() {
  if (g_fix_fps > 0) {
    LOG_INFO("fps is fixed, rate is %d\n", g_fix_fps);
    return g_fix_fps;
  }
  int fps = 30;
  switch (gc_hdr_mode) {
  case RK_AIQ_WORKING_MODE_NORMAL: {
    fps = (gc_frequency_mode == EXP_PWR_LINE_FREQ_50HZ ? 25 : 30);
    break;
  }
  case RK_AIQ_WORKING_MODE_ISP_HDR2: {
    fps = 25;
    break;
  }
  case RK_AIQ_WORKING_MODE_ISP_HDR3: {
    fps = 20;
    break;
  }
  default: {
    fps = 30;
    break;
  }
  }

  return fps;
}

int isp_fix_fps_set(int rate) {
  g_fix_fps = rate;
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  if (g_fix_fps > 0) {
    pthread_mutex_lock(&db_aiq_ctx_mutex);
    ret = isp_output_fps_set_no_mutex(rate);
    pthread_mutex_unlock(&db_aiq_ctx_mutex);
  } else {
    ret = isp_output_fps_set(isp_output_fps_get());
  }
  return ret;
}

int frequency_mode_set(expPwrLineFreq_t mode) {
  LOG_INFO("frequency mode is %d\n", mode);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (mode) {
  case EXP_PWR_LINE_FREQ_50HZ: {
    ret = rk_aiq_uapi_setExpPwrLineFreqMode(db_aiq_ctx, EXP_PWR_LINE_FREQ_50HZ);
    LOG_DEBUG("rk_aiq_uapi_setExpPwrLineFreqMode 50HZ ret is %d\n", ret);
    isp_output_fps_set_no_mutex(25);
    gc_frequency_mode = EXP_PWR_LINE_FREQ_50HZ;
    break;
  }
  case EXP_PWR_LINE_FREQ_60HZ: {
    ret = rk_aiq_uapi_setExpPwrLineFreqMode(db_aiq_ctx, EXP_PWR_LINE_FREQ_60HZ);
    LOG_DEBUG("rk_aiq_uapi_setExpPwrLineFreqMode 60HZ ret is %d\n", ret);
    isp_output_fps_set_no_mutex(30);
    gc_frequency_mode = EXP_PWR_LINE_FREQ_60HZ;
    break;
  }
  default: { LOG_WARN("set undefined frequencncy mode: %d\n", mode); }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int hdr_global_value_set(rk_aiq_working_mode_t hdr_mode) {
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (hdr_mode) {
    case RK_AIQ_WORKING_MODE_NORMAL:
      gc_hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
      break;
    case RK_AIQ_WORKING_MODE_ISP_HDR2:
      gc_hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
      break;
    case RK_AIQ_WORKING_MODE_ISP_HDR3:
      gc_hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
      break;
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return 0;
}

rk_aiq_working_mode_t hdr_global_value_get() {
  return gc_hdr_mode;
}

int hdr2_normal_set(rk_aiq_working_mode_t hdr_mode) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (hdr_mode) {
  case RK_AIQ_WORKING_MODE_NORMAL: {
    ret = rk_aiq_uapi_sysctl_swWorkingModeDyn(db_aiq_ctx,
                                              RK_AIQ_WORKING_MODE_NORMAL);
    gc_hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    LOG_INFO("set hdr off ,ret is %d, gc_hdr_mode\n", ret, gc_hdr_mode);
    break;
  }
  case RK_AIQ_WORKING_MODE_ISP_HDR2: {
    ret = rk_aiq_uapi_sysctl_swWorkingModeDyn(db_aiq_ctx,
                                              RK_AIQ_WORKING_MODE_ISP_HDR2);
    gc_hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
    LOG_INFO("set hdr2 ,ret is %d, gc_hdr_mode\n", ret, gc_hdr_mode);
    break;
  }
  default: { LOG_WARN("set undefined mode: %d\n", hdr_mode); }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

// TODO: hdr3 mode and restart flow
void hdr_mode_set(rk_aiq_working_mode_t mode, int ledIsOn) {
  LOG_INFO("set hdr mode %d, led status %d, gc_hdr_mode is %d\n", mode, ledIsOn,
           gc_hdr_mode);
  if (gc_hdr_mode == mode || (LED_ON == get_led_state() && 1 == ledIsOn)) {
    LOG_DEBUG("find no change in mode\n");
    return;
  }
  if (gc_hdr_mode == RK_AIQ_WORKING_MODE_ISP_HDR3 ||
      mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
    LOG_INFO("set hdr3\n");
    gc_hdr_mode = mode;
    reset_flow();
    // reboot?
    // rk_restart_image_process(hdr_mode);
  } else {
    hdr2_normal_set(mode);
    isp_output_fps_set(isp_output_fps_get());
  }
}

void hdr_mode_set4db(rk_aiq_working_mode_t mode) {
  hdr_mode_set(mode, gc_led_mode);
}

int blc_hdr_level_set(int level) {
  LOG_INFO("set level is %d\n", level);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  if (level)
    ret = rk_aiq_uapi_setMHDRStrth(db_aiq_ctx, true, level);
  else
    ret = rk_aiq_uapi_setMHDRStrth(db_aiq_ctx, true, 1);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int blc_hdr_level_enum_set(unsigned int level) {
  ahdr_attrib_t attr;
  Uapi_HdrExpAttr_t hdrExpAttr;
  LOG_DEBUG("set hdr level: %d, gc_hdr_mode: %d\n", level, gc_hdr_mode);
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  if (db_aiq_ctx && g_stream_on) {
    switch (level) {
    case 1: // auto
      rk_aiq_user_api_ahdr_GetAttrib(db_aiq_ctx, &attr);
      // attr.bEnable = true;
      attr.opMode = HDR_OpMode_SET_LEVEL;
      attr.stSetLevel.level = 50;
      rk_aiq_user_api_ahdr_SetAttrib(db_aiq_ctx, attr);

      rk_aiq_user_api_ae_getHdrExpAttr(db_aiq_ctx, &hdrExpAttr);
      hdrExpAttr.ExpRatioType = RKAIQ_HDRAE_RATIOTYPE_MODE_AUTO;
      rk_aiq_user_api_ae_setHdrExpAttr(db_aiq_ctx, hdrExpAttr);
      break;
    case 2:
      rk_aiq_user_api_ahdr_GetAttrib(db_aiq_ctx, &attr);
      // attr.bEnable = true;
      attr.opMode = HDR_OpMode_SET_LEVEL;
      attr.stSetLevel.level = 40;
      rk_aiq_user_api_ahdr_SetAttrib(db_aiq_ctx, attr);

      rk_aiq_user_api_ae_getHdrExpAttr(db_aiq_ctx, &hdrExpAttr);
      hdrExpAttr.ExpRatioType = RKAIQ_HDRAE_RATIOTYPE_MODE_FIX;
      hdrExpAttr.M2SRatioFix.fCoeff[0] = 28;
      hdrExpAttr.M2SRatioFix.fCoeff[1] = 28;
      hdrExpAttr.M2SRatioFix.fCoeff[2] = 28;
      hdrExpAttr.M2SRatioFix.fCoeff[3] = 28;
      hdrExpAttr.M2SRatioFix.fCoeff[4] = 28;
      hdrExpAttr.M2SRatioFix.fCoeff[5] = 28;
      rk_aiq_user_api_ae_setHdrExpAttr(db_aiq_ctx, hdrExpAttr);
      break;
    case 3:
      rk_aiq_user_api_ahdr_GetAttrib(db_aiq_ctx, &attr);
      // attr.bEnable = true;
      attr.opMode = HDR_OpMode_SET_LEVEL;
      attr.stSetLevel.level = 55;
      rk_aiq_user_api_ahdr_SetAttrib(db_aiq_ctx, attr);

      rk_aiq_user_api_ae_getHdrExpAttr(db_aiq_ctx, &hdrExpAttr);
      hdrExpAttr.ExpRatioType = RKAIQ_HDRAE_RATIOTYPE_MODE_FIX;
      hdrExpAttr.M2SRatioFix.fCoeff[0] = 32;
      hdrExpAttr.M2SRatioFix.fCoeff[1] = 32;
      hdrExpAttr.M2SRatioFix.fCoeff[2] = 32;
      hdrExpAttr.M2SRatioFix.fCoeff[3] = 32;
      hdrExpAttr.M2SRatioFix.fCoeff[4] = 32;
      hdrExpAttr.M2SRatioFix.fCoeff[5] = 32;
      rk_aiq_user_api_ae_setHdrExpAttr(db_aiq_ctx, hdrExpAttr);
      break;
    case 4:
      rk_aiq_user_api_ahdr_GetAttrib(db_aiq_ctx, &attr);
      // attr.bEnable = true;
      attr.opMode = HDR_OpMode_SET_LEVEL;
      attr.stSetLevel.level = 68;
      rk_aiq_user_api_ahdr_SetAttrib(db_aiq_ctx, attr);

      rk_aiq_user_api_ae_getHdrExpAttr(db_aiq_ctx, &hdrExpAttr);
      hdrExpAttr.ExpRatioType = RKAIQ_HDRAE_RATIOTYPE_MODE_FIX;
      hdrExpAttr.M2SRatioFix.fCoeff[0] = 48;
      hdrExpAttr.M2SRatioFix.fCoeff[1] = 48;
      hdrExpAttr.M2SRatioFix.fCoeff[2] = 48;
      hdrExpAttr.M2SRatioFix.fCoeff[3] = 48;
      hdrExpAttr.M2SRatioFix.fCoeff[4] = 48;
      hdrExpAttr.M2SRatioFix.fCoeff[5] = 48;
      rk_aiq_user_api_ae_setHdrExpAttr(db_aiq_ctx, hdrExpAttr);
      break;
    case 5:
#if 0
                rk_aiq_user_api_ahdr_GetAttrib(db_aiq_ctx, &attr);
                attr.bEnable = true;
                attr.opMode = HDR_OpMode_SET_LEVEL;
                attr.level = 80;
                rk_aiq_user_api_ahdr_SetAttrib(db_aiq_ctx, attr);

                rk_aiq_uapi_ae_getHdrExpAttr(db_aiq_ctx, &hdrExpAttr);
                hdrExpAttr.ExpRatioType = RKAIQ_HDRAE_RATIOTYPE_MODE_FIX;
                hdrExpAttr.M2SRatioFix.fCoeff[0] = 16;
                hdrExpAttr.M2SRatioFix.fCoeff[1] = 16;
                hdrExpAttr.M2SRatioFix.fCoeff[2] = 16;
                hdrExpAttr.M2SRatioFix.fCoeff[3] = 16;
                hdrExpAttr.M2SRatioFix.fCoeff[4] = 16;
                hdrExpAttr.M2SRatioFix.fCoeff[5] = 16;

                hdrExpAttr.L2MRatioFix.fCoeff[0] = 4;
                hdrExpAttr.L2MRatioFix.fCoeff[1] = 4;
                hdrExpAttr.L2MRatioFix.fCoeff[2] = 4;
                hdrExpAttr.L2MRatioFix.fCoeff[3] = 4;
                hdrExpAttr.L2MRatioFix.fCoeff[4] = 4;
                hdrExpAttr.L2MRatioFix.fCoeff[5] = 4;
                rk_aiq_uapi_ae_setHdrExpAttr(db_aiq_ctx, hdrExpAttr);
#else
      rk_aiq_user_api_ahdr_GetAttrib(db_aiq_ctx, &attr);
      // attr.bEnable = true;
      attr.opMode = HDR_OpMode_SET_LEVEL;
      attr.stSetLevel.level = 50;
      rk_aiq_user_api_ahdr_SetAttrib(db_aiq_ctx, attr);

      rk_aiq_user_api_ae_getHdrExpAttr(db_aiq_ctx, &hdrExpAttr);
      hdrExpAttr.ExpRatioType = RKAIQ_HDRAE_RATIOTYPE_MODE_AUTO;
      rk_aiq_user_api_ae_setHdrExpAttr(db_aiq_ctx, hdrExpAttr);
#endif
      break;
    default:
      LOG_WARN("HDR_LEVEL: %d\n", level);
      break;
    }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return 0;
}

int32_t str2time_sec(char *time_str) {
  if (NULL == time_str || (strlen(time_str) != 8 && strlen(time_str) != 5))
    return -1;
  int32_t second_rst = -1;
  if (strlen(time_str) != 8) {
    char hour_str[3] = {0};
    char minute_str[3] = {0};
    char second_str[3] = {0};
    strncpy(hour_str, time_str, 2);
    strncpy(minute_str, time_str + 3, 2);
    strncpy(second_str, time_str + 5, 2);
    int hour = atoi(hour_str);
    int minute = atoi(minute_str);
    int second = atoi(second_str);
    second_rst = (hour * 60 * 60) + (minute * 60) + second;
  } else {
    char hour_str[3] = {0};
    char minute_str[3] = {0};
    strncpy(hour_str, time_str, 2);
    strncpy(minute_str, time_str + 3, 2);
    int hour = atoi(hour_str);
    int minute = atoi(minute_str);
    second_rst = (hour * 60 * 60) + (minute * 60);
  }

  return second_rst;
}

void gc_cpsl_cfg_fill_light_mode_set(rk_aiq_cpsls_t mode) {
  pthread_mutex_lock(&cpsl_cfg_mutex);
  if (!gc_cpsl_cfg_init_flag)
    memset(&gc_cpsl_cfg, 0, sizeof(rk_aiq_cpsl_cfg_t));
  if (mode != RK_AIQ_CPSLS_INVALID)
    gc_cpsl_cfg.lght_src = mode;
  else if (!gc_cpsl_cfg_init_flag) {
    gc_cpsl_cfg.lght_src = RK_AIQ_CPSLS_IR;
    LOG_WARN("fill mode: %d is undefined, set ir by default\n", mode);
  }
  gc_cpsl_cfg_init_flag = 1;
  pthread_mutex_unlock(&cpsl_cfg_mutex);
}

// TODO: for night to day change
int rk_smart_get_scene_param(double *pdLumaDay, double *pdLumaNight,
                             double *pdRGratio, double *pdBGratio,
                             unsigned int *p_u32_exposure, double *pdExpAgain) {
  return 0;
}

int night_to_day_para_set(rk_aiq_cpsl_cfg_t compensate_light_cfg) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  if (compensate_light_cfg.mode != RK_AIQ_OP_MODE_INVALID) {
    pthread_mutex_lock(&db_aiq_ctx_mutex);
    ret = rk_aiq_uapi_sysctl_setCpsLtCfg(db_aiq_ctx, &compensate_light_cfg);
    pthread_mutex_unlock(&db_aiq_ctx_mutex);
    LOG_DEBUG("set night to day para, ret is %d\n", ret);
  } else {
    LOG_INFO("compensate_light_cfg.mode is invalid\n");
  }
  return ret;
}

int fill_light_brightness_set(int strength) {
  pthread_mutex_lock(&cpsl_cfg_mutex);
  if (!gc_cpsl_cfg_init_flag)
    gc_cpsl_cfg_fill_light_mode_set(RK_AIQ_CPSLS_IR);
  gc_cpsl_cfg.u.m.strength_led = strength / 5 + 3;
  gc_cpsl_cfg.u.m.strength_ir = strength / 5 + 3;
  int ret = night_to_day_para_set(gc_cpsl_cfg);
  pthread_mutex_unlock(&cpsl_cfg_mutex);
  LOG_INFO("set fill light brightness %d, ret is %d\n", strength, ret);
  return ret;
}

int set_gray_open_led(int fill_light_brightness) {
  int ret = -1;
  pthread_mutex_lock(&cpsl_cfg_mutex);
  gc_cpsl_cfg.mode = RK_AIQ_OP_MODE_MANUAL;
  gc_cpsl_cfg.gray_on = true;
  gc_cpsl_cfg.u.m.on = 1;
  gc_cpsl_cfg.u.m.strength_led = fill_light_brightness / 5 + 3;
  gc_cpsl_cfg.u.m.strength_ir = fill_light_brightness / 5 + 3;
  ret = night_to_day_para_set(gc_cpsl_cfg);
  pthread_mutex_unlock(&cpsl_cfg_mutex);
  LOG_DEBUG("set gray open, ret is %d\n", ret);
  return ret;
}

int set_color_close_led() {
  int ret = -1;
  pthread_mutex_lock(&cpsl_cfg_mutex);
  if (!gc_cpsl_cfg_init_flag)
    gc_cpsl_cfg_fill_light_mode_set(RK_AIQ_CPSLS_IR);
  gc_cpsl_cfg.mode = RK_AIQ_OP_MODE_MANUAL;
  gc_cpsl_cfg.gray_on = false;
  gc_cpsl_cfg.u.m.on = 0;
  gc_cpsl_cfg.u.m.strength_led = 0;
  gc_cpsl_cfg.u.m.strength_ir = 0;
  ret = night_to_day_para_set(gc_cpsl_cfg);
  pthread_mutex_unlock(&cpsl_cfg_mutex);
  LOG_DEBUG("set color close, ret is %d\n", ret);
  return ret;
}

int night_to_day_auto_mode_set(int filter_level, int filter_time) {
  int ret = -1;
  pthread_mutex_lock(&cpsl_cfg_mutex);
  if (!gc_cpsl_cfg_init_flag)
    gc_cpsl_cfg_fill_light_mode_set(RK_AIQ_CPSLS_IR);
  gc_cpsl_cfg.mode = RK_AIQ_OP_MODE_AUTO;
  gc_cpsl_cfg.gray_on = false;
  gc_cpsl_cfg.u.a.sensitivity = (float)filter_level;
  gc_cpsl_cfg.u.a.sw_interval = (uint32_t)filter_time;
  ret = night_to_day_para_set(gc_cpsl_cfg);
  pthread_mutex_unlock(&cpsl_cfg_mutex);
  LOG_INFO("set night to day auto mode, ret is %d\n", ret);
  return ret;
}

int set_night_mode(int fill_light_brightness) {
  int ret = -1;
  ret = set_gray_open_led(fill_light_brightness);
  LOG_INFO("set night mode, ret is %d\n", ret);
  set_led_state(LED_ON);
  if (gc_hdr_mode != RK_AIQ_WORKING_MODE_NORMAL) {
    hdr_mode_set(RK_AIQ_WORKING_MODE_NORMAL, 0);
    LOG_INFO("switch to hdr normal in night mode\n");
  }
  return ret;
}

int set_day_mode(int hdr_mode) {
  int ret = -1;
  ret = set_color_close_led();
  LOG_INFO("set day mode, ret is %d\n", ret);
  set_led_state(LED_OFF);
  // TODO:get WideDynamicLevel only set hdr2 now
  if (hdr_mode == RK_AIQ_WORKING_MODE_ISP_HDR3) {
    hdr_mode_set(RK_AIQ_WORKING_MODE_ISP_HDR3, 0);
    LOG_INFO("set hdr3 when switching to day mode\n");
  } else if (hdr_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) {
    hdr_mode_set(RK_AIQ_WORKING_MODE_ISP_HDR2, 0);
    LOG_INFO("set hdr2 when switching to day mode\n");
  }
  return ret;
}

int bypass_stream_rotation_set(int rotation) {
  LOG_INFO("rotation: %d\n", rotation);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  rk_aiq_rotation_t rk_rotation = RK_AIQ_ROTATION_0;
  if (rotation == 0) {
    rk_rotation = RK_AIQ_ROTATION_0;
  } else if (rotation == 90) {
    rk_rotation = RK_AIQ_ROTATION_90;
  } else if (rotation == 270) {
    rk_rotation = RK_AIQ_ROTATION_270;
  } else {
    LOG_INFO("invalid roation value %d, set 0 by default\n", rotation);
  }

  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = rk_aiq_uapi_sysctl_setSharpFbcRotation(db_aiq_ctx, rk_rotation);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);

  LOG_DEBUG("set rotation ret is %d\n", ret);
  return ret;
}

int mirror_mode_set(flip_mode_t mode) {
  LOG_INFO("set mirror mode %d\n", mode);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int mirror = 0;
  int flip = 0;
  switch (mode) {
  case FM_INVALID: {
    return -1;
  }
  case FM_CLOSE: {
    break;
  }
  case FM_FLIP: {
    flip = 1;
    break;
  }
  case FM_MIRROR: {
    mirror = 1;
    break;
  }
  case FM_CENTER: {
    flip = 1;
    mirror = 1;
    break;
  }
  }
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  ret = rk_aiq_uapi_setMirroFlip(db_aiq_ctx, mirror, flip, 4);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO("set mirror %d, set flip %d, ret is %d\n", mirror, flip, ret);
  return ret;
}

static int nr_level_set_no_mutex(int spatial_level, int temporal_level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  if (spatial_level >= 0) {
    ret = rk_aiq_uapi_setMSpaNRStrth(db_aiq_ctx, true, spatial_level); //[0,100]
    LOG_DEBUG("rk_aiq_uapi_setMSpaNRStrth level is %d, ret is %d\n",
              spatial_level, ret);
  }

  if (temporal_level >= 0) {
    ret = rk_aiq_uapi_setMTNRStrth(db_aiq_ctx, true, temporal_level); //[0,100]
    LOG_DEBUG("rk_aiq_uapi_setMTNRStrth level is %d, ret is %d\n",
              temporal_level, ret);
  }
  return ret;
}

int nr_level_set(int spatial_level, int temporal_level) {
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = nr_level_set_no_mutex(spatial_level, temporal_level);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int nr_para_set(nr_mode_t mode, int spatial_level, int temporal_level) {
  LOG_DEBUG("mode is %d, spatial level is %d, temporal level is %d\n", mode,
            spatial_level, temporal_level);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  ret = rk_aiq_uapi_sysctl_setModuleCtl(db_aiq_ctx, RK_MODULE_NR, true); // 2D
  LOG_DEBUG("rk_aiq_uapi_sysctl_setModuleCtl 2d ret is %d\n", ret);
  ret = rk_aiq_uapi_sysctl_setModuleCtl(db_aiq_ctx, RK_MODULE_TNR, true); // 3D
  LOG_DEBUG("rk_aiq_uapi_sysctl_setModuleCtl 3d ret is %d\n", ret);
  switch (mode) {
  case NR_INVAILD: {
    LOG_WARN("set invalide nr mode\n");
    pthread_mutex_unlock(&db_aiq_ctx_mutex);
    return -1;
  }
  case NR_DEFAULT: {
    spatial_level = DEFAULT_SPATIAL_DENOIZE_LEVEL;
    temporal_level = DEFAULT_TEMPORAL_DENOIZE_LEVEL;
    break;
  }
  case NR_2D: {
    temporal_level = DEFAULT_TEMPORAL_DENOIZE_LEVEL;
    break;
  }
  case NR_3D: {
    spatial_level = DEFAULT_SPATIAL_DENOIZE_LEVEL;
    break;
  }
  case NR_MIX: {
    break;
  }
  default: {
    spatial_level = DEFAULT_SPATIAL_DENOIZE_LEVEL;
    temporal_level = DEFAULT_TEMPORAL_DENOIZE_LEVEL;
    LOG_WARN("undefined nr mode %d. use default instead\n", mode);
    break;
  }
  }
  if (!ret) {
    ret = nr_level_set_no_mutex(spatial_level, temporal_level);
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

static int ldch_level_set_no_mutex(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  level = level < 0 ? 0 : level;
  level = (int)(level * 2.53 + 2);
  int ret = rk_aiq_uapi_setLdchCorrectLevel(db_aiq_ctx,
                                            level); // [1, 100] -> [2 , 255]
  LOG_DEBUG("set ldch level %d ret is %d\n", level, ret);
  return ret;
}

int ldch_level_set(int level) {
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = ldch_level_set_no_mutex(level);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO("set ldch level %d, ret is %d\n", level, ret);
  return ret;
}

/* not support modify dynamic */
static int fec_switch_no_mutex(bool swi) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  rk_aiq_fec_attrib_t attr;
  int ret = -1;
  ret = rk_aiq_uapi_setFecEn(db_aiq_ctx, swi);
  LOG_DEBUG("switch fec %d, ret: %d\n", swi, ret);
  return ret;
}

void gc_dc_mode_set(dc_mode_t mode) { gc_dc_mode = mode; }

static int fec_level_set_no_mutex(int fec_level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  fec_level = fec_level < 0 ? 0 : fec_level;
  fec_level = fec_level * 2.55;
  fec_level = fec_level > 255 ? 255 : fec_level;
  int ret = rk_aiq_uapi_setFecCorrectLevel(db_aiq_ctx,
                                           fec_level); // [0-100] -> [0->255]
  LOG_DEBUG("set fec level:%d, ret is %d\n", fec_level, ret);
}

int fec_level_set(int fec_level) {
  if (gc_dc_mode != DC_FEC) {
    LOG_INFO("no in fec mode, set level fail\n");
    return -1;
  }
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  ret = fec_level_set_no_mutex(fec_level);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int dc_para_set(dc_mode_t mode, int ldch_level, int fec_level) {
  LOG_INFO("mode is %d, ldch_level is %d, fec_level is %d\n", mode, ldch_level,
           fec_level);
  if (!db_aiq_ctx || !g_stream_on) {
    return -1;
  }
  int ret = 0;
  int reset_flag = 0;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (mode) {
  case DC_INVALID: {
    LOG_WARN("set invalid distortion correction mode\n");
    ret = -1;
    break;
  }
  case DC_LDCH: {
    if (gc_dc_mode == DC_FEC) {
      reset_flag = 1;
      LOG_INFO("close fec ret is %d\n", ret);
    } else {
      ret = rk_aiq_uapi_setLdchEn(db_aiq_ctx, true);
      LOG_INFO("open ldch ret is %d\n", ret);
      if (ldch_level >= 0)
        ret = ldch_level_set_no_mutex(ldch_level);
    }
    gc_dc_mode = DC_LDCH;
    break;
  }
  case DC_FEC: {
    if (gc_dc_mode != DC_FEC) {
      reset_flag = 1;
    } else {
      if (fec_level > 0)
        fec_level_set_no_mutex(fec_level);
    }
    if (gc_dc_mode == DC_LDCH) {
      ret = rk_aiq_uapi_setLdchEn(db_aiq_ctx, false);
      LOG_INFO("close ldch ret is %d\n", ret);
    }
    LOG_DEBUG("open fec\n");
    gc_dc_mode = DC_FEC;
    break;
  }
  case DC_CLOSE: {
    if (gc_dc_mode == DC_LDCH) {
      ret = rk_aiq_uapi_setLdchEn(db_aiq_ctx, false);
      LOG_INFO("close ldch ret is %d\n", ret);
    }
    if (gc_dc_mode == DC_FEC) {
      reset_flag = 1;
      LOG_INFO("close fec\n");
    }
    gc_dc_mode = DC_CLOSE;
    break;
  }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  if (reset_flag) {
    reset_flow();
  }
  return ret;
}

int dehaze_strength_set_no_mutex(int level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  if (level < 0)
    return ret;
  level = level < 1 ? 1 : level;
  level = level > 10 ? 10 : level;
  ret = rk_aiq_uapi_setMDhzStrth(db_aiq_ctx, true, level);
  LOG_INFO("set dehaze level %d, ret is %d\n", level, ret);
  return ret;
}

int dehaze_strength_set(int level) {
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = dehaze_strength_set_no_mutex(level);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int dehaze_para_set(work_mode_2_t mode, int level) {
  int ret = 0;
  LOG_DEBUG("dehaze mode is %d, dehaze level is %d \n", mode, level);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  if (WM2_INVALID_MODE == gc_dehaze_mode && WM2_INVALID_MODE != mode) {
    gc_dehaze_mode = mode;
  } else if (gc_dehaze_mode != mode) {
    gc_dehaze_mode = mode;
    reset_flow();
    return ret;
  }
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (mode) {
  case WM2_CLOSE_MODE: {
    ret = rk_aiq_uapi_disableDhz(db_aiq_ctx);
    break;
  }
  case WM2_OPEN_MODE: {
    ret = rk_aiq_uapi_enableDhz(db_aiq_ctx);
    ret = rk_aiq_uapi_setDhzMode(db_aiq_ctx, OP_MANUAL);
    ret = dehaze_strength_set_no_mutex(level);
    break;
  }
  case WM2_AUTO_MODE: {
    ret = rk_aiq_uapi_enableDhz(db_aiq_ctx);
    ret = rk_aiq_uapi_setDhzMode(db_aiq_ctx, OP_AUTO);
    break;
  }
  case WM2_INVALID_MODE: {
    LOG_WARN("set invalid dehaze mode\n");
    ret = -1;
    break;
  }
  default: {
    if (gc_dehaze_mode == WM2_OPEN_MODE)
      ret = dehaze_strength_set_no_mutex(level);
  }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}
int exposure_weight_str2array(char *weight,int32_t* g_weight){
    int ret = 0;
    gchar **arr;
    gint i;
    if(weight == NULL){
      return -1;
    }
    LOG_INFO("%s\n", weight);
    if(strstr(weight,",") == NULL){
      return -1;
    }
    arr = g_strsplit(weight, ",", 0);
    for (i = 0; arr[i] != NULL; i++){
	g_weight[i] = atoi(arr[i]);
    }
    return ret;
}
float exposure_time_str2float(char *time) {
  float numerator, denominator, rst;
  if (strcmp(time, "1")) {
    sscanf(time, "%f/%f", &numerator, &denominator);
    rst = numerator / denominator;
  } else {
    sscanf(time, "%f", &rst);
  }
  return rst;
}

int exposure_time_set(char *time) {
  LOG_INFO("time is %s\n", time);
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  paRange_t range;
  range.min = 1e-5; // TODO: obtained from capability
  float numerator, denominator;
  if (strcmp(time, "1")) {
    sscanf(time, "%f/%f", &numerator, &denominator);
    range.max = numerator / denominator;
  } else {
    sscanf(time, "%f", &range.max);
  }
  LOG_INFO("min is %f, max is %f\n", range.min, range.max);
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = rk_aiq_uapi_setExpTimeRange(db_aiq_ctx, &range);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int exposure_gain_set(int gain) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  paRange_t range;
  range.min = 1; // TODO: obtained from capability
  range.max = 1;
  if (gain)
    range.max = (float)gain;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  ret = rk_aiq_uapi_setExpGainRange(db_aiq_ctx, &range);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_DEBUG("set exposure min is %f, max is %f, ret is %d\n", range.min,
            range.max, ret);
  return ret;
}

int auto_exposure_set() {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  Uapi_ExpSwAttr_t stExpSwAttr;
  int ret = -1;

  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_user_api_ae_getExpSwAttr(db_aiq_ctx, &stExpSwAttr);
  stExpSwAttr.AecOpType = RK_AIQ_OP_MODE_AUTO;
  ret = rk_aiq_user_api_ae_setExpSwAttr(db_aiq_ctx, stExpSwAttr);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO("Set auto exposure, ret is %d\n", ret);
  return ret;
}

int exposure_info_get(Uapi_ExpQueryInfo_t *stExpInfo, rk_aiq_wb_cct_t *stCCT) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = 0;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  if (stExpInfo)
    ret = rk_aiq_user_api_ae_queryExpResInfo(db_aiq_ctx, stExpInfo);
  if (stCCT)
    ret = rk_aiq_user_api_awb_GetCCT(db_aiq_ctx, stCCT);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int manual_exposure_auto_gain_set_char(char *time) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  Uapi_ExpSwAttr_t stExpSwAttr;

  float expTime = exposure_time_str2float(time);
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_user_api_ae_getExpSwAttr(db_aiq_ctx, &stExpSwAttr);
  stExpSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
  stExpSwAttr.stManual.stLinMe.ManualGainEn = false;
  stExpSwAttr.stManual.stLinMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stLinMe.TimeValue = expTime;

  stExpSwAttr.stManual.stHdrMe.ManualGainEn = false;
  stExpSwAttr.stManual.stHdrMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[0] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[1] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[2] = expTime;
  int ret = rk_aiq_user_api_ae_setExpSwAttr(db_aiq_ctx, stExpSwAttr);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO(
      "set manual exposure and auto gain, exposure time is %f, ret is %d\n",
      expTime, ret);
  return ret;
}

int manual_exposure_auto_gain_set_float(float expTime) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  Uapi_ExpSwAttr_t stExpSwAttr;

  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_user_api_ae_getExpSwAttr(db_aiq_ctx, &stExpSwAttr);
  for (size_t i = 0; i < sizeof(exposure_uvc_time); i++)
  {
    if (expTime == exposure_uvc_time[i])
    {
      expTime = exposure_isp_time[i];
    }
  }

  stExpSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
  stExpSwAttr.stManual.stLinMe.ManualGainEn = false;
  stExpSwAttr.stManual.stLinMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stLinMe.TimeValue = expTime;

  stExpSwAttr.stManual.stHdrMe.ManualGainEn = false;
  stExpSwAttr.stManual.stHdrMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[0] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[1] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[2] = expTime;
  int ret = rk_aiq_user_api_ae_setExpSwAttr(db_aiq_ctx, stExpSwAttr);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO(
      "set manual exposure and auto gain, exposure time is %f, ret is %d\n",
      expTime, ret);
  return ret;
}

int manual_exposure_manual_gain_set_char(char *time, int gain) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  Uapi_ExpSwAttr_t stExpSwAttr;
  float expTime = exposure_time_str2float(time);
  // gain[1~100]; if (int)(gain *2.55) + 1.0 then gain[0~100] ->
  // GainValue[1~256]
  float gain_set = (gain * 1.0f);
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_user_api_ae_getExpSwAttr(db_aiq_ctx, &stExpSwAttr);
  stExpSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
  stExpSwAttr.stManual.stLinMe.ManualGainEn = true;
  stExpSwAttr.stManual.stLinMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stLinMe.TimeValue = expTime;
  stExpSwAttr.stManual.stLinMe.GainValue = gain_set;

  stExpSwAttr.stManual.stHdrMe.ManualGainEn = true;
  stExpSwAttr.stManual.stHdrMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[0] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[1] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[2] = expTime;
  stExpSwAttr.stManual.stHdrMe.GainValue.fCoeff[0] = gain_set;
  stExpSwAttr.stManual.stHdrMe.GainValue.fCoeff[1] = gain_set;
  stExpSwAttr.stManual.stHdrMe.GainValue.fCoeff[2] = gain_set;
  int ret = rk_aiq_user_api_ae_setExpSwAttr(db_aiq_ctx, stExpSwAttr);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO("expTime is %f, gain is %f, ret is %d\n", expTime, gain_set, ret);
  return ret;
}

int manual_exposure_manual_gain_set_float(float expTime, int gain) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  Uapi_ExpSwAttr_t stExpSwAttr;
  // gain[1~100]; if (int)(gain *2.55) + 1.0 then gain[0~100] ->
  // GainValue[1~256]
  float gain_set = (gain * 1.0f);
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_user_api_ae_getExpSwAttr(db_aiq_ctx, &stExpSwAttr);
  stExpSwAttr.AecOpType = RK_AIQ_OP_MODE_MANUAL;
  stExpSwAttr.stManual.stLinMe.ManualGainEn = true;
  stExpSwAttr.stManual.stLinMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stLinMe.TimeValue = expTime;
  stExpSwAttr.stManual.stLinMe.GainValue = gain_set;

  stExpSwAttr.stManual.stHdrMe.ManualGainEn = true;
  stExpSwAttr.stManual.stHdrMe.ManualTimeEn = true;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[0] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[1] = expTime;
  stExpSwAttr.stManual.stHdrMe.TimeValue.fCoeff[2] = expTime;
  stExpSwAttr.stManual.stHdrMe.GainValue.fCoeff[0] = gain_set;
  stExpSwAttr.stManual.stHdrMe.GainValue.fCoeff[1] = gain_set;
  stExpSwAttr.stManual.stHdrMe.GainValue.fCoeff[2] = gain_set;
  int ret = rk_aiq_user_api_ae_setExpSwAttr(db_aiq_ctx, stExpSwAttr);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  LOG_INFO("expTime is %f, gain is %f, ret is %d\n", expTime, gain_set, ret);
  return ret;
}

int manual_exposure_grid_weight_set(int32_t* weight,int evbias) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  LOG_INFO(" xy is %d,%d . wh is %d,%d\n",weight[0],weight[1],weight[2],weight[3] );
  Uapi_ExpSwAttr_t stExpSwAttr;
  Uapi_LinExpAttr_t pLinExpAttr;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  rk_aiq_user_api_ae_getExpSwAttr(db_aiq_ctx, &stExpSwAttr);
  rk_aiq_user_api_ae_getLinExpAttr(db_aiq_ctx, &pLinExpAttr);
  pLinExpAttr.Evbias = evbias;
  stExpSwAttr.stAdvanced.enable = true;
  uint8_t DayGridWeights[225]={
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
  for(int i = 0; i < 225; i++){
     if(weight[i] != 0 && weight[i] < 225){
	DayGridWeights[weight[i]] = 15;
     }
  }
  if(weight[0] == 0 && weight[1] == 0){
    for(int i = 0; i < 225; i++){
        DayGridWeights[i] = 1;
    }
    pLinExpAttr.Evbias = 0;
    LOG_INFO(" xy is %d,%d . wh is %d,%d\n",weight[0],weight[1],weight[2],weight[3]);
    //stExpSwAttr.stAdvanced.enable = false;
  }
  memcpy(stExpSwAttr.stAdvanced.DayGridWeights,DayGridWeights,sizeof(stExpSwAttr.stAdvanced.DayGridWeights));
  memcpy(stExpSwAttr.stAdvanced.NightGridWeights,DayGridWeights,sizeof(stExpSwAttr.stAdvanced.DayGridWeights));
  int ret = rk_aiq_user_api_ae_setExpSwAttr(db_aiq_ctx, stExpSwAttr);
  ret = rk_aiq_user_api_ae_setLinExpAttr(db_aiq_ctx, pLinExpAttr);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}
static int blc_region_strength_set_no_mutex(int strength) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  if (strength >= 0 && strength <= 100) {
    ret = rk_aiq_uapi_setBLCStrength(db_aiq_ctx, strength);
    LOG_INFO("rk_aiq_uapi_setBLCStrength set %d ret is %d\n", strength, ret);
  } else {
    LOG_WARN("rk_aiq_uapi_setBLCStrength set %d over range\n", strength);
  }
  return ret;
}

int blc_region_para_set(work_mode_1_t mode, int strength) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (mode) {
  case WM1_CLOSE_MODE: {
    ret = rk_aiq_uapi_setBLCMode(db_aiq_ctx, false, AE_MEAS_AREA_AUTO);
    LOG_INFO("rk_aiq_uapi_setBLCMode close ret is %d\n", ret);
    break;
  }
  case WM1_OPEN_MODE: {
    ret = rk_aiq_uapi_setBLCMode(db_aiq_ctx, true, AE_MEAS_AREA_AUTO);
    LOG_INFO("rk_aiq_uapi_setBLCMode open ret is %d\n", ret);
    usleep(30000);
    ret = blc_region_strength_set_no_mutex(strength);
    break;
  }
  default: { LOG_WARN("invalid blc region mode: %d\n", mode); }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int blc_region_strength_set(int strength) {
  int ret = blc_region_para_set(WM1_OPEN_MODE, strength);
  // pthread_mutex_lock(&db_aiq_ctx_mutex);
  // int ret = blc_region_strength_set_no_mutex(strength);
  // pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

static int blc_hlc_level_set_no_mutex(int hlc_level, int dark_level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = 0;
  if (hlc_level >= 0) {
    ret = rk_aiq_uapi_setHLCStrength(db_aiq_ctx, hlc_level); // level[1, 100]
    LOG_INFO("rk_aiq_uapi_setHLCStrength ret is %d\n", ret);
  }
  if (dark_level >= 0) {
    ret = rk_aiq_uapi_setDarkAreaBoostStrth(
        db_aiq_ctx, (int)(dark_level / 10)); // level[1, 100] -> [0, 10]
    LOG_INFO("rk_aiq_uapi_setDarkAreaBoostStrth ret is %d\n", ret);
  }
  return ret;
}

int blc_hlc_para_set(work_mode_1_t mode, int hlc_level, int dark_level) {
  if (!db_aiq_ctx || !g_stream_on)
    return -1;
  int ret = -1;
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  switch (mode) {
  case WM1_CLOSE_MODE: {
    ret = rk_aiq_uapi_setHLCMode(db_aiq_ctx, false);
    LOG_INFO("rk_aiq_uapi_setHLCMode close ret is %d\n", ret);
    break;
  }
  case WM1_OPEN_MODE: {
    ret = rk_aiq_uapi_setHLCMode(db_aiq_ctx, true);
    LOG_INFO("rk_aiq_uapi_setHLCMode open ret is %d\n", ret);
    blc_hlc_level_set_no_mutex(hlc_level, dark_level);
    break;
  }
  case WM1_INVALID_MODE: {
    LOG_WARN("invalid mode: %d\n", mode);
    break;
  }
  }
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int blc_hlc_level_set(int hlc_level, int dark_level) {
  pthread_mutex_lock(&db_aiq_ctx_mutex);
  int ret = blc_hlc_level_set_no_mutex(hlc_level, dark_level);
  pthread_mutex_unlock(&db_aiq_ctx_mutex);
  return ret;
}

int isp_status_sender_register(ispserver_status_signal_send send_func) {
  if (!send_func)
    return -1;
  g_send_func = send_func;
  LOG_INFO("register success: %p", g_send_func);
  return 0;
}
#endif
