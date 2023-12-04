#if CONFIG_DBSERVER

#include <errno.h>
#include <gdbus.h>
#include <glib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json-c/json.h"

#include "../utils/log.h"
#include "db_monitor.h"
#include "isp_func.h"
#include "isp_n2d_ctl.h"

#define AUTO_CALCULATE 1

static int gc_smart_ir_running_flag = 0;
static int night2day_loop_flag = 0;
static int ir_smart_running = 0;
static gint ir_smart_timeouttag = -1;
static gint led_brightness_timeouttag = -1;
static pthread_t tid;

static int32_t now_time_sec_get() {
  time_t i32_seconds = time(NULL);
  struct tm *time_s = localtime(&i32_seconds);
  int32_t hour_num = time_s->tm_hour;
  int32_t minute_num = time_s->tm_min;
  int32_t second_num = time_s->tm_sec;
  int32_t i32_seconds_rst =
      (hour_num * 60 * 60) + (minute_num * 60) + second_num;
  return i32_seconds_rst;
}

void ir_smart_control_loop(void *pdata) {
  int s32Ret = 0;
  unsigned char u8Num = 0;
  night2day_mode_t night2day_mode = ND_AUTO_MODE;
  double dLumaDay = 0.0;
  double dLumaNight = 0.0;
  double dBGratio = 0.0;
  double dRGratio = 0.0;
  unsigned int u32_exposure = 0;
  double dExpAgain = 0;
  int i_time = 0;

  json_object *j_cfg_test = (json_object *)hash_ir_smart_json_get();
  if (NULL == j_cfg_test) {
    LOG_WARN("get cfg fail\n");
    return;
  } else {
    json_object_put(j_cfg_test);
  }
  int hdr_mode_init = hash_image_hdr_mode_get_without_sync_gc();
  set_day_mode(hdr_mode_init);

  while (night2day_loop_flag) {
    json_object *j_cfg = (json_object *)hash_ir_smart_json_get();
    if (NULL == j_cfg) {
      LOG_WARN("get ir smart data is null\n");
      continue;
    }
    int sensitive = (int)json_object_get_int(
        json_object_object_get(j_cfg, "iNightToDayFilterLevel"));
    night2day_mode_t db_n2d_mode = (night2day_mode_t)json_object_get_int(
        json_object_object_get(j_cfg, "iNightToDay"));
    int db_fill_light_brightness = (int)json_object_get_int(
        json_object_object_get(j_cfg, "iLightBrightness"));
    int hdr_mode_loop = hash_image_hdr_mode_get_without_sync_gc();
#if 1
    if (0 == i_time % 4) {
      s32Ret = rk_smart_get_scene_param(&dLumaDay, &dLumaNight, &dRGratio,
                                        &dBGratio, &u32_exposure, &dExpAgain);
      i_time = 0;
      // LOG_DEBUG("u8Num = %d dLumaNight = %f dRGratio = %f dBGratio = %f
      // dExpAgain = %f \n", get_led_state(), dLumaNight, dRGratio, dBGratio,
      // dExpAgain);
    }
#endif

    switch (db_n2d_mode) {
    case ND_AUTO_MODE: {
      if (!AUTO_CALCULATE) {
        if (night2day_mode == db_n2d_mode) {
          u8Num = 0;
        }
      }
      work_mode_0_t auto_exposure_enable = (work_mode_0_t)json_object_get_int(
          json_object_object_get(j_cfg, "iAutoExposureEnabled"));
      if (auto_exposure_enable == WM0_MANAUL_MODE) {
        LOG_DEBUG("%d This is manual exposure mode\n", __LINE__);
        if (LED_ON == get_led_state()) {
          set_day_mode(hdr_mode_loop);
          u8Num = 0;
        }
        break;
      }
      if (AUTO_CALCULATE) {
        if (u8Num == 0) {
          int filter_level = (int)json_object_get_int(
              json_object_object_get(j_cfg, "iNightToDayFilterLevel"));
          int filter_time = (int)json_object_get_int(
              json_object_object_get(j_cfg, "iNightToDayFilterTime"));
          night_to_day_auto_mode_set(filter_level, filter_time);
          u8Num = 1;
        }
      } else {
        s32Ret = rk_smart_get_scene_param(&dLumaDay, &dLumaNight, &dRGratio,
                                          &dBGratio, &u32_exposure, &dExpAgain);
        if (-1 == s32Ret) {
          break;
        }

        int d2n_filter_time = (int)json_object_get_int(
            json_object_object_get(j_cfg, "iNightToDayFilterTime"));
        if (LED_OFF == get_led_state()) {

          if (u8Num == d2n_filter_time * 2) {
            set_night_mode(db_fill_light_brightness);
            u8Num = 0;
            sleep(1);
          }

          if (dExpAgain > 128.0 - (sensitive - 3) * 8) {
            u8Num++;
          } else {
            u8Num = 0;
          }
        }

        if (LED_ON == get_led_state()) {
          if (u8Num == d2n_filter_time * 2) {
            set_day_mode(hdr_mode_loop);
            u8Num = 0;
            sleep(1);
          }

          if ((dLumaNight > 9.0 - (sensitive - 3) * 0.3) &&
              (dRGratio < 0.78 || dBGratio < 0.78)) {
            u8Num++;
          } else {
            u8Num = 0;
          }
        }
      }
      break;
    }

    case ND_DAY_MODE: {
      if (LED_ON == get_led_state()) {
        set_day_mode(hdr_mode_loop);
      }
      break;
    }

    case ND_NIGHT_MODE: {
      if (LED_OFF == get_led_state()) {
        set_night_mode(db_fill_light_brightness);
      }
      break;
    }

    case ND_SCHEDULE_MODE: {
      int32_t begin_sec = (int32_t)json_object_get_int(
          json_object_object_get(j_cfg, "iDawnSeconds"));
      int32_t end_sec = (int32_t)json_object_get_int(
          json_object_object_get(j_cfg, "iDuskSeconds"));
      int32_t now_s = now_time_sec_get();
      LOG_DEBUG("dawn: %d, dark: %d, now: %d\n", begin_sec, end_sec, now_s);
      // north
      if (begin_sec < end_sec) {
        if (now_s >= begin_sec && now_s < end_sec) {
          if (LED_ON == get_led_state())
            set_day_mode(hdr_mode_loop);
        } else {
          if (LED_OFF == get_led_state())
            set_night_mode(db_fill_light_brightness);
        }
        // south
      } else if (begin_sec > end_sec) {
        if (now_s < begin_sec && now_s >= end_sec) {
          if (LED_OFF == get_led_state())
            set_night_mode(db_fill_light_brightness);
        } else {
          if (LED_ON == get_led_state())
            set_day_mode(hdr_mode_loop);
        }
        // Polar day
      } else {
        if (LED_ON == get_led_state()) {
          set_day_mode(hdr_mode_loop);
        }
      }
      break;
    }
    // day mode
    default: {
      if (LED_ON == get_led_state()) {
        set_day_mode(hdr_mode_loop);
      }
      break;
    }
    }

    night2day_mode = db_n2d_mode;
    json_object_put(j_cfg);
    usleep(500000);

    i_time++;
  }

  pthread_detach(pthread_self());
}

void night2day_loop_run(void) {
  if (night2day_loop_flag)
    return;
  night2day_loop_flag = 1;
  pthread_create(&tid, NULL, (void *)ir_smart_control_loop, NULL);
}

void night2day_loop_stop(void) {
  if (!night2day_loop_flag)
    return;
  night2day_loop_flag = 0;
  pthread_join(tid, NULL);
}

#endif