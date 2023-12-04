#pragma once
typedef enum{
    RK_UPGRADE_START,
    RK_UPGRADE_FINISHED,
    RK_UPGRADE_ERR,
}RK_Upgrade_Status_t;

typedef void(*RK_upgrade_callback)(void *user_data, RK_Upgrade_Status_t status);

void RK_ota_set_url(char *url);
void RK_ota_start(RK_upgrade_callback cb);
void RK_ota_stop();
int RK_ota_get_progress();
void RK_ota_get_sw_version(char *buffer, int maxLength);
bool RK_ota_check_version(const char *url);
