/*************************************************************************
    > File Name: flash_image.h
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Tue 21 May 2019 09:29:39 AM CST
 ************************************************************************/

#ifndef _FLASH_IMAGE_H
#define _FLASH_IMAGE_H
#include "defineHeader.h"

#pragma pack(1)
typedef struct
{
    char name[20];
    unsigned int offset;
    unsigned int size;
}STRUCT_PARAM_ITEM,*PSTRUCT_PARAM_ITEM;

typedef struct
{
    char name[20];
    char value[256];
}STRUCT_CONFIG_ITEM,*PSTRUCT_CONFIG_ITEM;
#pragma pack()

int flash_bootloader(char *dest_path, void *pupdate_cmd);
int flash_parameter(char *dest_path, void *pupdate_cmd);
int flash_normal(char *dest_path, void *pupdate_cmd);
int flash_register_partition_data(PSTRUCT_PARAM_ITEM p_param_item, long long *p_gpt_backup_offset);
#endif
