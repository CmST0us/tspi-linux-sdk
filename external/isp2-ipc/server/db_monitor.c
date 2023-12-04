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
#include "call_fun_ipc.h"
#include "config.h"
#include "db_monitor.h"
#include "dbserver.h"
#include "dbus_signal.h"
#include "libipcpro_log_control.h"
#include "json-c/json.h"
#include "mediactl/mediactl.h"
#include "rkaiq/common/rk_aiq_types.h"
#include "rkaiq/uAPI/rk_aiq_user_api_ae.h"
#include "rkaiq/uAPI/rk_aiq_user_api_imgproc.h"

enum Scenario {
  SCE_INVALID = -1,
  SCE_NORMAL = 0,
  SCE_FRONT_LIGHT,
  SCE_LOW_ILLUMINATION,
  SCE_CUSTOM1,
  SCE_CUSTOM2
} gc_scenario;

struct AdjustmentConfig {
  int Brightness; // [0-100]
  int Contrast;   // [0-100]
  int Saturation; // [0-100]
  int Sharpness;  // [0-100]
  int Hue;        // [0-100]
};

struct ExposureConfig {
  work_mode_0_t AutoExposureEnabled; // [0, 1]
  work_mode_0_t AutoGainEnabled;     // [0, 1]
  float ExposureTime;                // [0.00001-1]
  int ExposureGain;                  // [0-100]
};
struct Night2DayConfig {
  night2day_mode_t NightToDay;  // [0-3]
  int NightToDayFilterLevel;    // []
  int NightToDayFilterTime;     // []
  int32_t DawnSeconds;          // [0-86400]
  int32_t DuskSeconds;          // [0-86400]
  rk_aiq_cpsls_t FillLightMode; // [1-3]
  int LightBrightness;          // [1-100]
};

struct BLCConfig {
  work_mode_1_t BLC; // [0, 1]
  int BLCStrength;   // [1-100]
  rk_aiq_working_mode_t HDR;
  int HDRLevel;       // [1-4]
  work_mode_1_t HLC;  // [0, 1]
  int HLCLevel;       // [1-100]
  int DarkBoostLevel; // [1-100]
};

struct WhiteBalanceConfig {
  white_balance_mode_t WhiteBlanceStyle; // [0-6]
  int WhiteBalanceRed;                   // [1-100]
  int WhiteBalanceGreen;                 // [1-100]
  int WhiteBalanceBlue;                  // [1-100]
};

struct EnhancementConfig {
  nr_mode_t NoiseReduceMode;      // [0-3]
  int DenoiseLevel;               // [1-100]
  int SpatialDenoiseLevel;        // [1-100]
  int TemporalDenoiseLevel;       // [1-100]
  work_mode_2_t Dehaze;           // [0-3]
  int DehazeLevel;                // [1-10];
  gs_mode_t GrayScaleMode;        // set in ipcweb backend through mediaserver
  int ImageRotation;              // [0, 90, 270]
  dc_mode_t DistortionCorrection; // [0-2]
  int LdchLevel;                  // [0-100]
  int FecLevel;                   // [0-100]
};

struct VideoAdjustmentConfig {
  flip_mode_t ImageFlip;                   // [0-3]
  expPwrLineFreq_t PowerLineFrequencyMode; // [1, 2]
};

extern rk_aiq_sys_ctx_t *db_aiq_ctx;
static GHashTable *db_adjustment_hash = NULL;
static GHashTable *db_exposure_hash = NULL;
static GHashTable *db_night2day_hash = NULL;
static GHashTable *db_blc_hash = NULL;
static GHashTable *db_white_blance_hash = NULL;
static GHashTable *db_enhancement_hash = NULL;
static GHashTable *db_video_adjustment_hash = NULL;
static DBusConnection *connection = 0;
static int database_init_flag = 0;
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *scenario_str_get() {
  switch (gc_scenario) {
  case SCE_INVALID:
    return "-1";
  case SCE_NORMAL:
    return "0";
  case SCE_FRONT_LIGHT:
    return "1";
  case SCE_LOW_ILLUMINATION:
    return "2";
  case SCE_CUSTOM1:
    return "3";
  case SCE_CUSTOM2:
    return "4";
  default:
    return "0";
  }
}

static work_mode_0_t string2work_mode_0_t(char *work_mode) {
  if (NULL == work_mode) {
    return WM0_INVALID_MODE;
  } else if (g_str_equal(work_mode, "manual")) {
    return WM0_MANAUL_MODE;
  } else if (g_str_equal(work_mode, "auto")) {
    return WM0_AUTO_MODE;
  } else {
    return WM0_INVALID_MODE;
  }
}

static work_mode_1_t string2work_mode_1_t(char *work_mode) {
  if (NULL == work_mode) {
    return WM1_INVALID_MODE;
  } else if (g_str_equal(work_mode, "close")) {
    return WM1_CLOSE_MODE;
  } else if (g_str_equal(work_mode, "open")) {
    return WM1_OPEN_MODE;
  } else {
    return WM1_INVALID_MODE;
  }
}

static work_mode_2_t string2work_mode_2_t(char *work_mode) {
  if (NULL == work_mode) {
    return WM2_INVALID_MODE;
  } else if (g_str_equal(work_mode, "close")) {
    return WM2_CLOSE_MODE;
  } else if (g_str_equal(work_mode, "open")) {
    return WM2_OPEN_MODE;
  } else if (g_str_equal(work_mode, "auto")) {
    return WM2_AUTO_MODE;
  } else {
    return WM2_INVALID_MODE;
  }
}

static night2day_mode_t string2night2day_mode_t(char *mode) {
  if (NULL == mode) {
    return ND_INVALID_MODE;
  } else if (g_str_equal(mode, "day")) {
    return ND_DAY_MODE;
  } else if (g_str_equal(mode, "night")) {
    return ND_NIGHT_MODE;
  } else if (g_str_equal(mode, "auto")) {
    return ND_AUTO_MODE;
  } else if (g_str_equal(mode, "schedule")) {
    return ND_SCHEDULE_MODE;
  } else {
    return ND_INVALID_MODE;
  }
}

static rk_aiq_working_mode_t string2hdr_mode(char *mode) {
  if (NULL == mode) {
    return RK_AIQ_WORKING_MODE_NORMAL;
  } else if (g_str_equal(mode, "close")) {
    return RK_AIQ_WORKING_MODE_NORMAL;
  } else if (g_str_equal(mode, "HDR2")) {
    return RK_AIQ_WORKING_MODE_ISP_HDR2;
  } else if (g_str_equal(mode, "HDR3")) {
    return RK_AIQ_WORKING_MODE_ISP_HDR3;
  } else {
    return RK_AIQ_WORKING_MODE_NORMAL;
  }
}

static rk_aiq_cpsls_t fill_light_enum_get(char *mode) {
  if (NULL == mode)
    return RK_AIQ_CPSLS_INVALID;
  else if (g_str_equal(mode, "LED"))
    return RK_AIQ_CPSLS_LED;
  else if (g_str_equal(mode, "IR"))
    return RK_AIQ_CPSLS_IR;
  else if (g_str_equal(mode, "MIX"))
    return RK_AIQ_CPSLS_MIX;
  else
    return RK_AIQ_CPSLS_INVALID;
}

static white_balance_mode_t string2white_balance_mode_t(char *mode) {
  if (NULL == mode)
    return WB_INVALID;
  else if (g_str_equal(mode, "autoWhiteBalance"))
    return WB_AUTO;
  else if (g_str_equal(mode, "manualWhiteBalance"))
    return WB_MANUAL;
  else if (g_str_equal(mode, "lockingWhiteBalance"))
    return WB_LOCK;
  else if (g_str_equal(mode, "fluorescentLamp"))
    return WB_FLUORESCENT_LAMP;
  else if (g_str_equal(mode, "incandescent"))
    return WB_INCANDESCENT;
  else if (g_str_equal(mode, "warmLight"))
    return WB_WARM_LIGHT;
  else if (g_str_equal(mode, "naturalLight"))
    return WB_NATURE_LIGHT;
  else
    return WB_INVALID;
}

