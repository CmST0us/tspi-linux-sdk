/*************************************************************************
    > File Name: flash_image.cpp
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Tue 21 May 2019 09:29:30 AM CST
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include "flash_image.h"
#include "update.h"
#include "log.h"
#include "rktools.h"
#include "gpt.h"
#include "rkimage.h"
#include "defineHeader.h"
#include "rkboot.h"

extern "C" {
    #include "../mtdutils/mtdutils.h"
}
#define UBI_HEAD_MAGIC "UBI#"

static PSTRUCT_PARAM_ITEM gp_param_item = NULL;
static long long *gp_backup_gpt_offset = NULL;

// get greastest common divisor (gcd)
static int get_gcd ( long long x, long long y )
{
    while (x != y)//用大数减去小数并将结果保存起来
    {
        if (x > y) {
            x -= y;
        } else if(x < y) {
            y -= x;
        }
    }
    return x;
}

static bool is_ubi(char *src_path, long long offset)
{
    char magic[5] = {0};
    bool ret;

    int fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        LOGE("error opening %s.\n", src_path);
        return 0;
    }
    if ( lseek64(fd_src, offset, SEEK_SET) == -1) {
        close(fd_src);
        LOGE("lseek64 failed (%s:%d).\n", __func__, __LINE__);
        return 0;
    }
    read(fd_src, magic, 4);
    LOGI("image magic is %s\n", magic);
    if(strcmp(magic, UBI_HEAD_MAGIC) == 0)
        ret = 1;
    else
        ret = 0;
    close(fd_src);

    return ret;
}

static void mtd_read() {

}

static int mtd_write(char *src_path, long long offset, long long size, long long flash_offset, char *dest_path) {
    LOGI("mtd_write %s, offset = %#llx size = %#llx flash_offset = %lld.\n", dest_path, offset, size, flash_offset);

    struct stat sb;
    char mtd_write_erase_cmd[256] = {0};
    stat(dest_path, &sb);
    long long dd_bs = 1;
    long long dd_skip = offset;
    long long dd_count = size;

    if ((sb.st_mode & S_IFMT) == S_IFCHR) {
        memset(mtd_write_erase_cmd, 0, sizeof(mtd_write_erase_cmd)/sizeof(mtd_write_erase_cmd[0]));
        sprintf(mtd_write_erase_cmd, "flash_erase %s 0 0", dest_path);
        system(mtd_write_erase_cmd);

        dd_bs = get_gcd(offset, size);
        dd_skip = offset / dd_bs;
        dd_count = size / dd_bs;
        // dd if=/mnt/sdcard/sdupdate.img bs=4 skip=2727533 count=3646464 | nandwrite -p /dev/block/by-name/recovery
        memset(mtd_write_erase_cmd, 0, sizeof(mtd_write_erase_cmd)/sizeof(mtd_write_erase_cmd[0]));
        sprintf(mtd_write_erase_cmd, "dd if=%s bs=%lld skip=%lld count=%lld | nandwrite -p %s",
                src_path, dd_bs, dd_skip, dd_count, dest_path );
        system(mtd_write_erase_cmd);
    } else {
        LOGE("flash_erase: can't erase MTD \"%s\"\n", dest_path);
        return -1;
    }

    return 0;
}

static void block_read() {

}

static int block_write(char *src_path, long long offset, long long size, long long flash_offset, char *dest_path) {
    LOGI("block_write src %s dest %s.\n", src_path, dest_path);
    int fd_dest = 0, fd_src = 0;
    long long src_offset = 0, dest_offset = 0;
    long long src_remain, dest_remain;
    int src_step, dest_step;
    long long src_file_offset = 0;
    long long read_count, write_count;
    char data_buf[BLOCK_WRITE_LEN] = {0};

    fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        LOGE("Can't open %s\n", src_path);
        return -2;
    }
    src_offset = offset;
    dest_remain = src_remain = size;
    dest_step = src_step = BLOCK_WRITE_LEN;

    if (lseek64(fd_src, src_offset, SEEK_SET) == -1) {
        close(fd_src);
        LOGE("lseek64 failed (%s:%d).\n", __func__, __LINE__);
        return -2;
    }
    src_file_offset = src_offset;
    // dest_offset = flash_offset;
    // This step is going to write (src_path: sdupdate.img) to the file which is partition data (e.g. uboot)
    // So dest_offset is 0.
    dest_offset = 0;

    fd_dest = open(dest_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd_dest < 0) {
        close(fd_src);
        LOGE("Can't open %s\n", dest_path);
        return -2;
    }
    if ( lseek64(fd_dest, dest_offset, SEEK_SET) == -1 ) {
        LOGE("(%s:%d) lseek64 failed(%s).\n", __func__, __LINE__, strerror(errno));
        close(fd_src);
        close(fd_dest);
        return -2;
    }
    while (src_remain > 0 && dest_remain > 0) {
        memset(data_buf, 0, BLOCK_WRITE_LEN);
        read_count = src_remain>src_step?src_step:src_remain;

        if (read(fd_src, data_buf, read_count) != read_count) {
            close(fd_dest);
            close(fd_src);
            LOGE("Read failed(%s):(%s:%d)\n", strerror(errno), __func__, __LINE__);
            return -2;
        }

        src_remain -= read_count;
        src_file_offset += read_count;
        write_count = dest_remain>dest_step?dest_step:dest_remain;

        if (write(fd_dest, data_buf, write_count) != write_count) {
            close(fd_dest);
            close(fd_src);
            LOGE("(%s:%d) write failed(%s).\n", __func__, __LINE__, strerror(errno));
            return -2;
        }
        dest_remain -= write_count;
    }

    fsync(fd_dest);
    close(fd_dest);
    close(fd_src);
    return 0;
}

extern bool is_sdboot;
int flash_normal(char *src_path, void *pupdate_cmd) {
    LOGI("%s:%d start.\n", __func__, __LINE__);
    PUPDATE_CMD pcmd = (PUPDATE_CMD)pupdate_cmd;
    int ret = 0;
    if (is_sdboot || !isMtdDevice()) {
        //block
        ret = block_write(src_path, pcmd->offset, pcmd->size, pcmd->flash_offset, pcmd->dest_path);
    } else {
        //mtd
        printf("pcmd->flash_offset = %lld.\n", pcmd->flash_offset);
        ret = mtd_write(src_path, pcmd->offset, pcmd->size, pcmd->flash_offset, pcmd->dest_path);
    }
    return ret;
}

static void string_to_uuid(char* strUUid, char *uuid)
{
    unsigned int i;
    char value;
    memset(uuid, 0, 16);
    for (i =0; i < 256; i++) {
        if (strUUid[i] == '\0') {
            break;
        }
        value = 0;
        if ((strUUid[i] >= '0')&&(strUUid[i] <= '9'))
            value = strUUid[i] - '0';
        if ((strUUid[i] >= 'a')&&(strUUid[i] <= 'f'))
            value = strUUid[i] - 'a' + 10;
        if ((strUUid[i] >= 'A')&&(strUUid[i] <= 'F'))
            value = strUUid[i] - 'A' + 10;
        if ((i % 2) == 0)
            uuid[i / 2] += (value << 4);
        else
            uuid[i / 2] += value;
    }
    unsigned int *p32;
    unsigned short *p16;
    p32 = (unsigned int*)uuid;
    *p32 = cpu_to_be32(*p32);
    p16 = (unsigned short *)(uuid + 4);
    *p16 = cpu_to_be16(*p16);
    p16 = (unsigned short *)(uuid + 6);
    *p16 = cpu_to_be16(*p16);
}

static void getUuidFromString(char *str, PSTRUCT_CONFIG_ITEM  p_config) {
    char data_buf[strlen(str)];
    memcpy(data_buf, str, strlen(str));
    char *line = strtok(data_buf, "\n");
    while (line != NULL) {
        if (line[0] == '#') {
            continue;
        }

        char *pline = strstr(line, "uuid");
        if(pline != NULL && (pline = strstr(pline, ":")) != NULL) {
            pline++; //过滤掉冒号
            //过滤掉空格
            while(*pline == ' ') {
                pline++;
            }
            char tmp;
            char value[256] = {0};
            sscanf(pline, "%[^=]%c%s", p_config->name, &tmp, value);
            string_to_uuid(value, p_config->value);
            p_config++;
        }

        line = strtok(NULL, "\n");
    }
}

static void getParamFromString(char *str, PSTRUCT_PARAM_ITEM p_param) {
    char data_buf[strlen(str)];
    memcpy(data_buf, str, strlen(str));
    char *line = strtok(data_buf, "\n");
    while (line != NULL) {
        if (line[0] == '#') {
            continue;
        }

        char *pline = strstr(line, "mtdparts");
        if (pline != NULL && (pline = strstr(pline, ":")) != NULL) {
            pline++; //过滤掉冒号
            //过滤掉空格
            while (*pline == ' ') {
                pline++;
            }
            int pline_len = strlen(pline);
            char mtdparts_line[pline_len];
            memset(mtdparts_line, 0, pline_len);
            memcpy(mtdparts_line, pline, pline_len);
            //开始解析文件
            char *token = strtok(mtdparts_line, ",");
            while (token != NULL) {
                char size[20], offset[20];
                char tmp;
                if (*token == '-'){
                    sscanf(token, "-@0x%x(%[^)]", &p_param->offset, p_param->name);
                    p_param->size = 0xFFFFFFFF;
                    p_param++;
                } else {
                    sscanf(token, "0x%x@0x%x(%[^)]", &p_param->size, &p_param->offset, p_param->name);
                    p_param++;
                }
                token = strtok(NULL, ",");
            }
        }
        line = strtok(NULL, "\n");
    }
}

static void gen_rand_uuid(unsigned char *uuid_bin)
{
    efi_guid_t id;
    unsigned int *ptr = (unsigned int *)&id;
    unsigned int i;

    /* Set all fields randomly */
    for (i = 0; i < sizeof(id) / sizeof(*ptr); i++)
        *(ptr + i) = cpu_to_be32(rand());

    id.uuid.time_hi_and_version = (id.uuid.time_hi_and_version & 0x0FFF) | 0x4000;
    id.uuid.clock_seq_hi_and_reserved = id.uuid.clock_seq_hi_and_reserved | 0x80;

    memcpy(uuid_bin, id.raw, sizeof(id));
}

