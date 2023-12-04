#ifndef DEFINE_HEADER
#define DEFINE_HEADER
#ifndef _U
    #define _U _CTYPE_U
#endif
#ifndef _L
    #define _L _CTYPE_L
#endif
#ifndef _N
    #define _N _CTYPE_L
#endif
#ifndef _X
    #define _X _CTYPE_X
#endif
#ifndef _P
    #define _P _CTYPE_P
#endif
#ifndef _B
    #define _B _CTYPE_B
#endif
#ifndef _C
    #define _C _CTYPE_C
#endif
#ifndef _S
    #define _S _CTYPE_S
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
//#include <iconv.h>
#include <wchar.h>
#include <errno.h>
#include <ctype.h>

#include "Property.hpp"
#include <list>
#include <vector>
//#include <set>
#include <string>
#include <sstream>
#include <algorithm>
using namespace std;

typedef unsigned int u_int32;
typedef unsigned char u_int8;
typedef unsigned char BYTE;
typedef signed char CHAR;
typedef BYTE *PBYTE;
typedef unsigned char UCHAR;
typedef unsigned short WCHAR;
typedef unsigned short USHORT;
typedef unsigned int    UINT;
typedef unsigned int    DWORD;
//#ifdef _UNICODE
//    typedef wchar_t tchar;
//    typedef wstring tstring;
//    #define _T(x) L ## x
//    #define _tcslen  wcslen
//    #define _tcscpy wcscpy
//    #define _tcsicmp wcscmp
//    #define _tcscmp  wcscmp
//    #define _stprintf swprintf
//    #define _istprint iswprint
//#else
typedef char tchar;
typedef string tstring;
#define _T(x) x
#define _tcslen  strlen
#define _tcscpy strcpy
#define _tcsicmp strcasecmp
#define _tcscmp  strcmp
#define _stprintf sprintf
#define _istprint isprint
//#endif
typedef enum
{
    RKNONE_DEVICE = 0,
    RK27_DEVICE = 0x10,
    RKCAYMAN_DEVICE,
    RK28_DEVICE = 0x20,
    RK281X_DEVICE,
    RKPANDA_DEVICE,
    RKNANO_DEVICE = 0x30,
    RKSMART_DEVICE,
    RKCROWN_DEVICE = 0x40,
    RK29_DEVICE = 0x50,
    RK292X_DEVICE,
    RK30_DEVICE = 0x60,
    RK30B_DEVICE,
    RK31_DEVICE = 0x70,
    RK32_DEVICE = 0x80
} ENUM_RKDEVICE_TYPE;
typedef enum
{
    RK_OS = 0,
    ANDROID_OS = 0x1
} ENUM_OS_TYPE;

