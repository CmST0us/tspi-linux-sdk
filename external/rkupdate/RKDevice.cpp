#include "RKDevice.h"

const tchar *szManufName[] =
{
    _T("SAMSUNG"),
    _T("TOSHIBA"),
    _T("HYNIX"),
    _T("INFINEON"),
    _T("MICRON"),
    _T("RENESAS"),
    _T("ST"),
    _T("INTEL")
};

void CRKDevice::SetPrepareEraseFlag(bool value)
{
    m_bEraseInPrepare = value;
}

void CRKDevice::SetWorkFlow(UINT value)
{
    m_uiWorkFlow = value;
}

void CRKDevice::SetUid(PBYTE value)
{
    if (value)
    {
        if (!m_uid)
        {
            m_uid = new BYTE[RKDEVICE_UID_LEN];
            memset(m_uid, 0, RKDEVICE_UID_LEN);
        }
        memcpy(m_uid, value, RKDEVICE_UID_LEN);
    }
    else
    {
        if (m_uid)
        {
            delete []m_uid;
        }
        m_uid = value;
    }
}
void CRKDevice::SetMiscModifyFlag(ENUM_MISC_MODIFY_FLAG value)
{
    m_emMiscModifyFlag = value;
}

void CRKDevice::SetOsType(ENUM_OS_TYPE value)
{
    m_os = value;
}
ENUM_OS_TYPE CRKDevice::GetOsType()
{
    return m_os;
}

CRKLog *CRKDevice::GetLogObjectPointer()
{
    return m_pLog;
}

CRKComm *CRKDevice::GetCommObjectPointer()
{
    return m_pComm;
}

CRKDevice::CRKDevice(STRUCT_RKDEVICE_DESC &device)
{
    OsType.setContainer(this);
    OsType.getter(&CRKDevice::GetOsType);
    OsType.setter(&CRKDevice::SetOsType);

    Uid.setContainer(this);
    Uid.setter(&CRKDevice::SetUid);

    PrepareEraseFlag.setContainer(this);
    PrepareEraseFlag.setter(&CRKDevice::SetPrepareEraseFlag);

    WorkFlow.setContainer(this);
    WorkFlow.setter(&CRKDevice::SetWorkFlow);

    MiscModifyFlag.setContainer(this);
    MiscModifyFlag.setter(&CRKDevice::SetMiscModifyFlag);

    LogObjectPointer.setContainer(this);
    LogObjectPointer.getter(&CRKDevice::GetLogObjectPointer);

    CommObjectPointer.setContainer(this);
    CommObjectPointer.getter(&CRKDevice::GetCommObjectPointer);

    m_usb = device.emUsbType;
    m_device = device.emDeviceType;
    m_bcdUsb = device.usbcdUsb;

    memset(m_idBlockOffset, 0, sizeof(DWORD) * 5);
    memset(m_flashInfo.blockState, 0, IDBLOCK_TOP);
    m_flashInfo.usPhyBlokcPerIDB = 1;
    m_flashInfo.uiSecNumPerIDB = 0;
    m_chipData = NULL;
    m_pImage = NULL;
    m_pLog = NULL;
    m_pComm = NULL;
    m_customData = NULL;
    m_customDataSize = 0;
    m_customDataOffset = 0;
    m_sn = NULL;
    m_snSize = 0;
    m_mac = NULL;
    m_imei = NULL;
    m_blueTooth = NULL;
    m_uid = NULL;
    m_sysDiskSize = 0;
    m_cfgDiskSize = 0;
    m_bGetNewDiskSizeFlag = true;
    m_pBlockState = NULL;
    m_bWriteBack = false;
    m_pFlashInfoData = NULL;
    m_usFlashInfoDataLen = 0;
    m_usFlashInfoDataOffset = 0;
    memset(m_backupBuffer, 0, SECTOR_SIZE);
    memset(m_backupBuffer + SECTOR_SIZE, 0xFF, SPARE_SIZE);
    m_bUidWriteOK = false;
    m_remallocDisk = false;
    m_emMiscModifyFlag = MISC_MODIFY_NONE;
    m_bQuickCheckMode = false;
    m_bExistSector3Crc = false;
    m_bEmmc = false;
    m_usSector3Crc = 0;
    m_usWriteBackCrc = 0;
    m_usWriteBackCustomDataOffset = 0;
    m_usWriteBackCustomDataSize = 0;
}
CRKDevice::~CRKDevice()
{
    if (m_pComm)
    {
        delete m_pComm;
        m_pComm = NULL;
    }
    if (m_chipData)
    {
        delete []m_chipData;
        m_chipData = NULL;
    }
    if (m_customData)
    {
        delete []m_customData;
        m_customData = NULL;
    }
    if (m_sn)
    {
        delete []m_sn;
        m_sn = NULL;
    }
    if (m_mac)
    {
        delete []m_mac;
        m_mac = NULL;
    }
    if (m_imei)
    {
        delete []m_imei;
        m_imei = NULL;
    }
    if (m_blueTooth)
    {
        delete []m_blueTooth;
        m_blueTooth = NULL;
    }
    if (m_uid)
    {
        delete []m_uid;
        m_uid = NULL;
    }

    if (m_pFlashInfoData)
    {
        delete []m_pFlashInfoData;
        m_pFlashInfoData = NULL;
    }
}