#define tole(x)        (x)
unsigned int crc32table_le[] = {
    tole(0x00000000L), tole(0x77073096L), tole(0xee0e612cL), tole(0x990951baL),
    tole(0x076dc419L), tole(0x706af48fL), tole(0xe963a535L), tole(0x9e6495a3L),
    tole(0x0edb8832L), tole(0x79dcb8a4L), tole(0xe0d5e91eL), tole(0x97d2d988L),
    tole(0x09b64c2bL), tole(0x7eb17cbdL), tole(0xe7b82d07L), tole(0x90bf1d91L),
    tole(0x1db71064L), tole(0x6ab020f2L), tole(0xf3b97148L), tole(0x84be41deL),
    tole(0x1adad47dL), tole(0x6ddde4ebL), tole(0xf4d4b551L), tole(0x83d385c7L),
    tole(0x136c9856L), tole(0x646ba8c0L), tole(0xfd62f97aL), tole(0x8a65c9ecL),
    tole(0x14015c4fL), tole(0x63066cd9L), tole(0xfa0f3d63L), tole(0x8d080df5L),
    tole(0x3b6e20c8L), tole(0x4c69105eL), tole(0xd56041e4L), tole(0xa2677172L),
    tole(0x3c03e4d1L), tole(0x4b04d447L), tole(0xd20d85fdL), tole(0xa50ab56bL),
    tole(0x35b5a8faL), tole(0x42b2986cL), tole(0xdbbbc9d6L), tole(0xacbcf940L),
    tole(0x32d86ce3L), tole(0x45df5c75L), tole(0xdcd60dcfL), tole(0xabd13d59L),
    tole(0x26d930acL), tole(0x51de003aL), tole(0xc8d75180L), tole(0xbfd06116L),
    tole(0x21b4f4b5L), tole(0x56b3c423L), tole(0xcfba9599L), tole(0xb8bda50fL),
    tole(0x2802b89eL), tole(0x5f058808L), tole(0xc60cd9b2L), tole(0xb10be924L),
    tole(0x2f6f7c87L), tole(0x58684c11L), tole(0xc1611dabL), tole(0xb6662d3dL),
    tole(0x76dc4190L), tole(0x01db7106L), tole(0x98d220bcL), tole(0xefd5102aL),
    tole(0x71b18589L), tole(0x06b6b51fL), tole(0x9fbfe4a5L), tole(0xe8b8d433L),
    tole(0x7807c9a2L), tole(0x0f00f934L), tole(0x9609a88eL), tole(0xe10e9818L),
    tole(0x7f6a0dbbL), tole(0x086d3d2dL), tole(0x91646c97L), tole(0xe6635c01L),
    tole(0x6b6b51f4L), tole(0x1c6c6162L), tole(0x856530d8L), tole(0xf262004eL),
    tole(0x6c0695edL), tole(0x1b01a57bL), tole(0x8208f4c1L), tole(0xf50fc457L),
    tole(0x65b0d9c6L), tole(0x12b7e950L), tole(0x8bbeb8eaL), tole(0xfcb9887cL),
    tole(0x62dd1ddfL), tole(0x15da2d49L), tole(0x8cd37cf3L), tole(0xfbd44c65L),
    tole(0x4db26158L), tole(0x3ab551ceL), tole(0xa3bc0074L), tole(0xd4bb30e2L),
    tole(0x4adfa541L), tole(0x3dd895d7L), tole(0xa4d1c46dL), tole(0xd3d6f4fbL),
    tole(0x4369e96aL), tole(0x346ed9fcL), tole(0xad678846L), tole(0xda60b8d0L),
    tole(0x44042d73L), tole(0x33031de5L), tole(0xaa0a4c5fL), tole(0xdd0d7cc9L),
    tole(0x5005713cL), tole(0x270241aaL), tole(0xbe0b1010L), tole(0xc90c2086L),
    tole(0x5768b525L), tole(0x206f85b3L), tole(0xb966d409L), tole(0xce61e49fL),
    tole(0x5edef90eL), tole(0x29d9c998L), tole(0xb0d09822L), tole(0xc7d7a8b4L),
    tole(0x59b33d17L), tole(0x2eb40d81L), tole(0xb7bd5c3bL), tole(0xc0ba6cadL),
    tole(0xedb88320L), tole(0x9abfb3b6L), tole(0x03b6e20cL), tole(0x74b1d29aL),
    tole(0xead54739L), tole(0x9dd277afL), tole(0x04db2615L), tole(0x73dc1683L),
    tole(0xe3630b12L), tole(0x94643b84L), tole(0x0d6d6a3eL), tole(0x7a6a5aa8L),
    tole(0xe40ecf0bL), tole(0x9309ff9dL), tole(0x0a00ae27L), tole(0x7d079eb1L),
    tole(0xf00f9344L), tole(0x8708a3d2L), tole(0x1e01f268L), tole(0x6906c2feL),
    tole(0xf762575dL), tole(0x806567cbL), tole(0x196c3671L), tole(0x6e6b06e7L),
    tole(0xfed41b76L), tole(0x89d32be0L), tole(0x10da7a5aL), tole(0x67dd4accL),
    tole(0xf9b9df6fL), tole(0x8ebeeff9L), tole(0x17b7be43L), tole(0x60b08ed5L),
    tole(0xd6d6a3e8L), tole(0xa1d1937eL), tole(0x38d8c2c4L), tole(0x4fdff252L),
    tole(0xd1bb67f1L), tole(0xa6bc5767L), tole(0x3fb506ddL), tole(0x48b2364bL),
    tole(0xd80d2bdaL), tole(0xaf0a1b4cL), tole(0x36034af6L), tole(0x41047a60L),
    tole(0xdf60efc3L), tole(0xa867df55L), tole(0x316e8eefL), tole(0x4669be79L),
    tole(0xcb61b38cL), tole(0xbc66831aL), tole(0x256fd2a0L), tole(0x5268e236L),
    tole(0xcc0c7795L), tole(0xbb0b4703L), tole(0x220216b9L), tole(0x5505262fL),
    tole(0xc5ba3bbeL), tole(0xb2bd0b28L), tole(0x2bb45a92L), tole(0x5cb36a04L),
    tole(0xc2d7ffa7L), tole(0xb5d0cf31L), tole(0x2cd99e8bL), tole(0x5bdeae1dL),
    tole(0x9b64c2b0L), tole(0xec63f226L), tole(0x756aa39cL), tole(0x026d930aL),
    tole(0x9c0906a9L), tole(0xeb0e363fL), tole(0x72076785L), tole(0x05005713L),
    tole(0x95bf4a82L), tole(0xe2b87a14L), tole(0x7bb12baeL), tole(0x0cb61b38L),
    tole(0x92d28e9bL), tole(0xe5d5be0dL), tole(0x7cdcefb7L), tole(0x0bdbdf21L),
    tole(0x86d3d2d4L), tole(0xf1d4e242L), tole(0x68ddb3f8L), tole(0x1fda836eL),
    tole(0x81be16cdL), tole(0xf6b9265bL), tole(0x6fb077e1L), tole(0x18b74777L),
    tole(0x88085ae6L), tole(0xff0f6a70L), tole(0x66063bcaL), tole(0x11010b5cL),
    tole(0x8f659effL), tole(0xf862ae69L), tole(0x616bffd3L), tole(0x166ccf45L),
    tole(0xa00ae278L), tole(0xd70dd2eeL), tole(0x4e048354L), tole(0x3903b3c2L),
    tole(0xa7672661L), tole(0xd06016f7L), tole(0x4969474dL), tole(0x3e6e77dbL),
    tole(0xaed16a4aL), tole(0xd9d65adcL), tole(0x40df0b66L), tole(0x37d83bf0L),
    tole(0xa9bcae53L), tole(0xdebb9ec5L), tole(0x47b2cf7fL), tole(0x30b5ffe9L),
    tole(0xbdbdf21cL), tole(0xcabac28aL), tole(0x53b39330L), tole(0x24b4a3a6L),
    tole(0xbad03605L), tole(0xcdd70693L), tole(0x54de5729L), tole(0x23d967bfL),
    tole(0xb3667a2eL), tole(0xc4614ab8L), tole(0x5d681b02L), tole(0x2a6f2b94L),
    tole(0xb40bbe37L), tole(0xc30c8ea1L), tole(0x5a05df1bL), tole(0x2d02ef8dL)
};

