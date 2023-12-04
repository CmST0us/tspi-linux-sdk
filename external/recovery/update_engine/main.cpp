/*************************************************************************
    > File Name: main.cpp
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Mon 20 May 2019 10:56:06 AM CST
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/reboot.h>
#include "log.h"
#include "update.h"
#include "../bootloader.h"
#include "defineHeader.h"
#include "rktools.h"

extern bool is_sdboot;
RK_Upgrade_Status_t m_status = RK_UPGRADE_ERR;
FILE* cmd_pipe = NULL;

void handle_upgrade_callback(void *user_data, RK_Upgrade_Status_t status){
    if (status == RK_UPGRADE_FINISHED) {
        LOGI("rk ota success.\n");
        setSlotActivity();
    }
    m_status = status;
    LOGI("rk m_status = %d.\n", m_status);
}

void handle_print_callback(char *szPrompt)
{
    if(cmd_pipe != NULL){
        fprintf(cmd_pipe, "ui_print %s\n", szPrompt);
    }
}

static int MiscUpdate(char *url,  char *update_partition, char *save_path) {
    int partition;
    char *savepath = NULL;
    int slot = -1;
    if (url == NULL) {
        //如果没有传入URL，则可以去查找是否有存在
        LOGE("MiscUpdate URL must be set.\n");
        return -1;
    }
    slot = getCurrentSlot();
    if (update_partition == NULL) {
        //没有传入要升级的分区，默认升级
        if (slot == -1){
            // recovery mdoe
            // u-boot/trust/boot/recovery/boot/rootfs/oem
            partition = 0X3F0000;
        } else {
            // A/B mdoe
            // uboot_a/uboot_b/boot_a/boot_b/system_a/system_b
            partition = 0XFC00;
        }
    } else {
        partition = strtol(update_partition+2, NULL, 16);
    }

    if (save_path == NULL) {
        savepath = url;
    } else {
        savepath = save_path;
    }

    RK_ota_set_url(url, savepath);
    LOGI("url = %s.\n", url);
    LOGI("[%s:%d] save path: %s\n", __func__, __LINE__, savepath);
    // If it's recovery mode, upgrade recovery in normal system.
    if (slot == -1 && !is_sdboot){
        if (partition & 0x040000) {
            LOGI("update recovery in normal system.\n");
            partition = partition & 0xFBFFFF;

            // upgrade recoery in normal system
            if (!RK_ota_set_partition(0x040000)) {
                LOGE("ota file is error.\n");
                return -1;
            }
            RK_ota_start(handle_upgrade_callback, handle_print_callback);
            if (m_status != RK_UPGRADE_FINISHED) {
                return -1;
            }

            //写MISC
            struct bootloader_message msg;
            memset(&msg, 0, sizeof(msg));
            char recovery_str[] = "recovery\n--update_package=";
            strcpy(msg.command, "boot-recovery");
            sprintf(msg.recovery, "%s%s", recovery_str, savepath);
            msg.recovery[strlen(msg.recovery) + 1] = '\n';
            memcpy(msg.needupdate, &partition, 4);
            set_bootloader_message(&msg);
            return 0;
        }
    } else if (slot == 0) {
        LOGI("In A system, now upgrade B system.\n");
        partition = partition & 0x155ff;
    } else if (slot == 1) {
        LOGI("In B system, now upgrade A system.\n");
        partition = partition & 0x1a9ff;
    }

    if (!RK_ota_set_partition(partition)) {
        LOGE("ota file is error.\n");
        return -1;
    }
    RK_ota_start(handle_upgrade_callback, handle_print_callback);
    if (m_status != RK_UPGRADE_FINISHED) {
        return -1;
    }

    return 0;
}

void display() {
    LOGI("--misc=now             Linux A/B mode: Setting the current partition to bootable.\n");
    LOGI("--misc=other           Linux A/B mode: Setting another partition to bootable.\n");
    LOGI("--misc=update          Recovery mode: Setting the partition to be upgraded.\n");
    LOGI("--misc=display         Display misc info.\n");
    LOGI("--misc=wipe_userdata   Format data partition.\n");
    LOGI("--misc_custom= < op >  Operation on misc for custom cmdline");
    LOGI("        op:     read   Read custom cmdline to /tmp/custom_cmdline");
    LOGI("                write  Write /tmp/custom_cmdline to custom area");
    LOGI("                clean  clean custom area");
    LOGI("--update               Upgrade mode.\n");
    LOGI("--partition=0x3FFC00   Set the partition to be upgraded.(NOTICE: OTA not support upgrade loader and parameter)\n");
    LOGI("                       0x3FFC00: 0011 1111 1111 1100 0000 0000.\n");
    LOGI("                                 uboot trust boot recovery rootfs oem\n");
    LOGI("                                 uboot_a uboot_b boot_a boot_b system_a system_b.\n");
    LOGI("                       100000000000000000000000: Upgrade loader\n");
    LOGI("                       010000000000000000000000: Upgrade parameter\n");
    LOGI("                       001000000000000000000000: Upgrade uboot\n");
    LOGI("                       000100000000000000000000: Upgrade trust\n");
    LOGI("                       000010000000000000000000: Upgrade boot\n");
    LOGI("                       000001000000000000000000: Upgrade recovery\n");
    LOGI("                       000000100000000000000000: Upgrade rootfs\n");
    LOGI("                       000000010000000000000000: Upgrade oem\n");
    LOGI("                       000000001000000000000000: Upgrade uboot_a\n");
    LOGI("                       000000000100000000000000: Upgrade uboot_b\n");
    LOGI("                       000000000010000000000000: Upgrade boot_a\n");
    LOGI("                       000000000001000000000000: Upgrade boot_b\n");
    LOGI("                       000000000000100000000000: Upgrade system_a\n");
    LOGI("                       000000000000010000000000: Upgrade system_b\n");
    LOGI("                       000000000000001000000000: Upgrade misc\n");
    LOGI("                       000000000000000100000000: Upgrade userdata\n");
    LOGI("--reboot               Restart the machine at the end of the program.\n");
    LOGI("--version_url=url      The path to the file of version.\n");
    LOGI("--image_url=url        Path to upgrade firmware.\n");
    LOGI("--savepath=url         save the update.img to url.\n");

}

static const struct option engine_options[] = {
  { "update", optional_argument, NULL, 'u' },
  { "version_url", required_argument, NULL, 'v' + 'u' },
  { "image_url", required_argument, NULL, 'i' + 'u'},
  { "check", required_argument, NULL, 'c' },
  { "misc", required_argument, NULL, 'm' },
  { "misc_custom", required_argument, NULL, 'd' },
  { "partition", required_argument, NULL, 'p' },
  { "reboot", no_argument, NULL, 'r' },
  { "help", no_argument, NULL, 'h' },
  { "pipefd", required_argument, NULL, 'p' + 'f' },
  { "savepath", required_argument, NULL, 's'},
  { NULL, 0, NULL, 0 },
};

int main(int argc, char *argv[]) {
    LOGI("*** update_engine: Version V1.1.5 ***.\n");
    int arg;
    char *image_url = NULL;
    char *version_url = NULL;
    char *misc_func = NULL;
    char *save_path = NULL;
    char *partition = NULL;
    char *custom_define = NULL;
    bool is_update = false;
    bool is_reboot = false;
    int pipefd = -1;

    while ((arg = getopt_long(argc, argv, "", engine_options, NULL)) != -1) {
        switch (arg) {
        case 'u': is_update = true; if(optarg != NULL) is_sdboot = true; continue;
        case 'c': version_url = optarg; continue;
        case 'm': misc_func = optarg; continue;
        case 'p': partition = optarg; continue;
        case 's': save_path = optarg; continue;
        case 'r': is_reboot = true; continue;
        case 'v' + 'u': version_url = optarg; continue;
        case 'i' + 'u': image_url = optarg; continue;
        case 'p' + 'f': pipefd = atoi(optarg); continue;
        case 'h': display(); break;
        case 'd': custom_define = optarg; continue;
        case '?':
            LOGE("Invalid command argument\n");
            continue;
        }
    }

    if (pipefd != -1) {
        cmd_pipe = fdopen(pipefd, "wb");
        setlinebuf(cmd_pipe);
    }

    if (is_update) {
        if (is_sdboot) {
            int res = 0x3FFC00; //默认升级的分区
            LOGI("%s-%d: is sdboot update.\n", __func__, __LINE__);
            if (partition != NULL) {
                res = strtol(partition+2, NULL, 16);
            }
            RK_ota_set_url(image_url, save_path);
            if ( !RK_ota_set_partition(res) ){
                LOGE("ota file is error.\n");
                return -1;
            }

            if (version_url != NULL) {
                if (!RK_ota_check_version(version_url) ){
                    LOGE("you shouldn't update the device.\n");
                    return -1;
                }
            }

            RK_ota_start(handle_upgrade_callback, handle_print_callback);
        } else {
            LOGI("%s-%d: is ota update\n", __func__, __LINE__);
            if (MiscUpdate(image_url, partition, save_path) == 0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        }
    }else if (misc_func != NULL) {
        if (strcmp(misc_func, "now") == 0) {
            if (setSlotSucceed() ==0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        } else if (strcmp(misc_func, "other") == 0) {
            if (setSlotActivity() == 0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        } else if (strcmp(misc_func, "wipe_userdata") == 0) {
            if (wipe_userdata(0) == 0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        } else if (strcmp(misc_func, "update") == 0) {
            if (MiscUpdate(image_url, partition, save_path) == 0) {
                m_status = RK_UPGRADE_FINISHED;
            }
        } else if (strcmp(misc_func, "display") == 0) {
            miscDisplay();
        } else {
            LOGE("unknow misc cmdline : %s.\n", misc_func);
            return 0;
        }
    } else if (custom_define != NULL) {
        if (strcmp(custom_define, "read") == 0) {
            if (readCustomMiscCmdline())
                return -1;
        } else if (strcmp(custom_define, "write") == 0) {
            if (writeCustomMiscCmdline())
                return -1;
        } else if (strcmp(custom_define, "clean") == 0) {
            if (cleanCustomMiscCmdline())
                return -1;
        } else {
                LOGI("Not supported\n");
                return m_status;
        }
        m_status = RK_UPGRADE_FINISHED;
    }

    if (is_reboot && (m_status == RK_UPGRADE_FINISHED)) {
        sync();
        //reboot(RB_AUTOBOOT);
        system(" echo b > /proc/sysrq-trigger ");
    }

    return m_status;
}