static nr_mode_t string2nr_mode_t(char *mode) {
  if (NULL == mode)
    return NR_INVAILD;
  else if (g_str_equal(mode, "default"))
    return NR_DEFAULT;
  else if (g_str_equal(mode, "2dnr"))
    return NR_2D;
  else if (g_str_equal(mode, "3dnr"))
    return NR_3D;
  else if (g_str_equal(mode, "mixnr"))
    return NR_MIX;
  else
    return NR_INVAILD;
}

static gs_mode_t string2gs_mode_t(char *mode) {
  if (NULL == mode)
    return GS_INVALID;
  else if (g_str_equal(mode, "[0-255]"))
    return GS_0_255;
  else if (g_str_equal(mode, "[16-235]"))
    return GS_16_235;
  else
    return GS_INVALID;
}

static dc_mode_t string2dc_mode_t(char *mode) {
  if (NULL == mode)
    return DC_INVALID;
  else if (g_str_equal(mode, "LDCH"))
    return DC_LDCH;
  else if (g_str_equal(mode, "FEC"))
    return DC_FEC;
  else if (g_str_equal(mode, "close"))
    return DC_CLOSE;
  else
    return DC_INVALID;
}

static expPwrLineFreq_t string2frequency_mode(char *mode) {
  if (NULL == mode)
    return EXP_PWR_LINE_FREQ_60HZ;
  else if (g_str_equal(mode, "PAL(50HZ)"))
    return EXP_PWR_LINE_FREQ_50HZ;
  else if (g_str_equal(mode, "NTSC(60HZ)"))
    return EXP_PWR_LINE_FREQ_60HZ;
  else
    return EXP_PWR_LINE_FREQ_60HZ;
}

static flip_mode_t string2flip_mode(char *mode) {
  if (NULL == mode)
    return FM_INVALID;
  else if (g_str_equal(mode, "close"))
    return FM_CLOSE;
  else if (g_str_equal(mode, "flip"))
    return FM_FLIP;
  else if (g_str_equal(mode, "mirror"))
    return FM_MIRROR;
  else if (g_str_equal(mode, "centrosymmetric"))
    return FM_CENTER;
  else
    return FM_INVALID;
}

char *dbserver_image_hdr_mode_get(void) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_BLC);
  // LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image blc table is null\n");
    return NULL;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  char *HDR_mode =
      (char *)json_object_get_string(json_object_object_get(j_data, "sHDR"));
  json_object_put(j_cfg);
  free(json_str);
  return HDR_mode;
}

int hash_image_hdr_mode_get4init(rk_aiq_working_mode_t *hdr_mode) {
  char *sce_str = scenario_str_get();
  struct BLCConfig *blc_cfg = g_hash_table_lookup(db_blc_hash, sce_str);
  struct EnhancementConfig *enhancement_cfg =
      g_hash_table_lookup(db_enhancement_hash, sce_str);
  if (NULL == blc_cfg) {
    LOG_INFO("hash image blc table is null\n");
    return -1;
  } else {
    if (blc_cfg->HDR == RK_AIQ_WORKING_MODE_NORMAL ||
        get_led_state() == LED_ON) {
      hdr_global_value_set(RK_AIQ_WORKING_MODE_NORMAL);
      if (hdr_mode)
        *hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    } else if (blc_cfg->HDR == RK_AIQ_WORKING_MODE_ISP_HDR2) {
      hdr_global_value_set(RK_AIQ_WORKING_MODE_ISP_HDR2);
      if (hdr_mode)
        *hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
    } else if (blc_cfg->HDR == RK_AIQ_WORKING_MODE_ISP_HDR3) {
      hdr_global_value_set(RK_AIQ_WORKING_MODE_ISP_HDR3);
      if (hdr_mode)
        *hdr_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
    } else {
      hdr_global_value_set(RK_AIQ_WORKING_MODE_NORMAL);
      if (hdr_mode)
        *hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
    }
  }
  LOG_DEBUG("init_hdr_mode: %d\n", hdr_global_value_get());
  return 0;
}

int hash_image_hdr_mode_get_without_sync_gc() {
  char *sce_str = scenario_str_get();
  struct BLCConfig *blc_cfg = g_hash_table_lookup(db_blc_hash, sce_str);
  if (NULL == blc_cfg) {
    LOG_INFO("hash image blc table is null\n");
    return -1;
  } else {
    if (blc_cfg->HDR == RK_AIQ_WORKING_MODE_NORMAL) {
      return RK_AIQ_WORKING_MODE_NORMAL;
    } else if (blc_cfg->HDR == RK_AIQ_WORKING_MODE_ISP_HDR2) {
      return RK_AIQ_WORKING_MODE_ISP_HDR2;
    } else if (blc_cfg->HDR == RK_AIQ_WORKING_MODE_ISP_HDR3) {
      return RK_AIQ_WORKING_MODE_ISP_HDR3;
    } else {
      return RK_AIQ_WORKING_MODE_NORMAL;
    }
  }
}

int hash_image_fec_enable_get4init(int *fec_en, int *fec_level) {
  char *sce_str = scenario_str_get();
  struct EnhancementConfig *enhancement_cfg =
      g_hash_table_lookup(db_enhancement_hash, sce_str);
  if (NULL == enhancement_cfg) {
    LOG_WARN("enhancement_cfg is null\n");
    return -1;
  }
  if (fec_en) {
    if (enhancement_cfg->DistortionCorrection == DC_FEC) {
      *fec_en = 1;
      gc_dc_mode_set(DC_FEC);
    } else {
      *fec_en = 0;
    }
  }
  if (fec_level) {
    *fec_level = enhancement_cfg->FecLevel;
  }
  return 0;
}

void dbserver_image_blc_get(char *hdr_mode, int *hdr_level, int *blc_strength,
                            int *hlc_level, int *dark_level) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_BLC);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image blc table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  if (hdr_mode) {
    char *hdr =
        (char *)json_object_get_string(json_object_object_get(j_data, "sHDR"));
    strncpy(hdr_mode, hdr, strlen(hdr));
    hdr_mode[strlen(hdr)] = '\0';
  }
  if (hdr_level)
    *hdr_level =
        (int)json_object_get_int(json_object_object_get(j_data, "iHDRLevel"));
  if (blc_strength)
    *blc_strength = (int)json_object_get_int(
        json_object_object_get(j_data, "iBLCStrength"));
  if (hlc_level)
    *hlc_level =
        (int)json_object_get_int(json_object_object_get(j_data, "iHLCLevel"));
  if (dark_level)
    *dark_level = (int)json_object_get_int(
        json_object_object_get(j_data, "iDarkBoostLevel"));
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_blc_get(rk_aiq_working_mode_t *hdr_mode, int *hdr_level,
                       int *blc_strength, int *hlc_level, int *dark_level) {
  char *sce_str = scenario_str_get();
  struct BLCConfig *blc_cfg = g_hash_table_lookup(db_blc_hash, sce_str);
  if (blc_cfg == NULL) {
    LOG_WARN("blc_cfg is null\n");
    return -1;
  } else {
    if (hdr_mode)
      *hdr_mode = blc_cfg->HDR;
    if (hdr_level)
      *hdr_level = blc_cfg->HDRLevel;
    if (blc_strength)
      *blc_strength = blc_cfg->BLCStrength;
    if (hlc_level)
      *hlc_level = blc_cfg->HLCLevel;
    if (dark_level)
      *dark_level = blc_cfg->DarkBoostLevel;
  }
  return 0;
}

