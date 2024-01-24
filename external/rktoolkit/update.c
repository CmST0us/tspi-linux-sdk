/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * RecoverySystem contains methods for interacting with the Android
 * recovery system (the separate partition that can be used to install
 * system updates, wipe user data, etc.)
 */

//#include <direct.h>
#include <linux/reboot.h>
//#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "update_recv/update_recv.h"

#define LOG_FILE_LEN 512

#if 0
#define RECOVERY_PATH "/tmp/recovery"
#define LOG_FILE_PATH "/tmp/recovery/log"
#define COMMAND_FILE_PATH "/tmp/recovery/command"
#else
#define RECOVERY_PATH "/userdata/recovery"
#define LOG_FILE_PATH "/userdata/recovery/log"
#define COMMAND_FILE_PATH "/userdata/recovery/command"
#endif

#define SD_UPDATE_FILE "/sdcard/update.img"
#define DATA_UPDATE_FILE "/userdata/update.img"
#define MISC_FILE_PATH "/dev/block/by-name/misc"
#define MISC_MSG_OFFSET 16 * 1024

/* Bootloader Message (2-KiB)
 *
 * This structure describes the content of a block in flash
 * that is used for recovery and the bootloader to talk to
 * each other.
 *
 * The command field is updated by linux when it wants to
 * reboot into recovery or to update radio or bootloader firmware.
 * It is also updated by the bootloader when firmware update
 * is complete (to boot into recovery for any final cleanup)
 *
 * The status field is written by the bootloader after the
 * completion of an "update-radio" or "update-hboot" command.
 *
 * The recovery field is only written by linux and used
 * for the system to send a message to recovery or the
 * other way around.
 *
 * The stage field is written by packages which restart themselves
 * multiple times, so that the UI can reflect which invocation of the
 * package it is.  If the value is of the format "#/#" (eg, "1/3"),
 * the UI will add a simple indicator of that status.
 *
 * We used to have slot_suffix field for A/B boot control metadata in
 * this struct, which gets unintentionally cleared by recovery or
 * uncrypt. Move it into struct bootloader_message_ab to avoid the
 * issue.
 */
struct android_bootloader_message {
	char command[32];
	char status[32];
	char recovery[768];

	/* The 'recovery' field used to be 1024 bytes.	It has only ever
	 * been used to store the recovery command line, so 768 bytes
	 * should be plenty.  We carve off the last 256 bytes to store the
	 * stage string (for multistage packages) and possible future
	 * expansion. */
	char stage[32];

	/* The 'reserved' field used to be 224 bytes when it was initially
	 * carved off from the 1024-byte recovery field. Bump it up to
	 * 1184-byte so that the entire bootloader_message struct rounds up
	 * to 2048-byte. */
	char reserved[1184];
};



/**
 * Reboot into the recovery system with the supplied argument.
 * @param arg to pass to the recovery utility.
 */
static void bootCommand(char *arg){
	FILE *command_file;
	FILE *log_file;
	FILE *misc_file;
	char blank[LOG_FILE_LEN];

	if(!arg) return;
	printf("command: %s\n", arg);
	mkdir(RECOVERY_PATH,0775);
	if((command_file = fopen(COMMAND_FILE_PATH,"wb")) == NULL){
		printf("Open command file error.\n");
		return;
	}

	if((log_file = fopen(LOG_FILE_PATH,"wb")) == NULL){
		printf("Open log file error.\n");
		return;
	}

	if((misc_file = fopen(MISC_FILE_PATH,"wb")) == NULL){
		printf("Open misc file error.\n");
		return;
	}

	printf("update: write command to command file: ");
	fwrite(arg, strlen(arg), 1, command_file);
	fwrite("\n", 1, 1, command_file);
	fclose(command_file);
	printf("done\n");

	printf("update: write command to misc file: ");
	fseek(misc_file, MISC_MSG_OFFSET, SEEK_SET);
	struct android_bootloader_message msg;
	memset(&msg, 0, sizeof(msg));
	char recovery_str[] = "recovery\n";
	strcpy(msg.command, "boot-recovery");
	strcpy(msg.recovery, recovery_str);
	memcpy(msg.recovery + strlen(recovery_str), arg, ((strlen(arg) > sizeof(msg.recovery))? sizeof(msg.recovery) : strlen(arg)));
	msg.recovery[strlen(msg.recovery) + 1] = '\n';
	//strlcat(msg.recovery, update_file, sizeof(msg.recovery));
	//strlcat(msg.recovery, "\n", sizeof(msg.recovery));
	//strlcpy(msg.systemFlag, "false", sizeof(msg.systemFlag));
	fwrite(&msg, sizeof(msg), 1, misc_file);
	fclose(misc_file);
	printf("done\n");

	memset(blank, 0, LOG_FILE_LEN);
	fwrite(blank, LOG_FILE_LEN, 1, log_file);
	fclose(log_file);
	printf("update: reboot!\n");
	//reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2,
	//		 LINUX_REBOOT_CMD_RESTART2, "recovery");
	sync();
	reboot(RB_AUTOBOOT);
	return;
}


 /**
 * Reboots the device in order to install the given update
 * package.
 * Requires the {@link android.Manifest.permission#REBOOT} permission.
 *
 * @param packageFile  the update package to install.  Must be on
 * a partition mountable by recovery.  (The set of partitions
 * known to recovery may vary from device to device.  Generally,
 * /cache and /data are safe.)
 */
