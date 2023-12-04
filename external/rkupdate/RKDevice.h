#ifndef RKDEVICE_HEADER
#define RKDEVICE_HEADER
#include "RKImage.h"
#include "RKComm.h"
#include "RKLog.h"
#include "DefineHeader.h"



#define RK28_SEC3_RESERVED_LEN 419
#define RKANDROID_SEC3_RESERVED_LEN 419

#define RK28_SEC2_RESERVED_LEN 473
#define RKANDROID_SEC2_RESERVED_LEN 473


#define RKDEVICE_CUSTOMDATA_LEN 512
#define RKDEVICE_SN_LEN 30
#define RKDEVICE_UID_LEN 30
#define RKDEVICE_MAC_LEN 6
#define RKDEVICE_BT_LEN 6
#define RKDEVICE_IMEI_LEN 15

#define SECTOR_SIZE 512
#define SPARE_SIZE 16
#define IDB_BLOCKS 5
#define CHIPINFO_LEN 16
#define IDBLOCK_TOP 50
#define WBBUFFER_BOTTOM 5
#define WBBUFFER_TOP 10
#define CALC_UNIT(a, b)     ((a>0)?((a-1)/b+1):(a))
#define BYTE2SECTOR(x)      (CALC_UNIT(x, SECTOR_SIZE))
#define PAGEALIGN(x)        (CALC_UNIT(x, 4))
//#define MAX_TEST_BLOCKS       512
//#define MAX_ERASE_BLOCKS  128
#define MAX_WRITE_SECTOR    16
#define IOCTL_TRANSFER_SIZE     4096

#define CRC_TAG "CRC"
#pragma pack(1)
typedef struct _STRUCT_FLASH_INFO
{
    tchar   szManufacturerName[16];
    UINT    uiFlashSize;  //MB
    USHORT  usBlockSize;//KB
    UINT    uiPageSize;   //KB
    UINT    uiSectorPerBlock;
    BYTE    blockState[IDBLOCK_TOP];
    UINT    uiBlockNum;
    BYTE    bECCBits;
    BYTE    bAccessTime;  //两次访问Flash的间隔时间
    BYTE    bFlashCS;  // Flash片选(Flash片选存在置1，否则置0)
    USHORT  usValidSecPerBlock;//每块可以使用的扇区数=块大小/页大小*4
    USHORT  usPhyBlokcPerIDB;//每个IDBlock占用的物理块数量
    UINT    uiSecNumPerIDB;//每个IDBlock占用的扇区数
} STRUCT_FLASH_INFO, *PSTRUCT_FLASH_INFO;
typedef struct _STRUCT_FLASHINFO_CMD
{
    UINT    uiFlashSize;    // Flash大小（以Sector为单位）
    USHORT  usBlockSize;    // 物理的Block大小（以Sector为单位）
    BYTE    bPageSize;      // 物理的Page大小（以Sector为单位）
    BYTE    bECCBits;       // 8/14
    BYTE    bAccessTime;    // 两次访问Flash的间隔时间
    BYTE    bManufCode;     // 厂商识别码
    BYTE    bFlashCS;       // Flash片选(若Flash片选存在，则将相应的Bit置1，否则置0)
    BYTE    reserved[501];
} STRUCT_FLASHINFO_CMD, *PSTRUCT_FLASHINFO_CMD;
typedef struct
{
    BYTE bFlashCS;
    UINT uiBlockNum;
    USHORT usBlockStateSize;
    PBYTE pBlockStateData;
} STRUCT_BLOCK_STATE, *PSTRUCT_BLOCK_STATE;
#pragma pack()
typedef struct
{
    USHORT usVid;
    USHORT usPid;
} STRUCT_DEVICE_PROP;
typedef vector<STRUCT_DEVICE_PROP> DEVICE_PROP_SET;

class CRKDevice
{
public:

    ENUM_OS_TYPE GetOsType();
    void SetOsType(ENUM_OS_TYPE value);
    property<CRKDevice, ENUM_OS_TYPE, READ_WRITE> OsType;

    void SetUid(PBYTE value);
    property<CRKDevice, PBYTE, WRITE_ONLY> Uid;

    void SetPrepareEraseFlag(bool value);
    property<CRKDevice, bool, WRITE_ONLY> PrepareEraseFlag;

    void SetWorkFlow(UINT value);
    property<CRKDevice, UINT, WRITE_ONLY> WorkFlow;

    void SetMiscModifyFlag(ENUM_MISC_MODIFY_FLAG value);
    property<CRKDevice, ENUM_MISC_MODIFY_FLAG, WRITE_ONLY> MiscModifyFlag;

    CRKLog *GetLogObjectPointer();
    property<CRKDevice, CRKLog *, READ_ONLY> LogObjectPointer;

    CRKComm *GetCommObjectPointer();
    property<CRKDevice, CRKComm *, READ_ONLY> CommObjectPointer;