void dbserver_image_enhancement_get(char *nr_mode, char *fec_mode,
                                    char *dehaze_mode, int *denoise_level,
                                    int *spatial_level, int *temporal_level,
                                    int *dehaze_level, int *fec_level,
                                    int *rotation) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_ENHANCEMENT);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image enhancement table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  if (nr_mode) {
    char *nr = (char *)json_object_get_string(
        json_object_object_get(j_data, "sNoiseReduceMode"));
    strncpy(nr_mode, nr, strlen(nr));
    nr_mode[strlen(nr)] = '\0';
  }
  if (fec_mode) {
    char *fec =
        (char *)json_object_get_string(json_object_object_get(j_data, "sFEC"));
    strncpy(fec_mode, fec, strlen(fec));
    fec_mode[strlen(fec)] = '\0';
  }
  if (dehaze_mode) {
    char *dehaze = (char *)json_object_get_string(
        json_object_object_get(j_data, "sDehaze"));
    strncpy(dehaze_mode, dehaze, strlen(dehaze));
    dehaze_mode[strlen(dehaze)] = '\0';
  }
  if (denoise_level)
    *denoise_level = (int)json_object_get_int(
        json_object_object_get(j_data, "iDenoiseLevel"));
  if (spatial_level)
    *spatial_level = (int)json_object_get_int(
        json_object_object_get(j_data, "iSpatialDenoiseLevel"));
  if (temporal_level)
    *temporal_level = (int)json_object_get_int(
        json_object_object_get(j_data, "iTemporalDenoiseLevel"));
  if (dehaze_level)
    *dehaze_level = (int)json_object_get_int(
        json_object_object_get(j_data, "iDehazeLevel"));
  if (fec_level)
    *fec_level =
        (int)json_object_get_int(json_object_object_get(j_data, "iFecLevel"));
  if (rotation)
    *rotation = (int)json_object_get_int(
        json_object_object_get(j_data, "iImageRotation"));
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_enhancement_get(nr_mode_t *nr_mode, dc_mode_t *dc_mode,
                               work_mode_2_t *dehaze_mode, int *spatial_level,
                               int *temporal_level, int *dehaze_level,
                               int *ldch_level, int *fec_level, int *rotation) {
  char *sce_str = scenario_str_get();
  struct EnhancementConfig *enhancement_cfg =
      g_hash_table_lookup(db_enhancement_hash, sce_str);
  if (NULL == enhancement_cfg) {
    LOG_WARN("enhancement_cfg is null\n");
    return -1;
  } else {
    if (nr_mode)
      *nr_mode = enhancement_cfg->NoiseReduceMode;
    if (dc_mode)
      *dc_mode = enhancement_cfg->DistortionCorrection;
    if (dehaze_mode)
      *dehaze_mode = enhancement_cfg->Dehaze;
    if (spatial_level)
      *spatial_level = enhancement_cfg->SpatialDenoiseLevel;
    if (temporal_level)
      *temporal_level = enhancement_cfg->TemporalDenoiseLevel;
    if (dehaze_level)
      *dehaze_level = enhancement_cfg->DehazeLevel;
    if (ldch_level)
      *ldch_level = enhancement_cfg->LdchLevel;
    if (rotation)
      *rotation = enhancement_cfg->ImageRotation;
    if (fec_level)
      *fec_level = enhancement_cfg->FecLevel;
  }
}

void dbserver_image_adjustment_get(int *brightness, int *contrast,
                                   int *saturation, int *sharpness, int *hue,int *evbias) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_ADJUSTMENT);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image adjustment table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  *brightness =
      (int)json_object_get_int(json_object_object_get(j_data, "iBrightness"));
  *contrast =
      (int)json_object_get_int(json_object_object_get(j_data, "iContrast"));
  *saturation =
      (int)json_object_get_int(json_object_object_get(j_data, "iSaturation"));
  *sharpness =
      (int)json_object_get_int(json_object_object_get(j_data, "iSharpness"));
  *hue = (int)json_object_get_int(json_object_object_get(j_data, "iHue"));
  *evbias = (int)json_object_get_int(json_object_object_get(j_data, "iEvbias"));
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_adjustment_get(int *brightness, int *contrast, int *saturation,
                              int *sharpness, int *hue) {
  char *sce_str = scenario_str_get();
  struct AdjustmentConfig *adjustment_cfg =
      g_hash_table_lookup(db_adjustment_hash, sce_str);
  if (NULL == adjustment_cfg) {
    return -1;
  } else {
    if (brightness)
      *brightness = adjustment_cfg->Brightness;
    if (contrast)
      *contrast = adjustment_cfg->Contrast;
    if (saturation)
      *saturation = adjustment_cfg->Saturation;
    if (sharpness)
      *sharpness = adjustment_cfg->Sharpness;
    if (hue)
      *hue = adjustment_cfg->Hue;
  }
  return 0;
}

void dbserver_image_exposure_get(char *exposure_time, int *exposure_gain) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_EXPOSURE);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image exposure table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  char *time = (char *)json_object_get_string(
      json_object_object_get(j_data, "sExposureTime"));
  strncpy(exposure_time, time, strlen(time));
  exposure_time[strlen(time)] = '\0';
  *exposure_gain =
      (int)json_object_get_int(json_object_object_get(j_data, "iExposureGain"));
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_exposure_get(int *auto_exposure, float *exposure_time,
                            int *auto_gain, int *exposure_gain) {
  char *sce_str = scenario_str_get();
  struct ExposureConfig *exposure_cfg =
      g_hash_table_lookup(db_exposure_hash, sce_str);
  if (NULL == exposure_cfg) {
    return -1;
  } else {
    if (auto_exposure)
      *auto_exposure = exposure_cfg->AutoExposureEnabled;
    if (exposure_time)
      *exposure_gain = exposure_cfg->ExposureTime;
    if (auto_gain)
      *auto_gain = exposure_cfg->AutoGainEnabled;
    if (exposure_gain)
      *exposure_gain = exposure_cfg->ExposureGain;
  }
  return 0;
}

void dbserver_image_white_balance_get(char *white_balance_style, int *red_gain,
                                      int *green_gain, int *blue_gain) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_WHITE_BLANCE);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image white balance table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  if (white_balance_style) {
    char *style = (char *)json_object_get_string(
        json_object_object_get(j_data, "sWhiteBlanceStyle"));
    strncpy(white_balance_style, style, strlen(style));
    white_balance_style[strlen(style)] = '\0';
  }
  if (red_gain)
    *red_gain = (int)json_object_get_int(
        json_object_object_get(j_data, "iWhiteBalanceRed"));
  if (green_gain)
    *green_gain = (int)json_object_get_int(
        json_object_object_get(j_data, "iWhiteBalanceGreen"));
  if (blue_gain)
    *blue_gain = (int)json_object_get_int(
        json_object_object_get(j_data, "iWhiteBalanceBlue"));
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_white_balance_get(white_balance_mode_t *white_balance_style,
                                 int *red_gain, int *green_gain,
                                 int *blue_gain) {
  char *sce_str = scenario_str_get();
  struct WhiteBalanceConfig *white_balance_cfg =
      g_hash_table_lookup(db_white_blance_hash, sce_str);
  if (NULL == white_balance_cfg) {
    return -1;
  } else {
    if (white_balance_style) {
      *white_balance_style = white_balance_cfg->WhiteBlanceStyle;
    }
    if (red_gain)
      *red_gain = white_balance_cfg->WhiteBalanceRed;
    if (green_gain)
      *green_gain = white_balance_cfg->WhiteBalanceGreen;
    if (blue_gain)
      *blue_gain = white_balance_cfg->WhiteBalanceBlue;
  }
  return 0;
}

void dbserver_image_video_adjustment_get(char *frequency_mode,
                                         char *flip_mode) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_VIDEO_ADJUSTMEN);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image video adjustment table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  if (frequency_mode) {
    char *frequency = (char *)json_object_get_string(
        json_object_object_get(j_data, "sPowerLineFrequencyMode"));
    strncpy(frequency_mode, frequency, strlen(frequency));
    frequency_mode[strlen(frequency)] = '\0';
  }
  if (flip_mode) {
    char *flip = (char *)json_object_get_string(
        json_object_object_get(j_data, "sImageFlip"));
    strncpy(flip_mode, flip, strlen(flip));
    flip_mode[strlen(flip)] = '\0';
  }
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_video_adjustment_get(expPwrLineFreq_t *frequency_mode,
                                    flip_mode_t *flip_mode) {
  char *sce_str = scenario_str_get();
  struct VideoAdjustmentConfig *video_adjustment_cfg =
      g_hash_table_lookup(db_video_adjustment_hash, sce_str);
  if (NULL == video_adjustment_cfg) {
    LOG_WARN("video_adjustment_cfg is null\n");
    return -1;
  } else {
    if (frequency_mode)
      *frequency_mode = video_adjustment_cfg->PowerLineFrequencyMode;
    if (flip_mode)
      *flip_mode = video_adjustment_cfg->ImageFlip;
  }
  return 0;
}

