#if CONFIG_DBSERVER

#ifndef __DB_MONITOR_H
#define __DB_MONITOR_H

#include "isp_func.h"
#include "rkaiq/common/rk_aiq_comm.h"
#include "rkaiq/uAPI/rk_aiq_user_api_sysctl.h"

#define SCENARIO_MIN 0
#define SCENARIO_MAX 4

#ifdef __cplusplus
extern "C" {
#endif

int dbus_warn_log_status_get();
void dbus_warn_log_close();
void dbus_warn_log_open();
bool wait_dbus_init_func(void);
char *dbserver_image_hdr_mode_get(void);
void dbserver_image_adjustment_get(int *brightness, int *contrast,
                                   int *saturation, int *sharpness, int *hue,int *evbias);
void dbserver_image_exposure_get(char *exposure_time, int *exposure_gain);
void dbserver_image_blc_get(char *hdr_mode, int *hdr_level, int *blc_strength,
                            int *hlc_level, int *dark_level);
void dbserver_image_enhancement_get(char *nr_mode, char *fec_mode,
                                    char *dehaze_mode, int *denoise_level,
                                    int *spatial_level, int *temporal_level,
                                    int *dehaze_level, int *fec_level,
                                    int *rotation);
void dbserver_image_video_adjustment_get(char *frequency_mode, char *flip_mode);
void dbserver_image_night_to_day_get(rk_aiq_cpsl_cfg_t *cpsl_cfg);
void dbserver_image_white_balance_get(char *white_balance_style, int *red_gain,
                                      int *green_gain, int *blue_gain);

int hash_image_hdr_mode_get4init(rk_aiq_working_mode_t *hdr_mode);
int hash_image_hdr_mode_get_without_sync_gc();
int hash_image_fec_enable_get4init(int *fec_en, int *fec_level);
int hash_image_adjustment_get(int *brightness, int *contrast, int *saturation,
                              int *sharpness, int *hue);
int hash_image_exposure_get(int *auto_exposure, float *exposure_time,
                            int *auto_gain, int *exposure_gain);
int hash_image_blc_get(rk_aiq_working_mode_t *hdr_mode, int *hdr_level,
                       int *blc_strength, int *hlc_level, int *dark_level);
int hash_image_enhancement_get(nr_mode_t *nr_mode, dc_mode_t *dc_mode,
                               work_mode_2_t *dehaze_mode, int *spatial_level,
                               int *temporal_level, int *dehaze_level,
                               int *ldch_level, int *fec_level, int *rotation);
int hash_image_video_adjustment_get(expPwrLineFreq_t *frequency_mode,
                                    flip_mode_t *flip_mode);
int hash_image_night_to_day_get(rk_aiq_cpsl_cfg_t *cpsl_cfg);
int hash_image_white_balance_get(white_balance_mode_t *white_balance_style,
                                 int *red_gain, int *green_gain,
                                 int *blue_gain);
void *hash_ir_smart_json_get();

void exposure_para_set_by_db(void);
void night_to_day_para_set_by_db();
void night_to_day_para_cap_set_db(void);

int exposure_para_set_by_hash(void);
int blc_normal_mode_para_set_by_hash(void);
int white_balance_set_by_hash_table(void);

void database_init(void);
void database_hash_init(void);

#ifdef __cplusplus
}

#endif

#endif

#endif