    virtual int DownloadIDBlock() = 0;
    virtual int DownloadImage() = 0;
    virtual int PrepareIDB() = 0;
    virtual bool BufferWriteBack() = 0;


    bool CheckChip();
    bool GetFlashInfo();
    virtual int EraseIDB() = 0;
    virtual int EraseAllBlocks() = 0;
    bool SetObject(CRKImage *pImage, CRKComm *pComm, CRKLog *pLog);
    CRKDevice(STRUCT_RKDEVICE_DESC &device);
    virtual ~CRKDevice();
protected:
    STRUCT_FLASH_INFO m_flashInfo;
    PBYTE   m_pFlashInfoData;
    USHORT  m_usFlashInfoDataOffset;
    USHORT  m_usFlashInfoDataLen;
    DWORD m_idBlockOffset[IDB_BLOCKS];
    PBYTE  m_chipData;
    CRKImage *m_pImage;
    CRKComm  *m_pComm;
    CRKLog   *m_pLog;
    PBYTE  m_customData;
    USHORT m_customDataSize;
    USHORT m_customDataOffset;
    PBYTE  m_sn;
    BYTE   m_snSize;
    PBYTE  m_mac;
    PBYTE  m_blueTooth;
    PBYTE  m_uid;
    PBYTE  m_imei;
    USHORT m_sysDiskSize;
    USHORT m_cfgDiskSize;
    bool   m_bGetNewDiskSizeFlag;
    bool   m_bExistSector3Crc;
    USHORT m_usSector3Crc;
    USHORT m_usWriteBackCrc;
    USHORT m_usWriteBackCustomDataOffset;
    USHORT m_usWriteBackCustomDataSize;
    PSTRUCT_BLOCK_STATE m_pBlockState;
    BYTE   m_backupBuffer[SECTOR_SIZE + SPARE_SIZE];
    bool m_bWriteBack;
    UINT m_uiWorkFlow;
    bool m_bEraseInPrepare;
    bool m_bUidUseFlag;
    bool m_bUidWriteOK;
    bool  m_remallocDisk;
    bool m_bEmmc;
    bool GptFlag;
    bool DirectLBA;
    bool First4Access;
    ENUM_MISC_MODIFY_FLAG m_emMiscModifyFlag;
    bool m_bQuickCheckMode;
    bool BuildBlockStateMap(BYTE bFlashCS);
    int ReadMutilSector(DWORD dwPos, DWORD dwCount, PBYTE lpBuffer);
    bool EraseMutilBlock(BYTE bFlashCS, DWORD dwPos, DWORD dwCount, bool bForce);
    CHAR FindValidBlocks(char bBegin, char bLen);
    BYTE RandomByte(BYTE bLowLimit, BYTE bHighLimit);
    bool CheckCrc16(PBYTE pCheckData, USHORT usDataLength, USHORT usOldCrc);
    bool CheckUid(BYTE uidSize, BYTE *pUid);
    bool GetWriteBackData(UINT uiIDBCount, PBYTE lpBuf);
    bool GetIDBData(UINT uiIDBCount, PBYTE lpBuf, UINT uiSecCount);
    int EraseEmmcBlock(UCHAR ucFlashCS, DWORD dwPos, DWORD dwCount);
    virtual bool FindBackupBuffer() = 0;
    virtual char FindAllIDB() = 0;
    virtual CHAR FindIDBlock(char pos, char &IDBlockPos) = 0;
    virtual bool ReserveIDBlock(char iBlockIndex = 0, char iIdblockPos = 0) = 0;
    virtual bool OffsetIDBlock(char pos) = 0;
    virtual bool MakeSector0(PBYTE pSector) = 0;
    virtual void MakeSector1(PBYTE pSector) = 0;
    virtual bool MakeSector2(PBYTE pSector) = 0;
    virtual bool MakeSector3(PBYTE pSector) = 0;
    virtual int MakeIDBlockData(PBYTE lpIDBlock) = 0;
    virtual bool MakeSpareData(PBYTE lpIDBlock, DWORD dwSectorNum, PBYTE lpSpareBuffer) = 0;
    virtual int WriteIDBlock(PBYTE lpIDBlock, DWORD dwSectorNum, bool bErase) = 0;

private:
    ENUM_RKDEVICE_TYPE m_device;
    ENUM_OS_TYPE m_os;
    ENUM_RKUSB_TYPE m_usb;
    USHORT m_bcdUsb;
};

USHORT UshortToBCD(USHORT num);
BYTE   ByteToBCD(BYTE num);
extern USHORT CRC_16(BYTE *aData, UINT aSize);
extern UINT CRC_32(PBYTE pData, UINT ulSize, UINT uiPreviousValue = 0);
extern void P_RC4(BYTE *buf, USHORT len);
extern void bch_encode(BYTE *encode_in, BYTE *encode_out);
extern USHORT CRC_CCITT(UCHAR *p, UINT CalculateNumber);
extern void generate_gf();
extern void gen_poly();
#endif
