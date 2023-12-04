// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __MEDIASERVER_H__
#define __MEDIASERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

enum ParamType {
    PARAM_TYPE_NONE = 0,
    PARAM_TYPE_INT,
    PARAM_TYPE_STRING,
    PARAM_TYPE_ARRAY,
    PARAM_TYPE_ARRAY_DICT,
    PARAM_TYPE_STRUCT,
};

enum ActionType {
    ACTION_TYPE_NONE = 0,
    ACTION_TYPE_SET,
    ACTION_TYPE_RESET_PIPE,
    ACTION_TYPE_REBOOT,
};

struct KeyTable {
    const char *name;
    const char *method;
    enum ParamType param_type;
    enum ActionType action_type;
};

int mediaserver_set(char *table, int id, const char *data);
int mediaserver_bitrate_set(int id, const char *json);
int mediaserver_advanced_enc_set(int id, int table_id, void* param, int size);
int mediaserver_take_photo(int id, int count);
int mediaserver_start_record(int id);
int mediaserver_stop_record(int id);
int mediaserver_get_record_status(int id);
int mediaserver_feature_id_int(char * method, int id, int enable);
int mediaserver_sync_schedules();
int mediaserver_set_face_image(int id, const char *path);
int mediaserver_delete_face(int id, int face_id);
int mediaserver_clear_face_db();
void mediaserver_stop_flow();
void mediaserver_restart_flow();

#ifdef __cplusplus
}
#endif

#endif
