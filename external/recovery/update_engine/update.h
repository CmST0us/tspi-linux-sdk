/*************************************************************************
    > File Name: update.h
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Mon 20 May 2019 09:59:27 AM CST
 ************************************************************************/

#ifndef _UPDATE_H
#define _UPDATE_H

typedef int (*update_func)(char *src_path, void* pupdate_cmd);
typedef struct {
    char name[32];
    bool need_update;
    bool is_ab;
    long long size;
    long long offset;
    long long flash_offset;
    char dest_path[100];
    update_func cmd;
}UPDATE_CMD, *PUPDATE_CMD;

typedef enum{
    RK_UPGRADE_FINISHED,
    RK_UPGRADE_START,
    RK_UPGRADE_ERR,
}RK_Upgrade_Status_t;

typedef void(*RK_upgrade_callback)(void *user_data, RK_Upgrade_Status_t status);
typedef void (*RK_print_callback)(char *pszPrompt);

void RK_ota_set_url(char *url, char *savepath);
bool RK_ota_set_partition(int partition);
void RK_ota_start(RK_upgrade_callback cb, RK_print_callback print_cb);
//void RK_ota_stop();
int RK_ota_get_progress();
void RK_ota_get_sw_version(char *buffer, int maxLength);
bool RK_ota_check_version(char *url);

#endif