#define DO_CRC(x) crc = tab[ (crc ^ (x)) & 255 ] ^ (crc>>8)
static unsigned int crc32_le(unsigned int crc, unsigned char *p, unsigned int len)
{
    unsigned int      *b =(unsigned int *)p;
    unsigned int      *tab = crc32table_le;
    crc = crc ^ 0xFFFFFFFF;
    if((((long)b)&3 && len)){
        do {
            unsigned char *p = (unsigned char *)b;
            DO_CRC(*p++);
            b = (unsigned int *)p;
        } while ((--len) && ((long)b)&3 );
    }
    if((len >= 4)){
        unsigned int save_len = len & 3;
        len = len >> 2;
        --b;
        do {
            crc ^= *++b;
            DO_CRC(0);
            DO_CRC(0);
            DO_CRC(0);
            DO_CRC(0);
        } while (--len);
        b++;
        len = save_len;
    }
    if(len){
        do {
            unsigned char *p = (unsigned char *)b;
            DO_CRC(*p++);
            b = (unsigned int *)p;
        } while (--len);
    }
    crc = crc ^ 0xFFFFFFFF;
    return crc;

}

static void create_gpt_buffer(u8 *gpt, PSTRUCT_PARAM_ITEM p_param, int param_len, PSTRUCT_CONFIG_ITEM p_config, int config_len, u64 diskSectors)
{
    legacy_mbr *mbr = (legacy_mbr *)gpt;
    gpt_header *gptHead = (gpt_header *)(gpt + SECTOR_SIZE);
    gpt_entry *gptEntry = (gpt_entry *)(gpt + 2 * SECTOR_SIZE);

    /*1.protective mbr*/
    memset(gpt, 0, SECTOR_SIZE);
    mbr->signature = MSDOS_MBR_SIGNATURE;
    mbr->partition_record[0].sys_ind = EFI_PMBR_OSTYPE_EFI_GPT;
    mbr->partition_record[0].start_sect = 1;
    mbr->partition_record[0].nr_sects = (u32)-1;

      /*2.gpt header*/
    memset(gpt + SECTOR_SIZE, 0, SECTOR_SIZE);
    gptHead->signature = cpu_to_le64(GPT_HEADER_SIGNATURE);
    gptHead->revision = cpu_to_le32(GPT_HEADER_REVISION_V1);
    gptHead->header_size = cpu_to_le32(sizeof(gpt_header));
    gptHead->my_lba = cpu_to_le64(1);
    gptHead->alternate_lba = cpu_to_le64(diskSectors - 1);
    gptHead->first_usable_lba = cpu_to_le64(34);
    gptHead->last_usable_lba = cpu_to_le64(diskSectors - 34);
    gptHead->partition_entry_lba = cpu_to_le64(2);
    gptHead->num_partition_entries = cpu_to_le32(GPT_ENTRY_NUMBERS);
    gptHead->sizeof_partition_entry = cpu_to_le32(GPT_ENTRY_SIZE);
    gptHead->header_crc32 = 0;
    gptHead->partition_entry_array_crc32 = 0;
    gen_rand_uuid(gptHead->disk_guid.raw);

    /*3.gpt partition entry*/
    memset(gpt + 2 * SECTOR_SIZE, 0, 32 * SECTOR_SIZE);
    for (int i = 0; i < param_len; i++) {
        if (strcmp(p_param[i].name, "") == 0) {
            continue;
        }
        gen_rand_uuid(gptEntry->partition_type_guid.raw);
        gen_rand_uuid(gptEntry->unique_partition_guid.raw);
        gptEntry->starting_lba = cpu_to_le64(p_param[i].offset);
        gptEntry->ending_lba = cpu_to_le64(gptEntry->starting_lba + p_param[i].size - 1);
        gptEntry->attributes.raw = 0;

        char partition_name[20] = {0};
        strcpy(partition_name, p_param[i].name);
        char *p;
        if ( (p = strstr(partition_name, ":")) != NULL) {
            if (strstr(partition_name, "bootable") != NULL) {
                gptEntry->attributes.raw = PART_PROPERTY_BOOTABLE;
            }
            if (strstr(partition_name, "grow") != NULL) {
                gptEntry->ending_lba = cpu_to_le64(diskSectors - 34);
            }
            *p = '\0';
            strcpy(p_param[i].name, partition_name);
        }

        for (int j = 0; j < strlen(p_param[i].name); j++)
            gptEntry->partition_name[j] = p_param[i].name[j];
        for (int j = 0; j < config_len; j++) {
            if (strcmp(p_config[j].name, p_param[i].name) == 0) {
                memcpy(gptEntry->unique_partition_guid.raw, p_config[j].value, 16);
                break ;
            }
        }
        gptEntry++;
    }

    gptHead->partition_entry_array_crc32 = cpu_to_le32(crc32_le(0, gpt + 2 * SECTOR_SIZE, GPT_ENTRY_SIZE * GPT_ENTRY_NUMBERS));
    gptHead->header_crc32 = cpu_to_le32(crc32_le(0, gpt + SECTOR_SIZE, sizeof(gpt_header)));

}

