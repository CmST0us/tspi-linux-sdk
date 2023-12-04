/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include "bootloader.h"
#include "common.h"
#include "mtdutils/mtdutils.h"
#include "roots.h"
#include "rktools.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


static int get_bootloader_message_mtd(struct bootloader_message *out);
static int set_bootloader_message_mtd(const struct bootloader_message *in);
static int get_bootloader_message_block(struct bootloader_message *out);
static int set_bootloader_message_block(const struct bootloader_message *in);

int get_bootloader_message(struct bootloader_message *out) {
    if (isMtdDevice())
        return get_bootloader_message_mtd(out);

    return get_bootloader_message_block(out);
}

int set_bootloader_message(const struct bootloader_message *in) {
    if (isMtdDevice())
        return set_bootloader_message_mtd(in);

    return set_bootloader_message_block(in);
}

// ------------------------------
// for misc partitions on MTD
// ------------------------------

// The Bootloader message is at 16K(0x4000) offset, we gonna to read/write 20KB

#define MISC_SIZE ((16 + 4) << 10)
#define CMD_OFFSET (16 << 10)

#define MISC_NAME "misc"
static int get_bootloader_message_mtd(struct bootloader_message *out) {
    size_t write_size;
    mtd_scan_partitions();
    const MtdPartition *part = mtd_find_partition_by_name(MISC_NAME);
    if (part == NULL || mtd_partition_info(part, NULL, NULL, &write_size)) {
        LOGE("Can't find %s\n", MISC_NAME);
        return -1;
    }

    MtdReadContext *read = mtd_read_partition(part);
    if (read == NULL) {
        LOGE("Can't open %s\n(%s)\n", MISC_NAME, strerror(errno));
        return -1;
    }

    char data[MISC_SIZE];
    //to be align with write_size
    const ssize_t size = (MISC_SIZE / write_size) * write_size;
    ssize_t r = mtd_read_data(read, data, size);
    if (r != size) LOGE("Can't read %s\n(%s)\n", MISC_NAME, strerror(errno));
    mtd_read_close(read);
    if (r != size) return -1;

    memcpy(out, &data[CMD_OFFSET], sizeof(*out));
	printf("out->command = %s.\n", out->command);
	printf("out->status = %s.\n", out->status);
	printf("out->recovery = %s.\n", out->recovery);
	printf("out->systemFlag = %s.\n", out->systemFlag);

    return 0;
}
static int set_bootloader_message_mtd(const struct bootloader_message *in) {
    size_t write_size;
    mtd_scan_partitions();
    const MtdPartition *part = mtd_find_partition_by_name(MISC_NAME);
    if (part == NULL || mtd_partition_info(part, NULL, NULL, &write_size)) {
        LOGE("Can't find %s\n", MISC_NAME);
        return -1;
    }

    MtdReadContext *read = mtd_read_partition(part);
    if (read == NULL) {
        LOGE("Can't open %s\n(%s)\n", MISC_NAME, strerror(errno));
        return -1;
    }

    char data[MISC_SIZE];
    //to be align with write_size
    const ssize_t size = (MISC_SIZE / write_size) * write_size;
    ssize_t r = mtd_read_data(read, data, size);
    if (r != size) LOGE("Can't read %s\n(%s)\n", MISC_NAME, strerror(errno));
    mtd_read_close(read);
    if (r != size) return -1;

    memcpy(&data[CMD_OFFSET], in, sizeof(*in));

    MtdWriteContext *write = mtd_write_partition(part);
    if (write == NULL) {
        LOGE("Can't open %s\n(%s)\n", MISC_NAME, strerror(errno));
        return -1;
    }
    if (mtd_write_data(write, data, size) != size) {
        LOGE("Can't write %s\n(%s)\n", MISC_NAME, strerror(errno));
        mtd_write_close(write);
        return -1;
    }
    if (mtd_write_close(write)) {
        LOGE("Can't finish %s\n(%s)\n", MISC_NAME, strerror(errno));
        return -1;
    }

    LOGI("Set boot command \"%s\"\n", in->command[0] != 255 ? in->command : "");
    return 0;
}


// ------------------------------------
// for misc partitions on block devices
// ------------------------------------
static void wait_for_device(const char* fn) {
    int tries = 0;
    int ret;
    struct stat buf;
    do {
        ++tries;
        ret = stat(fn, &buf);
        if (ret) {
            printf("stat %s try %d: %s\n", fn, tries, strerror(errno));
            sleep(1);
        }
    } while (ret && tries < 10);
    if (ret) {
        printf("failed to stat %s\n", fn);
    }
}

static int get_bootloader_message_block(struct bootloader_message *out) {
    wait_for_device(MISC_PARTITION_NAME_BLOCK);
    FILE* f = fopen(MISC_PARTITION_NAME_BLOCK, "rb");
    if (f == NULL) {
        LOGE("Can't open %s\n(%s)\n", MISC_PARTITION_NAME_BLOCK, strerror(errno));
        return -1;
    }
    struct bootloader_message temp;
    fseek(f, BOOTLOADER_MESSAGE_OFFSET_IN_MISC, SEEK_SET);

    int count = fread(&temp, sizeof(temp), 1, f);
    if (count != 1) {
        LOGE("Failed reading %s\n(%s)\n", MISC_PARTITION_NAME_BLOCK, strerror(errno));
        return -1;
    }
    if (fclose(f) != 0) {
        LOGE("Failed closing %s\n(%s)\n", MISC_PARTITION_NAME_BLOCK, strerror(errno));
        return -1;
    }
    memcpy(out, &temp, sizeof(temp));
    return 0;
}

static int set_bootloader_message_block(const struct bootloader_message *in) {
    FILE* f = fopen(MISC_PARTITION_NAME_BLOCK, "wb");
    if (f == NULL) {
        LOGE("Can't open %s\n(%s)\n", MISC_PARTITION_NAME_BLOCK, strerror(errno));
        return -1;
    }
    fseek(f, BOOTLOADER_MESSAGE_OFFSET_IN_MISC, SEEK_SET);
    int count = fwrite(in, sizeof(*in), 1, f);
    if (count != 1) {
        LOGE("Failed writing %s\n(%s)\n", MISC_PARTITION_NAME_BLOCK, strerror(errno));
        return -1;
    }
    if (fclose(f) != 0) {
        LOGE("Failed closing %s\n(%s)\n", MISC_PARTITION_NAME_BLOCK, strerror(errno));
        return -1;
    }
    return 0;
}
