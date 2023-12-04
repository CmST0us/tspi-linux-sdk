/*************************************************************************
    > File Name: rkimage.h
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Fri 17 May 2019 02:32:13 PM CST
 ************************************************************************/

#ifndef _RKIMAGE_H
#define _RKIMAGE_H
#include "defineHeader.h"
#pragma pack(1)
typedef struct
{
    USHORT  usYear;
    BYTE    ucMonth;
    BYTE    ucDay;
    BYTE    ucHour;
    BYTE    ucMinute;
    BYTE    ucSecond;
}STRUCT_RKTIME, *PSTRUCT_RKTIME;

typedef struct
{
    UINT uiTag;     //标志，固定为0x57 0x46 0x4B 0x52
    USHORT usSize;  //结构体大小
    DWORD  dwVersion;   //Image 文件版本
    DWORD  dwMergeVersion;  //打包工具版本
    STRUCT_RKTIME stReleaseTime;    //生成时间
    ENUM_RKDEVICE_TYPE emSupportChip;   //使用芯片
    DWORD  dwBootOffset;    //Boot偏移
    DWORD  dwBootSize;  //Boot大小
    DWORD  dwFWOffset;  //固件偏移
    DWORD  dwFWSize;    //固件大小
    BYTE   reserved[61];    //预留空间，用于存放不同固件特征
}STRUCT_RKIMAGE_HEAD,*PSTRUCT_RKIMAGE_HEAD;

typedef struct tagRKIMAGE_ITEM
{
    char name[PART_NAME];
    char file[RELATIVE_PATH];
    unsigned int offset;
    unsigned int flash_offset;
    unsigned int usespace;
    unsigned int size;
}RKIMAGE_ITEM, *PRKIMAGE_ITEM;

typedef struct tagRKIMAGE_HDR
{
    unsigned int tag;
    unsigned int size;
    char machine_model[MAX_MACHINE_MODEL];
    char manufacturer[MAX_MANUFACTURER];
    unsigned int version;
    int item_count;
    RKIMAGE_ITEM item[MAX_PACKAGE_FILES];
}RKIMAGE_HDR, *PRKIMAGE_HDR;

typedef struct
{
    UINT uiTag;
    USHORT usSize;
    DWORD  dwVersion;
    DWORD  dwMergeVersion;
    STRUCT_RKTIME stReleaseTime;
    ENUM_RKDEVICE_TYPE emSupportChip;
    UCHAR uc471EntryCount;
    DWORD dw471EntryOffset;
    UCHAR uc471EntrySize;
    UCHAR uc472EntryCount;
    DWORD dw472EntryOffset;
    UCHAR uc472EntrySize;
    UCHAR ucLoaderEntryCount;
    DWORD dwLoaderEntryOffset;
    UCHAR ucLoaderEntrySize;
    UCHAR ucSignFlag;
    UCHAR ucRc4Flag;
    UCHAR reserved[BOOT_RESERVED_SIZE];
}STRUCT_RKBOOT_HEAD,*PSTRUCT_RKBOOT_HEAD;

typedef struct
{
    UCHAR ucSize;
    ENUM_RKBOOTENTRY emType;
    WCHAR szName[20];
    DWORD dwDataOffset;
    DWORD dwDataSize;
    DWORD dwDataDelay;//以秒为单位
}STRUCT_RKBOOT_ENTRY,*PSTRUCT_RKBOOT_ENTRY;
typedef    struct
{
    DWORD    dwTag;
    BYTE    reserved[4];
    UINT    uiRc4Flag;
    USHORT    usBootCode1Offset;
    USHORT    usBootCode2Offset;
    BYTE    reserved1[490];
    USHORT  usBootDataSize;
    USHORT    usBootCodeSize;
    USHORT    usCrc;
}RKANDROID_IDB_SEC0,*PRKANDROID_IDB_SEC0;

typedef struct
{
    USHORT  usSysReservedBlock;
    USHORT  usDisk0Size;
    USHORT  usDisk1Size;
    USHORT  usDisk2Size;
    USHORT  usDisk3Size;
    UINT    uiChipTag;
    UINT    uiMachineId;
    USHORT    usLoaderYear;
    USHORT    usLoaderDate;
    USHORT    usLoaderVer;
    USHORT  usLastLoaderVer;
    USHORT  usReadWriteTimes;
    DWORD    dwFwVer;
    USHORT  usMachineInfoLen;
    UCHAR    ucMachineInfo[30];
    USHORT    usManufactoryInfoLen;
    UCHAR    ucManufactoryInfo[30];
    USHORT    usFlashInfoOffset;
    USHORT    usFlashInfoLen;
    UCHAR    reserved[384];
    UINT    uiFlashSize;
    BYTE    reserved1;
    BYTE    bAccessTime;
    USHORT  usBlockSize;
    BYTE    bPageSize;
    BYTE    bECCBits;
    BYTE    reserved2[8];
    USHORT  usIdBlock0;
    USHORT  usIdBlock1;
    USHORT  usIdBlock2;
    USHORT  usIdBlock3;
    USHORT  usIdBlock4;
}RKANDROID_IDB_SEC1,*PRKANDROID_IDB_SEC1;

typedef struct
{
    USHORT  usInfoSize;
    BYTE    bChipInfo[CHIPINFO_LEN];
    BYTE    reserved[RKANDROID_SEC2_RESERVED_LEN];
    char    szVcTag[3];
    USHORT  usSec0Crc;
    USHORT  usSec1Crc;
    UINT    uiBootCodeCrc;
    USHORT  usSec3CustomDataOffset;
    USHORT  usSec3CustomDataSize;
    char    szCrcTag[4];
    USHORT  usSec3Crc;
}RKANDROID_IDB_SEC2,*PRKANDROID_IDB_SEC2;

typedef struct
{
    USHORT  usSNSize;
    BYTE    sn[RKDEVICE_SN_LEN];
    BYTE    reserved[RKANDROID_SEC3_RESERVED_LEN];
    BYTE    imeiSize;
    BYTE    imei[RKDEVICE_IMEI_LEN];
    BYTE    uidSize;
    BYTE    uid[RKDEVICE_UID_LEN];
    BYTE    blueToothSize;
    BYTE    blueToothAddr[RKDEVICE_BT_LEN];
    BYTE    macSize;
    BYTE    macAddr[RKDEVICE_MAC_LEN];
}RKANDROID_IDB_SEC3,*PRKANDROID_IDB_SEC3;

#pragma pack()

bool getImageVersion(const char *filepath, char *version, int maxLength) ;
int analyticImage(const char *filepath, PRKIMAGE_HDR phdr);
#endif
