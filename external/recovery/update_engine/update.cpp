/*************************************************************************
    > File Name: update.cpp
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Mon 20 May 2019 09:59:19 AM CST
 ************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "update.h"
#include "log.h"
#include "download.h"
#include "rkimage.h"
#include "flash_image.h"
#include "rktools.h"
#include "md5sum.h"
#include "defineHeader.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>

extern "C" {
    #include "../mtdutils/mtdutils.h"
}

#define	CMD4RECOVERY_FILENAME "/mnt/sdcard/cmd4recovery"
static char * _url = NULL;
static char const * _save_path = NULL;
static char _url_dir[128];
double processvalue = 0;

void RK_ota_set_url(char *url, char *savepath) {
    LOGI("start RK_ota_url url [%s] save path [%s].\n", url, savepath);
    if ( url == NULL ) {
        LOGE("RK_ota_set_url : url is NULL.\n");
        return ;
    }
    if (savepath == NULL) {
        _save_path = DEFAULT_DOWNLOAD_PATH;
    } else {
        _save_path = savepath;
    }
    LOGI("save image to %s.\n", _save_path);
    _url = url;

    sprintf(_url_dir, "%s", _url);
    dirname(_url_dir);
}

bool is_sdboot = false;

UPDATE_CMD update_cmd[] = {
           {"bootloader" , false , false , 0 , 0 , 0 , "" , flash_bootloader} ,
           {"parameter"  , false , false , 0 , 0 , 0 , "" , flash_parameter}  ,
           {"uboot"      , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"trust"      , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"boot"       , false , true  , 0 , 0 , 0 , "" , flash_normal}     ,
           {"recovery"   , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"rootfs"     , false , true  , 0 , 0 , 0 , "" , flash_normal}     ,
           {"oem"        , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"uboot_a"    , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"uboot_b"    , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"boot_a"     , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"boot_b"     , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"system_a"   , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"system_b"   , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"misc"       , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
           {"userdata"   , false , false , 0 , 0 , 0 , "" , flash_normal}     ,
};

bool RK_ota_set_partition(int partition) {
    //000000000000000000000000: 没有升级分区
    //100000000000000000000000: 升级loader分区
    //010000000000000000000000: 升级parameter分区
    //001000000000000000000000: 升级uboot分区
    //000100000000000000000000: 升级trust分区
    //000010000000000000000000: 升级boot分区
    //000001000000000000000000: 升级recovery分区
    //000000100000000000000000: 升级rootfs分区
    //000000010000000000000000: 升级oem分区
    //000000001000000000000000: 升级uboot_a分区
    //000000000100000000000000: 升级uboot_b分区
    //000000000010000000000000: 升级boot_a分区
    //000000000001000000000000: 升级boot_b分区
    //000000000000100000000000: 升级system_a分区
    //000000000000010000000000: 升级system_b分区
    //000000000000001000000000: 升级misc分区，sdboot使用
    //000000000000000100000000: 升级userdata分区

    int num = sizeof(update_cmd)/sizeof(UPDATE_CMD);
    LOGI("[%s:%d] num [%d]\n", __func__, __LINE__, num);

    if (partition == -1) {
        //设置目标分区大小
        RKIMAGE_HDR rkimage_hdr;
        if( analyticImage(_url, &rkimage_hdr) != 0){
            LOGE("analyticImage error.\n");
            return false;
        }

        for (int i = 0; i < num; i++) {
            if ( update_cmd[i].need_update || is_sdboot) {
                update_cmd[i].need_update = false;
                for (int j = 0; j < rkimage_hdr.item_count; j++) {
                    if (strcmp(rkimage_hdr.item[j].name, update_cmd[i].name) == 0) {
                        LOGI("found rkimage_hdr.item[%d].name = %s.\n", j, update_cmd[i].name);
                        if (rkimage_hdr.item[j].file[50]=='H') {
                            update_cmd[i].offset = *((DWORD *)(&rkimage_hdr.item[j].file[51]));
                            update_cmd[i].offset <<= 32;
                            update_cmd[i].offset += rkimage_hdr.item[j].offset;
                            LOGI("offset more than 4G, after adjusting is %lld.\n", update_cmd[i].offset);
                        } else {
                            update_cmd[i].offset = rkimage_hdr.item[j].offset;
                        }

                        if (rkimage_hdr.item[j].file[55]=='H') {
                            update_cmd[i].size = *((DWORD *)(&rkimage_hdr.item[j].file[56]));
                            update_cmd[i].size <<= 32;
                            update_cmd[i].size += rkimage_hdr.item[j].size;
                            LOGI("size more than 4G, after adjusting is %lld.\n", update_cmd[i].size);
                        } else {
                            update_cmd[i].size = rkimage_hdr.item[j].size;
                        }

                        if (is_sdboot) {
                            update_cmd[i].flash_offset = (long long)rkimage_hdr.item[j].flash_offset * SECTOR_SIZE;
                        }
                        update_cmd[i].need_update = true;
                        continue ;
                    }
                }
            }
        }

        if (!is_sdboot) {
            for ( int i=0; i<num; i++ ) {
                if (*update_cmd[i].dest_path && (update_cmd[i].need_update == false)) {

                    unsigned char len = strlen(update_cmd[i].name);
                    if (update_cmd[i].name[len-2] == '_' && (update_cmd[i].name[len-1] == 'a' || update_cmd[i].name[len-1] == 'b'))
                    {

                        char slot_find = (update_cmd[i].name[len-1] == 'a')? 'b':'a';

                        update_cmd[i].name[len-1] = slot_find;
                        for (int j = 0; j < rkimage_hdr.item_count; j++) {
                            if (strcmp(rkimage_hdr.item[j].name, update_cmd[i].name) == 0) {
                                LOGI("again found rkimage_hdr.item[%d].name = %s.\n", j, update_cmd[i].name);
                                if (rkimage_hdr.item[j].file[50]=='H') {
                                    update_cmd[i].offset = *((DWORD *)(&rkimage_hdr.item[j].file[51]));
                                    update_cmd[i].offset <<= 32;
                                    update_cmd[i].offset += rkimage_hdr.item[j].offset;
                                    LOGI("offset more than 4G, after adjusting is %lld.\n", update_cmd[i].offset);
                                } else {
                                    update_cmd[i].offset = rkimage_hdr.item[j].offset;
                                }

                                if (rkimage_hdr.item[j].file[55]=='H') {
                                    update_cmd[i].size = *((DWORD *)(&rkimage_hdr.item[j].file[56]));
                                    update_cmd[i].size <<= 32;
                                    update_cmd[i].size += rkimage_hdr.item[j].size;
                                    LOGI("size more than 4G, after adjusting is %lld.\n", update_cmd[i].size);
                                } else {
                                    update_cmd[i].size = rkimage_hdr.item[j].size;
                                }

                                update_cmd[i].need_update = true;
                                continue ;
                            }
                        }
                    }
                }
            }
        }
    // for ( int i=0; i<num; i++ ) {
    // printf ( "[%s:%d] update_cmd[%d].name [%s] dest path [%s] flash offset [%#llx] offset [%#llx] size [%#llx] \n",
    // __func__, __LINE__, i, update_cmd[i].name, update_cmd[i].dest_path, update_cmd[i].flash_offset, update_cmd[i].offset, update_cmd[i].size);
    // }

        return true;
    }

    for (int i = 0; i < num; i++) {
        // For OTA and SD update MUST read gpt from update***.img
        if ( (partition & 0x800000 || is_sdboot || (strcmp(update_cmd[i].name, "parameter") == 0) ) ) {
            LOGI("need update %s.\n", update_cmd[i].name);
            update_cmd[i].need_update = true;

            if (is_sdboot) {
                memset(update_cmd[i].dest_path, 0, sizeof(update_cmd[i].dest_path)/sizeof(update_cmd[i].dest_path[0]));
                if (strcmp(update_cmd[i].name, "parameter") == 0) {
                    sprintf(update_cmd[i].dest_path, "%s/gpt", _url_dir);
                } else {
                    sprintf(update_cmd[i].dest_path, "%s/%s", _url_dir, update_cmd[i].name);
                }
            } else {
                if (strcmp(update_cmd[i].name, "parameter") == 0) {
                    sprintf(update_cmd[i].dest_path, "/dev/block/by-name/gpt");
                } else {
                    if (!isMtdDevice()) {
                       sprintf(update_cmd[i].dest_path, "/dev/block/by-name/%s", update_cmd[i].name);
                    } else {
                       if ( update_cmd[i].need_update && (mtd_scan_partitions() > 0) ) {
                          const MtdPartition *mtdp = mtd_find_partition_by_name(update_cmd[i].name);
                          if (mtdp) {
                             sprintf(update_cmd[i].dest_path, "/dev/mtd%d", mtdp->device_index);
                             LOGI("need update %s ,.dest_path: %s.\n", update_cmd[i].name,update_cmd[i].dest_path);
                          }
                       } else {
                          sprintf(update_cmd[i].dest_path, "/dev/block/by-name/%s", update_cmd[i].name);
                       }
                    }
                }
            }
        }
        partition = (partition << 1);
    }

    return true;

}

static int ota_recovery_cmds (long long flash_offset, const char *dest_path)
{
    char data_buf[256];
    unsigned int write_count = 0;

    if (dest_path == NULL) {
        LOGE("[%s-%d] error dest path is NULL.\n", __func__, __LINE__);
        return -1;
    }

    LOGI("[%s:%d] parameter flash offset %#llx dest path %s\n", __func__, __LINE__, flash_offset, dest_path);
    memset(data_buf, 0, sizeof(data_buf)/sizeof(data_buf[0]));
    int fd = open(CMD4RECOVERY_FILENAME, O_CREAT|O_RDWR|O_SYNC|O_APPEND, 0644);
    if (fd < 0) {
        LOGE("[%s-%d] error opening %s.\n", __func__, __LINE__,  CMD4RECOVERY_FILENAME);
        return -1;
    }

    if (isMtdDevice()) {
        sprintf(data_buf, "nandwrite -p /dev/mtd0 -s %#llx %s; \n", flash_offset, dest_path);
    } else {
        char flash_name[20];
        getFlashPoint(flash_name);

#define    DD_MALLOC_MAX_SIZE     (0x100000*10)        /* 10MB */

        unsigned int dd_bs = 0;
        long long dd_seek  = 1;

        for ( int j=DD_MALLOC_MAX_SIZE/SECTOR_SIZE; j>0 ; j-- ) {
            dd_bs = j*SECTOR_SIZE;
            if ( !(flash_offset % dd_bs) ) {
                dd_seek  = flash_offset / dd_bs;
                printf ( "flash offset = [%#llx] j=%d bs=%#x  seek = %#llx  result = [%s]\n",
                        flash_offset , j, dd_bs, dd_seek , (dd_bs*dd_seek == flash_offset)? "YES":"NO" );
                break;
            } else {
                dd_bs = 1;
                dd_seek = flash_offset;
            }
        }
        sprintf(data_buf, "dd of=%s if=%s bs=%d seek=%lld; \t###flash offset [%#10llx] \n", flash_name, dest_path, dd_bs, dd_seek, flash_offset);
    }

    write_count = strlen(data_buf);

    if (write(fd, data_buf, write_count) != write_count) {
        close(fd);
        LOGE("[%s:%d] Write failed(%s)\n", __func__, __LINE__, strerror(errno));
        return -2;
    }

    close(fd);
    return 0;
}