bool CRKDevice::SetObject(CRKImage *pImage, CRKComm *pComm, CRKLog *pLog)
{
    /*pImage可以为空,用于完成不用固件参与的操作*/
    if (!pComm)
    {
        return false;
    }
    m_pImage = pImage;
    m_pComm = pComm;
    m_pLog = pLog;
    if (m_pImage)
    {
        m_os = m_pImage->OsType;
    }
    else
    {
        m_os = RK_OS;
    }
    return true;
}
int CRKDevice::EraseEmmcBlock(UCHAR ucFlashCS, DWORD dwPos, DWORD dwCount)
{
    int sectorOffset, nWrittenBlcok, iRet;
    BYTE emptyData[4 * (SECTOR_SIZE + SPARE_SIZE)];
    memset(emptyData, 0xff, 4 * (SECTOR_SIZE + SPARE_SIZE));
    nWrittenBlcok = 0;
    while (dwCount > 0)
    {
        sectorOffset = (ucFlashCS * m_flashInfo.uiBlockNum + dwPos + nWrittenBlcok) * m_flashInfo.uiSectorPerBlock;
        iRet = m_pComm->RKU_WriteSector(sectorOffset, 4, emptyData);
        if ((iRet != ERR_SUCCESS) && (iRet != ERR_FOUND_BAD_BLOCK))
        {
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:EraseEmmcBlock-->RKU_WriteSector failed,RetCode(%d)"), iRet);
            }
            return iRet;
        }
        dwCount--;
        nWrittenBlcok++;
    }
    return ERR_SUCCESS;
}
bool CRKDevice::GetFlashInfo()
{
    STRUCT_FLASHINFO_CMD info;
    BYTE flashID[5];
    int iRet;
    UINT uiRead;
    iRet = m_pComm->RKU_ReadFlashInfo((PBYTE)&info, &uiRead);
    if (ERR_SUCCESS == iRet)
    {
        if (m_pLog)
        {
            tstring strFlashInfo;
            m_pLog->PrintBuffer(strFlashInfo, (PBYTE)&info, 11);
            m_pLog->Record(_T("INFO:FlashInfo:%s"), strFlashInfo.c_str());
        }

        #if 0   //closed by chad.ma
        if ((info.usBlockSize == 0) || (info.bPageSize == 0))
        {
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:GetFlashInfo-->RKU_ReadFlashInfo failed,pagesize or blocksize is zero"));
            }
            return false;
        }
        if ((info.bManufCode >= 0) && (info.bManufCode <= 7))
        {
            _tcscpy(m_flashInfo.szManufacturerName, szManufName[info.bManufCode]);
        }
        else
        {
            _tcscpy(m_flashInfo.szManufacturerName, _T("UNKNOWN"));
        }
        #endif

        //m_flashInfo.uiFlashSize = info.uiFlashSize/2/1024;//MB
        m_flashInfo.uiFlashSize = info.uiFlashSize / 1024;//MB
        m_flashInfo.uiBlockNum = info.uiFlashSize * 2;
        printf("%s: %d  info.uiFlashSize = %d total uiBlockNum = %d\n", __func__, __LINE__, info.uiFlashSize, m_flashInfo.uiBlockNum);
        printf("%s: %d  FlashSize = %d MB\n", __func__, __LINE__, m_flashInfo.uiFlashSize);

        #if 0   //closed by chad.ma
        m_flashInfo.uiPageSize = info.bPageSize / 2; //KB
        m_flashInfo.usBlockSize = info.usBlockSize / 2; //KB
        m_flashInfo.bECCBits = info.bECCBits;
        m_flashInfo.bAccessTime = info.bAccessTime;
        m_flashInfo.uiBlockNum = m_flashInfo.uiFlashSize * 1024 / m_flashInfo.usBlockSize;
        m_flashInfo.uiSectorPerBlock = info.usBlockSize;
        m_flashInfo.bFlashCS = info.bFlashCS;
        m_flashInfo.usValidSecPerBlock = (info.usBlockSize / info.bPageSize) * 4;
        if (m_pFlashInfoData)
        {
            delete []m_pFlashInfoData;
            m_pFlashInfoData = NULL;
        }
        m_usFlashInfoDataLen = BYTE2SECTOR(uiRead);
        m_pFlashInfoData = new BYTE[SECTOR_SIZE * m_usFlashInfoDataLen];
        memset(m_pFlashInfoData, 0, SECTOR_SIZE * m_usFlashInfoDataLen);
        memcpy(m_pFlashInfoData, (PBYTE)&info, uiRead);
        #endif
    }
    else
    {
        if (m_pLog)
        {
            m_pLog->Record(_T("ERROR:GetFlashInfo-->RKU_ReadFlashInfo failed,RetCode(%d)"), iRet);
        }

        return false;
    }
    return true;
}
bool CRKDevice::BuildBlockStateMap(BYTE bFlashCS)
{
    BYTE blockState[64];
    int iRet, i, j ;
    memset(blockState, 0, 64);
    iRet = m_pComm->RKU_TestBadBlock(bFlashCS, 0, MAX_TEST_BLOCKS, blockState);
    if (ERR_SUCCESS == iRet) //无坏块
    {
        //      return true;
        //  }
        //  else if(ERR_FOUND_BAD_BLOCK == iRet)//有坏块
        //  {
        for (i = 0; i < 64; i++)
        {
            for (j = 0; j < 8; j++)
            {
                if (blockState[i] & (1 << j))
                {
                    m_flashInfo.blockState[i * 8 + j] = 1;
                }
                if (i * 8 + j > (IDBLOCK_TOP - 2))
                {
                    break;
                }
            }
            if (j < 8)
            {
                break;
            }
        }
        return true;
    }
    else//操作失败
    {
        if (m_pLog)
        {
            m_pLog->Record(_T("ERROR:BuildBlockStateMap-->RKU_TestBadBlock failed,RetCode(%d)"), iRet);
        }
        return false;
    }
}
int CRKDevice::ReadMutilSector(DWORD dwPos, DWORD dwCount, PBYTE lpBuffer)
{
    DWORD dwReadSector = 0, dwMaxReadWriteOnce;
    int iUsedSecCount, iUsedBlockCount, iValidSecCount;
    int iRet = 0, iCurPos;
    iUsedBlockCount = dwPos / m_flashInfo.uiSectorPerBlock;
    iUsedSecCount = dwPos - (iUsedBlockCount * m_flashInfo.uiSectorPerBlock);
    iValidSecCount = m_flashInfo.usValidSecPerBlock - iUsedSecCount;

    dwMaxReadWriteOnce = MAX_WRITE_SECTOR;

    while (dwCount > 0)
    {
        dwReadSector = (dwCount >= dwMaxReadWriteOnce) ? dwMaxReadWriteOnce : dwCount;
        if (dwReadSector > iValidSecCount)
        {
            dwReadSector = iValidSecCount;
        }
        iCurPos = iUsedBlockCount * m_flashInfo.uiSectorPerBlock + (m_flashInfo.usValidSecPerBlock - iValidSecCount);
        iRet = m_pComm->RKU_ReadSector(iCurPos, dwReadSector, lpBuffer);
        if (iRet != ERR_SUCCESS)
        {
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:ReadMutilSector-->RKU_ReadSector failed,RetCode(%d)"), iRet);
            }
            break;
        }

        dwCount -= dwReadSector;
        iUsedSecCount += dwReadSector;
        iValidSecCount -= dwReadSector;
        if (iValidSecCount <= 0)
        {
            iUsedBlockCount++;
            iValidSecCount = m_flashInfo.usValidSecPerBlock;
        }
        lpBuffer += dwReadSector * 512;
    }
    return iRet;
}
bool CRKDevice::EraseMutilBlock(BYTE bFlashCS, DWORD dwPos, DWORD dwCount, bool bForce)
{
    DWORD dwTimes = 0;
    UCHAR eraseType;
    eraseType = bForce ? ERASE_FORCE : ERASE_NORMAL;
    int iRet;
    while (dwCount >= MAX_ERASE_BLOCKS)
    {
        iRet = m_pComm->RKU_EraseBlock(bFlashCS, dwPos + dwTimes * MAX_ERASE_BLOCKS, MAX_ERASE_BLOCKS, eraseType);
        if (ERR_FOUND_BAD_BLOCK == iRet)
        {
            dwCount -= MAX_ERASE_BLOCKS;
            ++dwTimes;
        }
        else if (ERR_SUCCESS == iRet)
        {
            dwCount -= MAX_ERASE_BLOCKS;
            ++dwTimes;
        }
        else
        {
            //操作失败
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:EraseMutilBlock-->RKU_EraseBlock failed,RetCode(%d)"), iRet);
            }
            return false;
        }
    }
    if (dwCount > 0)
    {
        iRet = m_pComm->RKU_EraseBlock(bFlashCS, dwPos + dwTimes * MAX_ERASE_BLOCKS, dwCount, eraseType);
        if (ERR_SUCCESS == iRet)
        {
            dwCount = 0;
        }
        else if (ERR_FOUND_BAD_BLOCK == iRet)
        {
            dwCount = 0;
        }
        else
        {
            //操作失败
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:EraseMutilBlock-->RKU_EraseBlock failed,RetCode(%d)"), iRet);
            }
            return false;
        }
    }
    return true;
}


