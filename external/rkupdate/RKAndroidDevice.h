#ifndef RKANDROIDDEVICE_HEADER
#define RKANDROIDDEVICE_HEADER
#include "RKDevice.h"
#include "gpt.h"
#pragma pack(1)
typedef struct
{
    DWORD   dwTag;
    BYTE    reserved[4];
    UINT    uiRc4Flag;
    USHORT  usBootCode1Offset;
    USHORT  usBootCode2Offset;
    BYTE    reserved1[490];
    USHORT  usBootDataSize;
    USHORT  usBootCodeSize;
    USHORT  usCrc;
} RKANDROID_IDB_SEC0, *PRKANDROID_IDB_SEC0;

typedef struct
{
    USHORT  usSysReservedBlock;
    USHORT  usDisk0Size;
    USHORT  usDisk1Size;
    USHORT  usDisk2Size;
    USHORT  usDisk3Size;
    UINT    uiChipTag;
    UINT    uiMachineId;
    USHORT  usLoaderYear;
    USHORT  usLoaderDate;
    USHORT  usLoaderVer;
    USHORT  usLastLoaderVer;
    USHORT  usReadWriteTimes;
    DWORD   dwFwVer;
    USHORT  usMachineInfoLen;
    UCHAR   ucMachineInfo[30];
    USHORT  usManufactoryInfoLen;
    UCHAR   ucManufactoryInfo[30];
    USHORT  usFlashInfoOffset;
    USHORT  usFlashInfoLen;
    UCHAR   reserved[384];
    UINT    uiFlashSize;//以sector为单位
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
} RKANDROID_IDB_SEC1, *PRKANDROID_IDB_SEC1;

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
} RKANDROID_IDB_SEC2, *PRKANDROID_IDB_SEC2;

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
} RKANDROID_IDB_SEC3, *PRKANDROID_IDB_SEC3;
typedef struct
{
    DWORD  dwTag;
    USHORT usSnSize;
    BYTE   btSnData[RKDEVICE_SN_LEN];
    BYTE   btReserve[RKANDROID_SEC3_RESERVED_LEN - 6];
    BYTE   btImeiSize;
    BYTE   btImeiData[RKDEVICE_IMEI_LEN];
    BYTE   btUidSize;
    BYTE   btUidData[RKDEVICE_UID_LEN];
    BYTE   btBlueToothSize;
    BYTE   btBlueToothData[RKDEVICE_BT_LEN];
    BYTE   btMacSize;
    BYTE   btMacData[RKDEVICE_MAC_LEN];
    USHORT usCrc;
    BYTE   btSpare[SPARE_SIZE];
} STRUCT_RKANDROID_WBBUFFER, *PSTRUCT_RKANDROID_WBBUFFER;
const BYTE Wipe_Data[] = {0x72, 0x65, 0x63, 0x6F, 0x76, 0x65,
                          0x72, 0x79, 0x0A, 0x2D, 0x2D, 0x77,
                          0x69, 0x70, 0x65, 0x5F, 0x64, 0x61, 0x74, 0x61, 0x00
                         };
const BYTE Wipe_All[] = {0x72, 0x65, 0x63, 0x6F, 0x76, 0x65,
                         0x72, 0x79, 0x0A, 0x2D, 0x2D, 0x77,
                         0x69, 0x70, 0x65, 0x5F, 0x61, 0x6C, 0x6C, 0x00
                        };
#define LBA_TRANSFER_SIZE       16*1024
#define LBA_LOOP_SIZE   1024*1024

#define MAX_PACKAGE_FILES           16
#define RKIMAGE_TAG             0x46414B52
#define PARTNAME_PACKAGE_FILE   "package-file"
#define PARTNAME_BOOTLOADER     "bootloader"
#define PARTNAME_PARAMETER      "parameter"
#define PARTNAME_KERNEL         "kernel"
#define PARTNAME_BOOT           "boot"
#define PARTNAME_RECOVERY       "recovery"
#define PARTNAME_UBOOT          "uboot"
#define PARTNAME_SYSTEM         "system"
#define PARTNAME_ROOTFS         "rootfs"
#define PARTNAME_MISC           "misc"
#define PARTNAME_BACKUP         "backup"
#define PARTNAME_OEM            "oem"
#define PARTNAME_USERDATA       "userdata"
#define PARTNAME_USER           "user"

#define INCLUDE_LOADER          "bootloader"

#define MAX_MANUFACTURER        60
#define MAX_MACHINE_INFO        30
#define MAX_MACHINE_MODEL       34
#define RELATIVE_PATH           60
#define PART_NAME               32

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])
typedef struct
{
    char name[PART_NAME];// 分区名称
    char file[RELATIVE_PATH];// 相对路径名，提取文件时用到
    unsigned int part_size;//分区占用扇区数
    unsigned int offset;// 文件在Image中的偏移
    unsigned int flash_offset;// 烧写到Flash中的位置(以sector为单位)
    unsigned int usespace;// 文件占用空间（按PAGE对齐)
    unsigned int size;// 字节数，实际文件大小
} STRUCT_RKIMAGE_ITEM, *PSTRUCT_RKIMAGE_ITEM;

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
} STRUCT_RKIMAGE_HDR, *PSTRUCT_RKIMAGE_HDR;
typedef struct
{
    UINT uiTag;
    UINT uiVer;
    UINT uiFlag;
    UINT uiHeadSize;
    UINT uiReserved[3];
    USHORT usHashBit;
    USHORT usRsaBit;
    BYTE nFactor[256];
    BYTE eFactor[256];
    BYTE cFactor[256];
    BYTE dataHash[32];
    UINT dataLoadAddr;
    BYTE codeHash[32];
    UINT codeLoadAddr;
    BYTE headSignValue[256];
} RK_SECURE_HEADER, *PRK_SECURE_HEADER;

