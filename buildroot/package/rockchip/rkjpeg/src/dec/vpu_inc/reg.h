#ifndef _REG_H_
#define _REG_H_
/***************************************************************************************************
    File:
        reg.h
    Description:
        rkdec register class
    Author:
        Jian Huan
    Date:
        2010-12-8 16:43:41
***************************************************************************************************/
#ifdef DRM_LINUX
#include "vpu_macro.h"
#else
#include "vpu_api.h"
#endif
#include "rkregdrv.h"

class rkdecregister
{

public:
    rkdecregister();
    ~rkdecregister();

    void    SetRegisterFile(RK_U32 id, RK_U32 value);
    RK_U32  GetRegisterFile(RK_U32 id);

    void  SetRegisterMapAddr(RK_U32 *regMemAddr);


private:
    RK_U32  *regBase;

};
#endif