void *hash_ir_smart_json_get() {
  char *sce_str = scenario_str_get();
  struct ExposureConfig *exposure_cfg =
      g_hash_table_lookup(db_exposure_hash, sce_str);
  struct Night2DayConfig *night2day_cfg =
      g_hash_table_lookup(db_night2day_hash, sce_str);

  if (exposure_cfg && night2day_cfg) {
    json_object *j_cfg = json_object_new_object();
    json_object_object_add(j_cfg, "iNightToDay",
                           json_object_new_int(night2day_cfg->NightToDay));
    json_object_object_add(j_cfg, "iDawnSeconds",
                           json_object_new_int(night2day_cfg->DawnSeconds));
    json_object_object_add(j_cfg, "iDuskSeconds",
                           json_object_new_int(night2day_cfg->DuskSeconds));
    json_object_object_add(j_cfg, "iFillLightMode",
                           json_object_new_int(night2day_cfg->FillLightMode));
    json_object_object_add(
        j_cfg, "iNightToDayFilterLevel",
        json_object_new_int(night2day_cfg->NightToDayFilterLevel));
    json_object_object_add(
        j_cfg, "iNightToDayFilterTime",
        json_object_new_int(night2day_cfg->NightToDayFilterTime));
    json_object_object_add(j_cfg, "iLightBrightness",
                           json_object_new_int(night2day_cfg->LightBrightness));
    json_object_object_add(
        j_cfg, "iAutoExposureEnabled",
        json_object_new_int(exposure_cfg->AutoExposureEnabled));
    return (void *)j_cfg;
  }

  return NULL;
}

void dbserver_image_night_to_day_get(rk_aiq_cpsl_cfg_t *cpsl_cfg) {
  char *json_str = NULL;
  json_str = dbserver_media_get(TABLE_IMAGE_NIGHT_TO_DAY);
  LOG_INFO("%s, json_str is %s\n", __func__, json_str);
  if (json_str == NULL) {
    LOG_INFO("image video adjustment table is null\n");
    return;
  }
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_array = json_object_object_get(j_cfg, "jData");
  json_object *j_data = json_object_array_get_idx(j_data_array, 0);
  char *mode = (char *)json_object_get_string(
      json_object_object_get(j_data, "sNightToDay"));
  if (!strcmp(mode, "auto")) {
    cpsl_cfg->mode = RK_AIQ_OP_MODE_AUTO;
    cpsl_cfg->gray_on = false;
    cpsl_cfg->u.a.sensitivity = (float)json_object_get_int(
        json_object_object_get(j_data, "iNightToDayFilterLevel"));
    cpsl_cfg->u.a.sw_interval = (uint32_t)json_object_get_int(
        json_object_object_get(j_data, "iNightToDayFilterTime"));
  } else if (!strcmp(mode, "day")) {
    cpsl_cfg->mode = RK_AIQ_OP_MODE_MANUAL;
    cpsl_cfg->gray_on = false;
    cpsl_cfg->u.m.on = 1;
    // set iLightBrightness=0 to force switch to day mode
    cpsl_cfg->u.m.strength_led = (float)0;
  } else if (!strcmp(mode, "night")) {
    cpsl_cfg->mode = RK_AIQ_OP_MODE_MANUAL;
    cpsl_cfg->gray_on = true;
    cpsl_cfg->u.m.on = 1;
    cpsl_cfg->u.m.strength_led = (float)json_object_get_int(
        json_object_object_get(j_data, "iLightBrightness"));
  } else {
    LOG_INFO("Not currently supported\n");
    cpsl_cfg->mode = RK_AIQ_OP_MODE_INVALID;
  }
  char *fill_light_mode = (char *)json_object_get_string(
      json_object_object_get(j_data, "sFillLightMode"));
  if (!strcmp(fill_light_mode, "LED"))
    cpsl_cfg->lght_src = RK_AIQ_CPSLS_LED;
  else if (!strcmp(fill_light_mode, "IR"))
    cpsl_cfg->lght_src = RK_AIQ_CPSLS_IR;
  else if (!strcmp(fill_light_mode, "MIX"))
    cpsl_cfg->lght_src = RK_AIQ_CPSLS_MIX;
  json_object_put(j_cfg);
  free(json_str);
}

int hash_image_night_to_day_get(rk_aiq_cpsl_cfg_t *cpsl_cfg) {
  char *sce_str = scenario_str_get();
  struct Night2DayConfig *night2day_cfg =
      g_hash_table_lookup(db_night2day_hash, sce_str);
  if (NULL == night2day_cfg) {
    return -1;
  } else {
    if (night2day_cfg->NightToDay == ND_AUTO_MODE) {
      cpsl_cfg->mode = RK_AIQ_OP_MODE_AUTO;
      cpsl_cfg->gray_on = false;
      cpsl_cfg->u.a.sensitivity = (float)night2day_cfg->NightToDayFilterLevel;
      cpsl_cfg->u.a.sw_interval = (uint32_t)night2day_cfg->NightToDayFilterTime;
    } else if (night2day_cfg->NightToDay == ND_DAY_MODE) {
      cpsl_cfg->mode = RK_AIQ_OP_MODE_MANUAL;
      cpsl_cfg->gray_on = false;
      cpsl_cfg->u.m.on = 1;
      // set iLightBrightness=0 to force switch to day mode
      cpsl_cfg->u.m.strength_led = (float)0;
    } else if (night2day_cfg->NightToDay == ND_NIGHT_MODE) {
      cpsl_cfg->mode = RK_AIQ_OP_MODE_MANUAL;
      cpsl_cfg->gray_on = true;
      cpsl_cfg->u.m.on = 1;
      cpsl_cfg->u.m.strength_led = (float)night2day_cfg->LightBrightness;
    } else {
      LOG_INFO("Not currently supported\n");
      cpsl_cfg->mode = RK_AIQ_OP_MODE_INVALID;
    }
    cpsl_cfg->lght_src = night2day_cfg->FillLightMode;
  }
  return 0;
}

static void blc_para_set_by_hash(json_object *j_data) {
  char *sce_str = scenario_str_get();
  struct BLCConfig *blc_cfg = g_hash_table_lookup(db_blc_hash, sce_str);
  if (NULL == blc_cfg) {
    LOG_WARN("blc_cfg is null, when datachanged\n");
    return;
  }
  json_object *sHDR = json_object_object_get(j_data, "sHDR");
  json_object *iHDRLevel = json_object_object_get(j_data, "iHDRLevel");
  json_object *sBLCRegion = json_object_object_get(j_data, "sBLCRegion");
  json_object *iBLCStrength = json_object_object_get(j_data, "iBLCStrength");
  json_object *sHLC = json_object_object_get(j_data, "sHLC");
  json_object *iHLCLevel = json_object_object_get(j_data, "iHLCLevel");
  json_object *iDarkBoostLevel =
      json_object_object_get(j_data, "iDarkBoostLevel");

  if (sHDR) {
    hdr_mode_set4db(blc_cfg->HDR);
    if (hdr_global_value_get() == RK_AIQ_WORKING_MODE_ISP_HDR2) {
      blc_hdr_level_enum_set(blc_cfg->HDRLevel);
      return;
    }
  }
  if (iHDRLevel) {
    LOG_INFO("hdr level set gc : %d\n", hdr_global_value_get());
    if (hdr_global_value_get() == RK_AIQ_WORKING_MODE_ISP_HDR2) {
      blc_hdr_level_enum_set(blc_cfg->HDRLevel);
      return;
    }
  }
  if (hdr_global_value_get() != RK_AIQ_WORKING_MODE_NORMAL) {
    return;
  }
  if (sBLCRegion) {
    blc_region_para_set(blc_cfg->BLC, blc_cfg->BLCStrength);
  } else if (iBLCStrength != NULL && blc_cfg->BLC == WM1_OPEN_MODE) {
    blc_region_strength_set(blc_cfg->BLCStrength);
  }
  if (blc_cfg->BLC == WM1_OPEN_MODE) {
    return;
  }
  if (sHLC || iHLCLevel || iDarkBoostLevel) {
    blc_hlc_para_set(blc_cfg->HLC, blc_cfg->HLCLevel, blc_cfg->DarkBoostLevel);
  }
}

