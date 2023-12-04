#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "RKComm.h"
#include "RKLog.h"
#include "RKAndroidDevice.h"

CRKComm::CRKComm(CRKLog *pLog)
{
    m_log = pLog;
    m_bEmmc = false;
    m_hDev = m_hLbaDev = -1;
}
CRKComm::~CRKComm()
{
}

CRKUsbComm::CRKUsbComm(CRKLog *pLog): CRKComm(pLog)
{
    //char bootmode[100];
    //property_get("ro.bootmode", bootmode, "unknown");
    //if(!strcmp(bootmode, "emmc"))
    //  m_bEmmc = true;
    //else
    //  m_bEmmc = false;
    char *emmc_point = getenv(EMMC_POINT_NAME);
    m_hLbaDev = open(emmc_point, O_RDWR | O_SYNC, 0);
    if (m_hLbaDev < 0)
    {
        if (pLog)
        {
            pLog->Record(_T("INFO:is nand devices..."));
        }
        m_bEmmc = false;
    }
    else
    {
        if (pLog)
        {
            pLog->Record(_T("INFO:is emmc devices..."));
        }
        m_bEmmc = true;
        close(m_hLbaDev);
    }

    m_log = pLog;

    if (m_bEmmc)
    {
        if (pLog)
        {
            pLog->Record(_T("INFO:CRKUsbComm-->is emmc."));
        }
        m_hDev = open(EMMC_DRIVER_DEV_VENDOR, O_RDWR, 0);
        if (m_hDev < 0)
        {
            if (pLog)
            {
                pLog->Record(_T("ERROR:CRKUsbComm-->open %s failed,err=%s"), EMMC_DRIVER_DEV_VENDOR, strerror(errno));
                pLog->Record(_T("ERROR:CRKUsbComm-->try to read %s."), EMMC_DRIVER_DEV);
            }

            m_hDev = open(EMMC_DRIVER_DEV, O_RDWR, 0);
            if (m_hDev < 0)
            {
                if (pLog)
                {
                    pLog->Record(_T("ERROR:CRKUsbComm-->open %s failed,err=%s"), EMMC_DRIVER_DEV, strerror(errno));
                    pLog->Record(_T("ERROR:CRKUsbComm-->please to check drmboot.ko."));
                }
            }
            else
            {
                if (pLog)
                {
                    pLog->Record(_T("INFO:CRKUsbComm-->%s=%d"), EMMC_DRIVER_DEV, m_hDev);
                }
            }
        }
        else
        {
            if (pLog)
            {
                pLog->Record(_T("INFO:CRKUsbComm-->%s=%d"), EMMC_DRIVER_DEV_VENDOR, m_hDev);
            }
        }
        //get EMMC_DRIVER_DEV_LBA from
        m_hLbaDev = open(emmc_point, O_RDWR | O_SYNC, 0);
        if (m_hLbaDev < 0)
        {
            if (pLog)
            {
                pLog->Record(_T("ERROR:CRKUsbComm-->open %s failed,err=%d"), emmc_point, errno);
            }
        }
        else
        {
            if (pLog)
            {
                pLog->Record(_T("INFO:CRKUsbComm-->%s=%d"), emmc_point, m_hLbaDev);
            }
        }
    }
    else
    {
        if (pLog)
        {
            pLog->Record(_T("INFO:CRKUsbComm-->is nand."));
        }
        m_hDev = open(NAND_DRIVER_DEV_VENDOR, O_RDWR, 0);
        if (m_hDev < 0)
        {
            if (pLog)
            {
                pLog->Record(_T("ERROR:CRKUsbComm-->open %s failed,err=%d"), NAND_DRIVER_DEV_VENDOR, strerror(errno));
                pLog->Record(_T("ERROR:CRKUsbComm-->try to read from %s."), NAND_DRIVER_DEV_VENDOR);
            }
            m_hDev = open(NAND_DRIVER_DEV, O_RDWR, 0);
            if (pLog)
            {
                pLog->Record(_T("ERROR:CRKUsbComm-->open %s failed,err=%d"), NAND_DRIVER_DEV, strerror(errno));
            }
            else
            {
                if (pLog)
                {
                    pLog->Record(_T("INFO:CRKUsbComm-->%s=%d"), NAND_DRIVER_DEV, m_hDev);
                }
            }
        }
        else
        {
            if (pLog)
            {
                pLog->Record(_T("INFO:CRKUsbComm-->%s=%d"), NAND_DRIVER_DEV_VENDOR, m_hDev);
            }
        }
    }

}
void CRKUsbComm::RKU_ReopenLBAHandle()
{
    if (m_bEmmc)
    {
        return;
    }
    if (m_hLbaDev > 0)
    {
        close(m_hLbaDev);
        m_hLbaDev = -1;
    }
    //  if (m_bEmmc)
    //  {
    //      m_hLbaDev= open(EMMC_DRIVER_DEV_LBA,O_RDWR|O_SYNC,0);
    //      if (m_hLbaDev<0)
    //      {
    //          if (m_log)
    //              m_log->Record(_T("ERROR:RKU_ReopenLBAHandle-->open %s failed,err=%d"),EMMC_DRIVER_DEV_LBA,errno);
    //      }
    //      else
    //      {
    //          if (m_log)
    //              m_log->Record(_T("INFO:RKU_ReopenLBAHandle-->%s=%d"),EMMC_DRIVER_DEV_LBA,m_hLbaDev);
    //      }

    //  }
    //  else
    //  {
    m_hLbaDev = open(NAND_DRIVER_DEV_LBA, O_RDWR | O_SYNC, 0);
    if (m_hLbaDev < 0)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_ReopenLBAHandle-->open %s failed,err=%d"), NAND_DRIVER_DEV_LBA, errno);
        }
    }
    else
    {
        if (m_log)
        {
            m_log->Record(_T("INFO:RKU_ReopenLBAHandle-->%s=%d"), NAND_DRIVER_DEV_LBA, m_hLbaDev);
        }
    }
    //  }

}
int CRKUsbComm::RKU_ShowNandLBADevice()
{
    if (m_bEmmc)
    {
        return ERR_SUCCESS;
    }
    BYTE blockState[64];
    memset(blockState, 0, 64);
    int iRet;
    iRet = RKU_TestBadBlock(0, 0, MAX_TEST_BLOCKS, blockState);
    if (iRet != ERR_SUCCESS)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_ShowNandLBADevice-->RKU_TestBadBlock failed,ret=%d"), iRet);
        }
    }
    return iRet;
}

