/*************************************************************************
    > File Name: crc.h
    > Author: jkand.huang
    > Mail: jkand.huang@rock-chips.com
    > Created Time: Tue 04 Jun 2019 02:34:40 PM CST
 ************************************************************************/

#ifndef _CRC_H
#define _CRC_H
#include "defineHeader.h"
extern USHORT CRC_16(BYTE * aData, UINT aSize);
extern UINT CRC_32(PBYTE pData, UINT ulSize, UINT uiPreviousValue=0);
extern void P_RC4(BYTE * buf, USHORT len);
extern void bch_encode(BYTE *encode_in, BYTE *encode_out);
extern USHORT CRC_CCITT(UCHAR *p, UINT CalculateNumber);
extern void generate_gf();
extern void gen_poly();
#endif
