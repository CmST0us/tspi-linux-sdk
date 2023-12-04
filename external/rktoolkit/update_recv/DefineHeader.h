#ifndef DEFINE_HEADER
#define DEFINE_HEADER

typedef unsigned int u_int32;
typedef unsigned char u_int8;
typedef unsigned char BYTE;
typedef signed char CHAR;
typedef BYTE *PBYTE;
typedef unsigned char UCHAR;
typedef unsigned short WCHAR;
typedef unsigned short USHORT;
typedef unsigned int	UINT;
typedef unsigned int	DWORD;

typedef char tchar;

#define LBA_TRANSFER_SIZE		16*1024
#define LBA_LOOP_SIZE			1024*1024

#define MAX_PACKAGE_FILES		16
#define RKIMAGE_TAG				0x46414B52
#define PARTNAME_BOOTLOADER		"bootloader"
#define PARTNAME_PARAMETER		"parameter"
#define PARTNAME_KERNEL			"kernel"
#define PARTNAME_BOOT			"boot"
#define PARTNAME_RECOVERY		"recovery"
#define PARTNAME_SYSTEM			"system"
#define PARTNAME_MISC			"misc"
#define PARTNAME_BACKUP			"backup"
#define PARTNAME_USERDATA		"userdata"
#define PARTNAME_USER			"user"

#define MAX_MANUFACTURER		60
#define MAX_MACHINE_INFO		30
#define MAX_MACHINE_MODEL		34
#define RELATIVE_PATH			60
#define PART_NAME				32
#define  IMAGE_RESERVED_SIZE	61

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
	RK_OS=0,
	ANDROID_OS=0x1
}ENUM_OS_TYPE;

#pragma pack(1)
typedef struct
{
	char name[PART_NAME];// 分区名称
	char file[RELATIVE_PATH];// 相对路径名，提取文件时用到
	unsigned int part_size;//分区占用扇区数
	unsigned int offset;// 文件在Image中的偏移
	unsigned int flash_offset;// 烧写到Flash中的位置(以sector为单位)
	unsigned int usespace;// 文件占用空间（按PAGE对齐)
	unsigned int size;// 字节数，实际文件大小
}STRUCT_RKIMAGE_ITEM,*PSTRUCT_RKIMAGE_ITEM;

typedef struct
{
	USHORT	usYear;
	BYTE	ucMonth;
	BYTE  	ucDay;
	BYTE  	ucHour;
	BYTE  	ucMinute;
	BYTE  	ucSecond;
}STRUCT_RKTIME,*PSTRUCT_RKTIME;

typedef struct
{
	char szItemName[20];
	UINT uiItemOffset;
	UINT uiItemSize;
}STRUCT_PARAM_ITEM,*PSTRUCT_PARAM_ITEM;

typedef struct
{
	char szItemName[20];
	char szItemValue[256];
}STRUCT_CONFIG_ITEM,*PSTRUCT_CONFIG_ITEM;

typedef struct
{
	UINT uiTag;
	USHORT usSize;
	DWORD  dwVersion;
	DWORD  dwMergeVersion;
	STRUCT_RKTIME stReleaseTime;
	ENUM_RKDEVICE_TYPE emSupportChip;
	DWORD  dwBootOffset;
	DWORD  dwBootSize;
	DWORD  dwFWOffset;
	DWORD  dwFWSize;
	BYTE   reserved[IMAGE_RESERVED_SIZE];
}STRUCT_RKIMAGE_HEAD,*PSTRUCT_RKIMAGE_HEAD;

typedef struct tagRKIMAGE_HDR
{
	unsigned int tag;
	unsigned int size;// 文件大小，不含末尾的CRC校验码
	char machine_model[MAX_MACHINE_MODEL];
	char machine_info[MAX_MACHINE_INFO];
	char manufacturer[MAX_MANUFACTURER];
	unsigned int dwFWVer;
	int item_count;
	STRUCT_RKIMAGE_ITEM item[MAX_PACKAGE_FILES];
}STRUCT_RKIMAGE_HDR,*PSTRUCT_RKIMAGE_HDR;

#pragma pack()

#endif
