/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "camera_pu_control.h"
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "json-c/json.h"
#include <pthread.h>
#include "uvc_log.h"

#if DBSERVER_SUPPORT
#include "dbserver.h"

extern "C" int video_record_set_brightness(int brightness) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iBrightness", json_object_new_int(brightness));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_contrast(int contrast) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iContrast", json_object_new_int(contrast));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_hue(int hue) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iHue", json_object_new_int(hue));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_staturation(int staturation) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iSaturation", json_object_new_int(staturation));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_sharpness(int sharpness) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iSharpness", json_object_new_int(sharpness));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_fps(int fixfps) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iFPS", json_object_new_int(fixfps));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_gamma(int gamma) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iGamma", json_object_new_int(gamma));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_white_balance_temperature(int balance) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_WHITE_BLANCE;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   //[2800,6500]->[0,100]
   //int bg_level = (balance - 2800)/((6500 - 2800)/100);
   LOG_INFO("iWhiteBalanceRed CT is %d",balance);
   json_object_object_add(js, "iWhiteBalanceRed", json_object_new_int(balance));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_white_balance_temperature_auto(int balance) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_WHITE_BLANCE;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   if (balance == 1)
     json_object_object_add(js, "sWhiteBlanceStyle", json_object_new_string("autoWhiteBalance"));
   else
     json_object_object_add(js, "sWhiteBlanceStyle", json_object_new_string("manualWhiteBalanceCT"));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_gain(int gain) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_EXPOSURE;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   if (gain == 1) {
     json_object_object_add(js, "sGainMode", json_object_new_string("auto"));
   } else {
     json_object_object_add(js, "sExposureMode", json_object_new_string("manual"));
     json_object_object_add(js, "sGainMode", json_object_new_string("manual"));
     json_object_object_add(js, "iExposureGain", json_object_new_int(gain));
   }
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_hue_auto(int hue_auto) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_ADJUSTMENT;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   json_object_object_add(js, "iHueAuto", json_object_new_int(hue_auto));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_frequency_mode(int mode) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_VIDEO_ADJUSTMEN;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
   if (mode == 1)
     json_object_object_add(js, "sPowerLineFrequencyMode", json_object_new_string("PAL(50HZ)"));
   else
     json_object_object_add(js, "sPowerLineFrequencyMode", json_object_new_string("PAL(60HZ)"));
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}

extern "C" int video_record_set_roll_mode(int mode) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_VIDEO_ADJUSTMEN;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
        LOG_DEBUG("+++new json object failed.\n");
        return -1;
   }
    switch (mode) {
    case 0: {
       json_object_object_add(js, "sImageFlip", json_object_new_string("close"));
       break;
    }
    case 1: {
       json_object_object_add(js, "sImageFlip", json_object_new_string("flip"));
       break;
    }
    case 2: {
       json_object_object_add(js, "sImageFlip", json_object_new_string("centrosymmetric"));
       break;
    }
    case 3: {
       json_object_object_add(js, "sImageFlip", json_object_new_string("mirror"));
       break;
    }
    default: {
       json_object_object_add(js, "sImageFlip", json_object_new_string("close"));
       break;
    }
    }
   ret = dbserver_media_set(table, (char*)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}
extern "C" int video_record_set_exposure_time(int time) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_EXPOSURE;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
      LOG_DEBUG("+++new json object failed.\n");
      return -1;
   }
   LOG_DEBUG("exposure time = %d\n", time);
   json_object_object_add(js, "sGainMode", json_object_new_string("auto"));
   char str[48];
   sprintf(str, "%d/10000", time);
   LOG_DEBUG("sExposureTime = %s\n", str);
   json_object_object_add(js, "sExposureTime", json_object_new_string(str));
   ret = dbserver_media_set(table, (char *)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}
extern "C" int video_record_set_ae_mode(int mode) {
   char *ret = NULL;
   char *table = TABLE_IMAGE_EXPOSURE;
   struct json_object *js = NULL;
   js = json_object_new_object();
   if (NULL == js)
   {
      LOG_DEBUG("+++new json object failed.\n");
      return -1;
   }
   if ( mode == 2){
      LOG_DEBUG("AE:set auto mode!!\n");
      json_object_object_add(js, "sExposureMode", json_object_new_string("auto"));
   }else {
      LOG_DEBUG("AE:set  manual mode!!\n");
      json_object_object_add(js, "sExposureMode", json_object_new_string("manual"));
   }
   ret = dbserver_media_set(table, (char *)json_object_to_json_string(js), 0);
   json_object_put(js);
   dbserver_free(ret);
   return 0;
}
#endif

extern "C" void camera_pu_control_init(int type,int def,int min,int max)
{
    LOG_DEBUG("%s!\n", __func__);
}

extern "C" int camera_pu_control_get(int type, int def)
{
    LOG_DEBUG("%s!\n", __func__);
    return def;
}

extern "C" int check_ispserver_work()
{
    int ret = 0;
    FILE  *fsfp;
    char str[32];
    char *filename = "/tmp/.ispserver_cam0";
    fsfp = fopen(filename, "r");
    if (!fsfp) {
          ret = 0;
          goto error_free;
    }
    if (fscanf(fsfp, "%s\n", str) != 1) {
          ret = 0;
          if (fclose(fsfp))
               LOG_ERROR("read_sysfs_string(): Failed to close dir");
          goto error_free;
    }
    if (fclose(fsfp))
          ret = 0;
    LOG_DEBUG("read_fs_string:file:%s,str:%s\n",filename,str);
    if(!strcmp(str,"1"))
          ret = 1;

error_free:
    return ret;
}


extern "C" int camera_pu_control_set(int type, int value)
{
    LOG_DEBUG("%s! type is %d,value is %d\n", __func__,type,value);
#if DBSERVER_SUPPORT
    if(!check_ispserver_work()){
       LOG_INFO("check ispserver is no ready,pu set fail!\n");
       return 0;
    }
    switch (type) {
        case UVC_PU_BRIGHTNESS_CONTROL:
            video_record_set_brightness(value);
            break;
        case UVC_PU_CONTRAST_CONTROL:
            video_record_set_contrast(value);
            break;
        case UVC_PU_HUE_CONTROL:
            video_record_set_hue(value);
            break;
        case UVC_PU_SATURATION_CONTROL:
            video_record_set_staturation(value);
            break;
        case UVC_PU_SHARPNESS_CONTROL:
            video_record_set_sharpness(value);
            break;
        case UVC_PU_GAMMA_CONTROL:
            //video_record_set_gamma(value);
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            video_record_set_white_balance_temperature(value);
            break;
        case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            video_record_set_white_balance_temperature_auto(value);
            break;
        case UVC_PU_GAIN_CONTROL:
            video_record_set_gain(value);
            break;
        case UVC_PU_HUE_AUTO_CONTROL:
            //video_record_set_hue_auto(value);
            break;
        case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
            video_record_set_frequency_mode(value);
            break;
        case UVC_PU_FPS_CONTROL:
            video_record_set_fps(value);
            break;
        case UVC_PU_ROLL_CONTROL:
            video_record_set_roll_mode(value);
            break;
        case UVC_PU_EXPOSURE_TIME_CONTROL:
            video_record_set_exposure_time(value);
            break;
        case UVC_PU_AE_MODE_CONTROL:
            video_record_set_ae_mode(value);
            break;
      default:
         LOG_DEBUG("====unknow pu cmd.\n");
         break;

    }
#endif
    return 0;
}

extern "C" int camera_pu_control_check(int deviceid)
{
    LOG_DEBUG("%s!\n", __func__);
    return 0;
}