bool CRKDevice::CheckChip()
{
    int iRet;
    BYTE bChipInfo[CHIPINFO_LEN];
    ENUM_RKDEVICE_TYPE curDeviceType = RKNONE_DEVICE;
    memset(bChipInfo, 0, CHIPINFO_LEN);
    iRet = m_pComm->RKU_ReadChipInfo(bChipInfo);
    if (iRet == ERR_SUCCESS)
    {
        if (!m_chipData)
        {
            m_chipData = new BYTE[CHIPINFO_LEN];
        }
        memset(m_chipData, 0, CHIPINFO_LEN);
        memcpy(m_chipData, bChipInfo, CHIPINFO_LEN);
        DWORD *pValue;
        pValue = (DWORD *)(&bChipInfo[0]);

        if ((ENUM_RKDEVICE_TYPE)(*pValue) == m_device)
        {
            return true;
        }

        if (*pValue == 0x524B3237)
        {
            curDeviceType = RK27_DEVICE;
        }
        else if (*pValue == 0x32373341)
        {
            curDeviceType = RKCAYMAN_DEVICE;
        }
        else if (*pValue == 0x524B3238)
        {
            curDeviceType = RK28_DEVICE;
        }
        else if (*pValue == 0x32383158)
        {
            curDeviceType = RK281X_DEVICE;
        }
        else if (*pValue == 0x32383242)
        {
            curDeviceType = RKPANDA_DEVICE;
        }
        else if (*pValue == 0x32393058)
        {
            curDeviceType = RK29_DEVICE;
        }
        else if (*pValue == 0x32393258)
        {
            curDeviceType = RK292X_DEVICE;
        }
        else if (*pValue == 0x33303041)
        {
            curDeviceType = RK30_DEVICE;
        }
        else if (*pValue == 0x33313041)
        {
            curDeviceType = RK30B_DEVICE;
        }
        else if (*pValue == 0x33313042)
        {
            curDeviceType = RK31_DEVICE;
        }
        else if (*pValue == 0x33323041)
        {
            curDeviceType = RK32_DEVICE;
        }
        else if (*pValue == 0x32363243)
        {
            curDeviceType = RKSMART_DEVICE;
        }
        else if (*pValue == 0x6E616E6F)
        {
            curDeviceType = RKNANO_DEVICE;
        }
        else if (*pValue == 0x4E4F5243)
        {
            curDeviceType = RKCROWN_DEVICE;
        }

        if (curDeviceType == m_device)
        {
            return true;
        }
        else
        {
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:CheckChip-->Chip is not match,firmware(0x%x),device(0x%x)"), m_device, *pValue);
            }
            return false;
        }
    }
    else
    {
        if (m_pLog)
        {
            m_pLog->Record(_T("ERROR:CheckChip-->RKU_ReadChipInfo failed,RetCode(%d)"), iRet);
        }
        return false;
    }
}
CHAR CRKDevice::FindValidBlocks(char bBegin, char bLen)
{
    char bCount = 0;
    char bIndex = bBegin;
    while (bBegin < IDBLOCK_TOP)
    {
        if (0 == m_flashInfo.blockState[bBegin++])
        {
            ++bCount;
        }
        else
        {
            bCount = 0;
            bIndex = bBegin;
        }
        if (bCount >= bLen)
        {
            break;
        }
    }
    if (bBegin >= IDBLOCK_TOP)
    {
        bIndex = -1;
    }

    return bIndex;
}
USHORT UshortToBCD(USHORT num)
{
    USHORT bcd = 0;
    bcd = (num % 10) | (((num / 10) % 10) << 4) | (((num / 100) % 10) << 8) | (((num / 1000) % 10) << 12);
    return bcd;
}
BYTE   ByteToBCD(BYTE num)
{
    BYTE bcd = 0;
    bcd = (num % 10) | (((num / 10) % 10) << 4);
    return bcd;
}
BYTE CRKDevice::RandomByte(BYTE bLowLimit, BYTE bHighLimit)
{
    BYTE k;
    double d;

    d = (double)rand() / ((double)RAND_MAX + 1);
    k = (BYTE)(d * (bHighLimit - bLowLimit + 1));
    return (bLowLimit + k);
}
bool CRKDevice::CheckCrc16(PBYTE pCheckData, USHORT usDataLength, USHORT usOldCrc)
{
    USHORT usNewCrc;
    usNewCrc = CRC_16(pCheckData, usDataLength);
    return (usNewCrc == usOldCrc) ? true : false;
}
bool CRKDevice::CheckUid(BYTE uidSize, BYTE *pUid)
{
    if (uidSize != RKDEVICE_UID_LEN)
    {
        return false;
    }
    USHORT oldCrc, newCrc;
    oldCrc = *(USHORT *)(pUid + RKDEVICE_UID_LEN - 2);
    newCrc = CRC_CCITT(pUid, RKDEVICE_UID_LEN - 2);
    if (oldCrc != newCrc)
    {
        return false;
    }
    return true;
}

