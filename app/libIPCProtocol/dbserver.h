// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __DBSERVER_H
#define __DBSERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#define DBSERVER  "rockchip.dbserver"
#define DBSERVER_PATH      "/"

#define DBSERVER_NET_INTERFACE  DBSERVER ".net"
#define DBSERVER_STORAGE_INTERFACE  DBSERVER ".storage"
#define DBSERVER_MEDIA_INTERFACE  DBSERVER ".media"
#define DBSERVER_SYSTEM_INTERFACE  DBSERVER ".system"
#define DBSERVER_EVENT_INTERFACE  DBSERVER ".event"
#define DBSERVER_PERIPHERALS_INTERFACE  DBSERVER ".peripherals"

#define DS_SIGNAL_DATACHANGED   "DataChanged"

/* network */
#define TABLE_NETWORK_IP            "NetworkIP"
#define TABLE_NETWORK_SERVICE       "NetworkService"
#define TABLE_NETWORK_POWER         "NetworkPower"
#define TABLE_NTP                   "ntp"
#define TABLE_ZONE                  "zone"
#define TABLE_PORT                  "port"

/* storage */
#define TABLE_STORAGE_DISK_PATH     "StorageDiskPath"
#define TABLE_STORAGE_MEDIA_FOLDER  "StorageMediaFolder"
#define TABLE_STORAGE_CONFIG        "StorageConfig"
#define TABLE_STORAGE_PLAN_SNAP     "StoragePlanSnap"
#define TABLE_STORAGE_ADVANCE_PARA  "StorageAdvancePara"

/* media */
#define TABLE_VIDEO             "video"
#define TABLE_VIDEO_ADVANCED_ENC "video_advanced_enc"
#define TABLE_VIDEO_REGION_CLIP "video_region_clip"
#define TABLE_AUDIO             "audio"
#define TABLE_STREAM_URL        "stream_url"

#define TABLE_IMAGE_SCENARIO            "image_scenario"
#define TABLE_IMAGE_ADJUSTMENT          "image_adjustment"
#define TABLE_IMAGE_EXPOSURE            "image_exposure"
#define TABLE_IMAGE_NIGHT_TO_DAY        "image_night_to_day"
#define TABLE_IMAGE_BLC                 "image_blc"
#define TABLE_IMAGE_WHITE_BLANCE        "image_white_blance"
#define TABLE_IMAGE_ENHANCEMENT         "image_enhancement"
#define TABLE_IMAGE_VIDEO_ADJUSTMEN     "image_video_adjustment"

#define TABLE_NORMALIZED_SCREEN_SIZE        "normalized_screen_size"
#define TABLE_OSD                           "osd"
#define TABLE_ROI                           "roi"
#define TABLE_MOVE_DETECTION                "MoveDetection"
#define TABLE_REGIONAL_INVASION             "RegionalInvasion"
#define TABLE_PROFILE                       "profile"
#define TABLE_METADATA                      "metadata"
#define TABLE_VIDEO_SOURCE                  "video_source"
#define TABLE_VIDEO_SOURCE_CONFIGURATION    "video_source_configuration"

/* system */
#define TABLE_SYSTEM_DEVICE_INFO        "SystemDeviceInfo"
#define TABLE_SYSTEM_PARA               "SystemPara"
#define TABLE_SYSTEM_USER               "SystemUser"
#define TABLE_SYSTEM_DISCOVERY          "SystemDiscovery"
#define TABLE_SYSTEM_SCOPES             "SystemScopes"

/* event */
#define TABLE_EVENT_TRIGGERS            "EventTriggers"
#define TABLE_EVENT_SCHEDULES           "EventSchedules"
#define TABLE_SMART_COVER               "SmartCover"
#define TABLE_SMART_COVER_OVERLAY       "SmartCoverOverlay"

#define TABLE_FACE_CONFIG               "FaceConfig"
#define TABLE_FACE_LIST                 "FaceList"
#define TABLE_FACE_SNAPSHOT_RECORD      "FaceSnapshotRecord"
#define TABLE_FACE_CONTROL_RECORD       "FaceControlRecord"

/* peripherals */
#define TABLE_PERIPHERALS_RELAY               "PeripheralsRelay"
#define TABLE_PERIPHERALS_WEIGEN              "PeripheralsWeigen"
#define TABLE_PERIPHERALS_FILL_LIGHT          "PeripheralsFillLight"

#define TYPE_VIDEO         0
#define TYPE_PHOTO         1
#define TYPE_BLACK_LIST    2
#define TYPE_SNAPSHOT      3
#define TYPE_WHITE_LIST    4
#define MAX_OSD_NUM        15

struct period
{
    int start_minute;
    int end_minute;
    char type[20];
};

struct day
{
    struct period day_period[8];
};

struct week
{
    struct day week_day[7];
};

struct DynamicLocation
{
    char *cap_name;
    char *dynamic_key;
    char *dynamic_val;
    char *target_key;
};

struct StaticLocation
{
    char *cap_name;
    char *target_key;
};

struct RangeJsonPara
{
    int min;
    int max;
    int step;
};

/* storage */
char *dbserver_update_storage_media_folder_duty(int camid, int type, int duty, int maxnum);
char *dbserver_update_storage_config_mountpath(char *path);
char *dbserver_update_storage_config_freesize(int freesize);
char *dbserver_get_storage_disk_path(char *mountpath);
char *dbserver_get_storage_media_folder(void);
char *dbserver_get_storage_config(void);
char *dbserver_get_storage_plan_snap(int id);
char *dbserver_set_storage_plan_snap(char *json, int id);
void dbserver_storage_snap_plan_parse(int type_id, int *ebaled, int *quality, int *interval, int *num);
void dbserver_storage_video_plan_parse(int *enbaled);
char *dbserver_storage_get(char *table);
char *dbserver_storage_set(char *table, char *json, int id);
char *dbserver_storage_disk_path_get_by_name(char *name);
char *dbserver_storage_disk_path_set_by_name(char *name, char *mountpath);

