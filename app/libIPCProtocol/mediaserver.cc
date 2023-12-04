// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>

#include "json-c/json.h"
#include "mediaserver.h"
#include "dbserver.h"

#include "mediaserver_proxy.h"
#include "dbus_connection.h"
#include "libipcpro_log_control.h"

extern int ipc_pro_log_ctl;

#define MEDIA_DBUSSEND(path, interface, member, id, argout) \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        auto proxy_ = std::make_shared<MediaControl>(conn, path, interface); \
        if (proxy_) \
          argout = proxy_->SetParam(member, id); \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
    } \
    dbus_mutex_unlock();

#define MEDIA_DBUSSEND_PARAM(path, interface, member, id, param, argout) \
    dbus_mutex_lock(); \
    try { \
        DBus::Connection conn = get_dbus_conn(); \
        auto proxy_ = std::make_shared<MediaControl>(conn, path, interface); \
        if (proxy_) \
          argout = proxy_->SetParam(member, id, param); \
    } catch (DBus::Error err) { \
        ipc_pro_log_ctl && printf("DBus::Error - %s\n", err.what()); \
    } \
    dbus_mutex_unlock();

/*
bitrate set in setBitRate
*/
static struct KeyTable g_encoder_table[] = {
    { "iGOP",            "SetGOP",            PARAM_TYPE_INT,    ACTION_TYPE_SET},
    // { "iMaxRate",        "SetMaxRate",        PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iStreamSmooth",   "SetStreamSmooth",   PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "sFrameRate",      "SetFrameRate",      PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sResolution",     "SetResolution",     PARAM_TYPE_STRING, ACTION_TYPE_RESET_PIPE},
    { "sRCQuality",      "SetRCQuality",      PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sOutputDataType", "SetOutputDataType", PARAM_TYPE_STRING, ACTION_TYPE_RESET_PIPE},
    { "sRCMode",         "SetRCMode",         PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sH264Profile",    "SetH264Profile",    PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sSmart",          "SetSmart",          PARAM_TYPE_STRING, ACTION_TYPE_RESET_PIPE},
    { "sSVC",            "SetSVC",            PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sVideoType",      "SetVideoType",      PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sGrayScaleMode",  "SetFullRange",      PARAM_TYPE_STRING, ACTION_TYPE_RESET_PIPE},
};

static struct KeyTable g_advanced_encoder_table[] = {
    { NULL, "SSetQp",      PARAM_TYPE_ARRAY,    ACTION_TYPE_SET},
    { NULL, "SSetSplit",   PARAM_TYPE_ARRAY,    ACTION_TYPE_SET},
};

static struct KeyTable g_audio_table[] = {
    { "iSampleRate", "SetSampleRate", PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iVolume",     "SetVolume",     PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iBitRate",    "SetBitRate",    PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "sInput",      "SetInput",      PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sEncodeType", "SetEncodeType", PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sANS",        "SetANS",        PARAM_TYPE_STRING, ACTION_TYPE_SET},
};

static struct KeyTable g_camera_table[] = {
    { NULL, "StartCamera",     PARAM_TYPE_NONE,   ACTION_TYPE_RESET_PIPE},
    { NULL, "StopCamera",      PARAM_TYPE_NONE,   ACTION_TYPE_RESET_PIPE},
};

static struct KeyTable g_feature_table[] = {
    { NULL, "TakePicture",      PARAM_TYPE_INT,        ACTION_TYPE_SET},
    { NULL, "StartRecord",      PARAM_TYPE_NONE,       ACTION_TYPE_SET},
    { NULL, "StopRecord",       PARAM_TYPE_NONE,       ACTION_TYPE_SET},
    { NULL, "SetOsd",           PARAM_TYPE_ARRAY_DICT, ACTION_TYPE_SET},
    { NULL, "SetROI",           PARAM_TYPE_STRING,     ACTION_TYPE_SET},
    { NULL, "SetRegionInvade",  PARAM_TYPE_ARRAY,      ACTION_TYPE_SET},
    { NULL, "SetMoveDetect",    PARAM_TYPE_ARRAY,      ACTION_TYPE_SET},
    { NULL, "SetFaceDetectEn",  PARAM_TYPE_INT,        ACTION_TYPE_SET},
    { NULL, "SetDrawFaceEn",    PARAM_TYPE_INT,        ACTION_TYPE_SET},
    { NULL, "SetFaceCaptureEn", PARAM_TYPE_INT,        ACTION_TYPE_SET},
    { NULL, "SetFaceRegEn",     PARAM_TYPE_INT,        ACTION_TYPE_SET},
};

static struct KeyTable g_face_config_table[] = {
    { "iPromptVolume",             "SetPromptVolume",             PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iLiveDetectThreshold",      "SetLiveDetectThreshold",      PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iFaceDetectionThreshold",   "SetFaceDetectionThreshold",   PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iFaceRecognitionThreshold", "SetFaceRecognitionThreshold", PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iFaceMinPixel",             "SetFaceMinPixel",             PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iLeftCornerX",              "SetLeftCornerX",              PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iLeftCornerY",              "SetLeftCornerY",              PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iDetectWidth",              "SetDetectWidth",              PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "iDetectHeight",             "SetDetectHeight",             PARAM_TYPE_INT,    ACTION_TYPE_SET},
    { "sLiveDetect",               "SetLiveDetectEn",             PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sLiveDetectBeginTime",      "SetLiveDetectBeginTime",      PARAM_TYPE_STRING, ACTION_TYPE_SET},
    { "sLiveDetectEndTime",        "SetLiveDetectEndTime",        PARAM_TYPE_STRING, ACTION_TYPE_SET},
};

static int mediaserver_cmd(const char *path, const char *interface,
                           const char *cmd, int id)
{
    int ret;
    MEDIA_DBUSSEND(path, interface, cmd, id, ret);
    return ret;
}

static int mediaserver_cmd_int(const char *path, const char *interface,
                                const char *cmd, int id, int param)
{
    int ret;
    MEDIA_DBUSSEND_PARAM(path, interface, cmd, id, param, ret);
    return ret;
}

static int mediaserver_cmd_array(const char *path, const char *interface,
                                    const char *cmd, int id, void *param, int size)
{
    int ret;
    int32_t *values = (int32_t *)param;
    std::vector<int32_t> params;
    for (int i = 0; i < size; i++)
        params.push_back(values[i]);
    MEDIA_DBUSSEND_PARAM(path, interface, cmd, id, params, ret);
    return ret;
}

static int mediaserver_cmd_string(const char *path, const char *interface,
                                    const char *cmd, int id, const char *param)
{
    int ret;
    std::string params = param;
    MEDIA_DBUSSEND_PARAM(path, interface, cmd, id, params, ret);
    return ret;
}

static int mediaserver_cmd_array_dict(const char *path, const char *interface,
                                    const char *cmd, int id, const char *param)
{
    int ret;
    std::map<std::string, std::string> params;
    json_object *new_obj = json_tokener_parse(param);
    json_object_object_foreach(new_obj, key, val) {
        const char * value_str = json_object_get_string(val);
        params.emplace(key, value_str);
    }
    json_object_put(new_obj);
    MEDIA_DBUSSEND_PARAM(path, interface, cmd, id, params, ret);
    return ret;
}

static int find_method_index(struct KeyTable *table,int table_size,
                                  const char* key) {
    for (int index = 0; index < table_size; index++) {
        if (!strcmp(key, table[index].name))
			return index;
	}
	return -1;
}

static int find_need_reset_index(struct KeyTable *table, int table_size,
                                  const char* key) {
    for (int index = 0; index < table_size; index++) {
        if (!strcmp(key, table[index].name) &&
            table[index].action_type == ACTION_TYPE_RESET_PIPE) {
            return index;
        }
    }
    return -1;
}

static int do_mediaserver_cmd(const char * path, const char * interface,
                                const char * method, int type,
                                int id, const char* value_str) {
    int ret = 0;
    if (type == PARAM_TYPE_STRING) {
        ret = mediaserver_cmd_string(path, interface, method, id, value_str);
    } else if (type == PARAM_TYPE_INT) {
        int value = atoi(value_str);
        ret = mediaserver_cmd_int(path, interface, method, id, value);
    } else if (type == PARAM_TYPE_NONE) {
        ret = mediaserver_cmd(path, interface, method, id);
    } else if (type == PARAM_TYPE_ARRAY_DICT) {
        ret = mediaserver_cmd_array_dict(path, interface, method, id, value_str);
    }
    return ret;
}

static int do_mediaserver_set(const char * path, const char * interface,
                              struct KeyTable *table, int table_size,
                              int id, const char *data) {
    int ret = 0;
    json_object *new_obj = json_tokener_parse(data);
    json_object_object_foreach(new_obj, find_key, find_val) {
        const char * value_str = json_object_get_string(find_val);
        int index = find_need_reset_index(table, table_size, find_key);
        if (index >= 0) {
            const char * method = table[index].method;
            int type = table[index].param_type;
            ret = do_mediaserver_cmd(path, interface, method, type, id, value_str);
            json_object_put(new_obj);
            return ret;
        }
    }
    json_object_object_foreach(new_obj, key, val) {
        const char * value_str = json_object_get_string(val);
        int index = find_method_index(table, table_size, key);
        if (index >= 0) {
            const char * method = table[index].method;
            int type = table[index].param_type;
            ret = do_mediaserver_cmd(path, interface, method, type, id, value_str);
        }
    }
    json_object_put(new_obj);
    return ret;
}

static int mediaserver_video_set(int id, const char *data) {
    int ret = 0;
    int table_size = sizeof(g_encoder_table) / sizeof(struct KeyTable);
	ret = do_mediaserver_set(MEDIASERVER_ENCODER_PATH,
		                     MEDIASERVER_ENCODER_INTERFACE,
		                     g_encoder_table, table_size, id, data);
    return ret;
}

static int mediaserver_audio_set(int id, const char *data) {
    int ret = 0;
    int table_size = sizeof(g_audio_table) / sizeof(struct KeyTable);
	ret = do_mediaserver_set(MEDIASERVER_AUDIO_PATH,
		                     MEDIASERVER_AUDIO_INTERFACE,
		                     g_audio_table, table_size, id, data);
    return ret;
}

static int mediaserver_osd_set(int id, const char *data) {
    int ret = 0;
	ret = mediaserver_cmd_array_dict(MEDIASERVER_FEATURE_PATH,
                                 MEDIASERVER_FEATURE_INTERFACE,
	                             "SetOsd", id, data);
    return ret;
}

static int mediaserver_roi_set(int id, const char *data) {
    int ret = 0;
    ret = mediaserver_cmd_string(MEDIASERVER_FEATURE_PATH,
                                 MEDIASERVER_FEATURE_INTERFACE,
                                 "SetRoi", id, data);
    return ret;
}

static int mediaserver_move_detect_set(int id, const char *data) {
    int ret = 0;
    ret = mediaserver_cmd_array_dict(MEDIASERVER_FEATURE_PATH,
                                     MEDIASERVER_FEATURE_INTERFACE,
                                     "SetMoveDetect", id, data);
    return ret;
}

static int mediaserver_region_invade_set(int id, const char *data) {
    int ret = 0;
    ret = mediaserver_cmd_array_dict(MEDIASERVER_FEATURE_PATH,
                                     MEDIASERVER_FEATURE_INTERFACE,
                                     "SetRegionInvade", id, data);
    return ret;
}

static int mediaserver_face_config_set(int id, const char *data) {
    int ret = 0;
    int table_size = sizeof(g_face_config_table) / sizeof(struct KeyTable);
    ret = do_mediaserver_set(MEDIASERVER_FACE_RECOGNIZE_PATH,
                             MEDIASERVER_FACE_RECOGNIZE_INTERFACE,
                             g_face_config_table, table_size, id, data);
    return ret;
}

int mediaserver_set(char *table, int id, const char *data) {
    int ret = 0;

    if (!strcmp(table, TABLE_VIDEO)) {
		ret = mediaserver_video_set(id, data);
	}else if (!strcmp(table, TABLE_AUDIO)) {
		ret = mediaserver_audio_set(id, data);
	} else if (!strcmp(table, TABLE_OSD)) {
		ret = mediaserver_osd_set(id, data);
	} else if (!strcmp(table, TABLE_ROI)) {
		ret = mediaserver_roi_set(id, data);
	} else if (!strcmp(table, TABLE_MOVE_DETECTION)) {
		ret = mediaserver_move_detect_set(id, data);
	} else if (!strcmp(table, TABLE_REGIONAL_INVASION)) {
		ret = mediaserver_region_invade_set(id, data);
	} else if (!strcmp(table, TABLE_FACE_CONFIG)) {
		mediaserver_face_config_set(id, data);
	}

    return ret;
}

int mediaserver_bitrate_set(int id, const char *json) {
    int ret = 0;
    ret = mediaserver_cmd_string(MEDIASERVER_ENCODER_PATH,
                                 MEDIASERVER_ENCODER_INTERFACE,
                                 "SetBitRate", id, json);
    return ret;
}

int mediaserver_advanced_enc_set(int id, int table_id, void* param, int size) {
    int ret = 0;
    ret =mediaserver_cmd_array(MEDIASERVER_ADVANCED_ENCODER_PATH,
                               MEDIASERVER_ADVANCED_ENCODER_INTERFACE,
                               g_advanced_encoder_table[table_id].method,
                               id, param, size);
    return ret;
}

int mediaserver_take_photo(int id, int count) {
    int ret = 0;
    ret = mediaserver_cmd_int(MEDIASERVER_FEATURE_PATH, MEDIASERVER_FEATURE_INTERFACE,
                              "TakePicture", id, count);
    return ret;
}

int mediaserver_start_record(int id) {
    int ret = 0;
    ret = mediaserver_cmd(MEDIASERVER_FEATURE_PATH, MEDIASERVER_FEATURE_INTERFACE,
                          "StartRecord", id);
    return ret;
}

int mediaserver_stop_record(int id) {
    int ret = 0;
    ret = mediaserver_cmd(MEDIASERVER_FEATURE_PATH, MEDIASERVER_FEATURE_INTERFACE,
                          "StopRecord", id);
    return ret;
}

int mediaserver_get_record_status(int id) {
    int ret = 0;
    ret = mediaserver_cmd(MEDIASERVER_FEATURE_PATH, MEDIASERVER_FEATURE_INTERFACE,
                          "GetRecordStatus", id);
    return ret;
}

int mediaserver_feature_id_int(char * method, int id, int enable) {
    int ret = 0;
    ret = mediaserver_cmd_int(MEDIASERVER_FEATURE_PATH, MEDIASERVER_FEATURE_INTERFACE,
                              method, id, enable);
    return ret;
}

int mediaserver_sync_schedules() {
    int ret = 0;
    ret = mediaserver_cmd(MEDIASERVER_FEATURE_PATH, MEDIASERVER_FEATURE_INTERFACE,
                          "SyncSchedules", 0);
    return ret;
}

int mediaserver_set_face_image(int id, const char *path) {
    int ret = 0;
    ret = mediaserver_cmd_string(MEDIASERVER_FACE_RECOGNIZE_PATH,
                                 MEDIASERVER_FACE_RECOGNIZE_INTERFACE,
                                 "SetImageToRegn", id, path);
    return ret;
}

int mediaserver_delete_face(int id, int face_id) {
    int ret = 0;
    ret = mediaserver_cmd_int(MEDIASERVER_FACE_RECOGNIZE_PATH,
                                 MEDIASERVER_FACE_RECOGNIZE_INTERFACE,
                                 "DeleteFaceInfo", id, face_id);
    return ret;
}

int mediaserver_clear_face_db() {
    int ret = 0;
    ret = mediaserver_cmd_int(MEDIASERVER_FACE_RECOGNIZE_PATH,
                                 MEDIASERVER_FACE_RECOGNIZE_INTERFACE,
                                 "ClearFaceDB", 0, 0);
    return ret;
}

void mediaserver_stop_flow() {
    mediaserver_cmd(MEDIASERVER_ENCODER_PATH,
                    MEDIASERVER_ENCODER_INTERFACE,
                    "StopFlow", 0);
}

void mediaserver_restart_flow() {
    mediaserver_cmd(MEDIASERVER_ENCODER_PATH,
                    MEDIASERVER_ENCODER_INTERFACE,
                    "RestartFlow", 0);
}