typedef enum
{
    RKUSB_NONE = 0x0,
    RKUSB_MASKROM = 0x01,
    RKUSB_LOADER = 0x02,
    RKUSB_MSC = 0x04
} ENUM_RKUSB_TYPE;
typedef enum
{
    ENTRY471 = 1,
    ENTRY472 = 2,
    ENTRYLOADER = 4
} ENUM_RKBOOTENTRY;
#define  MSC_ANDROID_OPER 0xFF
typedef enum
{
    MSC_NONE_OPER = 0x0,
    MSC_FORMAT_OPER = 0x1,
    MSC_COPY_OPER = 0x2,
    MSC_FORMAT_DATA_OPER = 0x4,
    MSC_COPY_DATA_OPER = 0x8
} ENUM_MSC_OPER;
typedef enum
{
    MISC_MODIFY_NONE = 0,
    MISC_MODIFY_WIPE_ALL,
    MISC_MODIFY_WIPE_DATA,
} ENUM_MISC_MODIFY_FLAG;
typedef enum
{
    WF_UPGRADE = 1,
    WF_RESTORE,
    WF_GETOLDDISKSIZE,
    WF_READSN,
    WF_WRITESN,
    WF_ERASEFLASH,
    WF_ERASEIDB,
    WF_GETBLOCKSTATE,
    WF_READMAC,
    WF_WRITEMAC,
    WF_READBT,
    WF_WRITEBT,
    WF_READIMEI,
    WF_WRITEIMEI,
    WF_READUID,
    WF_READCUSTOMDATA,
    WF_WRITECUSTOMDATA,
    WF_READALLINFO,
    WF_WRITEALLINFO,
    WF_DOWNLOADBOOT
} ENUM_WORKFLOW;
#pragma pack(1)
typedef struct
{
    USHORT  usYear;
    BYTE    ucMonth;
    BYTE    ucDay;
    BYTE    ucHour;
    BYTE    ucMinute;
    BYTE    ucSecond;
} STRUCT_RKTIME, *PSTRUCT_RKTIME;
typedef struct
{
    char szItemName[20];
    UINT uiItemOffset;
    UINT uiItemSize;
} STRUCT_PARAM_ITEM, *PSTRUCT_PARAM_ITEM;
typedef struct
{
    char szItemName[20];
    char szItemValue[256];
} STRUCT_CONFIG_ITEM, *PSTRUCT_CONFIG_ITEM;
typedef struct _STRUCT_RKDEVICE_DESC
{
    USHORT usVid;
    USHORT usPid;
    USHORT usbcdUsb;
    UINT     uiLocationID;
    ENUM_RKUSB_TYPE emUsbType;
    ENUM_RKDEVICE_TYPE emDeviceType;
    void   *pUsbHandle;
} STRUCT_RKDEVICE_DESC, *PSTRUCT_RKDEVICE_DESC;
#pragma pack()
typedef list<STRUCT_RKDEVICE_DESC> RKDEVICE_DESC_SET;
typedef RKDEVICE_DESC_SET::iterator device_list_iter;
typedef vector<tstring> STRING_VECTOR;
typedef vector<UINT> UINT_VECTOR;
typedef vector<STRUCT_PARAM_ITEM> PARAM_ITEM_VECTOR;
typedef vector<STRUCT_CONFIG_ITEM> CONFIG_ITEM_VECTOR;
//typedef enum
//{
//      DOWNLOADBOOT_START=1,
//      DOWNLOADBOOT_FAIL=2,
//      DOWNLOADBOOT_PASS=3,
//      DOWNLOADIDBLOCK_START=4,
//      DOWNLOADIDBLOCK_FAIL=5,
//      DOWNLOADIDBLOCK_PASS=6,
//      DOWNLOADIMAGE_START=7,
//      DOWNLOADIMAGE_FAIL=8,
//      DOWNLOADIMAGE_PASS=9,
//      TESTDEVICE_START=10,
//      TESTDEVICE_FAIL=11,
//      TESTDEVICE_PASS=12,
//      RESETDEVICE_START=13,
//      RESETDEVICE_FAIL=14,
//      RESETDEVICE_PASS=15,
//      FORMATDISK_START=16,
//      FORMATDISK_FAIL=17,
//      FORMATDISK_PASS=18,
//      COPYDATA_START=19,
//      COPYDATA_FAIL=20,
//      COPYDATA_PASS=21,
//      WAITMSC_START=22,
//      WAITMSC_FAIL=23,
//      WAITMSC_PASS=24,
//      WAITLOADER_START=25,
//      WAITLOADER_FAIL=26,
//      WAITLOADER_PASS=27,
//      WAITMASKROM_START=28,
//      WAITMASKROM_FAIL=29,
//      WAITMASKROM_PASS=30,
//      ERASEIDB_START=31,
//      ERASEIDB_FAIL=32,
//      ERASEIDB_PASS=33,
//      SWITCHMSC_START=34,
//      SWITCHMSC_FAIL=35,
//      SWITCHMSC_PASS=36,
//      CHECKCHIP_START=37,
//      CHECKCHIP_FAIL=38,
//      CHECKCHIP_PASS=39,
//      PREPAREIDB_START=40,
//      PREPAREIDB_FAIL=41,
//      PREPAREIDB_PASS=42,
//      MUTEXRESETDEVICE_START=43,
//      MUTEXRESETDEVICE_FAIL=44,
//      MUTEXRESETDEVICE_PASS=45,
//      GETOLDDISKSIZE_START=46,
//      GETOLDDISKSIZE_FAIL=47,
//      GETOLDDISKSIZE_PASS=48,
//      READSN_START=49,
//      READSN_FAIL=50,
//      READSN_PASS=51,
//      WRITESN_START=52,
//      WRITESN_FAIL=53,
//      WRITESN_PASS=54,
//      ERASEALLBLOCKS_START=55,
//      ERASEALLBLOCKS_FAIL=56,
//      ERASEALLBLOCKS_PASS=57,
//      GETBLOCKSTATE_START=58,
//      GETBLOCKSTATE_FAIL=59,
//      GETBLOCKSTATE_PASS=60,
//      GETFLASHINFO_START=61,
//      GETFLASHINFO_FAIL=62,
//      GETFLASHINFO_PASS=63,
//      WRITEBACK_START=64,
//      WRITEBACK_FAIL=65,
//      WRITEBACK_PASS=66,
//      FINDUSERDISK_START=67,
//      FINDUSERDISK_FAIL=68,
//      FINDUSERDISK_PASS=69,
//      SHOWUSERDISK_START=70,
//      SHOWUSERDISK_FAIL=71,
//      SHOWUSERDISK_PASS=72,
//      READMAC_START=73,
//      READMAC_FAIL=74,
//      READMAC_PASS=75,
//      WRITEMAC_START=76,
//      WRITEMAC_FAIL=77,
//      WRITEMAC_PASS=78,
//      READBT_START=79,
//      READBT_FAIL=80,
//      READBT_PASS=81,
//      WRITEBT_START=82,
//      WRITEBT_FAIL=83,
//      WRITEBT_PASS=84,
//      LOWERFORMAT_START=85,
//      LOWERFORMAT_FAIL=86,
//      LOWERFORMAT_PASS=87,
//      READIMEI_START=88,
//      READIMEI_FAIL=89,
//      READIMEI_PASS=90,
//      WRITEIMEI_START=91,
//      WRITEIMEI_FAIL=92,
//      WRITEIMEI_PASS=93,
//      SHOWDATADISK_START=94,
//      SHOWDATADISK_FAIL=95,
//      SHOWDATADISK_PASS=96,
//      FINDDATADISK_START=97,
//      FINDDATADISK_FAIL=98,
//      FINDDATADISK_PASS=99,
//      FORMATDATADISK_START=100,
//      FORMATDATADISK_FAIL=101,
//      FORMATDATADISK_PASS=102,
//      COPYDATADISK_START=103,
//      COPYDATADISK_FAIL=104,
//      COPYDATADISK_PASS=105,
//      READUID_START=106,
//      READUID_FAIL=107,
//      READUID_PASS=108,
//      READCUSTOMDATA_START=109,
//      READCUSTOMDATA_FAIL=110,
//      READCUSTOMDATA_PASS=111,
//      WRITECUSTOMDATA_START=112,
//      WRITECUSTOMDATA_FAIL=113,
//      WRITECUSTOMDATA_PASS=114,
//      SETRESETFLAG_START=115,
//      SETRESETFLAG_FAIL=116,
//      SETRESETFLAG_PASS=117,
//      POWEROFF_START=118,
//      POWEROFF_FAIL=119,
//      POWEROFF_PASS=120,
//      READALLINFO_START=121,
//      READALLINFO_FAIL=122,
//      READALLINFO_PASS=123,
//      WRITEALLINFO_START=124,
//      WRITEALLINFO_FAIL=125,
//      WRITEALLINFO_PASS=126,
//      RESETMSC_START=127,
//      RESETMSC_FAIL=128,
//      RESETMSC_PASS=129
//}ENUM_UPGRADE_PROMPT;
//typedef enum
//{
//  TESTDEVICE_PROGRESS,
//  DOWNLOADIMAGE_PROGRESS,
//  CHECKIMAGE_PROGRESS,
//  TAGBADBLOCK_PROGRESS,
//  TESTBLOCK_PROGRESS,
//  ERASEFLASH_PROGRESS,
//  ERASESYSTEM_PROGRESS,
//  LOWERFORMAT_PROGRESS,
//  ERASEUSERDATA_PROGRESS
//}ENUM_PROGRESS_PROMPT;
//#define   MSC_SWITCHROCKUSB   0xFFFFFFFE
//#define MSC_GETVERSIONINFO    0xFFFFFFFF
//#define MSC_RESETDEVICE       0xFFFFFFFD
//#define MSC_GETCHIPINFO       0xFFFFFFFC
//#define MSC_SHOWUSERDISK  0xFFFFFFFB
//#define MSC_GETDEVIVEUID  0xFFFFFFF7
//#define MSC_SHOWDATADISK  0xFFFFFFF6
//#define MSC_GETPRODUCTMODEL 0xFFFFFFF3
//#define MSC_GETPARAMETER  0xFFFFFFF2
//#define MSC_GETIDBSECTOR  0xFFFFFFF1
//#define MSC_GETPRODUCTSN  0xFFFFFFEF
//typedef enum
//{
//  CALL_FIRST,
//  CALL_MIDDLE,
//  CALL_LAST
//}ENUM_CALL_STEP;

//typedef void (*UpgradeStepPromptCB)(DWORD deviceLayer,ENUM_UPGRADE_PROMPT promptID,DWORD oldDeviceLayer);
//typedef void (*ProgressPromptCB)(DWORD deviceLayer,ENUM_PROGRESS_PROMPT promptID,long long totalValue,long long currentValue,ENUM_CALL_STEP emCall);

//bool WideStringToString(wchar_t *pszSrc,char *&pszDest);
//bool StringToWideString(char *pszSrc,wchar_t *&pszDest);
//bool transform(string &src,bool lowercase);
typedef void (*UpgradeCallbackFunc)(char *pszPrompt);
typedef void (*UpgradeProgressCallbackFunc)(float portion, float seconds);
#endif
