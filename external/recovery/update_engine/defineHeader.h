/*************************************************************************
    > File Name: defineHeader.h
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Tue 04 Jun 2019 09:06:31 AM CST
 ************************************************************************/

#ifndef _DEFINEHEADER_H
#define _DEFINEHEADER_H

#define DEFAULT_DOWNLOAD_PATH "/tmp/update.img"
#define BLOCK_WRITE_LEN (16 * 1024)
#define MTD_SIZE 2048
#define SECTOR_SIZE 512

#define BYTE2SECTOR(x)         ((x>0)?((x-1)/SECTOR_SIZE + 1):(x))
#define PAGEALIGN(x)           ((x>0)?((x-1) / 4 + 1):(x))

typedef unsigned char BYTE;
typedef BYTE *PBYTE;
typedef unsigned short USHORT;
typedef unsigned int    UINT;
typedef unsigned int    DWORD;
typedef unsigned char UCHAR;
typedef unsigned short WCHAR;
typedef signed char CHAR;

typedef enum
{
    RKNONE_DEVICE=0,
    RK27_DEVICE=0x10,
    RKCAYMAN_DEVICE,
    RK28_DEVICE=0x20,
    RK281X_DEVICE,
    RKPANDA_DEVICE,
    RKNANO_DEVICE=0x30,
    RKSMART_DEVICE,
    RKCROWN_DEVICE=0x40,
    RK29_DEVICE=0x50,
    RK292X_DEVICE,
    RK30_DEVICE=0x60,
    RK30B_DEVICE,
    RK31_DEVICE=0x70,
    RK32_DEVICE=0x80
}ENUM_RKDEVICE_TYPE;

typedef enum
{
        ENTRY471=1,
        ENTRY472=2,
        ENTRYLOADER=4
}ENUM_RKBOOTENTRY;

#define SHA_DIGEST_SIZE 20
#define PART_NAME 32
#define RELATIVE_PATH 64
#define MAX_PARTS 20
#define MAX_MACHINE_MODEL 64
#define MAX_MANUFACTURER 60
#define MAX_PACKAGE_FILES 32
#define RKIMAGE_TAG 0x46414B52
#define IMAGE_RESERVED_SIZE 61
#define BOOT_RESERVED_SIZE 57
#define IDB_BLOCKS 5
#define IDBLOCK_TOP 50
#define CHIPINFO_LEN 16
#define RKANDROID_SEC2_RESERVED_LEN 473
#define RKDEVICE_SN_LEN 30
#define RKANDROID_SEC3_RESERVED_LEN 419
#define RKDEVICE_IMEI_LEN 15
#define RKDEVICE_UID_LEN 30
#define RKDEVICE_BT_LEN 6
#define RKDEVICE_MAC_LEN 6
#define SPARE_SIZE 16

#define GPT_BACKUP_FILE_NAME "gpt_backup.img"

#endif
