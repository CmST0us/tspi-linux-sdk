#ifndef RKCOMM_HEADER
#define RKCOMM_HEADER
#include "DefineHeader.h"
typedef enum
{
    USB_BULK_READ = 0,
    USB_BULK_WRITE,
    USB_CONTROL,
} USB_ACCESS_TYPE;
typedef enum
{
    TU_NONE_SUBCODE = 0,
    TU_ERASESYSTEM_SUBCODE = 0xFE,
    TU_LOWERFORMAT_SUBCODE = 0xFD,
    TU_ERASEUSERDATA_SUBCODE = 0xFB,
    TU_GETUSERSECTOR_SUBCODE = 0xF9
} TESTUNIT_SUBCODE;
typedef enum
{
    RST_NONE_SUBCODE = 0,
    RST_RESETMSC_SUBCODE,
    RST_POWEROFF_SUBCODE,
    RST_RESETMASKROM_SUBCODE,
    RST_DISCONNECTRESET_SUBCODE
} RESET_SUBCODE;
typedef enum
{
    RWMETHOD_IMAGE = 0,
    RWMETHOD_LBA,
} RW_SUBCODE;
/* √¸¡Ó */
typedef enum
{
    TEST_UNIT_READY = 0,
    READ_FLASH_ID = 0x01,
    TEST_BAD_BLOCK = 0x03,
    READ_SECTOR = 0x04,
    WRITE_SECTOR = 0x05,
    ERASE_NORMAL = 0x06,
    ERASE_FORCE = 0x0B,
    READ_LBA = 0x14,
    WRITE_LBA = 0x15,
    ERASE_SYSTEMDISK = 0x16,
    READ_SDRAM = 0x17,
    WRITE_SDRAM = 0x18,
    EXECUTE_SDRAM = 0x19,
    READ_FLASH_INFO = 0x1A,
    READ_CHIP_INFO = 0x1B,
    SET_RESET_FLAG = 0x1E,
    WRITE_EFUSE = 0x1F,
    READ_EFUSE = 0x20,
    READ_SPI_FLASH = 0x21,
    WRITE_SPI_FLASH = 0x22,
    SWITCH_MASKROM = 0xFE,
    DEVICE_RESET = 0xFF
} USB_OPERATION_CODE;

#pragma pack(1)

typedef struct
{
    BYTE    ucOperCode;
    BYTE    ucReserved;
    DWORD   dwAddress;
    BYTE    ucReserved2;
    USHORT  usLength;
    BYTE    ucReserved3[7];
} CBWCB, *PCBWCB;

typedef struct
{
    DWORD   dwCBWSignature;
    DWORD   dwCBWTag;
    DWORD   dwCBWTransferLength;
    BYTE    ucCBWFlags;
    BYTE    ucCBWLUN;
    BYTE    ucCBWCBLength;
    CBWCB   cbwcb;
} CBW, *PCBW;

typedef struct
{
    DWORD   dwCSWSignature;
    DWORD   dwCSWTag;
    DWORD   dwCBWDataResidue;
    BYTE    ucCSWStatus;
} CSW, *PCSW;
typedef struct
{
    UINT    uiSize;
    UINT    uiCrc;
    UINT    uiBlock[5];
} STRUCT_END_WRITE_SECTOR, *PSTRUCT_END_WRITE_SECTOR;

#pragma pack()
#define NAND_DRIVER_DEV "/dev/rknand_sys_storage"
#define NAND_DRIVER_DEV_VENDOR "/dev/rknand_sys_storage"
#define NAND_DRIVER_DEV_LBA "/dev/rkflash0"
#define EMMC_DRIVER_DEV "/dev/rknand_sys_storage"
#define EMMC_DRIVER_DEV_VENDOR "/dev/vendor_storage"
//#define EMMC_DRIVER_DEV_LBA "/dev/block/mmcblk0"
#define EMMC_DRIVER_DEV_LBA "/dev/mmcblk0"
#define EMMC_POINT_NAME "emmc_point_name"

#define READ_SECTOR_IO          _IOW('r', READ_SECTOR, unsigned int)
#define WRITE_SECTOR_IO         _IOW('r', WRITE_SECTOR, unsigned int)
#define READ_LBA_IO             _IOW('r', READ_LBA, unsigned int)
#define WRITE_LBA_IO            _IOW('r', WRITE_LBA, unsigned int)
#define START_WRITE_SECTOR_IO   _IOW('r', 0x51, unsigned int)
#define END_WRITE_SECTOR_IO     _IOW('r', 0x52, unsigned int)
#define GET_FLASH_INFO_IO       _IOW('r', READ_FLASH_INFO, unsigned int)
#define GET_BAD_BLOCK_IO        _IOW('r', TEST_BAD_BLOCK, unsigned int)
#define GET_LOCK_FLAG_IO        _IOW('r', 0x53, unsigned int)
#define GET_PUBLIC_KEY_IO       _IOW('r', 0x54, unsigned int)

#define DISABLE_NAND_LBA_WRITE_IO  _IO('V',0)
#define ENABLE_NAND_LBA_WRITE_IO   _IO('V',1)
#define DISABLE_NAND_LBA_READ_IO   _IO('V',2)
#define ENABLE_NAND_LBA_READ_IO    _IO('V',3)


#define CMD_TIMEOUT 0

