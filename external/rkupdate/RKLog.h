#ifndef RKLOG_HEADER
#define RKLOG_HEADER
#include "DefineHeader.h"

class CRKLog
{
public:
    bool GetEnableLog();
    void SetEnableLog(bool bEnable);
    property<CRKLog, bool, READ_WRITE> EnableLog;
    property<CRKLog, tstring, READ_ONLY> LogSavePath;
    CRKLog(bool enable = true);
    ~CRKLog();
    bool SaveBuffer(tstring fileName, PBYTE lpBuffer, DWORD dwSize);
    void PrintBuffer(tstring &strOutput, PBYTE lpBuffer, DWORD dwSize, UINT uiLineCount = 16);
    void Record(const tchar *lpFmt, ...);

protected:
private:
    bool    m_enable;
};

#endif