typedef struct _RK_UPDATE_ORDER
{
    UINT uiOrder;
    char partName[32];
    STRUCT_RKIMAGE_ITEM stItem;
    long long ulItemSize;
} RK_UPDATE_ORDER;

#pragma pack()

class CRKAndroidDevice : public CRKDevice
{
public:
    static bool bGptFlag;
    virtual int PrepareIDB();
    virtual int DownloadIDBlock();
    virtual int DownloadImage();
    virtual int EraseIDB();
    virtual int EraseAllBlocks();
    virtual bool BufferWriteBack();
    CRKAndroidDevice(STRUCT_RKDEVICE_DESC &device);
    virtual ~CRKAndroidDevice();
    int UpgradePartition();
    bool GetPublicKey(unsigned char *pKey, unsigned int &nKeySize);
    bool IsExistBootloaderInFw();
    bool IsExistPartitionInFw(const char *partName, UINT &offset, UINT &size);
    UpgradeCallbackFunc m_pCallback;
    UpgradeProgressCallbackFunc m_pProcessCallback;
protected:
private:
    DWORD  m_dwLoaderSize;
    DWORD  m_dwLoaderDataSize;
    DWORD  m_dwLoaderHeadSize;
    DWORD  m_dwBackupOffset;
    char   m_oldIDBCounts;
    USHORT m_usFlashDataSec;
    USHORT m_usFlashBootSec;
    USHORT m_usFlashHeadSec;
    BYTE   *m_paramBuffer;
    BYTE *m_gptBuffer;
    UINT   m_uiParamFileSize;
    UINT   m_uiResevedBlockSize;
    RKANDROID_IDB_SEC0 *m_oldSec0;
    RKANDROID_IDB_SEC1 *m_oldSec1;
    RKANDROID_IDB_SEC2 *m_oldSec2;
    RKANDROID_IDB_SEC3 *m_oldSec3;
    UINT m_uiLBATimes;
    UINT m_uiUserSectors;

    bool GetLoaderSize();
    bool GetLoaderDataSize();
    bool GetLoaderHeadSize();
    bool GetOldSectorData();
    bool CalcIDBCount();
    bool IsExistSector3Crc(PRKANDROID_IDB_SEC2 pSec);

    virtual bool FindBackupBuffer();
    virtual CHAR FindIDBlock(char pos, char &IDBlockPos);
    virtual char FindAllIDB();
    virtual bool ReserveIDBlock(char iBlockIndex = 0, char iIdblockPos = 0);
    virtual bool OffsetIDBlock(char pos);
    virtual bool MakeSector0(PBYTE pSector);
    virtual void MakeSector1(PBYTE pSector);
    virtual bool MakeSector2(PBYTE pSector);
    virtual bool MakeSector3(PBYTE pSector);
    virtual int MakeIDBlockData(PBYTE lpIDBlock);
    virtual int MakeNewIDBlockData(PBYTE lpIDBlock);
    virtual bool  MakeSpareData(PBYTE lpIDBlock, DWORD dwSectorNum, PBYTE lpSpareBuffer);
    virtual int WriteIDBlock(PBYTE lpIDBlock, DWORD dwSectorNum, bool bErase);
    bool RKA_Param_Download(STRUCT_RKIMAGE_ITEM &entry, long long &currentByte, long long totalByte);
    bool RKA_Param_Check(STRUCT_RKIMAGE_ITEM &entry, long long &currentByte, long long totalByte);
    bool RKA_File_Download(STRUCT_RKIMAGE_ITEM &entry, long long &currentByte, long long totalByte);
    bool RKA_File_Check(STRUCT_RKIMAGE_ITEM &entry, long long &currentByte, long long totalByte);
    bool RKA_Gpt_Download(STRUCT_RKIMAGE_ITEM &entry, long long &currentByte, long long totalByte);
    bool RKA_Gpt_Check(STRUCT_RKIMAGE_ITEM &entry, long long &currentByte, long long totalByte);

    bool GetParameterPartSize(STRUCT_RKIMAGE_ITEM &paramItem);
    bool ParsePartitionInfo(string &strPartInfo, string &strName, UINT &uiOffset, UINT &uiLen);
    bool MakeParamFileBuffer(STRUCT_RKIMAGE_ITEM &entry);
    bool CheckParamPartSize(STRUCT_RKIMAGE_HDR &rkImageHead, int iParamPos);
    bool write_partition_upgrade_flag(DWORD dwOffset, BYTE *pMd5, UINT uiFlag);
    bool read_partition_upgrade_flag(DWORD dwOffset, BYTE *pMd5, UINT *uiFlag);
    bool GetParameterGptFlag(STRUCT_RKIMAGE_ITEM &paramItem);
};
void create_gpt_buffer(u8 *gpt, PARAM_ITEM_VECTOR &vecParts, CONFIG_ITEM_VECTOR &vecUuid, u64 diskSectors);
void prepare_gpt_backup(u8 *master, u8 *backup);
void gen_rand_uuid(unsigned char *uuid_bin);
unsigned int crc32_le(unsigned int crc, unsigned char *p, unsigned int len);
bool parse_parameter(char *pParameter, PARAM_ITEM_VECTOR &vecItem);
bool get_uuid_from_parameter(char *pParameter, CONFIG_ITEM_VECTOR &vecItem);
bool ParsePartitionInfo(string &strPartInfo, string &strName, UINT &uiOffset, UINT &uiLen);
bool ParseUuidInfo(string &strUuidInfo, string &strName, string &strUUid);
void string_to_uuid(string strUUid, char *uuid);
int find_uuid_item(CONFIG_ITEM_VECTOR &vecItems, char *pszName);
#endif