bool CRKDevice::GetIDBData(UINT uiIDBCount, PBYTE lpBuf, UINT uiSecCount)
{
    PBYTE pIDB;
    pIDB = new BYTE[uiSecCount * 512];
    memset(pIDB, 0, uiSecCount * 512);
    int i, j, iResult;
    int nSrc = -1, nDst = -1;
    bool bRet;
    for (i = 0; i < uiIDBCount; i++)
    {
        if (nSrc == -1)
        {
            iResult = ReadMutilSector(m_flashInfo.uiSectorPerBlock * m_idBlockOffset[i], uiSecCount, lpBuf);
            if (iResult != ERR_SUCCESS)
            {
                if (m_pLog)
                {
                    m_pLog->Record(_T("ERROR:GetIDBData-->RKU_ReadSector failed,RetCode(%d)"), iResult);
                }
                continue;
            }
            nSrc = i;
            continue;
        }
        if (nDst == -1)
        {
            iResult = ReadMutilSector(m_flashInfo.uiSectorPerBlock * m_idBlockOffset[i], uiSecCount, pIDB);
            if (iResult != ERR_SUCCESS)
            {
                if (m_pLog)
                {
                    m_pLog->Record(_T("ERROR:GetIDBData-->RKU_ReadSector failed,RetCode(%d)"), iResult);
                }
                continue;
            }
            nDst = i;
        }

        bRet = true;

        for (j = 0; j < uiSecCount; j++)
        {
            bRet = memcmp(lpBuf + 512 * j, pIDB + 512 * j, 512) == 0;
            if (!bRet)
            {
                break;
            }
        }

        if (bRet)
        {
            //相同
            delete []pIDB;
            pIDB = NULL;
            return true;
        }
        else
        {
            if (m_pLog)
            {
                m_pLog->Record(_T("ERROR:GetIDBData-->memcmp failed,src(%d),Dst(%d)"), nSrc, nDst);
            }
            memcpy(lpBuf, pIDB, 512 * uiSecCount);
            nSrc = nDst;
            nDst = -1;
            continue;
        }
    }
    delete []pIDB;
    pIDB = NULL;
    if (nSrc != -1)
    {
        return true;
    }
    return false;
}

bool CRKDevice::GetWriteBackData(UINT uiIDBCount, PBYTE lpBuf)
{
    bool bRet;
    bRet = GetIDBData(uiIDBCount, lpBuf, 4);
    return bRet;
}