static void prepare_gpt_backup(u8 *master, u8 *backup)
{
    gpt_header *gptMasterHead = (gpt_header *)(master + SECTOR_SIZE);
    gpt_header *gptBackupHead = (gpt_header *)(backup + 32 * SECTOR_SIZE);
    u32 calc_crc32;
    u64 val;

    /* recalculate the values for the Backup GPT Header */
    val = le64_to_cpu(gptMasterHead->my_lba);
    gptBackupHead->my_lba = gptMasterHead->alternate_lba;
    gptBackupHead->alternate_lba = cpu_to_le64(val);
    gptBackupHead->partition_entry_lba = cpu_to_le64(le64_to_cpu(gptMasterHead->last_usable_lba) + 1);
    gptBackupHead->header_crc32 = 0;

    calc_crc32 = crc32_le(0, (unsigned char *)gptBackupHead, le32_to_cpu(gptBackupHead->header_size));
    gptBackupHead->header_crc32 = cpu_to_le32(calc_crc32);
}

int flash_register_partition_data(PSTRUCT_PARAM_ITEM p_param_item, long long *p_gpt_backup_offset)
{
    if (p_param_item) {
        gp_param_item = p_param_item;
    }

    if (p_gpt_backup_offset) {
        gp_backup_gpt_offset = p_gpt_backup_offset;
    }

    return 0;
}