static void exposure_para_set_by_json(json_object *j_data) {
  json_object *sExposureMode = json_object_object_get(j_data, "sExposureMode");
  json_object *sGainMode = json_object_object_get(j_data, "sGainMode");
  json_object *sExposureTime = json_object_object_get(j_data, "sExposureTime");
  json_object *iExposureGain = json_object_object_get(j_data, "iExposureGain");

  if (sExposureMode) {
    char *exposure_mode = (char *)json_object_get_string(sExposureMode);
    if (g_str_equal(exposure_mode, "auto")) {
      auto_exposure_set();
      return;
    }
  }
  if (sGainMode) {
    char *gain_mode = (char *)json_object_get_string(sGainMode);
    if (g_str_equal(gain_mode, "auto")) {
      if (sExposureTime) {
        char *time = (char *)json_object_get_string(sExposureTime);
        manual_exposure_auto_gain_set_char(time);
      }
    } else {
      if (sExposureTime && iExposureGain) {
        char *time = (char *)json_object_get_string(sExposureTime);
        int gain = (int)json_object_get_int(iExposureGain);
        manual_exposure_manual_gain_set_char(time, gain);
      }
    }
    return;
  }
  LOG_WARN("version mismatch!\n");
}

static void video_adjustment_para_set_by_hash(json_object *j_data) {
  char *sce_str = scenario_str_get();
  struct VideoAdjustmentConfig *video_adjustment_cfg =
      g_hash_table_lookup(db_video_adjustment_hash, sce_str);
  if (NULL == video_adjustment_cfg) {
    LOG_WARN("video_adjustment_cfg is null, when datachanged\n");
    return;
  }

  json_object *sImageFlip = json_object_object_get(j_data, "sImageFlip");
  json_object *sPowerLineFrequencyMode =
      json_object_object_get(j_data, "sPowerLineFrequencyMode");
  if (sPowerLineFrequencyMode) {
    frequency_mode_set(video_adjustment_cfg->PowerLineFrequencyMode);
  }
  if (sImageFlip) {
    mirror_mode_set(video_adjustment_cfg->ImageFlip);
  }
}

static void enhancement_para_set_by_hash(json_object *j_data) {
  char *sce_str = scenario_str_get();
  struct EnhancementConfig *enhancement_cfg =
      g_hash_table_lookup(db_enhancement_hash, sce_str);
  if (NULL == enhancement_cfg) {
    LOG_WARN("enhancement_cfg is null, when datachanged\n");
    return;
  }

  json_object *sNoiseReduceMode =
      json_object_object_get(j_data, "sNoiseReduceMode");
  json_object *iSpatialDenoiseLevel =
      json_object_object_get(j_data, "iSpatialDenoiseLevel");
  json_object *iTemporalDenoiseLevel =
      json_object_object_get(j_data, "iTemporalDenoiseLevel");
  if (sNoiseReduceMode || iSpatialDenoiseLevel || iTemporalDenoiseLevel)
    nr_para_set(enhancement_cfg->NoiseReduceMode,
                enhancement_cfg->SpatialDenoiseLevel,
                enhancement_cfg->TemporalDenoiseLevel);

  json_object *sDistortionCorrection =
      json_object_object_get(j_data, "sDistortionCorrection");
  json_object *iLdchLevel = json_object_object_get(j_data, "iLdchLevel");
  json_object *iFecLevel = json_object_object_get(j_data, "iFecLevel");
  if (sDistortionCorrection)
    dc_para_set(enhancement_cfg->DistortionCorrection,
                enhancement_cfg->LdchLevel, enhancement_cfg->FecLevel);
  else if (enhancement_cfg->DistortionCorrection == DC_LDCH &&
           iLdchLevel != NULL)
    ldch_level_set(enhancement_cfg->LdchLevel);
  else if (enhancement_cfg->DistortionCorrection == DC_FEC && iFecLevel != NULL)
    fec_level_set(enhancement_cfg->FecLevel);

  json_object *iImageRotation =
      json_object_object_get(j_data, "iImageRotation");
  if (iImageRotation) {
    bypass_stream_rotation_set(enhancement_cfg->ImageRotation);
  }

  json_object *sDehaze = json_object_object_get(j_data, "sDehaze");
  json_object *iDehazeLevel = json_object_object_get(j_data, "iDehazeLevel");
  if (sDehaze) {
    dehaze_para_set(enhancement_cfg->Dehaze, enhancement_cfg->DehazeLevel);
  } else if (iDehazeLevel) {
    if (enhancement_cfg->Dehaze == WM2_OPEN_MODE)
      dehaze_strength_set(enhancement_cfg->DehazeLevel);
  }
}

static void image_adjustment_set_by_json(json_object *j_data) {
  json_object *iBrightness = json_object_object_get(j_data, "iBrightness");
  json_object *iContrast = json_object_object_get(j_data, "iContrast");
  json_object *iSaturation = json_object_object_get(j_data, "iSaturation");
  json_object *iSharpness = json_object_object_get(j_data, "iSharpness");
  json_object *iHue = json_object_object_get(j_data, "iHue");
  json_object *iFPS = json_object_object_get(j_data, "iFPS");
  json_object *sGridWeight = json_object_object_get(j_data, "sGridWeight");
  if (iBrightness) {
    int brightness = (int)json_object_get_int(iBrightness);
    brightness_set(brightness);
  }
  if (iContrast) {
    int contrast = (int)json_object_get_int(iContrast);
    contrast_set(contrast);
  }
  if (iSaturation) {
    int saturation = (int)json_object_get_int(iSaturation);
    saturation_set(saturation);
  }
  if (iSharpness) {
    int sharpness = (int)json_object_get_int(iSharpness);
    sharpness_set(sharpness);
  }
  if (iHue) {
    int hue = (int)json_object_get_int(iHue);
    hue_set(hue);
  }
  if (iFPS) {
    int fps = json_object_get_int(iFPS);
    isp_fix_fps_set(fps);
  }
  if(sGridWeight) {
    char *weight_data = (char *)json_object_get_string(sGridWeight);
    //LOG_INFO("set weight,%s \n", weight_data);
    int32_t g_weight[225] = {0};
    int brightness,contrast,saturation,sharpness,hue,evbias = 0;
    memset(g_weight, 0, sizeof(g_weight));
    int ret = exposure_weight_str2array(weight_data,g_weight);
    if(ret == 0){
      dbserver_image_adjustment_get(&brightness,&contrast,&saturation,&sharpness,&hue,&evbias);
      LOG_INFO("set weight,%s ,evbias = %d\n", weight_data,evbias);
      manual_exposure_grid_weight_set(g_weight,evbias);
    }
  }
}

static void white_balance_set_by_hash(json_object *j_data) {
  json_object *sWhiteBalanceStyle =
      json_object_object_get(j_data, "sWhiteBlanceStyle");
  json_object *iWhiteBalanceRed =
      json_object_object_get(j_data, "iWhiteBalanceRed");
  json_object *iWhiteBalanceBlue =
      json_object_object_get(j_data, "iWhiteBalanceBlue");
  json_object *iWhiteBalanceGreen =
      json_object_object_get(j_data, "iWhiteBalanceGreen");

  char *sce_str = scenario_str_get();
  struct WhiteBalanceConfig *white_balance_cfg =
      g_hash_table_lookup(db_white_blance_hash, sce_str);
  if (NULL == white_balance_cfg) {
    LOG_WARN("white_balance_cfg is null\n");
    return;
  }
  if (sWhiteBalanceStyle) {
    if (white_balance_cfg->WhiteBlanceStyle == WB_MANUAL) {
      manual_white_balance_set(white_balance_cfg->WhiteBalanceRed,
                               white_balance_cfg->WhiteBalanceGreen,
                               white_balance_cfg->WhiteBalanceBlue);
    } else {
      white_balance_style_set(white_balance_cfg->WhiteBlanceStyle);
    }
  } else if ((iWhiteBalanceRed) || (iWhiteBalanceBlue) ||
             (iWhiteBalanceGreen)) {
    if (white_balance_cfg->WhiteBlanceStyle == WB_MANUAL)
      manual_white_balance_level_set(white_balance_cfg->WhiteBalanceRed,
                                     white_balance_cfg->WhiteBalanceGreen,
                                     white_balance_cfg->WhiteBalanceBlue);
    else
      LOG_WARN("set rgb balance not in manual mode\n");
  }
}