/* network */
char *dbserver_network_power_get(char *type);
char *dbserver_network_ipv4_set(char *interface, char *Method, char *Address, char *Netmask, char *Gateway);
char *dbserver_network_dns_set(char *interface, char *dns1, char *dns2);
char *dbserver_network_nicspeed_set(char *interface, char *nicspeed);
char *dbserver_network_ip_get(char *interface);
char *dbserver_network_service_delete(char *service);
char *dbserver_network_service_connect_set(char *service, char *password, int *favorite, int *autoconnect);
char *dbserver_network_service_get(char *service);
char *dbserver_network_power_set(char *type, int power);
char *dbserver_wifi_power_set(int power);
char *dbserver_ethernet_power_set(int power);
char *dbserver_wifi_power_get(void);
char *dbserver_ethernet_power_get(void);
char *dbserver_ntp_set(char *servers, char *timezone, char *timezonefile, char *timezonefiledst, int *autodst, int *automode, int *time);
char *dbserver_ntp_get(void);
char *dbserver_zone_get(void);
char *dbserver_port_set(char *json, int id);
char *dbserver_port_get(void);

/* media */
char *dbserver_media_set(char *table, char *json, int id);
char *dbserver_media_set_by_token(char *table, char *json, char *token);
void dbserver_media_del_by_token(char *table, char *token);
char *dbserver_media_get(char *table);
char *dbserver_media_get_by_id(char *table, int id);
char *dbserver_osd_get(void);
char *dbserver_audio_set(char *json);
char *dbserver_audio_get(void);
char *dbserver_video_set(char *json, char *stream_type);
char *dbserver_video_get(void);
char *dbserver_video_region_clip_set(char *json, int id);
char *dbserver_video_region_clip_get(void);
char *dbserver_stream_url_set(char *json, int id);
char *dbserver_stream_url_get(void);
char *dbserver_media_profile_get(char *token);
char *dbserver_media_get_by_key_char(char *table, char *key_word, char *val);
char *dbserver_video_source_cfg_get(char *token);
char *dbserver_video_source_get(char *token);
char *dbserver_video_enc_cfg_get(char *token);

/* system */
char *dbserver_system_set(char *table, char *json, int id);
char *dbserver_system_get(char *table);
char *dbserver_system_get_by_key_char(char *table, char *key_word, char *val);
char *dbserver_system_para_get_by_name(char *para_name);
char *dbserver_system_para_set_by_name(char *para_name, char *para);
void dbserver_set_static_cap_option(struct StaticLocation location, char *options);
void dbserver_set_dynamic_cap_option(struct DynamicLocation location, char *options);
void dbserver_set_static_cap_range(struct StaticLocation location, struct RangeJsonPara range);
void dbserver_set_dynamic_cap_range(struct DynamicLocation location, struct RangeJsonPara range);

void dbserver_system_user_delete(int id);
char *dbserver_system_user_add(char *username, char *password, int *authlevel, int *userlevel, int *fixed);
char *dbserver_system_user_del_username(char *username);
char *dbserver_system_user_set(char *username, char *password, int *authlevel, int *userlevel);
char *dbserver_system_user_get(char *username, char *password, int *authlevel, int *userlevel);
int dbserver_system_user_num_get(char *username, char *password, int *authlevel, int *userlevel);
void dbserver_scopes_add(char *scopesItem);
void dbserver_system_del_by_key_char(char *table, char *key_word, char* key_content);
void dbserver_system_del_by_key_int(char *table, char *key_word, int key_content);

/* event */
char *dbserver_event_set(char *table, char *json, int id);
char *dbserver_event_set_by_char_key(char *table, char *json, char* key_name, char* key_content);
char *dbserver_event_get(char *table);
char *dbserver_event_get_by_id(char *table, int id);
char *dbserver_event_get_by_key_int(char *table, char *key_word, int val);
char *dbserver_event_get_by_key_char(char *table, char *key_word, char *val);
void dbserver_event_delete_by_key_int(char *table, char *key_word, int val);
void dbserver_event_delete_by_key_char(char *table, char *key_word, char *val);

void dbserver_face_list_add(int id, char *path, char *name, char *type);
void dbserver_face_load_complete(int id, int flag);
void dbserver_face_load_complete_by_path(char* path, int flag, int face_db_id);
char *dbserver_face_list_delete(int id);
void dbserver_snapshot_record_set(char *path);
void dbserver_face_reset(char *table);
void dbserver_control_record_set(int face_id, char *path, char *status, char *similarity);
void dbserver_event_schedules_parse(struct week *schedule, int type_id);
void dbserver_event_triggers_parse(int id, int *record_ebaled);

/* peripherals */
char *dbserver_peripherals_set(char *table, char *json, int id);
char *dbserver_peripherals_get(char *table);

/* basic */
char *dbserver_select(char *json, char *interface);
char *dbserver_update(char *json, char *interface);
char *dbserver_delete(char *json, char *interface);
char *dbserver_sql(char *sql, char *interface);
char *dbserver_drop_table(char *table, char *interface);
char *dbserver_create_table(char *table, char *col, char *interface);
int dbserver_reset_face_table(char *table);

void dbserver_free(char *ret_str);
#ifdef __cplusplus
}
#endif

#endif