bool CRKUsbComm::RKU_IsEmmcFlash()
{
    return m_bEmmc ? true : false;
}

CRKUsbComm::~CRKUsbComm()
{
    if (m_hDev > 0)
    {
        close(m_hDev);
    }
    if (m_hLbaDev > 0)
    {
        //      if (!m_bEmmc)
        //      {
        //          CtrlNandLbaRead(false);
        //          CtrlNandLbaWrite(false);
        //      }
        close(m_hLbaDev);
    }
}

int CRKUsbComm::RKU_EraseBlock(BYTE ucFlashCS, DWORD dwPos, DWORD dwCount, BYTE ucEraseType)
{
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_ReadChipInfo(BYTE *lpBuffer)
{
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_ReadFlashID(BYTE *lpBuffer)
{
    return ERR_SUCCESS;
}

void rknand_print_hex_data(char *s, unsigned int *buf, unsigned int len)
{
    unsigned int i, j, count;

    printf("%s\n", s);
    for (i = 0; i < len; i += 4)
    {
        printf("%08x %08x %08x %08x\n", buf[i], buf[i + 1], buf[i + 2], buf[i + 3]);
    }
}


int CRKUsbComm::RKU_ReadFlashInfo(BYTE *lpBuffer, UINT *puiRead)
{
    long long ret;

    #if 0   //close by chad.ma
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }

    ret = ioctl(m_hDev, GET_FLASH_INFO_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_ReadFlashInfo ioctl failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    *puiRead = 11;
    #else
    /////////////////////////////////////////////////////////////////
    // get flashsize directly
    m_log->Record(_T("INFO: m_bEmmc = %d, m_hLbaDev = %d"), m_bEmmc, m_hLbaDev);

    if (m_hLbaDev < 0)
    {
        m_hLbaDev = open(NAND_DRIVER_DEV_LBA, O_RDWR | O_SYNC, 0);
        if (m_hLbaDev < 0)
        {
            if (m_log)
            {
                m_log->Record(_T("ERROR:RKU_ReadFlashInfo-->open %s failed,err=%d"), NAND_DRIVER_DEV_LBA, errno);
            }
            return ERR_FAILED;
        }
        else
        {
            if (m_log)
            {
                m_log->Record(_T("INFO:RKU_ReadFlashInfo-->open %s ok,handle=%d"), NAND_DRIVER_DEV_LBA, m_hLbaDev);
            }

            ret = lseek64(m_hLbaDev, 0, SEEK_END);

            if (ret < 0)
            {
                if (m_log)
                {
                    m_log->Record(_T("ERROR:RKU_ReadFlashInfo-->get %s file length fail"), NAND_DRIVER_DEV_LBA);
                }
                return ERR_FAILED;
            }
            else
            {
                char str[20] = {0};
                lseek64(m_hLbaDev, 0, SEEK_SET);  //reset the cfo to begin
                snprintf(str, sizeof(str), "%d", ret / 1024);
                *(UINT *)lpBuffer = (ret / 1024);
            }
        }
    }
    else
    {
        ret = lseek64(m_hLbaDev, 0, SEEK_END);
        m_log->Record(_T("INFO: lseek64 result = %lld"), ret);
        if (ret < 0)
        {
            if (m_log)
            {
                if (m_bEmmc)
                    m_log->Record(_T("ERROR:RKU_ReadFlashInfo-->get %s file length fail"),
                                  getenv(EMMC_POINT_NAME));
                else
                    m_log->Record(_T("ERROR:RKU_ReadFlashInfo-->get %s file length fail"),
                                  NAND_DRIVER_DEV_LBA);
            }
            return ERR_FAILED;
        }
        else
        {
            char str[20] = {0};
            lseek64(m_hLbaDev, 0, SEEK_SET); //reset the cfo to begin
            snprintf(str, sizeof(str), "%d", ret / 1024);
            *(UINT *)lpBuffer = (ret / 1024);
        }
    }
    #endif
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_ReadLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode)
{
    long long ret;
    long long dwPosBuf;
    if (m_hLbaDev < 0)
    {
        if (!m_bEmmc)
        {
            m_hLbaDev = open(NAND_DRIVER_DEV_LBA, O_RDWR | O_SYNC, 0);
            if (m_hLbaDev < 0)
            {
                if (m_log)
                {
                    m_log->Record(_T("ERROR:RKU_ReadLBA-->open %s failed,err=%d"), NAND_DRIVER_DEV_LBA, errno);
                }
                return ERR_DEVICE_OPEN_FAILED;
            }
            else
            {
                if (m_log)
                {
                    m_log->Record(_T("INFO:RKU_ReadLBA-->open %s ok,handle=%d"), NAND_DRIVER_DEV_LBA, m_hLbaDev);
                }
            }
        }
        else
        {
            return ERR_DEVICE_OPEN_FAILED;
        }
    }
    if (m_bEmmc && !CRKAndroidDevice::bGptFlag)
    {
        dwPos += 8192;
    }

    dwPosBuf = dwPos;

    ret = lseek64(m_hLbaDev, (off64_t)dwPosBuf * 512, SEEK_SET);
    if (ret < 0)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_ReadLBA seek failed,err=%d,ret=%lld."), errno, ret);
            m_log->Record(_T("the dwPosBuf = dwPosBuf*512,dwPosBuf:%lld!"), dwPosBuf * 512);
        }
        return ERR_FAILED;
    }
    ret = read(m_hLbaDev, lpBuffer, dwCount * 512);
    if (ret != dwCount * 512)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_ReadLBA read failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_ReadSector(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer)
{
    int ret;
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }
    DWORD *pOffsetSec = (DWORD *)(lpBuffer);
    DWORD *pCountSec = (DWORD *)(lpBuffer + 4);
    *pOffsetSec = dwPos;
    *pCountSec = dwCount;
    ret = ioctl(m_hDev, READ_SECTOR_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_ReadSector failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_ResetDevice(BYTE bySubCode)
{
    return ERR_SUCCESS;
}

int CRKUsbComm::RKU_TestBadBlock(BYTE ucFlashCS, DWORD dwPos, DWORD dwCount, BYTE *lpBuffer)
{
    int ret;
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }
    ret = ioctl(m_hDev, GET_BAD_BLOCK_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_TestBadBlock failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    if (m_log)
    {
        string strOutput;
        m_log->PrintBuffer(strOutput, lpBuffer, 64);
        m_log->Record(_T("INFO:BadBlockState:\r\n%s"), strOutput.c_str());
    }
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_TestDeviceReady(DWORD *dwTotal, DWORD *dwCurrent, BYTE bySubCode)
{
    return ERR_DEVICE_READY;
}
int CRKUsbComm::RKU_WriteLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode)
{
    long long ret;
    long long dwPosBuf;
    if (m_hLbaDev < 0)
    {
        if (!m_bEmmc)
        {
            m_hLbaDev = open(NAND_DRIVER_DEV_LBA, O_RDWR | O_SYNC, 0);
            if (m_hLbaDev < 0)
            {
                if (m_log)
                {
                    m_log->Record(_T("ERROR:RKU_WriteLBA-->open %s failed,err=%d"), NAND_DRIVER_DEV_LBA, errno);
                }
                return ERR_DEVICE_OPEN_FAILED;
            }
            else
            {
                if (m_log)
                {
                    m_log->Record(_T("INFO:RKU_WriteLBA-->open %s ok,handle=%d"), NAND_DRIVER_DEV_LBA, m_hLbaDev);
                }
            }
        }
        else
        {
            return ERR_DEVICE_OPEN_FAILED;
        }
    }
    if (m_bEmmc && !CRKAndroidDevice::bGptFlag)
    {
        dwPos += 8192;
    }

    dwPosBuf = dwPos;

    ret = lseek64(m_hLbaDev, (off64_t)dwPosBuf * 512, SEEK_SET);
    if (ret < 0)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_WriteLBA seek failed,err=%d,ret:%lld"), errno, ret);
            m_log->Record(_T("the dwPosBuf = dwPosBuf*512,dwPosBuf:%lld!"), dwPosBuf * 512);
        }

        return ERR_FAILED;
    }

    ret = write(m_hLbaDev, lpBuffer, dwCount * 512);
    if (ret != dwCount * 512)
    {
        sleep(1);
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_WriteLBA write failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }

    return ERR_SUCCESS;
}