static void fill_light_set_by_json(json_object *j_data) {
  /* fill mode set in hashupdate
  json_object* sFillLightMode = json_object_object_get(j_data,
  "sFillLightMode");
  */
  json_object *iLightBrightness =
      json_object_object_get(j_data, "iLightBrightness");
  if (iLightBrightness) {
    int brightness = (int)json_object_get_int(iLightBrightness);
    char *sce_str = scenario_str_get();
    struct Night2DayConfig *night2day_cfg =
        g_hash_table_lookup(db_night2day_hash, sce_str);
    if (night2day_cfg->NightToDay != ND_DAY_MODE)
      fill_light_brightness_set(brightness);
  }
}

int exposure_para_set_by_hash(void) {
  char *sce_str = scenario_str_get();
  struct ExposureConfig *exposure_cfg =
      g_hash_table_lookup(db_exposure_hash, sce_str);

  if (NULL == exposure_cfg) {
    LOG_WARN("get no hash data\n");
    return -1;
  }

  if (exposure_cfg->AutoExposureEnabled) {
    auto_exposure_set();
  } else if (exposure_cfg->AutoGainEnabled) {
    manual_exposure_auto_gain_set_float(exposure_cfg->ExposureTime);
  } else {
    manual_exposure_manual_gain_set_float(exposure_cfg->ExposureTime,
                                          exposure_cfg->ExposureGain);
  }
  return 0;
}

int blc_normal_mode_para_set_by_hash(void) {
  char *sce_str = scenario_str_get();
  struct BLCConfig *blc_cfg = g_hash_table_lookup(db_blc_hash, sce_str);
  if (NULL == blc_cfg) {
    LOG_WARN("blc_cfg is null\n");
    return -1;
  }
  LOG_INFO("blc region mode: %d, blc strength: %d, hlc mode: %d, hlc strength: "
           "%d, darkboost: %d\n",
           blc_cfg->BLC, blc_cfg->BLCStrength, blc_cfg->HLC, blc_cfg->HLCLevel,
           blc_cfg->DarkBoostLevel);
  if (blc_cfg->HDR != RK_AIQ_WORKING_MODE_NORMAL) {
    LOG_WARN("not in common mode\n");
    return -1;
  } else {
    blc_region_para_set(blc_cfg->BLC, blc_cfg->BLCStrength);
    if (blc_cfg->BLC != WM1_OPEN_MODE) {
      blc_hlc_para_set(blc_cfg->HLC, blc_cfg->HLCLevel,
                       blc_cfg->DarkBoostLevel);
    }
  }
  return 0;
}

void exposure_para_set_by_db(void) {
  char *json_str = dbserver_media_get((char *)TABLE_IMAGE_EXPOSURE);
  json_object *j_cfg = json_tokener_parse(json_str);
  json_object *j_data_arr = json_object_object_get(j_cfg, "jData");
  if (!j_data_arr) {
    LOG_INFO("no exposure para found!\n");
    return;
  }
  json_object *j_data = json_object_array_get_idx(j_data_arr, 0);

  exposure_para_set_by_json(j_data);

  json_object_put(j_cfg);
  g_free(json_str);
}

void night_to_day_para_cap_set_db(void) {
  rk_aiq_cpsl_cap_t compensate_light_cap;
  memset(&compensate_light_cap, 0, sizeof(rk_aiq_cpsl_cap_t));
  /* Write the supported capability set to the database */
  rk_aiq_uapi_sysctl_queryCpsLtCap(db_aiq_ctx, &compensate_light_cap);
  json_object *j_cfg = json_object_new_array();
  for (int i = 0; i < RK_AIQ_CPSLS_MAX; i++) {
    int lght_src = compensate_light_cap.supported_lght_src[i];
    if ((!lght_src) || (lght_src >= RK_AIQ_CPSLS_MAX))
      continue;
    if (lght_src == RK_AIQ_CPSLS_LED)
      json_object_array_add(j_cfg, json_object_new_string("LED"));
    else if (lght_src == RK_AIQ_CPSLS_IR)
      json_object_array_add(j_cfg, json_object_new_string("IR"));
    else if (lght_src == RK_AIQ_CPSLS_MIX)
      json_object_array_add(j_cfg, json_object_new_string("MIX"));
  }
  char *support_list = (char *)json_object_get_string(j_cfg);
  LOG_INFO("lght src support list is %s\n", support_list);
  struct StaticLocation st_lo = {.cap_name = "image_night_to_day",
                                 .target_key = "sFillLightMode"};
  dbserver_set_static_cap_option(st_lo, support_list);
  json_object_put(j_cfg);
}

void night_to_day_para_set_by_db() {
  rk_aiq_cpsl_cfg_t compensate_light_cfg;
  memset(&compensate_light_cfg, 0, sizeof(rk_aiq_cpsl_cfg_t));
  dbserver_image_night_to_day_get(&compensate_light_cfg);
  night_to_day_para_set(compensate_light_cfg);
}

int str2scenario(char *sce) {
  if (g_str_equal(sce, "normal")) {
    return (int)SCE_NORMAL;
  } else if (g_str_equal(sce, "frontlight")) {
    return (int)SCE_FRONT_LIGHT;
  } else if (g_str_equal(sce, "lowIllumination")) {
    return (int)SCE_LOW_ILLUMINATION;
  } else if (g_str_equal(sce, "custom1")) {
    return (int)SCE_CUSTOM1;
  } else if (g_str_equal(sce, "custom2")) {
    return (int)SCE_CUSTOM2;
  }
  return (int)SCE_INVALID;
}

static void updatehash_adjustment(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct AdjustmentConfig *adjustment_cfg =
      g_hash_table_lookup(db_adjustment_hash, sce_str);

  if (adjustment_cfg == NULL) {
    adjustment_cfg = malloc(sizeof(struct AdjustmentConfig));
    memset(adjustment_cfg, 0, sizeof(struct AdjustmentConfig));
    g_hash_table_replace(db_adjustment_hash, g_strdup(sce_str),
                         (gpointer)adjustment_cfg);
  }

  if (g_str_equal(name, "iBrightness")) {
    adjustment_cfg->Brightness = *(int *)data;
  } else if (g_str_equal(name, "iContrast")) {
    adjustment_cfg->Contrast = *(int *)data;
  } else if (g_str_equal(name, "iSaturation")) {
    adjustment_cfg->Saturation = *(int *)data;
  } else if (g_str_equal(name, "iSharpness")) {
    adjustment_cfg->Sharpness = *(int *)data;
  } else if (g_str_equal(name, "iHue")) {
    adjustment_cfg->Hue = *(int *)data;
  }
}

static void updatehash_exposure(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct ExposureConfig *exposure_cfg =
      g_hash_table_lookup(db_exposure_hash, sce_str);

  if (exposure_cfg == NULL) {
    exposure_cfg = malloc(sizeof(struct ExposureConfig));
    memset(exposure_cfg, 0, sizeof(struct ExposureConfig));
    g_hash_table_replace(db_exposure_hash, g_strdup(sce_str),
                         (gpointer)exposure_cfg);
  }
  if (g_str_equal(name, "sExposureMode")) {
    exposure_cfg->AutoExposureEnabled = string2work_mode_0_t((char *)data);
  } else if (g_str_equal(name, "sGainMode")) {
    exposure_cfg->AutoGainEnabled = string2work_mode_0_t((char *)data);
  } else if (g_str_equal(name, "sExposureTime")) {
    exposure_cfg->ExposureTime = atof(data);
  } else if (g_str_equal(name, "iExposureGain")) {
    exposure_cfg->ExposureGain = *(int *)data;
  }
}

