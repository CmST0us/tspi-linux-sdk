#include "RKImage.h"
#include "MD5Checksum.h"

extern int IsRK3308_Platform();
extern int Compatible_rk3308bs_loader();

DWORD CRKImage::GetVersion()
{
    return m_version;
}
DWORD CRKImage::GetMergeVersion()
{
    return m_mergeVersion;
}
STRUCT_RKTIME CRKImage::GetReleaseTime()
{
    return m_releaseTime;
}
ENUM_RKDEVICE_TYPE CRKImage::GetSupportDevice()
{
    return m_supportDevice;
}
ENUM_OS_TYPE CRKImage::GetOsType()
{
    UINT *pOsType;
    pOsType = (UINT *)&m_reserved[4];
    return (ENUM_OS_TYPE) * pOsType;
}
USHORT CRKImage::GetBackupSize()
{
    USHORT *pBackupSize;
    pBackupSize = (USHORT *)&m_reserved[12];
    return *pBackupSize;
}
DWORD CRKImage::GetBootOffset()
{
    return m_bootOffset;
}
DWORD CRKImage::GetBootSize()
{
    return m_bootSize;
}
DWORD CRKImage::GetFWOffset()
{
    return m_fwOffset;
}
long long CRKImage::GetFWSize()
{
    return m_fwSize;
}

FILE *CRKImage::GetFWFileHandle()
{
    return m_pFile;
}

char *CRKImage::GetFwPath()
{
    return m_imgPath;
}