int flash_parameter(char *src_path, void *pupdate_cmd) {
    LOGI("flash_parameter start src_path [%s].\n", src_path);
    PUPDATE_CMD pcmd = (PUPDATE_CMD)pupdate_cmd;

    unsigned int m_uiParamFileSize = pcmd->size;

    if (m_uiParamFileSize % SECTOR_SIZE != 0) {
        m_uiParamFileSize = (m_uiParamFileSize/SECTOR_SIZE + 1) * SECTOR_SIZE;
    }

    // 1. 读取parameter 数据
    unsigned char data_buf[m_uiParamFileSize];
    memset(data_buf, 0, m_uiParamFileSize);
    int fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        LOGE("Can't open %s, %s\n", src_path, strerror(errno));
        return -2;
    }
    if (lseek64(fd_src, pcmd->offset, SEEK_SET) == -1) {
        LOGE("lseek64 failed (%s:%d), %s.\n", __func__, __LINE__, strerror(errno));
        close(fd_src);
        return -2;
    }
    if (read(fd_src, data_buf, pcmd->size) != pcmd->size) {
        close(fd_src);
        return -2;
    }
    close(fd_src);

    // 2. 获取分区大小和uuid
    STRUCT_PARAM_ITEM param_item[20] = {0};
    STRUCT_CONFIG_ITEM config_item[10] = {0};
    getParamFromString((char *)data_buf + 8, param_item);
    getUuidFromString((char *)data_buf + 8, config_item);

    if (gp_param_item) {
        memcpy(gp_param_item, param_item, sizeof(param_item) );
    }

    LOGI("%s-%d: List partitions:\n", __func__, __LINE__);
    for (int j = 0; j < sizeof(param_item)/sizeof(param_item[0]); j++) {
        LOGI("    param_item[%d].name [%s]\n", j, param_item[j].name);
    }

    // 3. 获得flash 的大小，和块数
    long long block_num;
    if (getFlashSize(NULL, NULL, &block_num) != 0) {
        LOGE("getFlashSize error.\n");
        return -2;
    }

    LOGI("%s, block_num = %lld.\n", __func__, block_num);

    // 4. 创建gpt 表
    unsigned char write_buf[SECTOR_SIZE * 67];
    unsigned char *backup_gpt;
    backup_gpt = write_buf+34*SECTOR_SIZE;
    memset(write_buf, 0, SECTOR_SIZE * 67);
    create_gpt_buffer(write_buf, param_item, 20, config_item, 10, block_num);
    memcpy(backup_gpt, write_buf + 2* SECTOR_SIZE, 32 * SECTOR_SIZE);
    memcpy(backup_gpt + 32 * SECTOR_SIZE, write_buf + SECTOR_SIZE, SECTOR_SIZE);
    prepare_gpt_backup(write_buf, backup_gpt);

    if (gp_backup_gpt_offset) {
		*gp_backup_gpt_offset = (block_num - 33) * SECTOR_SIZE;
    }

    //5. 写入主GPT表
    int fd_dest = open(pcmd->dest_path, O_CREAT|O_RDWR| O_TRUNC, 0644);
    if (fd_dest < 0) {
        LOGE("Can't open %s, %s\n", pcmd->dest_path, strerror(errno));
        return -2;
    }
    lseek64(fd_dest, 0, SEEK_SET);
    if (write(fd_dest, write_buf, 34*SECTOR_SIZE) != 34*SECTOR_SIZE) {
        LOGE("write error %s: (%s:%d).\n", strerror(errno), __func__, __LINE__);
        close(fd_dest);
        return -2;
    }
    //6. 尾部写入GPT表到文件
	/*
     * char gpt_backup_dest_path[100] = {0};
	 * memset(gpt_backup_dest_path, 0, sizeof(gpt_backup_dest_path)/sizeof(gpt_backup_dest_path[0]));
	 * memcpy(gpt_backup_dest_path, pcmd->dest_path, strlen(pcmd->dest_path));
	 * dirname(gpt_backup_dest_path);
	 * sprintf(gpt_backup_dest_path, "%s/%s", gpt_backup_dest_path, GPT_BACKUP_FILE_NAME);
	 */
	
