/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include "mtdutils/mtdutils.h"
#include "mtdutils/mounts.h"
#include "roots.h"
#include "common.h"
#include "rktools.h"
#include "mtdutils/rk29.h"
//#include "make_ext4fs.h"

static int num_volumes = 0;
static Volume* device_volumes = NULL;
static char mount_point[256] = {0};

char * get_link_path(const char* linkpath, char * buf, int count)
{
	int i;
	int rslt;
	char path[256];

	memset(path, 0, sizeof(path));
	strcpy(path, linkpath);
	for (i = strlen(linkpath); i > 0; i--) {
		if (path[i] == '/') {
			path[i] = '\0';
			break;
		}
	}

	rslt = readlink(path, buf, count - 1);
	if (rslt < 0 || (rslt >= count - 1)) {
		// printf("No link to path [%s]!!! \n", path);
		return NULL;
	}

	if (buf[0] != '/') {
		char tmp[256];
		memset(tmp, 0, sizeof(tmp));
		memcpy(tmp, buf, strlen(buf));

		memset(buf, 0, sizeof(buf));
		memcpy(buf + 1, tmp,strlen(tmp));

		buf[0] = '/';
	}

	// printf("buf = %s \n", buf);
	return buf;
}

const char* get_mounted_device_from_path(const char* path)
{
	const MountedVolume *volume;

	int result= scan_mounted_volumes();
	if (result < 0) {
		LOGE("failed to scan mounted volumes\n");
		return NULL;
	}

	volume = find_mounted_volume_by_mount_point(path);
	if (!volume) {
		LOGE("failed to get volume from %s\n", path);
		return NULL;
	}

	return volume->device;
}

void load_volume_table() {
	int alloc = 2;
	device_volumes = malloc(alloc * sizeof(Volume));

	// Insert an entry for /tmp, which is the ramdisk and is always mounted.
	device_volumes[0].mount_point = "/tmp";
	device_volumes[0].fs_type = "ramdisk";
	device_volumes[0].device = NULL;
	device_volumes[0].device2 = NULL;
	device_volumes[0].option = NULL;
	device_volumes[0].dump = NULL;
	device_volumes[0].pass = NULL;
	num_volumes = 1;

	FILE* fstab = fopen("/etc/fstab", "r");
	if (fstab == NULL) {
		LOGE("failed to open /etc/fstab (%d)\n", errno);
		return;
	}

	char buffer[1024];
	char file_system[1024];
	char mount_point[1024];
	char fs_type[1024];
	char option[1024];
	char dump[1024];
	char pass[1024];
	char device[1024];
	int i;
	while (fgets(buffer, sizeof(buffer)-1, fstab)) {
		i = sscanf(buffer, "%s %s %s %s %s %s", file_system,
			   mount_point, fs_type, option, dump, pass);
		if (file_system[0] == '#') continue;
		//printf("load_volume_table file_system:%s, mount_point:%s, fs_type:%s, option:%s, dump:%s, pass:%s\n", file_system, mount_point, fs_type, option, dump, pass);
		/* HACK: Convert PARTLABEL to "by-name" symlink */
		if (file_system == strstr(file_system, "PARTLABEL="))
			snprintf(device, sizeof(device), "/dev/block/by-name/%s",
				 file_system + strlen("PARTLABEL="));
		else
			strcpy(device, file_system);

		if (i == 6) {
			while (num_volumes >= alloc) {
				alloc *= 2;
				device_volumes = realloc(device_volumes, alloc*sizeof(Volume));
			}
			device_volumes[num_volumes].mount_point = strdup(mount_point);
			device_volumes[num_volumes].fs_type = strdup(fs_type);
			device_volumes[num_volumes].option = strdup(option);
			device_volumes[num_volumes].dump = strdup(dump);
			device_volumes[num_volumes].pass = strdup(pass);
			device_volumes[num_volumes].device = strdup(device);;
			device_volumes[num_volumes].device2 = NULL;
			++num_volumes;
		} else {
			LOGE("skipping malformed recovery.fstab line: %s\n", buffer);
		}
	}

	fclose(fstab);

	printf("recovery filesystem table\n");
	printf("=========================\n");
	for (i = 0; i < num_volumes; ++i) {
		Volume* v = &device_volumes[i];
		printf("  %d %s %s %s %s %s %s\n", i, v->device, v->mount_point, v->fs_type, v->option, v->dump, v->pass);
	}
	printf("\n");
}

