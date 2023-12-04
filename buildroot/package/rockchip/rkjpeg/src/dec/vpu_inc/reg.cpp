/***************************************************************************************************
    File:
        reg.cpp
    Decription:
        register operation
    Author:
        Jian Huan
    Date:
        2010-12-8 16:40:04
 **************************************************************************************************/
#include "reg.h"

rkdecregister::rkdecregister()
{
    regBase = 0;
}

rkdecregister::~rkdecregister()
{
    regBase = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*HW Config as follows*/
const RK_U32 regMask[33] = {        0x00000000,
                                    0x00000001, 0x00000003, 0x00000007, 0x0000000F,
                                    0x0000001F, 0x0000003F, 0x0000007F, 0x000000FF,
                                    0x000001FF, 0x000003FF, 0x000007FF, 0x00000FFF,
                                    0x00001FFF, 0x00003FFF, 0x00007FFF, 0x0000FFFF,
                                    0x0001FFFF, 0x0003FFFF, 0x0007FFFF, 0x000FFFFF,
                                    0x001FFFFF, 0x003FFFFF, 0x007FFFFF, 0x00FFFFFF,
                                    0x01FFFFFF, 0x03FFFFFF, 0x07FFFFFF, 0x0FFFFFFF,
                                    0x1FFFFFFF, 0x3FFFFFFF, 0x7FFFFFFF, 0xFFFFFFFF
                           };

/* { SWREG, BITS, POSITION } */
RK_U32 hwDecRegSpec[HWIF_LAST_REG + 1][3] = {
    /* include script-generated part */
#include "6731table.h"
    /* HWIF_DEC_IRQ_STAT */ {55,  10, 4},
    /* HWIF_PP_IRQ_STAT */  {40, 2, 2},
    /* dummy entry */       { 0, 0,  0},
};


/***************************************************************************************************
    Func:
        SetRegisterFile
    Description:
        config register-mapped memory (not hw register)
    Author:
        Jian Huan
    Date:
        2010-12-8 16:57:35
 **************************************************************************************************/
void rkdecregister::SetRegisterFile(RK_U32 id, RK_U32 value)
{
    RK_U32 tmp;

    tmp = regBase[hwDecRegSpec[id][0]];
    tmp &= ~(regMask[hwDecRegSpec[id][1]] << hwDecRegSpec[id][2]);
    tmp |= (value & regMask[hwDecRegSpec[id][1]]) << hwDecRegSpec[id][2];
    regBase[hwDecRegSpec[id][0]] = tmp;
}

void  rkdecregister::SetRegisterMapAddr(RK_U32 *regMemAddr)
{
    regBase = regMemAddr;
}

/***************************************************************************************************
    Func:
        GetRegisterFile
    Description:
        read register-mapped memory (not hw register)
    Author:
        Jian Huan
    Date:
        2010-12-8 16:57:35
 **************************************************************************************************/
RK_U32 rkdecregister::GetRegisterFile(RK_U32 id)
{
    RK_U32 tmp;

    tmp = regBase[hwDecRegSpec[id][0]];
    tmp = tmp >> hwDecRegSpec[id][2];
    tmp &= regMask[hwDecRegSpec[id][1]];
    return (tmp);
}


extern "C"
void *get_rkdecregister_handle(void)
{
    return (void*)new rkdecregister();
}

extern "C"
void  release_rkdecregister_handle(void * handle)
{
    delete (rkdecregister *)handle;
    return;
}

extern "C"
void  set_rkdecregister_map(void * handle, RK_U32 *regMemAddr)
{
    rkdecregister *Creg = (rkdecregister *)handle;
    return Creg->SetRegisterMapAddr(regMemAddr);
}


extern "C"
void rk_SetRegisterFile(void * handle, RK_U32 id, RK_U32 value)
{
    rkdecregister *Creg = (rkdecregister *)handle;
    return Creg->SetRegisterFile(id, value);
}

extern "C"
RK_U32 rk_GetRegisterFile(void * handle, RK_U32 id)
{
    rkdecregister *Creg = (rkdecregister *)handle;
    return Creg->GetRegisterFile(id);
}

