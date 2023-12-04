#include "RKLog.h"

bool CRKLog::GetEnableLog()
{
    return m_enable;
}
void CRKLog::SetEnableLog(bool bEnable)
{
    m_enable = bEnable;
}

CRKLog::CRKLog(bool enable)
{

    EnableLog.setContainer(this);
    EnableLog.getter(&CRKLog::GetEnableLog);
    EnableLog.setter(&CRKLog::SetEnableLog);

    m_enable = enable;
}
CRKLog::~CRKLog()
{
}
void CRKLog::Record(const tchar *lpFmt, ...)
{
    /************************* 输出到日志 ***********************/
    va_list ap;
    va_start(ap, lpFmt);
    printf("librkupdate_");
    vfprintf(stdout, lpFmt, ap);
    printf("\r\n");
    va_end(ap);
}

bool CRKLog::SaveBuffer(tstring fileName, PBYTE lpBuffer, DWORD dwSize)
{
    FILE *file;
    file = fopen(fileName.c_str(), _T("wb+"));
    if (!file)
    {
        return false;
    }
    fwrite(lpBuffer, 1, dwSize, file);
    fclose(file);
    return true;
}
void CRKLog::PrintBuffer(tstring &strOutput, PBYTE lpBuffer, DWORD dwSize, UINT uiLineCount)
{
    UINT i, count;
    tchar strHex[32];
    strOutput = _T("");
    for (i = 0, count = 0; i < dwSize; i++, count++)
    {
        if (count >= uiLineCount)
        {
            strOutput += _T("\r\n");
            count = 0;
        }
        sprintf(strHex, _T("%02X"), lpBuffer[i]);
        strOutput = strOutput + _T(" ") + strHex;

    }
}