void RK_ota_start(RK_upgrade_callback cb, RK_print_callback print_cb) {
    LOGI("start RK_ota_start.\n");
    processvalue = 95;
    cb(NULL, RK_UPGRADE_START);

    //确认升级路径
    if (_url == NULL) {
        LOGE("url is NULL\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }

    // 1. 获取文件
    int res = download_file(_url, _save_path);
    if (res == 0) {
        _url = (char *)_save_path;
    } else if (res == -1) {
        LOGE("download_file error.\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }

    // 2. 获取文件信息
    if (!RK_ota_set_partition(-1)) {
        LOGE("RK_ota_set_partition failed.\n");
        cb(NULL, RK_UPGRADE_ERR);
        return ;
    }

    STRUCT_PARAM_ITEM param_item[20] = {0};
    long long gpt_backup_offset = -1;
    memset(param_item, 0, sizeof(param_item));
    flash_register_partition_data(param_item, &gpt_backup_offset);

    int is_mtd_flag = isMtdDevice();

    // 3. 下载文件到分区并校验
    int num = sizeof(update_cmd)/sizeof(UPDATE_CMD);
    char prompt[128] = {0};
    for (int i = 0; i < num; i++ ) {
        if (update_cmd[i].need_update) {
            if (update_cmd[i].cmd != NULL) {
                LOGI("now write %s to %s.\n", update_cmd[i].name, update_cmd[i].dest_path);
                sprintf(prompt,"[%s] upgrade start...\n", update_cmd[i].name);
                print_cb(prompt);
                if (!is_sdboot &&
                        ( (strcmp(update_cmd[i].name, "misc") == 0) ||
                          (strcmp(update_cmd[i].name, "parameter") == 0) )) {
                    LOGI("ingore misc.\n");
                    continue;
                }
                // 下载固件到分区
                printf("update_cmd.flash_offset = %lld.\n", update_cmd[i].flash_offset);
                if (update_cmd[i].cmd(_url, (void*)(&update_cmd[i])) != 0) {
                    LOGE("update %s error.\n", update_cmd[i].dest_path);
                    sprintf(prompt,"[%s] upgrade fail\n", update_cmd[i].name);
                    print_cb(prompt);
                    cb(NULL, RK_UPGRADE_ERR);
                    return ;
                } else {
                    sprintf(prompt, "[%s] upgraede success!\n", update_cmd[i].name);
                    print_cb(prompt);
                }
                if (is_sdboot) {
                    if (ota_recovery_cmds(update_cmd[i].flash_offset, update_cmd[i].dest_path)) {
                        LOGE("write recovery cmds to %s failed.\n", CMD4RECOVERY_FILENAME);
                        cb(NULL, RK_UPGRADE_ERR);
                        return ;
                    }
                    LOGI("not check in sdboot.\n");
                    continue;
                }
                // parameter 和loader 先不校验
                if (strcmp(update_cmd[i].name, "parameter") == 0 || strcmp(update_cmd[i].name, "bootloader") == 0) {
                    LOGI("not check parameter and loader.\n");
                    continue;
                }
                // 校验分区
                if (comparefile(update_cmd[i].dest_path, _url, update_cmd[i].flash_offset, update_cmd[i].offset, update_cmd[i].size))
                {
                    LOGI("check %s ok.\n", update_cmd[i].dest_path);
                } else {
                    LOGE("check %s failed.\n", update_cmd[i].dest_path);
                    cb(NULL, RK_UPGRADE_ERR);
                    return ;
                }
            }
        }
    }

    /*
     * Fix if update_xxx.img not found some A/B partition image.
     */
    if (is_sdboot) {
        for (int i = 0; i < num; i++) {
            if ( (!update_cmd[i].need_update) || (update_cmd[i].cmd == NULL)) {
                continue;
            }
            unsigned char len = strlen(update_cmd[i].name);
            if (update_cmd[i].name[len-2] == '_' && (update_cmd[i].name[len-1] == 'a' || update_cmd[i].name[len-1] == 'b') ) {
                char slot_find = (update_cmd[i].name[len-1] == 'a')? 'b':'a';
                int part_need_fix = 1;
                char part_name[32];
                memset(part_name, 0, sizeof(part_name)/sizeof(part_name[0]));
                memcpy(part_name, update_cmd[i].name, len);
                part_name[len-1] = slot_find;

                for (int k = 0; k < num; k++) {
                    if ( (!update_cmd[k].need_update) || (update_cmd[k].cmd == NULL)) {
                        continue;
                    }
                    if ( (strcmp(update_cmd[k].name, part_name) == 0) ) {
                        part_need_fix = 0;
                    }
                }

                if (part_need_fix) {
                    for (int j = 0; j < sizeof(param_item)/sizeof(param_item[0]); j++) {
                        if (strcmp(param_item[j].name, part_name) == 0) {
                            if (ota_recovery_cmds(param_item[j].offset * SECTOR_SIZE, update_cmd[i].dest_path)) {
                                LOGE("sdboot fix write recovery cmds to %s failed.\n", CMD4RECOVERY_FILENAME);
                                cb(NULL, RK_UPGRADE_ERR);
                                return ;
                            }
                        }
                    }
                }
            }
        }

        // write gpt backup
        /*
         * char gpt_backup_img_path[100] = {0};
         * sprintf(gpt_backup_img_path, "%s/%s", _url_dir, GPT_BACKUP_FILE_NAME);
         * if (ota_recovery_cmds(gpt_backup_offset, gpt_backup_img_path)) {
         *     LOGE("write gpt backup to recovery cmds failed (%s).\n", gpt_backup_img_path);
         *     cb(NULL, RK_UPGRADE_ERR);
         *     return ;
         * }
         */

    }

    // 4. 是否设置misc

    LOGI("RK_ota_start is ok!");
    processvalue = 100;
    cb(NULL, RK_UPGRADE_FINISHED);
    print_cb((char *)"updateEngine upgrade OK!\n");
}

int RK_ota_get_progress() {
    return processvalue;
}

void RK_ota_get_sw_version(char *buffer, int  maxLength) {
    getLocalVersion(buffer, maxLength);
}

bool RK_ota_check_version(char *url) {
    char source_version[20] = {0};
    char target_version[20] = {0};
    if (!getLocalVersion(source_version, sizeof(source_version))) {
        return false;
    }

    if (strncmp(url, "http", 4) == 0) {
        //如果是远程文件，从远程获取版本号
        if (!getRemoteVersion(url, target_version, sizeof(target_version))) {
            return false;
        }
    } else {
        //如果是本地文件，从固件获取版本号
        if (!getImageVersion(url, target_version, sizeof(target_version))) {
            return false;
        }
    }

    LOGI("check version new:%s  old:%s", target_version, source_version);
    if (strcmp(target_version, source_version) > 0) {
        return true;
    }
    return false;
}