bool CRKImage::Md5Check(long long nCheckSize)
{
    printf("In Md5Check\n");
    tstring strNewMd5 = _T("");
    strNewMd5 = CMD5Checksum::GetMD5(m_pFile, nCheckSize);
    if (strNewMd5.size() == 32)
    {
        BYTE newMd5[32];
        memcpy(newMd5, strNewMd5.c_str(), 32);
        int i, j;
        printf("New Md5:\n");
        for (i = 0; i < 2; i++)
        {
            for (j = 0; j < 16; j++)
            {
                printf("%02X ", newMd5[i * 16 + j]);
            }
            printf("\r\n");
        }
        printf("Old Md5:\n");
        for (i = 0; i < 2; i++)
        {
            for (j = 0; j < 16; j++)
            {
                printf("%02X ", m_md5[i * 16 + j]);
            }
            printf("\r\n");
        }
        if (memcmp(newMd5, m_md5, 32) != 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

bool CRKImage::SaveBootFile(tstring filename)
{
    FILE *file = NULL;
    file = fopen(filename.c_str(), _T("wb+"));
    if (!file)
    {
        return false;
    }
    BYTE buffer[1024];
    DWORD dwBufferSize = 1024;
    DWORD dwBootSize = m_bootSize;
    DWORD dwReadSize;
    fseek(m_pFile, m_bootOffset, SEEK_SET);
    do
    {
        dwReadSize = (dwBootSize >= 1024) ? dwBufferSize : dwBootSize;
        size_t ret = fread(buffer, 1, dwReadSize, m_pFile);
        if (ret != dwReadSize)
        {
            printf("fread error: read:%d, req read:%d", ret, dwReadSize);
        }
        fwrite(buffer, 1, dwReadSize, file);
        dwBootSize -= dwReadSize;
    } while (dwBootSize > 0);
    fclose(file);
    return true;
}

bool CRKImage::SaveFWFile(tstring filename)
{
    FILE *file = NULL;
    file = fopen(filename.c_str(), _T("wb+"));
    if (!file)
    {
        return false;
    }
    BYTE buffer[1024];
    DWORD dwBufferSize = 1024;
    long long dwFWSize = m_fwSize;
    DWORD dwReadSize;
    fseeko(m_pFile, m_fwOffset, SEEK_SET);
    do
    {
        dwReadSize = (dwFWSize >= 1024) ? dwBufferSize : dwFWSize;
        size_t ret = fread(buffer, 1, dwReadSize, m_pFile);
        if (ret != dwReadSize)
        {
            printf("fread error: read:%d, req read:%d", ret, dwReadSize);
        }

        fwrite(buffer, 1, dwReadSize, file);
        dwFWSize -= dwReadSize;
    } while (dwFWSize > 0);
    fclose(file);
    return true;
}

bool CRKImage::GetData(long long dwOffset, DWORD dwSize, PBYTE lpBuffer)
{
    if (dwOffset < 0 || dwSize == 0)
    {
        return false;
    }
    if (dwOffset + dwSize > m_fileSize)
    {
        return false;
    }

    fseeko64(m_pFile, dwOffset, SEEK_SET);
    UINT uiActualRead;
    uiActualRead = fread(lpBuffer, 1, dwSize, m_pFile);
    if (dwSize != uiActualRead)
    {
        return false;
    }
    return true;
}
void CRKImage::GetReservedData(PBYTE &lpData, USHORT &usSize)
{
    lpData = m_reserved;
    usSize = IMAGE_RESERVED_SIZE;
}
int CRKImage::GetMd5Data(PBYTE &lpMd5, PBYTE &lpSignMd5)
{
    lpMd5 = m_md5;
    lpSignMd5 = m_signMd5;
    return m_signMd5Size;
}

CRKImage::CRKImage(tstring filename, bool &bCheck)
{
    Version.setContainer(this);
    Version.getter(&CRKImage::GetVersion);
    MergeVersion.setContainer(this);
    MergeVersion.getter(&CRKImage::GetMergeVersion);
    ReleaseTime.setContainer(this);
    ReleaseTime.getter(&CRKImage::GetReleaseTime);
    SupportDevice.setContainer(this);
    SupportDevice.getter(&CRKImage::GetSupportDevice);
    OsType.setContainer(this);
    OsType.getter(&CRKImage::GetOsType);
    BackupSize.setContainer(this);
    BackupSize.getter(&CRKImage::GetBackupSize);
    BootOffset.setContainer(this);
    BootOffset.getter(&CRKImage::GetBootOffset);
    BootSize.setContainer(this);
    BootSize.getter(&CRKImage::GetBootSize);
    FWOffset.setContainer(this);
    FWOffset.getter(&CRKImage::GetFWOffset);
    FWSize.setContainer(this);
    FWSize.getter(&CRKImage::GetFWSize);
    FwPath.setContainer(this);
    FwPath.getter(&CRKImage::GetFwPath);

    bool bDoMdb5Check = bCheck;
    struct stat64 statBuf;
    m_bootObject = NULL;
    m_pFile = NULL;

    m_signMd5Size = 0;
    memset(m_md5, 0, 32);
    memset(m_signMd5, 0, 256);

    tchar szName[256];
    _tcscpy(szName, filename.c_str());
    if (stat64(szName, &statBuf) < 0)
    {
        bCheck = false;
        printf("CRKImage : stat <%s> happen error.error_reason = %s\n", szName, strerror(errno));
        return;
    }
    if (S_ISDIR(statBuf.st_mode))
    {
        bCheck = false;
        printf("CRKImage : Error! stat mode is DIR\n");
        return;
    }
    m_fileSize = statBuf.st_size;
    memcpy(m_imgPath, szName, sizeof(szName));

    bool bOnlyBootFile = false;
    transform(filename.begin(), filename.end(), filename.begin(), (int(*)(int))tolower);
    if (filename.find(_T(".bin")) != tstring::npos)
    {
        bOnlyBootFile = true;
    }

    m_pFile = fopen(szName, "rb");
    if (!m_pFile)
    {
        bCheck = false;
        printf("CRKImage : fopen <%s> fail,will try use fopen64 \n", szName);

        m_pFile = fopen64(szName, "rb");
        if (!m_pFile)
        {
            bCheck = false;
            printf("CRKImage : fopen64 <%s> fail\n", szName);
            return;
        }
    }

    //code will be error if firmware is signed.md5 is not last 32 byte.
    //  fseeko(m_pFile,-32,SEEK_END);
    //  fread(m_md5,1,32,m_pFile);
    //  fseeko(m_pFile,0,SEEK_SET);
    //  if (!Md5Check())
    //  {
    //      bCheck = false;
    //      return;
    //  }

    int nMd5DataSize;
    long long ulFwSize;
    STRUCT_RKIMAGE_HEAD imageHead;
    if (!bOnlyBootFile)
    {
        fseeko64(m_pFile, 0, SEEK_SET);
        size_t ret = fread((PBYTE)(&imageHead), 1, sizeof(STRUCT_RKIMAGE_HEAD), m_pFile);
        if (ret != sizeof(STRUCT_RKIMAGE_HEAD))
        {
            return;
        }

        if (imageHead.uiTag != 0x57464B52)
        {
            bCheck = false;
            printf("CRKImage :Error! imageHead.uiTag != 0x57464B52\n");
            return;
        }
        if ((imageHead.reserved[14] == 'H') && (imageHead.reserved[15] == 'I'))
        {
            ulFwSize = *((DWORD *)(&imageHead.reserved[16]));
            ulFwSize <<= 32;
            ulFwSize += imageHead.dwFWOffset;
            ulFwSize += imageHead.dwFWSize;
        }
        else
        {
            ulFwSize = imageHead.dwFWOffset + imageHead.dwFWSize;
        }
        nMd5DataSize = GetImageSize() - ulFwSize;
        if (nMd5DataSize >= 160)
        {
            //sign image
            m_bSignFlag = true;
            m_signMd5Size = nMd5DataSize - 32;
            fseeko64(m_pFile, ulFwSize, SEEK_SET);
            size_t ret = fread(m_md5, 1, 32, m_pFile);
            if (ret != 32)
            {
                printf("%s:read error\n", __func__);
            }
            ret = fread(m_signMd5, 1, nMd5DataSize - 32, m_pFile);
            if (ret != nMd5DataSize - 32)
            {
                printf("%s :ret = %d\n", __func__, ret);
            }
        }
        else
        {
            fseeko64(m_pFile, -32, SEEK_END);
            size_t read = fread(m_md5, 1, 32, m_pFile);
            if (read != 32)
            {
                printf("%s: read error\n", __func__);
            }
        }
        if (bDoMdb5Check)
        {
            if (!Md5Check(ulFwSize))
            {
                printf("Md5Check update.img ulFwSize:%ld", ulFwSize);
                //bCheck = false;
                //return;
                bCheck = true;
            }
        }

        m_version = imageHead.dwVersion;
        m_mergeVersion = imageHead.dwMergeVersion;
        m_releaseTime.usYear = imageHead.stReleaseTime.usYear;
        m_releaseTime.ucMonth = imageHead.stReleaseTime.ucMonth;
        m_releaseTime.ucDay = imageHead.stReleaseTime.ucDay;
        m_releaseTime.ucHour = imageHead.stReleaseTime.ucHour;
        m_releaseTime.ucMinute = imageHead.stReleaseTime.ucMinute;
        m_releaseTime.ucSecond = imageHead.stReleaseTime.ucSecond;
        m_supportDevice = imageHead.emSupportChip;
        m_bootOffset = imageHead.dwBootOffset;
        m_bootSize = imageHead.dwBootSize;
        m_fwOffset = imageHead.dwFWOffset;
        m_fwSize = ulFwSize - m_fwOffset;

        memcpy(m_reserved, imageHead.reserved, IMAGE_RESERVED_SIZE);
    }
    else
    {
        m_bootOffset = 0;
        m_bootSize = m_fileSize;
    }

    PBYTE lpBoot;
    lpBoot = new BYTE[m_bootSize];
    fseeko64(m_pFile, m_bootOffset, SEEK_SET);
    size_t ret = fread(lpBoot, 1, m_bootSize, m_pFile);
    if (ret != m_bootSize)
    {
        printf("%s : read error\n", __func__);
    }
    bool bRet;
    m_bootObject = new CRKBoot(lpBoot, m_bootSize, bRet);
    if (!bRet)
    {
        bCheck = false;
        printf("CRKImage :Error! new CRKBoot fail!\n");
        return;
    }
    if (bOnlyBootFile)
    {
        m_supportDevice = m_bootObject->SupportDevice;
        UINT *pOsType;
        pOsType = (UINT *)&m_reserved[4];
        *pOsType = (UINT)RK_OS;
        fclose(m_pFile);
        m_pFile = NULL;
    }
    bCheck = true;
}
CRKImage::~CRKImage()
{
    if (m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    if (m_bootObject)
    {
        delete m_bootObject;
        m_bootObject = NULL;
    }
}

long long CRKImage::GetImageSize()
{
    return m_fileSize;
}