static int updatehash_night2day(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct Night2DayConfig *night2day_cfg =
      g_hash_table_lookup(db_night2day_hash, sce_str);

  if (night2day_cfg == NULL) {
    night2day_cfg = malloc(sizeof(struct Night2DayConfig));
    memset(night2day_cfg, 0, sizeof(struct Night2DayConfig));
    g_hash_table_replace(db_night2day_hash, g_strdup(sce_str),
                         (gpointer)night2day_cfg);
  }

  if (g_str_equal(name, "sNightToDay")) {
    night2day_cfg->NightToDay = string2night2day_mode_t((char *)data);
  } else if (g_str_equal(name, "iNightToDayFilterLevel")) {
    night2day_cfg->NightToDayFilterLevel = *(int *)data;
  } else if (g_str_equal(name, "iNightToDayFilterTime")) {
    night2day_cfg->NightToDayFilterTime = *(int *)data;
  } else if (g_str_equal(name, "sDawnTime")) {
    night2day_cfg->DawnSeconds = str2time_sec((char *)data);
  } else if (g_str_equal(name, "sDuskTime")) {
    night2day_cfg->DuskSeconds = str2time_sec((char *)data);
  } else if (g_str_equal(name, "sFillLightMode")) {
    night2day_cfg->FillLightMode = fill_light_enum_get((char *)data);
    gc_cpsl_cfg_fill_light_mode_set(night2day_cfg->FillLightMode);
  } else if (g_str_equal(name, "iLightBrightness")) {
    night2day_cfg->LightBrightness = *(int *)data;
  }
}

static void updatehash_blc(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct BLCConfig *blc_cfg = g_hash_table_lookup(db_blc_hash, sce_str);

  if (blc_cfg == NULL) {
    blc_cfg = malloc(sizeof(struct BLCConfig));
    memset(blc_cfg, 0, sizeof(struct BLCConfig));
    g_hash_table_replace(db_blc_hash, g_strdup(sce_str), (gpointer)blc_cfg);
  }

  if (g_str_equal(name, "sBLCRegion")) {
    blc_cfg->BLC = string2work_mode_1_t((char *)data);
  } else if (g_str_equal(name, "iBLCStrength")) {
    blc_cfg->BLCStrength = *(int *)data;
  } else if (g_str_equal(name, "sHDR")) {
    blc_cfg->HDR = string2hdr_mode((char *)data);
  } else if (g_str_equal(name, "iHDRLevel")) {
    blc_cfg->HDRLevel = *(int *)data;
  } else if (g_str_equal(name, "sHLC")) {
    blc_cfg->HLC = string2work_mode_1_t((char *)data);
  } else if (g_str_equal(name, "iHLCLevel")) {
    blc_cfg->HLCLevel = *(int *)data;
  } else if (g_str_equal(name, "iDarkBoostLevel")) {
    blc_cfg->DarkBoostLevel = *(int *)data;
  }
}

static void updatehash_white_balance(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct WhiteBalanceConfig *white_balance_cfg =
      g_hash_table_lookup(db_white_blance_hash, sce_str);

  if (white_balance_cfg == NULL) {
    white_balance_cfg = malloc(sizeof(struct WhiteBalanceConfig));
    memset(white_balance_cfg, 0, sizeof(struct WhiteBalanceConfig));
    g_hash_table_replace(db_white_blance_hash, g_strdup(sce_str),
                         (gpointer)white_balance_cfg);
  }

  if (g_str_equal(name, "sWhiteBlanceStyle")) {
    white_balance_cfg->WhiteBlanceStyle =
        string2white_balance_mode_t((char *)data);
  } else if (g_str_equal(name, "iWhiteBalanceRed")) {
    white_balance_cfg->WhiteBalanceRed = *(int *)data;
  } else if (g_str_equal(name, "iWhiteBalanceGreen")) {
    white_balance_cfg->WhiteBalanceGreen = *(int *)data;
  } else if (g_str_equal(name, "iWhiteBalanceBlue")) {
    white_balance_cfg->WhiteBalanceBlue = *(int *)data;
  }
}

static void updatehash_enhancement(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct EnhancementConfig *enhancement_cfg =
      g_hash_table_lookup(db_enhancement_hash, sce_str);

  if (enhancement_cfg == NULL) {
    enhancement_cfg = malloc(sizeof(struct EnhancementConfig));
    memset(enhancement_cfg, 0, sizeof(struct EnhancementConfig));
    g_hash_table_replace(db_enhancement_hash, g_strdup(sce_str),
                         (gpointer)enhancement_cfg);
  }

  if (g_str_equal(name, "sNoiseReduceMode")) {
    enhancement_cfg->NoiseReduceMode = string2nr_mode_t((char *)data);
  } else if (g_str_equal(name, "iDenoiseLevel")) {
    enhancement_cfg->DenoiseLevel = *(int *)data;
  } else if (g_str_equal(name, "iSpatialDenoiseLevel")) {
    enhancement_cfg->SpatialDenoiseLevel = *(int *)data;
  } else if (g_str_equal(name, "iTemporalDenoiseLevel")) {
    enhancement_cfg->TemporalDenoiseLevel = *(int *)data;
  } else if (g_str_equal(name, "sDehaze")) {
    enhancement_cfg->Dehaze = string2work_mode_2_t((char *)data);
  } else if (g_str_equal(name, "iDehazeLevel")) {
    enhancement_cfg->DehazeLevel = *(int *)data;
  } else if (g_str_equal(name, "sGrayScaleMode")) {
    enhancement_cfg->GrayScaleMode = string2gs_mode_t((char *)data);
  } else if (g_str_equal(name, "iImageRotation")) {
    enhancement_cfg->ImageRotation = *(int *)data;
  } else if (g_str_equal(name, "sDistortionCorrection")) {
    enhancement_cfg->DistortionCorrection = string2dc_mode_t((char *)data);
  } else if (g_str_equal(name, "iLdchLevel")) {
    enhancement_cfg->LdchLevel = *(int *)data;
  } else if (g_str_equal(name, "iFecLevel")) {
    enhancement_cfg->FecLevel = *(int *)data;
  }
}

static void updatehash_video_adjustment(int sce, char *name, void *data) {
  char sce_str[10] = {0};
  memset(sce_str, 0, sizeof(sce_str));
  sprintf(sce_str, "%d", sce);
  struct VideoAdjustmentConfig *video_adjustment_cfg =
      g_hash_table_lookup(db_video_adjustment_hash, sce_str);

  if (video_adjustment_cfg == NULL) {
    video_adjustment_cfg = malloc(sizeof(struct VideoAdjustmentConfig));
    memset(video_adjustment_cfg, 0, sizeof(struct VideoAdjustmentConfig));
    g_hash_table_replace(db_video_adjustment_hash, g_strdup(sce_str),
                         (gpointer)video_adjustment_cfg);
  }

  if (g_str_equal(name, "sImageFlip")) {
    video_adjustment_cfg->ImageFlip = string2flip_mode((char *)data);
  } else if (g_str_equal(name, "sPowerLineFrequencyMode")) {
    video_adjustment_cfg->PowerLineFrequencyMode =
        string2frequency_mode((char *)data);
  }
}

static int scenario_get(void) {
  int ret = -1;
  char *json_str = dbserver_media_get(TABLE_IMAGE_SCENARIO);
  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    if (len >= 1) {
      json_object *j_obj = json_object_array_get_idx(j_array, 0);
      char *sce = (char *)json_object_get_string(
          json_object_object_get(j_obj, "sScenario"));
      int sce_num = str2scenario(sce);
      if (sce_num >= SCENARIO_MIN && sce_num <= SCENARIO_MAX) {
        gc_scenario = (enum Scenario)sce_num;
        LOG_DEBUG("set scenario %d\n", gc_scenario);
      } else {
        gc_scenario = (enum Scenario)SCENARIO_MIN;
        LOG_WARN("unknown scenario, set scenario normal by default\n");
      }
      ret = 0;
    }
    json_object_put(j_ret);
    g_free(json_str);
  }

  return ret;
}

