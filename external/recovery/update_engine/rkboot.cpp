/*************************************************************************
    > File Name: rkboot.cpp
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Mon 03 Jun 2019 03:46:18 PM CST
 ************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "rkimage.h"
#include "log.h"
#include "rktools.h"
#include "crc.h"
extern "C" {
    #include "../mtdutils/mtdutils.h"
}

USHORT m_usFlashDataSec;
USHORT m_usFlashBootSec;
DWORD  m_dwLoaderSize;
DWORD  m_dwLoaderDataSize;
DWORD uiSecNumPerIDB;
size_t uiFlashPageSize;
size_t uiFlashBlockSize;
USHORT usPhyBlokcPerIDB;
bool m_bRc4Disable;
DWORD m_idBlockOffset[IDB_BLOCKS];

long long m_FlashSize;
long long m_FlasBlockNum;
PSTRUCT_RKBOOT_HEAD pBootHead;


static bool CheckUid(BYTE uidSize,BYTE *pUid)
{
    if (uidSize!=RKDEVICE_UID_LEN)
    {
        return false;
    }
    USHORT oldCrc,newCrc;
    oldCrc = *(USHORT *)(pUid+RKDEVICE_UID_LEN-2);
    newCrc = CRC_CCITT(pUid,RKDEVICE_UID_LEN-2);
    if (oldCrc!=newCrc)
    {
        return false;
    }
    return true;
}

static USHORT UshortToBCD(USHORT num) {
    USHORT bcd = 0;
    bcd = (num % 10) | ( ((num/10 )% 10)<< 4 )|( ((num/100) %10) << 8)|( ((num/1000) %10) << 12);
    return bcd;
}
static BYTE ByteToBCD(BYTE num)
{
    BYTE bcd = 0;
    bcd = (num % 10) | ( ((num/10 )% 10)<< 4 );
    return bcd;
}

static void WCHAR_To_char(WCHAR *src, char *dst, int len) {
    memset(dst, 0, len*sizeof(char));
    for (int i = 0; i < len; i++) {
        memcpy(dst, src, 1);
        src++;
        dst++;
    }
}

static DWORD getLoaderSizeAndData(char * loaderName, const unsigned char * data_buf, unsigned char **loaderBuffer) {

    DWORD dwOffset;
    UCHAR ucCount, ucSize;
    DWORD dwSize = 0;

    dwOffset = pBootHead->dwLoaderEntryOffset;
    ucCount = pBootHead->ucLoaderEntryCount;
    ucSize = pBootHead->ucLoaderEntrySize;
    for(UCHAR i = 0; i < ucCount; i++)
    {
        PSTRUCT_RKBOOT_ENTRY pEntry;
        pEntry = (PSTRUCT_RKBOOT_ENTRY)(data_buf+dwOffset+(ucSize*i));

        char szName[20];
        WCHAR_To_char(pEntry->szName, szName, 20);
        if (strcmp(loaderName, szName) == 0)
        {
            LOGI("pEntry->szName = %s.\n", szName);
            dwSize = pEntry->dwDataSize;
            *loaderBuffer = (unsigned char *)malloc(dwSize);
            if (*loaderBuffer == NULL) {
                LOGE("malloc error.\n");
            }
            memset(*loaderBuffer, 0, dwSize);
            memcpy(*loaderBuffer, data_buf+pEntry->dwDataOffset, pEntry->dwDataSize);
            LOGI("pEntry->dwDataOffset = %d, pEntry->dwDataSize = %d.\n", pEntry->dwDataOffset, pEntry->dwDataSize);

        }
    }
    return dwSize;
}

static CHAR FindValidBlocks(char bBegin, char bLen, PBYTE pblockState)
{
    char bCount = 0;
    char bIndex = bBegin;
    while(bBegin < IDBLOCK_TOP)
    {
        //if(0 == m_flashInfo.blockState[bBegin++])
        if (0 == pblockState[bBegin++])
        ++bCount;
        else
        {
            bCount = 0;
            bIndex = bBegin;
        }
        if(bCount >= bLen)
        break;
    }
    if(bBegin >= IDBLOCK_TOP)
    bIndex = -1;

    return bIndex;
}

static bool MakeSector0(PBYTE pSector)
{
    PRKANDROID_IDB_SEC0 pSec0;
    memset(pSector, 0, SECTOR_SIZE);
    pSec0 = (PRKANDROID_IDB_SEC0)pSector;

    pSec0->dwTag = 0x0FF0AA55;
    if (m_bRc4Disable)
    {
        pSec0->uiRc4Flag = 1;
    }
    pSec0->usBootCode1Offset = 0x4;
    pSec0->usBootCode2Offset = 0x4;
    pSec0->usBootDataSize = m_usFlashDataSec;
    pSec0->usBootCodeSize = m_usFlashDataSec + m_usFlashBootSec;
    return true;
}

static void MakeSector1(PBYTE pSector)
{
    PRKANDROID_IDB_SEC1 pSec1;
    memset(pSector,0,SECTOR_SIZE);
    pSec1 = (PRKANDROID_IDB_SEC1)pSector;
    USHORT usSysReserved;
    if ((m_idBlockOffset[4]+1)%12==0)
    {
        usSysReserved=m_idBlockOffset[4]+13;
    }
    else
    {
        usSysReserved =((m_idBlockOffset[4]+1)/12+1)*12;
    }
    if (usSysReserved>IDBLOCK_TOP)
    {
        usSysReserved = IDBLOCK_TOP;
    }
    pSec1->usSysReservedBlock = usSysReserved;


    pSec1->usDisk0Size = 0;
    pSec1->usDisk1Size = 0;
    pSec1->usDisk2Size = 0;
    pSec1->usDisk3Size = 0;
    pSec1->uiChipTag = 0x38324B52;
    pSec1->uiMachineId = 0;

    pSec1->usLoaderYear = UshortToBCD(pBootHead->stReleaseTime.usYear);
    pSec1->usLoaderDate = ByteToBCD(pBootHead->stReleaseTime.ucMonth);
    pSec1->usLoaderDate = (pSec1->usLoaderDate<<8)|ByteToBCD(pBootHead->stReleaseTime.ucDay);
    pSec1->usLoaderVer =  pBootHead->dwVersion;

    pSec1->usLastLoaderVer = 0;
    pSec1->usReadWriteTimes = 1;

    pSec1->uiFlashSize = m_FlashSize*1024;
    LOGI("m_FlashSize * 1024 = %lld.\n", m_FlashSize*1024);
    //pSec1->usBlockSize = m_flashInfo.usBlockSize*2;
    //pSec1->bPageSize = m_flashInfo.uiPageSize*2;
    //pSec1->bECCBits = m_flashInfo.bECCBits;
    //pSec1->bAccessTime = m_flashInfo.bAccessTime;

    pSec1->usFlashInfoLen = 0;
    pSec1->usFlashInfoOffset = 0;


    pSec1->usIdBlock0 = m_idBlockOffset[0];
    pSec1->usIdBlock1 = m_idBlockOffset[1];
    pSec1->usIdBlock2 = m_idBlockOffset[2];
    pSec1->usIdBlock3 = m_idBlockOffset[3];
    pSec1->usIdBlock4 = m_idBlockOffset[4];
}

static bool MakeSector2(PBYTE pSector)
{
    PRKANDROID_IDB_SEC2 pSec2;
    pSec2 = (PRKANDROID_IDB_SEC2)pSector;

    pSec2->usInfoSize = 0;
    memset(pSec2->bChipInfo,0,CHIPINFO_LEN);

    memset(pSec2->reserved,0,RKANDROID_SEC2_RESERVED_LEN);
    pSec2->usSec3CustomDataOffset = 0;//debug
    pSec2->usSec3CustomDataSize = 0;//debug

    strcpy(pSec2->szVcTag,"VC");
    strcpy(pSec2->szCrcTag,"CRC");
    return true;
}
static bool MakeSector3(PBYTE pSector)
{
    PRKANDROID_IDB_SEC3 pSec3;
    memset(pSector,0,SECTOR_SIZE);
    pSec3 = (PRKANDROID_IDB_SEC3)pSector;
    return true;
}

static int MakeIDBlockData(PBYTE lpIDBlock, PBYTE loaderCodeBuffer, PBYTE loaderDataBuffer) {

    RKANDROID_IDB_SEC0 sector0Info;
    RKANDROID_IDB_SEC1 sector1Info;
    RKANDROID_IDB_SEC2 sector2Info;
    RKANDROID_IDB_SEC3 sector3Info;

    MakeSector0((PBYTE)&sector0Info);
    MakeSector1((PBYTE)&sector1Info);
    if (!MakeSector2((PBYTE)&sector2Info))
    {
        return -6;
    }

    if (!MakeSector3((PBYTE)&sector3Info))
    {
        return -7;
    }

    sector2Info.usSec0Crc = CRC_16((PBYTE)&sector0Info,SECTOR_SIZE);
    sector2Info.usSec1Crc = CRC_16((PBYTE)&sector1Info,SECTOR_SIZE);
    sector2Info.usSec3Crc = CRC_16((PBYTE)&sector3Info,SECTOR_SIZE);
    memcpy(lpIDBlock, &sector0Info, SECTOR_SIZE);
    memcpy(lpIDBlock+SECTOR_SIZE, &sector1Info, SECTOR_SIZE);
    memcpy(lpIDBlock+SECTOR_SIZE*3, &sector3Info, SECTOR_SIZE);

    //close rc4 encryption
    if (sector0Info.uiRc4Flag) {
        for (int i=0;i<m_dwLoaderDataSize/SECTOR_SIZE;i++) {
            P_RC4(loaderDataBuffer+SECTOR_SIZE*i, SECTOR_SIZE);
        }
        for (int i=0;i<m_dwLoaderSize/SECTOR_SIZE;i++) {
            P_RC4(loaderCodeBuffer+SECTOR_SIZE*i, SECTOR_SIZE);
        }
    }
    memcpy(lpIDBlock+SECTOR_SIZE*4, loaderDataBuffer, m_dwLoaderDataSize);
    memcpy(lpIDBlock+SECTOR_SIZE*(4+m_usFlashDataSec), loaderCodeBuffer, m_dwLoaderSize);
    sector2Info.uiBootCodeCrc = CRC_32((PBYTE)(lpIDBlock+SECTOR_SIZE*4),sector0Info.usBootCodeSize*SECTOR_SIZE);
    memcpy(lpIDBlock+SECTOR_SIZE*2, &sector2Info, SECTOR_SIZE);
    for(int i=0; i<4; i++)
    {
        if(i == 1)
        {
            continue;
        }
        else
        {
            P_RC4(lpIDBlock+SECTOR_SIZE*i, SECTOR_SIZE);
        }
    }
    return 0;
}

static void calcIDBCount() {
    uiSecNumPerIDB = 4 + m_usFlashDataSec + m_usFlashBootSec;
    usPhyBlokcPerIDB = ((uiSecNumPerIDB > 0)?((uiSecNumPerIDB - 1) / 8 + 1):(uiSecNumPerIDB));
    LOGI("usPhyBlokcPerIDB = %d.\n", usPhyBlokcPerIDB);
}

static int reserveIDBlock() {
    // 3. ReserveIDBlock
    CHAR iRet;
    char iBlockIndex = 0;

    BYTE blockState[IDBLOCK_TOP];
    memset(m_idBlockOffset, 0, sizeof(DWORD)*IDB_BLOCKS);
    memset(blockState, 0, IDBLOCK_TOP);
    for (char i = 0; i < IDB_BLOCKS; i++) {
        iRet = iBlockIndex = FindValidBlocks(iBlockIndex, usPhyBlokcPerIDB, blockState);
        if (iRet < 0 ) {
            return -1;
            LOGE("FindValidBlocks Error.\n");
        }
        m_idBlockOffset[i] = iBlockIndex;
        iBlockIndex += usPhyBlokcPerIDB;
    }

    return 0;
}

static int WriteIDBlock(PBYTE lpIDBlock, DWORD dwSectorNum, char *dest_path)
{
    LOGE("WriteIDBlock start %s \n", dest_path);
    // int fd_dest = open("/tmp/loader2.txt", O_RDWR|O_SYNC, 0);
    int fd_dest = open(dest_path, O_CREAT|O_RDWR|O_SYNC|O_TRUNC, 0);
    if (fd_dest < 0) {
        LOGE("WriteIDBlock open %s failed. %s\n", dest_path, strerror(errno));
        return -2;
    }

    for(int i = 0; i <= 4; i++)
    {
        //debug for 3308
        //256 为128k 的起始位置
        //每256k 备份一份
        lseek64(fd_dest, (i * 512)*SECTOR_SIZE, SEEK_SET);
        if (write(fd_dest, lpIDBlock, dwSectorNum*SECTOR_SIZE) != dwSectorNum * SECTOR_SIZE) {
			close(fd_dest);
            LOGE("[%s:%d] error (%s).\n", __func__, __LINE__, strerror(errno));
            return -1;
        }
    }
    sync();
	close(fd_dest);
    return 0;
}

bool download_loader(PBYTE data_buf, int size, char *dest_path) {
    generate_gf();
    gen_poly();

    if (getFlashInfo(NULL, &uiFlashBlockSize, &uiFlashPageSize) != 0) {
        LOGE("%s-%d: get mtd info error\n", __func__, __LINE__);
        return false;
    }
    // 1. 获取头部信息,和文件内容
    pBootHead = (PSTRUCT_RKBOOT_HEAD)(data_buf);

    if (pBootHead->uiTag!=0x544F4F42) {
        LOGE("pBootHead->uiTag!=0x544F4F42\n");
        return false;
    }

    if (pBootHead->ucRc4Flag) {
        m_bRc4Disable = true;
    } else {
        m_bRc4Disable = false;
    }

    char loaderDataName[] = "FlashData";
    unsigned char *loaderDataBuffer = NULL;
    m_dwLoaderDataSize = getLoaderSizeAndData(loaderDataName, data_buf, &loaderDataBuffer);
    m_usFlashDataSec = PAGEALIGN(BYTE2SECTOR(m_dwLoaderDataSize))*4;
    LOGI("m_usFlashDataSec = %d, m_dwLoaderDataSize = %d.\n", m_usFlashDataSec, m_dwLoaderDataSize);

    char loaderName[] = "FlashBoot";
    unsigned char *loaderCodeBuffer = NULL;
    m_dwLoaderSize = getLoaderSizeAndData(loaderName, data_buf, &loaderCodeBuffer);
    m_usFlashBootSec = PAGEALIGN(BYTE2SECTOR(m_dwLoaderSize))*4;
    LOGI("m_usFlashBootSec = %d, m_dwLoaderSize = %d.\n", m_usFlashBootSec, m_dwLoaderSize);

    calcIDBCount();
    reserveIDBlock();

    // 4. MakeIDBlockData
    if (getFlashSize(dest_path, &m_FlashSize, &m_FlasBlockNum) != 0) {
        LOGE("getFlashSize error.\n");
        return false;
    }
	LOGI("[%s:%d] m_FlashSize [%lld]\n", __func__, __LINE__, m_FlashSize);

    // 5. download IDBlock
    PBYTE pIDBData=NULL;
    pIDBData = (PBYTE)malloc(uiSecNumPerIDB*SECTOR_SIZE);
    if ( !pIDBData )
        return false;
    memset(pIDBData, 0, uiSecNumPerIDB*SECTOR_SIZE);
    if (MakeIDBlockData(pIDBData, loaderCodeBuffer, loaderDataBuffer) !=0 ) {
		LOGE("[%s:%d] MakeIDBlockData failed.\n", __func__, __LINE__);
        return false;
    }

    if (WriteIDBlock(pIDBData, uiSecNumPerIDB, dest_path) != 0) {
		LOGE("[%s:%d] WriteIDBlock failed.\n", __func__, __LINE__);
        return false;
    }
    free(pIDBData);
    free(loaderCodeBuffer);
    free(loaderDataBuffer);

    return true;
}

