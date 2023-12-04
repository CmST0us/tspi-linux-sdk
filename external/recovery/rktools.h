#ifndef _RKTOOLS_H
#define _RKTOOLS_H
#include "common.h"

#define PATH_LEN 50
#define usb_path "/mnt/udisk/"
#define sd_path "/mnt/sdcard/"

#define OFF_VALUE 0
#define ON_VALUE 1

#define EMMC_POINT_NAME "emmc_point_name"
#define SD_POINT_NAME "sd_point_name"
#define SD_POINT_NAME_2 "sd_point_name_2"

static const char *point_items[] = {
    "/dev/mmcblk0",
    "/dev/mmcblk1",
    "/dev/mmcblk2",
    "/dev/mmcblk3",
};

enum type{
    MMC,
    SD,
    SDIO,
    SDcombo,
};

static const char *typeName[] = {
    "MMC",
    "SD",
    "SDIO",
    "SDcombo",
};

char* getSerial();
void setFlashPoint();
extern Volume* volume_for_path(const char* path);
bool isMtdDevice();

#endif