int CRKUsbComm::RKU_LoaderWriteLBA(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer, BYTE bySubCode)
{
    long long ret;
    long long dwPosBuf;
    if (m_hLbaDev < 0)
    {
        if (!m_bEmmc)
        {
            m_hLbaDev = open(NAND_DRIVER_DEV_LBA, O_RDWR | O_SYNC, 0);
            if (m_hLbaDev < 0)
            {
                if (m_log)
                {
                    m_log->Record(_T("ERROR:RKU_WriteLBA-->open %s failed,err=%d"), NAND_DRIVER_DEV_LBA, errno);
                }
                return ERR_DEVICE_OPEN_FAILED;
            }
            else
            {
                if (m_log)
                {
                    m_log->Record(_T("INFO:RKU_WriteLBA-->open %s ok,handle=%d"), NAND_DRIVER_DEV_LBA, m_hLbaDev);
                }
            }
        }
        else
        {
            return ERR_DEVICE_OPEN_FAILED;
        }
    }

    dwPosBuf = dwPos;
    //if (m_log)
    //    m_log->Record(_T("INFO: dwPosBuf = %d ,will seek to pos = 0x%08x"), dwPosBuf, dwPosBuf*512);

    ret = lseek64(m_hLbaDev, (off64_t)dwPosBuf * 512, SEEK_SET);
    if (ret < 0)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_WriteLBA seek failed,err=%d,ret:%lld"), errno, ret);
            m_log->Record(_T("the dwPosBuf = dwPosBuf*512,dwPosBuf:%lld!"), dwPosBuf * 512);
        }

        return ERR_FAILED;
    }

    ret = write(m_hLbaDev, lpBuffer, dwCount * 512);
    if (ret != dwCount * 512)
    {
        sleep(1);
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_WriteLBA write failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }

    return ERR_SUCCESS;
}