static void installPackage(char *update_file){
	char arg[512];
	char *str_update_package = "--update_package=";
	int str_update_package_len = strlen(str_update_package);
	int str_update_file_len = strlen(update_file);

	memset(arg, 0, 512);
	strcpy(arg, str_update_package);
	strcpy(arg + str_update_package_len, update_file);
	arg[str_update_package_len + str_update_file_len] = 0;
	bootCommand(arg);
}

static void sdUpdate(){
	installPackage(SD_UPDATE_FILE);
}

static void dataUpdate(){
	installPackage(DATA_UPDATE_FILE);
}

/**
 * Reboots the device and wipes the user data partition.  This is
 * sometimes called a "factory reset", which is something of a
 * misnomer because the system partition is not restored to its
 * factory state.
 * Requires the {@link android.Manifest.permission#REBOOT} permission.
 *
 * @param context  the Context to use
 *
 */
void rebootWipeUserData(){
	printf("update: --wipe_all\n");
	bootCommand("--wipe_all");
}

int rebootUpdate(char *path){

	if(path){
		printf("find %s\n", path);
		installPackage(path);
		return 0;
	}

	if(access(DATA_UPDATE_FILE,F_OK) == -1){
		printf("%s does not exist! try to use %s to update\n",
			DATA_UPDATE_FILE, SD_UPDATE_FILE);
		if(access(SD_UPDATE_FILE,F_OK) == -1){
			printf("%s does not exist!\n", SD_UPDATE_FILE);
			return -1;
		}
		printf("find %s\n", SD_UPDATE_FILE);
		installPackage(SD_UPDATE_FILE);
		return 0;
	}

	printf("find %s\n", DATA_UPDATE_FILE);
	installPackage(DATA_UPDATE_FILE);
	return 0;
}

int main(int argc, char** argv){
	char* partition_name = "recovery";
	printf("update: Rockchip Update Tool\n");

	if(argc == 1) {
		rebootWipeUserData();
	} else if(argc == 2){
		if(!strcmp(argv[1], "ota") || !strcmp(argv[1], "update"))
			rebootUpdate(0);
		else if(!strcmp(argv[1], "factory") || !strcmp(argv[1], "reset"))
			rebootWipeUserData();
		else  return -1;

		return 0;

	} else if(argc == 3){
		if(!strcmp(argv[1], "ota") || !strcmp(argv[1], "update")) {
			if(argv[2]) {
				int ret;
				ret = WriteFwData(argv[2], partition_name);
				if (ret < 0) {
					if (ret == -1) {
						printf(" Update partition %s fail \n", partition_name);
						//means no find recovery partition in update.img
						//return -1;
					} else if (ret == -2) {
						printf("Some errors happen, update process break...\n");
						return -1;
					}
				} else {
					if (!CheckFwData(argv[2], partition_name)){
						printf(" Check partition %s fail \n", partition_name);
						return -1;
					}
				}
				return rebootUpdate(argv[2]);
			}
		}
	}

	return -1;
}

