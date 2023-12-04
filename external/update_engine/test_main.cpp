#include <iostream>
#include <stdio.h>
#include "update.h"
#include <unistd.h>
#include <string.h>
#include <sys/reboot.h>
#include "download.h"

bool update_ok = false;
void handle_upgrade_callback(void *user_data, RK_Upgrade_Status_t status){
    if(status == RK_UPGRADE_FINISHED)
        update_ok = true;
}

void handle_upgrade_callback_reboot(void *user_data, RK_Upgrade_Status_t status){
    if(status == RK_UPGRADE_FINISHED){
        update_ok = true;
        reboot(RB_AUTOBOOT);
    }
}

int main(int argc, char *argv[]){
    //char url[256] = "http://148.70.52.169:8080/update.img";
    printf("version 1.0.0\n");
    if(argc >= 2){
        if(strcmp(argv[1], "check") == 0){
            if(RK_ota_check_version(argv[2])){
                printf("version check ok, need update..\n");
                return 0;
            }
            else{
                return -1;
            }
        }else if(strcmp(argv[1], "update") == 0){
            if(argc == 3){
                RK_ota_set_url(argv[2]);
                RK_ota_start(handle_upgrade_callback);
                RK_ota_get_progress();
                if(update_ok)
                    return 0;
                else
                    return -1;
            }else if(argc == 4){
                RK_ota_set_url(argv[2]);
                RK_ota_start(handle_upgrade_callback_reboot);
                RK_ota_get_progress();
            }
        }
    }else{
        printf("============useage===============\n");
        printf("1. update_engine check url\n");
        printf("2. update_engine update url\n");
        printf("3. update_engine update url reboot\n");
    }
    return 0;
}
