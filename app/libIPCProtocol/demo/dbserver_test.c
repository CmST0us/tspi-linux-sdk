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
#include "dbserver.h"

void dbserver_test(void)
{
    char *json_str;
    //netserver_set_power(TYPE_WIFI, 1);

    json_str = dbserver_wifi_power_set(1);
    printf("%s, dbserver_wifi_power_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_ethernet_power_set(1);
    printf("%s, dbserver_ethernet_power_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_wifi_power_get();
    printf("%s, dbserver_wifi_power_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_ethernet_power_get();
    printf("%s, dbserver_ethernet_power_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_network_ip_get(NULL);
    printf("%s, dbserver_network_ip_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
/*
    json_str = dbserver_network_ipv4_set("eth0", "manual", "192.168.0.113", "255.255.255.0", "192.168.0.1");
    printf("%s, dbserver_network_ipv4_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
*/
    json_str = dbserver_network_ipv4_set("eth0", "dhcp", NULL, NULL, NULL);
    printf("%s, dbserver_network_ipv4_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_network_dns_set("eth0", "10.10.10.188", "58.22.96.67");
    printf("%s, dbserver_network_dns_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_zone_get();
    printf("%s, dbserver_zone_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    int automode = 1;
    int autodst = 0;
    int time = 60;
    //dbserver_ntp_set(NULL, "UTC", &automode, &time);
    json_str = dbserver_ntp_set(NULL, "ChinaStandardTime-8", "posix/Etc/GMT-8","posix/Asia/Shanghai", &autodst, &automode, &time);
    printf("%s, dbserver_ntp_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_ntp_get();
    printf("%s, dbserver_ntp_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    int faorite = 1;
    int autoconnect = 1;
    json_str = dbserver_network_service_connect_set("wifi_8cf710c4d079_54502d4c494e4b5f48_managed_psk", "ABC0123456789", &faorite, &autoconnect);
    printf("%s, dbserver_network_service_connect_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_network_ip_get("eth0");
    printf("%s, dbserver_network_ip_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_network_ip_get("wlan0");
    printf("%s, dbserver_network_ip_get, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_update_storage_media_folder_duty(0, 0, 45, -1);
    printf("%s, dbserver_update_storage_media_folder_duty, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_update_storage_media_folder_duty(0, 1, 5, -1);
    printf("%s, dbserver_update_storage_media_folder_duty, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_update_storage_config_freesize(1024);
    printf("%s, dbserver_update_storage_config_freesize, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_update_storage_config_mountpath("/userdata");
    printf("%s, dbserver_update_storage_config_mountpath, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_get_storage_config();
    printf("%s, dbserver_get_storage_config, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_get_storage_media_folder();
    printf("%s, dbserver_get_storage_media_folder, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_get_storage_disk_path(NULL);
    printf("%s, dbserver_get_storage_disk_path, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_sql("create table test(id INTEGER PRIMARY KEY,sName TEXT);", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_sql, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_sql("insert into test (id,sName) values (0, 'name0');", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_sql, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_sql("insert into test (id,sName) values (1, 'name1');", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_sql, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_sql("select * from test", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_sql, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_sql("drop table test", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_sql, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_create_table("test", "id INTEGER PRIMARY KEY,sName TEXT", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_create_table, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);

    json_str = dbserver_drop_table("test", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_drop_table, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
        
    json_str = dbserver_sql("select name from sqlite_master where type=\"table\"", DBSERVER_STORAGE_INTERFACE);
    printf("%s, dbserver_sql, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}



int get_user_number(void)
{
    return dbserver_system_user_num_get(NULL, NULL, NULL, NULL);
}

int get_users(void)
{
    int num = -1;
    struct account *account = NULL;
    char *json_str = dbserver_system_user_get(NULL, NULL, NULL, NULL);

    if (json_str) {
        json_object *j_array;
        json_object *j_ret;

        j_ret = json_tokener_parse(json_str);
        j_array = json_object_object_get(j_ret, "jData");
        num = json_object_array_length(j_array);

        if (num > 0) {
            char *user, *pass;
            int authlevel, fixed, userlevel;
            for (int i = 0; i < num; i++) {
                json_object *j_data = json_object_array_get_idx(j_array, i);
                user = json_object_get_string(json_object_object_get(j_data, "sUserName"));
                pass = json_object_get_string(json_object_object_get(j_data, "sPassword"));
                authlevel = json_object_get_int(json_object_object_get(j_data, "iAuthLevel"));
                fixed = json_object_get_int(json_object_object_get(j_data, "iFixed"));
                userlevel = json_object_get_int(json_object_object_get(j_data, "iUserLevel"));
                printf("%d:user:%s, pass:%s, authlevel:%d, userlevel = %d, fixed:%d\n", i, user, pass, authlevel, userlevel, fixed);
            }
        }

        json_object_put(j_ret);
        g_free(json_str);
    }

    return num;
}

int remove_user(char *username)
{
    int ret = -1;
    char *json_str = dbserver_system_user_del_username(username);

    if (json_str) {
        json_object *j_ret = json_tokener_parse(json_str);
        ret = json_object_get_int(json_object_object_get(j_ret, "iReturn"));

        json_object_put(j_ret);
        g_free(json_str);
    }

    return ret;
}

int add_user(char *username, char *password, int *authlevel, int *userlevel)
{
    int ret = -1;
    char *json_str;

    json_str = dbserver_system_user_add(username, password, &authlevel, &userlevel, NULL);

    if (json_str) {
        json_object *j_ret = json_tokener_parse(json_str);
        ret = json_object_get_int(json_object_object_get(j_ret, "iReturn"));

        json_object_put(j_ret);
        g_free(json_str);
    }

    return ret;
}

int set_user(char *username, char *password, int *authlevel, int *userlevel)
{
    int ret = -1;
    char *json_str = dbserver_system_user_set(username, password, authlevel, userlevel);

    if (json_str) {
        json_object *j_ret = json_tokener_parse(json_str);
        ret = json_object_get_int(json_object_object_get(j_ret, "iReturn"));

        json_object_put(j_ret);
        g_free(json_str);
    }

    return ret;
}

void network_power_get()
{
    char *json_str = NULL;
    char type[32] = {0};
    printf("network type(wifi/ethernet):");
    scanf("%s", &type);

    if (!strcmp(type, "wifi"))
        json_str = dbserver_wifi_power_get();
    else if (!strcmp(type, "ethernet"))
        json_str = dbserver_ethernet_power_get();

    if (json_str) {
        json_object *j_cfg = json_tokener_parse(json_str);
        if (j_cfg) {
            int ret = json_object_get_int(json_object_object_get(j_cfg, "iReturn"));
            if (ret == 0) {
                json_object *j_array = json_object_object_get(j_cfg, "jData");
                int len = json_object_array_length(j_array);

                for (int i = 0; i < len; i++) {
                    json_object *j_data = json_object_array_get_idx(j_array, i);
                    char *type = (char *)json_object_get_string(json_object_object_get(j_data, "sType"));
                    if (type) {
                         int power = json_object_get_int(json_object_object_get(j_data, "iPower"));
                         printf("\n************************\n");
                         if (power)
                             printf("%s: is on\n", type);
                         else
                             printf("%s: is off\n", type);
                         printf("************************\n");
                    }
                }
            } else {
                printf("fail\n");
            }
            json_object_put(j_cfg);
        }
        g_free(json_str);
    }
}

void network_power_set()
{
    char *json_str = NULL;
    char type[32] = {0};
    char status[32] = {0};
    printf("network type(wifi/ethernet):");
    scanf("%s", &type);

    if (strcmp(type, "wifi") && strcmp(type, "ethernet")) {
        printf("type: %s is err\n", type);
        return;
    }

    printf("status(on/off):");
    scanf("%s", &status);

    if (strcmp(status, "on") && strcmp(status, "off")) {
        printf("status: %s is err\n", status);
        return;
    }

    if (!strcmp(status, "on"))
        json_str = dbserver_network_power_set(type, 1);
    else if (!strcmp(status, "off"))
        json_str = dbserver_network_power_set(type, 0);

    if (json_str) {
        json_object *j_cfg = json_tokener_parse(json_str);
        if (j_cfg) {
            int ret = json_object_get_int(json_object_object_get(j_cfg, "iReturn"));
            if (ret == 0) {
                printf("success\n");
            } else {
                printf("fail\n");
            }
            json_object_put(j_cfg);
        }
        g_free(json_str);
    }
}

void connect_wifi(void)
{
    char *json_str = NULL;
    char service[128] = {0};
    char password[128] = {0};
    printf("\nservice:");
    scanf("%s", &service);
    printf("\npassword:");
    scanf("%s", &password);

    int faorite = 1;
    int autoconnect = 1;
    json_str = dbserver_network_service_connect_set(service, password, &faorite, &autoconnect);
    printf("%s, dbserver_network_service_connect_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

void remove_wifi(void)
{
    char *json_str = NULL;
    char service[128] = {0};
    printf("service:");
    scanf("%s", &service);

    json_str = dbserver_network_service_delete(service);
    printf("%s, dbserver_network_service_connect_set, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

void get_wifi_list(void)
{
    char *json_str = NULL;

    json_str = dbserver_network_service_get(NULL);
    printf("************************\n");
    if (json_str) {
        json_object *j_cfg = json_tokener_parse(json_str);
        if (j_cfg) {
            int ret = json_object_get_int(json_object_object_get(j_cfg, "iReturn"));
            if (ret == 0) {
                json_object *j_array = json_object_object_get(j_cfg, "jData");

                if (j_array) {
                    int num = json_object_array_length(j_array);
                    for (int i = 0; i < num; i++) {
                        json_object *j_cfg = json_object_array_get_idx(j_array, i);

                        printf("service: %s\n", (char *)json_object_get_string(json_object_object_get(j_cfg, "sService")));
                        printf("password: %s\n", (char *)json_object_get_string(json_object_object_get(j_cfg, "sPassword")));
                        printf("favorite: %d\n", json_object_get_int(json_object_object_get(j_cfg, "iFavorite")));
                        printf("\n");
                    }
                }
            } else {
                printf("fail\n");
            }
            json_object_put(j_cfg);
        }
        g_free(json_str);
    }
    printf("************************\n");
}

void network_power_get_all(void)
{
    char *json_str = NULL;

    json_str = dbserver_network_power_get(NULL);
    printf("************************\n");
    if (json_str) {
        json_object *j_cfg = json_tokener_parse(json_str);
        if (j_cfg) {
            int ret = json_object_get_int(json_object_object_get(j_cfg, "iReturn"));
            if (ret == 0) {
                json_object *j_array = json_object_object_get(j_cfg, "jData");

                if (j_array) {
                    int num = json_object_array_length(j_array);
                    for (int i = 0; i < num; i++) {
                        json_object *j_cfg = json_object_array_get_idx(j_array, i);

                        printf("%s:", (char *)json_object_get_string(json_object_object_get(j_cfg, "sType")));
                        if (json_object_get_int(json_object_object_get(j_cfg, "iPower")) == 0)
                            printf("off\n");
                        else
                            printf("on\n");
                    }
                }
            } else {
                printf("fail\n");
            }
            json_object_put(j_cfg);
        }
        g_free(json_str);
    }
    printf("************************\n");
}

void schedules_parse(void)
{
    struct week *schedule = (struct week *)malloc(sizeof(struct week));
    dbserver_event_schedules_parse(schedule, 2);
    for (int i = 0 ; i < 7 ; i++) {
        printf("day index is %d\n", i);
        for (int j = 0 ; j < 8 ; j++) {
            double t1 = schedule->week_day[i].day_period[j].start_minute;
            double t2 = schedule->week_day[i].day_period[j].end_minute;
            char *type = schedule->week_day[i].day_period[j].type;
            printf("start_time is %f, end_time is %f, type is %s\n",t1,t2,type);
        }
    }
    free(schedule);
    schedule = NULL;
}

void snap_plan_parse(void)
{
    int type_id = 0;
    int ebaled = 0;
    int quality = 0;
    int interval = 0;
    int num = 0;
    dbserver_storage_snap_plan_parse(type_id, &ebaled, &quality, &interval, &num);
    printf("ebaled is %d, quality is %d, interval is %d, num is %d\n",ebaled,quality,interval,num);
}

void media_profile_get(void) {
    char *json_str = NULL;
    json_str = dbserver_media_get(TABLE_VIDEO);
    printf("************************\n");
    printf("video sources is %s\n", json_str);
    g_free(json_str);

    json_str = dbserver_video_source_get("mainVSToken");
    printf("************************\n");
    printf("video source(mainVSToken) is %s\n", json_str);
    g_free(json_str);

    json_str = dbserver_media_get(TABLE_PROFILE);
    printf("************************\n");
    printf("profiles is %s\n", json_str);
    g_free(json_str);

    json_str = dbserver_media_profile_get("mainProfileToken");
    printf("************************\n");
    printf("profile(mainProfileToken) is %s\n", json_str);
    g_free(json_str);

    json_str = dbserver_video_source_cfg_get("mainVSCToken");
    printf("************************\n");
    printf("video source cfg(mainVSCToken) is %s\n", json_str);
    g_free(json_str);

    json_str = dbserver_video_enc_cfg_get("mainVECToken");
    printf("************************\n");
    printf("video enc cfg(mainVECToken) is %s\n", json_str);
    g_free(json_str);
    printf("************************\n");
}

void get_disk_path(void)
{
    char *json_str = dbserver_get_storage_disk_path(NULL);

    if (json_str) {
        json_object *j_ret = json_tokener_parse(json_str);
        json_object *j_array = json_object_object_get(j_ret, "jData");
        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);

            char *path = (char *)json_object_get_string(json_object_object_get(j_obj, "sPath"));
            int id = (int)json_object_get_int(json_object_object_get(j_obj, "id"));
            char *name = (char *)json_object_get_string(json_object_object_get(j_obj, "sName"));
            int mount = (int)json_object_get_int(json_object_object_get(j_obj, "iMount"));
            printf("{\n");
            printf("  id = %d\n", id);
            printf("  sPath = %s\n", path);
            printf("  sName = %s\n", name);
            if (i < len - 1)
                printf("},\n");
            else
                printf("}\n");
        }
        json_object_put(j_ret);

        g_free(json_str);
        return 0;
    }

    return -1;
}

void get_storage_config(void)
{
    char *json_str = dbserver_get_storage_config();

    if (json_str) {
        json_object *j_ret = json_tokener_parse(json_str);
        json_object *j_array = json_object_object_get(j_ret, "jData");
        int len = json_object_array_length(j_array);

        for (int i = 0; i < len; i++) {
            json_object *j_obj = json_object_array_get_idx(j_array, i);

            char *path = (char *)json_object_get_string(json_object_object_get(j_obj, "sMountPath"));
            int id = (int)json_object_get_int(json_object_object_get(j_obj, "id"));
            int freesize = (int)json_object_get_int(json_object_object_get(j_obj, "iFreeSize"));
            int freesizenotice = (int)json_object_get_int(json_object_object_get(j_obj, "iFreeSizeNotice"));
            printf("{\n");
            printf("  id = %d\n", id);
            printf("  sMountPath = %s\n", path);
            printf("  iFreeSize = %dMB\n", freesize);
            printf("  iFreeSizeNotice = %dMB\n", freesizenotice);
            if (i < len - 1)
                printf("},\n");
            else
                printf("}\n");
        }
        json_object_put(j_ret);

        g_free(json_str);
        return 0;
    }

    return -1;
}

void set_storage_config_mountpath(void)
{
    char *json_str = NULL;
    char mountpath[128] = {0};
    printf("\nmountpath:");
    scanf("%s", &mountpath);

    json_str = dbserver_update_storage_config_mountpath(mountpath);
    printf("%s, dbserver_update_storage_config_mountpath, %s\n", __func__, json_str);
    if (json_str)
        g_free(json_str);
}

void get_wait_complete(void)
{
    char *json_str = NULL;
    json_str = dbserver_sql( "select count(*) from FaceList where (iLoadCompleted == 0)", DBSERVER_EVENT_INTERFACE);
    printf("%s", json_str);
    if (json_str)
        g_free(json_str);
    else
        printf("no string++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    fflush(stdout);
    sleep(1);
}

void modify_system_para()
{
    printf("a.dbserver_set_static_cap_option\n");
    printf("b.dbserver_set_static_cap_range\n");
    printf("c.dbserver_set_dynamic_cap_option\n");
    printf("d.dbserver_set_dynamic_cap_range\n");
    char cmd = 0;
    printf("please enter:");
    struct DynamicLocation dy_lo = {
        .cap_name = "StoragePlanSnap",
        .dynamic_key = "id",
        .dynamic_val = "0",
        .target_key = "timeUnit"
    };
    struct StaticLocation st_lo = {
        .cap_name = "StoragePlanSnap",
        .target_key = "iImageQuality"
    };
    struct RangeJsonPara range = {
        .min = 1,
        .max = 100,
        .step = 1
    };
    char *option = "[\"test1\",\"test2\"]";
    char *old = NULL;
    char *new = NULL;
    switch(cmd) {
        case 'a':
            old = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("old param is %s\n", old);
            dbserver_set_static_cap_option(st_lo, option);
            new = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("new param is %s\n", new);
            break;
        case 'b':
            old = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("old param is %s\n", old);
            dbserver_set_static_cap_range(st_lo, range);
            new = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("new param is %s\n", new);
            break;
    /*
    static raw data:
    "static": {
        "iImageQuality": {
            "options": [
                1,
                5,
                10
            ],
            "type": "options"
        }
    }
    dbserver_set_static_cap_option set:
    "static": {
        "iImageQuality": {
            "options": [
                "test1",
                "test2"
            ],
            "type": "options"
        }
    }
    dbserver_set_static_cap_range
    "static": {
        "iImageQuality": {
            "range": {
                "min": 1,
                "max": 100,
                "step": 1
            },
            "type": "range"
        }
    }
    */
        case 'c':
            old = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("old param is %s\n", old);
            dbserver_set_dynamic_cap_option(dy_lo, option);
            new = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("new param is %s\n", new);
            break;
        case 'd':
            old = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("old param is %s\n", old);
            dbserver_set_dynamic_cap_range(dy_lo, range);
            new = dbserver_system_para_get_by_name("StoragePlanSnap");
            printf("new param is %s\n", new);
            break;
    /*
    dynamic raw data:
    "dynamic": {
        "id": {
            "0": {
                "timeUnit": {
                    "options": [
                        "seconds",
                        "minutes",
                        "hours",
                        "days"
                    ],
                    "type": "options"
                }
            }
        }
    }
    dbserver_set_dynamic_cap_option set:
    "dynamic": {
        "id": {
            "0": {
                "timeUnit": {
                    "options": [
                        "test1",
                        "test2"
                    ],
                    "type": "options"
                }
            }
        }
    }
    dbserver_set_dynamic_cap_range set:
    "dynamic": {
        "id": {
            "0": {
                "timeUnit": {
                    "range": {
                        "min": 1,
                        "max": 100,
                        "step": 1
                    },
                    "type": "range"
                }
            }
        }
    }
    */
    }
}

void help_printf(void)
{
    printf("************************\n");
    printf("0.help\n");
    printf("1.get all network power\n");
    printf("2.set network power\n");
    printf("3.get wifi list\n");
    printf("4.connect wifi\n");
    printf("5.remove wifi\n");
    printf("6.get all users\n");
    printf("7.schedules parse\n");
    printf("8.snap plan parse\n");
    printf("9.get media profile\n");
    printf("a.get all storage disk path\n");
    printf("b.get storage config\n");
    printf("c.set storage config mountpath\n");
    printf("d.wait complete signal\n");
    printf("e.modify system para\n");
    printf("************************\n");
}

int main( int argc , char ** argv)
{
    help_printf();
    while (1) {
        char cmd = 0;
        printf("please enter:");
again:
        scanf("%c", &cmd);
        switch(cmd) {
            case '0':
                help_printf();
                break;
            case '1':
                network_power_get_all();
                break;
            case '2':
                network_power_set();
                break;
            case '3':
                get_wifi_list();
                break;
            case '4':
                connect_wifi();
                break;
            case '5':
                remove_wifi();
                break;
            case '6':
                get_users();
                break;
            case '7':
                schedules_parse();
                break;
            case '8':
                snap_plan_parse();
                break;
            case '9':
                media_profile_get();
                break;
            case 'a':
                get_disk_path();
                break;
            case 'b':
                get_storage_config();
                break;
            case 'c':
                set_storage_config_mountpath();
                break;
            case 'd':
                get_wait_complete();
                break;
            case 'e':
                modify_system_para();
                break;
            case 0xa:
                continue;
                break;
        }
        goto again;
    }

    return 0;
}