#if 0
    int fd_backup = open(gpt_backup_dest_path, O_CREAT|O_RDWR| O_TRUNC, 0644);
    if (fd_backup < 0) {
        LOGE("Can't open %s, %s\n", gpt_backup_dest_path, strerror(errno));
        return -2;
    }

	if (write(fd_backup, backup_gpt, 33*SECTOR_SIZE) != 33*SECTOR_SIZE) {
		LOGE("write error %s: (%s:%d).\n", strerror(errno), __func__, __LINE__);
		close(fd_backup);
		return -2;
	}
    close(fd_backup);
#endif
    close(fd_dest);
    sync();
    return 0;
}


int flash_bootloader(char *src_path, void *pupdate_cmd) {
    PUPDATE_CMD pcmd = (PUPDATE_CMD)pupdate_cmd;

    if (isMtdDevice()) {
        // bootrom read IDBlock from the offset which is equal to block size for Nand Flash
        size_t block_size = 0;
        if (getFlashInfo(NULL, &block_size, NULL) != 0) {
            LOGE("%s-%d: get mtd info error\n", __func__, __LINE__);
            return false;
        }
        pcmd->flash_offset = block_size;
    } else {
        // bootrom read IDBlock from the offset (32KB + 512KB * n <n=0,1,2,3...>) for eMMC
        pcmd->flash_offset = 32*1024;
    }

    // 1. 读取bootloader
    unsigned char data_buf[pcmd->size];
    memset(data_buf, 0, pcmd->size);
    int fd_src = open(src_path, O_RDONLY);
    if (fd_src < 0) {
        LOGE("Can't open %s, %s\n", src_path, strerror(errno));
        return -2;
    }
    if (lseek64(fd_src, pcmd->offset, SEEK_SET) == -1) {
        LOGE("(%s:%d) lseek64 failed: %s.\n", __func__, __LINE__, strerror(errno));
        close(fd_src);
        return -2;
    }
    if (read(fd_src, data_buf, pcmd->size) != pcmd->size) {
        close(fd_src);
        LOGE("read error(%s:%d) : %s.\n", __func__, __LINE__, strerror(errno));
        return -2;
    }
    close(fd_src);

    if (!download_loader(data_buf, pcmd->size, pcmd->dest_path)) {
        LOGE("download_loader error.\n");
        return -1;
    }

    return 0;
}