Volume* volume_for_path(const char* path) {
	int i;

	memset(mount_point, 0, sizeof(mount_point));
	char* tmp = get_link_path(path, mount_point, 1024);

	// if ( tmp != NULL)
	// 	printf(" ### get mount_ponit = %s ### \n", mount_point);

	for (i = 0; i < num_volumes; ++i) {
		Volume* v = device_volumes+i;
		int len = strlen(v->mount_point);
		if (strncmp(path, v->mount_point, len) == 0 &&
			(path[len] == '\0' || path[len] == '/')) {
			//printf(" ===path = %s, v-mount_point = %s ===\n",path, v->mount_point);
			return v;
		} else {
			//add by chad.ma for symbol link file. eg. sdcard/ --->mnt/sdcard
			if (!tmp)
				continue;

			//printf(" # v->mount_point = %s\n", v->mount_point);
			if (strncmp(mount_point, v->mount_point, len) == 0 &&
				(path[len] == '\0' || path[len] == '/')) {
				return v;
			}
		}
	}
	return NULL;
}

int ensure_path_mounted(const char* path) {
	if (access(path, F_OK) == 0)
		return 0;

	Volume* v = volume_for_path(path);
	if (v == NULL) {
		if (mount_point[0] != 0) {
			int result= scan_mounted_volumes();
			if (result < 0) {
				LOGE("failed to scan mounted volumes\n");
				return -1;
			}

			const MountedVolume* mv =
				find_mounted_volume_by_mount_point(mount_point);
			if (mv) {
				// volume is already mounted
				LOGI("%s already mounted \n", path);
				return 0;
			}
		}

		LOGE("unknown volume for path [%s]\n", path);
		return -1;
	}
	if (strcmp(v->fs_type, "ramdisk") == 0) {
		// the ramdisk is always mounted.
		return 0;
	}

	int result;
	result = scan_mounted_volumes();
	if (result < 0) {
		LOGE("failed to scan mounted volumes\n");
		return -1;
	}

	const MountedVolume* mv =
		find_mounted_volume_by_mount_point(v->mount_point);
	if (mv) {
		// volume is already mounted
		return 0;
	}

	mkdir(v->mount_point, 0755);  // in case it doesn't already exist

	if (strcmp(v->fs_type, "yaffs2") == 0) {
		// mount an MTD partition as a YAFFS2 filesystem.
		mtd_scan_partitions();
		const MtdPartition* partition;
		partition = mtd_find_partition_by_name(v->device);
		if (partition == NULL) {
			LOGE("failed to find \"%s\" partition to mount at \"%s\"\n",
				 v->device, v->mount_point);
			return -1;
		}
		return mtd_mount_partition(partition, v->mount_point, v->fs_type, 0);
	} else if (strcmp(v->fs_type, "ext4") == 0 ||
			   strcmp(v->fs_type, "vfat") == 0 ||
			   strcmp(v->fs_type, "ext2") == 0) {
		char *blk_device;
		blk_device = (char*)v->device;
		if(strcmp("/mnt/sdcard", v->mount_point) == 0){
			blk_device = getenv(SD_POINT_NAME);
			if(blk_device == NULL){
				setFlashPoint();
				blk_device = getenv(SD_POINT_NAME);
			}
			result = mount(blk_device, v->mount_point, v->fs_type,
							MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
			if (result == 0) return 0;
		}
		result = mount(v->device, v->mount_point, v->fs_type,
					   MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
		if (result == 0) return 0;

		if (v->device2) {
			LOGW("failed to mount %s (%s); trying %s\n",
				 v->device, strerror(errno), v->device2);
			result = mount(v->device2, v->mount_point, v->fs_type,
						   MS_NOATIME | MS_NODEV | MS_NODIRATIME, "");
			if (result == 0) return 0;
		}
		LOGE("failed to mount %s (%s)\n", v->mount_point, strerror(errno));
		return -1;
	}

	LOGE("unknown fs_type \"%s\" for %s\n", v->fs_type, v->mount_point);
	return -1;
}

int ensure_ex_path_unmounted(const char* path) {
	int result;

	result = scan_mounted_volumes();
	if (result < 0) {
		LOGE("unknown volume for path [%s]\n", path);
		return -1;
	}

	const MountedVolume* mv =
		find_mounted_volume_by_mount_point(path);
	if (mv == NULL) {
		LOGE("path: %s is already unmounted or not existed\n");
		return 0;
	}

	return unmount_mounted_volume(mv);
}

int ensure_path_unmounted(const char* path) {
	Volume* v = volume_for_path(path);
	if (v == NULL) {
		LOGE("unknown volume for path [%s]\n", path);
		return -1;
	}
	if (strcmp(v->fs_type, "ramdisk") == 0) {
		// the ramdisk is always mounted; you can't unmount it.
		return -1;
	}

	int result;
	result = scan_mounted_volumes();
	if (result < 0) {
		LOGE("failed to scan mounted volumes\n");
		return -1;
	}

	const MountedVolume* mv =
		find_mounted_volume_by_mount_point(v->mount_point);
	if (mv == NULL) {
		// volume is already unmounted
		return 0;
	}

	return unmount_mounted_volume(mv);
}

int format_volume(const char* volume) {
	Volume* v = volume_for_path(volume);
	if (v == NULL) {
		LOGE("unknown volume \"%s\"\n", volume);
		return -1;
	}
	if (strcmp(v->fs_type, "ramdisk") == 0) {
		// you can't format the ramdisk.
		LOGE("can't format_volume \"%s\"", volume);
		return -1;
	}
	if (strcmp(v->mount_point, volume) != 0) {
		LOGE("can't give path \"%s\" to format_volume\n", volume);
		return -1;
	}

	if (ensure_path_unmounted(volume) != 0) {
		LOGE("format_volume failed to unmount \"%s\"\n", v->mount_point);
		return -1;
	}

	if (strcmp(v->fs_type, "yaffs2") == 0 || strcmp(v->fs_type, "mtd") == 0 || strcmp(v->fs_type, "ubifs") == 0 ) {
		mtd_scan_partitions();
		char filepath[20];
		if(strstr(v->device, "userdata") != NULL){
			strcpy(filepath, "userdata");
			LOGW("change v->device from %s to %s.\n", v->device, filepath);
		}else{
			strcpy(filepath, v->device);
		}

		const MtdPartition* partition = mtd_find_partition_by_name(filepath);
		if (partition == NULL) {
			LOGE("format_volume: no MTD partition \"%s\"\n", v->device);
			return -1;
		}

		MtdWriteContext *write = mtd_write_partition(partition);
		if (write == NULL) {
			LOGW("format_volume: can't open MTD \"%s\"\n", v->device);
			return -1;
		} else if (mtd_erase_blocks(write, -1) == (off_t) -1) {
			LOGW("format_volume: can't erase MTD \"%s\"\n", v->device);
			mtd_write_close(write);
			return -1;
		} else if (mtd_write_close(write)) {
			LOGW("format_volume: can't close MTD \"%s\"\n", v->device);
			return -1;
		}
		return 0;
	}

	if (strcmp(v->fs_type, "ext4") == 0) {
		//reset_ext4fs_info();
		int result = rk_make_ext4fs(v->device, 0, v->mount_point);//make_ext4fs(v->device, NULL, NULL, 0, 0, 0);
		if (result != 0) {
			LOGE("format_volume: make_extf4fs failed on %s\n", v->device);
			return -1;
		}
		return 0;
	}

	if (strcmp(v->fs_type, "ext2") == 0) {
		int result = rk_make_ext2fs(v->device);//make_ext2fs(v->device, NULL, NULL, 0, 0, 0);
		if (result != 0) {
			LOGE("format_volume: make_extf2fs failed on %s\n", v->device);
			return -1;
		}
		return 0;
	}
	if (strcmp(v->fs_type, "vfat") == 0) {
		int result = make_vfat(v->device, v->mount_point);
		if (result != 0) {
			LOGE("format_volume: make_vfat failed on %s\n", v->device);
			return -1;
		}
		return 0;
	}

	if (strcmp(v->fs_type, "ntfs") == 0) {
		int result = make_ntfs(v->device, v->mount_point);
		if (result != 0) {
			LOGE("format_volume: make_ntfs failed on %s\n", v->device);
			return -1;
		}
		return 0;
	}

	LOGE("format_volume: fs_type \"%s\" unsupported\n", v->fs_type);
	return -1;
}

int resize_volume(const char* volume) {
	Volume* v = volume_for_path(volume);
	if (v == NULL) {
		LOGE("unknown volume \"%s\"\n", volume);
		return -1;
	}
	if (strcmp(v->fs_type, "ramdisk") == 0) {
		// you can't format the ramdisk.
		LOGE("can't format_volume \"%s\"", volume);
		return -1;
	}
	if (strcmp(v->mount_point, volume) != 0) {
		LOGE("can't give path \"%s\" to format_volume\n", volume);
		return -1;
	}

	if (ensure_path_unmounted(volume) != 0) {
		LOGE("format_volume failed to unmount \"%s\"\n", v->mount_point);
		return -1;
	}

	if ((strcmp(v->fs_type, "ext4") == 0) || (strcmp(v->fs_type, "ext2") == 0)){
		int result = rk_check_and_resizefs(v->device);
		if (result != 0) {
			LOGE("resize_volume: resizefs failed on %s\n", v->device);
			return -1;
		}
		return 0;
	}

	LOGE("format_volume: fs_type \"%s\" unsupported\n", v->fs_type);
	return -1;
}