#define CBW_SIGN            0x43425355  /* "USBC" */
#define CSW_SIGN            0x53425355  /* "USBS" */


#define DIRECTION_OUT       0x00
#define DIRECTION_IN        0x80
#define MAX_TEST_BLOCKS     512
#define MAX_ERASE_BLOCKS    128
#define  MAX_CLEAR_LEN  16*1024

#ifndef ERR_SUCCESS
    #define ERR_SUCCESS     0
#endif
#define ERR_DEVICE_READY        0
#define ERR_DEVICE_OPEN_FAILED  -1
#define ERR_CSW_OPEN_FAILED     -2
#define ERR_DEVICE_WRITE_FAILED -3
#define ERR_DEVICE_READ_FAILED  -4
#define ERR_CMD_NOTMATCH        -5
#define ERR_DEVICE_UNREADY      -6
#define ERR_FOUND_BAD_BLOCK     -7
#define ERR_FAILED              -8
#define ERR_CROSS_BORDER        -9
#define ERR_DEVICE_NOT_SUPPORT  -10
#define ERR_REQUEST_NOT_SUPPORT  -11
#define ERR_REQUEST_FAIL        -12
#define ERR_BUFFER_NOT_ENOUGH   -13

#define UFI_CHECK_SIGN(cbw, csw) ((CSW_SIGN == (csw).dwCSWSignature) && ((csw).dwCSWTag == (cbw).dwCBWTag))

class CRKLog;
class CRKComm
{
public:
    virtual int RKU_EraseBlock(BYTE ucFlashCS, DWORD dwPos, DWORD dwCount, BYTE ucEraseType) = 0;
    virtual int RKU_ReadChipInfo(BYTE *lpBuffer) = 0;
    virtual int RKU_ReadFlashID(BYTE *lpBuffer) = 0;
    virtual int RKU_ReadFlashInfo(BYTE *lpBuffer, UINT *puiRead = NULL) = 0;
    virtual int RKU_ReadLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode = RWMETHOD_IMAGE) = 0;
    virtual int RKU_ReadSector(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer) = 0;
    virtual int RKU_ResetDevice(BYTE bySubCode = RST_NONE_SUBCODE) = 0;
    virtual int RKU_TestBadBlock(BYTE ucFlashCS, DWORD dwPos, DWORD dwCount, BYTE *lpBuffer) = 0;
    virtual int RKU_TestDeviceReady(DWORD *dwTotal = NULL, DWORD *dwCurrent = NULL, BYTE bySubCode = TU_NONE_SUBCODE) = 0;
    virtual int RKU_WriteLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode = RWMETHOD_IMAGE) = 0;
    virtual int RKU_LoaderWriteLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode = RWMETHOD_IMAGE) = 0;
    virtual int RKU_WriteSector(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer) = 0;
    virtual int RKU_EndWriteSector(BYTE *lpBuffer) = 0;
    virtual int RKU_GetLockFlag(BYTE *lpBuffer) = 0;
    virtual int RKU_GetPublicKey(BYTE *lpBuffer) = 0;
    virtual void RKU_ReopenLBAHandle() = 0;
    virtual int RKU_ShowNandLBADevice() = 0;
    virtual bool RKU_IsEmmcFlash() = 0;
    CRKComm(CRKLog *pLog);
    virtual ~CRKComm();
protected:
    CRKLog *m_log;
    bool m_bEmmc;
    int m_hDev;
    int m_hLbaDev;
private:

};
class CRKUsbComm: public CRKComm
{
public:
    virtual int RKU_EraseBlock(BYTE ucFlashCS, DWORD dwPos, DWORD dwCount, BYTE ucEraseType);
    virtual int RKU_ReadChipInfo(BYTE *lpBuffer);
    virtual int RKU_ReadFlashID(BYTE *lpBuffer);
    virtual int RKU_ReadFlashInfo(BYTE *lpBuffer, UINT *puiRead = NULL);
    virtual int RKU_ReadLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode = RWMETHOD_IMAGE);
    virtual int RKU_ReadSector(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer);
    virtual int RKU_ResetDevice(BYTE bySubCode = RST_NONE_SUBCODE);
    virtual int RKU_TestBadBlock(BYTE ucFlashCS, DWORD dwPos, DWORD dwCount, BYTE *lpBuffer);
    virtual int RKU_TestDeviceReady(DWORD *dwTotal = NULL, DWORD *dwCurrent = NULL, BYTE bySubCode = TU_NONE_SUBCODE);
    virtual int RKU_WriteLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode = RWMETHOD_IMAGE);
    virtual int RKU_LoaderWriteLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode = RWMETHOD_IMAGE);
    virtual int RKU_WriteSector(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer);
    virtual int RKU_EndWriteSector(BYTE *lpBuffer);
    virtual int RKU_GetLockFlag(BYTE *lpBuffer);
    virtual int RKU_GetPublicKey(BYTE *lpBuffer);
    virtual void RKU_ReopenLBAHandle();
    virtual int RKU_ShowNandLBADevice();
    virtual bool RKU_IsEmmcFlash();
    CRKUsbComm(CRKLog *pLog);
    ~CRKUsbComm();

protected:
    STRUCT_RKDEVICE_DESC m_deviceDesc;
    CRKLog *m_log;
private:
    bool CtrlNandLbaWrite(bool bEnable = true);
    bool CtrlNandLbaRead(bool bEnable = true);

};

#endif