static int adjustment_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_ADJUSTMENT);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_adjustment(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static int exposure_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_EXPOSURE);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_exposure(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static int night2day_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_NIGHT_TO_DAY);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_night2day(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static int blc_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_BLC);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_blc(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static int white_balance_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_WHITE_BLANCE);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_white_balance(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static int enhancement_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_ENHANCEMENT);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_enhancement(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static int video_adjustment_get(void) {
  char *json_str = dbserver_media_get(TABLE_IMAGE_VIDEO_ADJUSTMEN);

  if (json_str) {
    json_object *j_array;
    json_object *j_ret;

    j_ret = json_tokener_parse(json_str);
    j_array = json_object_object_get(j_ret, "jData");
    int len = json_object_array_length(j_array);
    int order = -1;
    if ((int)gc_scenario <= len) {
      order = (int)gc_scenario;
    } else {
      order = 0;
    }
    json_object *j_data = json_object_array_get_idx(j_array, order);
    int sec = (int)json_object_get_int(json_object_object_get(j_data, "id"));
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_video_adjustment(sec, key, data);
    }
    json_object_put(j_ret);
    g_free(json_str);
    return 0;
  }
  return -1;
}

static void hash_data_init_by_id(int id) {
  if (id < SCENARIO_MIN || id > SCENARIO_MAX) {
    int dbus_log_status = dbus_warn_log_status_get();
    while (scenario_get() != 0) {
      if (dbus_log_status) {
        dbus_warn_log_close();
      }
      LOG_INFO("scenario_get, wait dbserver.\n");
      usleep(50000);
    }
    if (dbus_log_status) {
      dbus_warn_log_open();
    }
  } else {
    gc_scenario = (enum Scenario)id;
  }
  while (adjustment_get() != 0) {
    LOG_INFO("adjustment_get, wait dbserver.\n");
    usleep(50000);
  }
  while (exposure_get() != 0) {
    LOG_INFO("exposure_get, wait dbserver.\n");
    usleep(50000);
  }
  while (night2day_get() != 0) {
    LOG_INFO("night2day_get(), wait dbserver.\n");
    usleep(50000);
  }
  while (blc_get() != 0) {
    LOG_INFO("night2day_get(), wait dbserver.\n");
    usleep(50000);
  }
  while (white_balance_get() != 0) {
    LOG_INFO("white_balance_get(), wait dbserver.\n");
    usleep(50000);
  }
  while (enhancement_get() != 0) {
    LOG_INFO("enhancement_get(), wait dbserver.\n");
    usleep(50000);
  }
  while (video_adjustment_get() != 0) {
    LOG_INFO("video_adjustment_get(), wait dbserver.\n");
    usleep(50000);
  }
  LOG_INFO("hashtable init complete!\n");
}

int white_balance_set_by_hash_table(void) {
  char *sce_str = scenario_str_get();
  struct WhiteBalanceConfig *white_balance_cfg =
      g_hash_table_lookup(db_white_blance_hash, sce_str);

  if (NULL == white_balance_cfg) {
    LOG_WARN("white_balance_cfg is null\n");
    return -1;
  }
  LOG_INFO("white balance mode is %d, r: %d, g: %d, b: %d\n",
           white_balance_cfg->WhiteBlanceStyle,
           white_balance_cfg->WhiteBalanceRed,
           white_balance_cfg->WhiteBalanceGreen,
           white_balance_cfg->WhiteBalanceBlue);
  if (white_balance_cfg->WhiteBlanceStyle == WB_MANUAL) {
    manual_white_balance_set(white_balance_cfg->WhiteBalanceRed,
                             white_balance_cfg->WhiteBalanceGreen,
                             white_balance_cfg->WhiteBalanceBlue);
  } else {
    white_balance_style_set(white_balance_cfg->WhiteBlanceStyle);
  }
  return 0;
}

void DataChanged(void *user_data) {
  char *json_str = (char *)user_data;
  LOG_INFO("DataChanged, json is %s\n", json_str);
  json_object *j_cfg;
  json_object *j_key = 0;
  json_object *j_data = 0;
  char *table = 0;

  j_cfg = json_tokener_parse(json_str);
  table =
      (char *)json_object_get_string(json_object_object_get(j_cfg, "table"));
  j_key = json_object_object_get(j_cfg, "key");
  j_data = json_object_object_get(j_cfg, "data");
  char *cmd =
      (char *)json_object_get_string(json_object_object_get(j_cfg, "cmd"));

  if (g_str_equal(table, TABLE_IMAGE_BLC)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_blc((int)gc_scenario, key, data);
    }
    blc_para_set_by_hash(j_data);
  } else if (g_str_equal(table, TABLE_IMAGE_ENHANCEMENT)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_enhancement((int)gc_scenario, key, data);
    }
    enhancement_para_set_by_hash(j_data);
  } else if (g_str_equal(table, TABLE_IMAGE_ADJUSTMENT)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_adjustment((int)gc_scenario, key, data);
    }
    image_adjustment_set_by_json(j_data);
  } else if (g_str_equal(table, TABLE_IMAGE_EXPOSURE)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_exposure((int)gc_scenario, key, data);
    }
    exposure_para_set_by_hash();
  } else if (g_str_equal(table, TABLE_IMAGE_WHITE_BLANCE)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_white_balance((int)gc_scenario, key, data);
    }
    white_balance_set_by_hash(j_data);
  } else if (g_str_equal(table, TABLE_IMAGE_VIDEO_ADJUSTMEN)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_video_adjustment((int)gc_scenario, key, data);
    }
    video_adjustment_para_set_by_hash(j_data);
  } else if (g_str_equal(table, TABLE_IMAGE_NIGHT_TO_DAY)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    // update hash table
    json_object_object_foreach(j_data, key, val) {
      void *data;
      int tmp;
      if (json_object_get_type(val) == json_type_int) {
        tmp = (int)json_object_get_int(val);
        data = (void *)&tmp;
      } else
        data = (void *)json_object_get_string(val);
      updatehash_night2day((int)gc_scenario, key, data);
    }
    fill_light_set_by_json(j_data);
  } else if (g_str_equal(table, TABLE_IMAGE_SCENARIO)) {
    if (!g_str_equal(cmd, "Update")) {
      json_object_put(j_cfg);
      return;
    }
    /* disabled now */
    return;
    json_object *sScenario = json_object_object_get(j_data, "sScenario");
    if (sScenario) {
      char *sce_mode = (char *)json_object_get_string(sScenario);
      int sce_num = str2scenario(sce_mode);
      if (sce_num >= SCENARIO_MIN && sce_num <= SCENARIO_MAX) {
        gc_scenario = sce_num;
        // disable_loop();
        hash_data_init_by_id(sce_num);
        // TODO:reset flow
      }
    }
  }
  json_object_put(j_cfg);
}

void database_init() {
  pthread_mutex_lock(&init_mutex);
  if (database_init_flag) {
    pthread_mutex_unlock(&init_mutex);
    return;
  }
  database_init_flag = 1;
  LOG_INFO("database_init\n");
  disable_loop();
  hash_data_init_by_id(-1);
  dbus_monitor_signal_registered(DBSERVER_MEDIA_INTERFACE,
                                 DS_SIGNAL_DATACHANGED, &DataChanged);
  LOG_INFO("database_init over\n");
  pthread_mutex_unlock(&init_mutex);
}

void database_hash_init(void) {
  pthread_mutex_lock(&init_mutex);
  if (db_adjustment_hash) {
    pthread_mutex_unlock(&init_mutex);
    return;
  }
  db_adjustment_hash =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  db_exposure_hash =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  db_night2day_hash =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  db_blc_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  db_white_blance_hash =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  db_enhancement_hash =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  db_video_adjustment_hash =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
  LOG_INFO("database_hash_init init complete\n");
  pthread_mutex_unlock(&init_mutex);
}

bool wait_dbus_init_func(void) {
  return (scenario_get() == 0);
}

void dbus_warn_log_close() {
  IPCProtocol_log_en_set(false);
}

void dbus_warn_log_open() {
  IPCProtocol_log_en_set(true);
}

int dbus_warn_log_status_get() {
  return IPCProtocol_log_en_get();
};

#endif