int CRKUsbComm::RKU_WriteSector(DWORD dwPos, DWORD dwCount, BYTE *lpBuffer)
{
    int ret;
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }
    DWORD *pOffset = (DWORD *)(lpBuffer);
    DWORD *pCount = (DWORD *)(lpBuffer + 4);
    *pOffset = dwPos;
    *pCount = dwCount;
    ret = ioctl(m_hDev, WRITE_SECTOR_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_WriteSector failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    return ERR_SUCCESS;
}

int CRKUsbComm::RKU_EndWriteSector(BYTE *lpBuffer)
{
    int ret;
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }
    ret = ioctl(m_hDev, END_WRITE_SECTOR_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_EndWriteSector failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_GetLockFlag(BYTE *lpBuffer)
{
    int ret;
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }
    ret = ioctl(m_hDev, GET_LOCK_FLAG_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_GetLockFlag failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    DWORD *pFlag = (DWORD *)lpBuffer;
    if (m_log)
    {
        m_log->Record(_T("INFO:LockFlag:0x%08x"), *pFlag);
    }
    return ERR_SUCCESS;
}
int CRKUsbComm::RKU_GetPublicKey(BYTE *lpBuffer)
{
    int ret;
    if (m_hDev < 0)
    {
        return ERR_DEVICE_OPEN_FAILED;
    }
    ret = ioctl(m_hDev, GET_PUBLIC_KEY_IO, lpBuffer);
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:RKU_GetPublicKey failed,err=%d"), errno);
        }
        return ERR_FAILED;
    }
    return ERR_SUCCESS;
}
bool CRKUsbComm::CtrlNandLbaWrite(bool bEnable)
{
    int ret;
    if (m_bEmmc)
    {
        return false;
    }
    if (m_hLbaDev < 0)
    {
        return false;
    }
    if (bEnable)
    {
        ret = ioctl(m_hLbaDev, ENABLE_NAND_LBA_WRITE_IO);
    }
    else
    {
        ret = ioctl(m_hLbaDev, DISABLE_NAND_LBA_WRITE_IO);
    }
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:CtrlNandLbaWrite failed,enable=%d,err=%d"), bEnable, errno);
        }
        return false;
    }

    return true;
}
bool CRKUsbComm::CtrlNandLbaRead(bool bEnable)
{
    int ret;
    if (m_bEmmc)
    {
        return false;
    }
    if (m_hLbaDev < 0)
    {
        return false;
    }
    if (bEnable)
    {
        ret = ioctl(m_hLbaDev, ENABLE_NAND_LBA_READ_IO);
    }
    else
    {
        ret = ioctl(m_hLbaDev, DISABLE_NAND_LBA_READ_IO);
    }
    if (ret)
    {
        if (m_log)
        {
            m_log->Record(_T("ERROR:CtrlNandLbaRead failed,enable=%d,err=%d"), bEnable, errno);
        }
        return false;
    }
    return true;
}




