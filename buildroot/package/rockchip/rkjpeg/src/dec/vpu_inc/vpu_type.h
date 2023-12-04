/***************************************************************************************************
    File:
        vpu_type.h
    Description:
        type definition
    Author:
        Jian Huan
    Date:
        2010-12-8 16:48:32
***************************************************************************************************/
#ifndef _VPU_TYPE_H
#define _VPU_TYPE_H

#ifdef WIN32
typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned __int64        RK_U64;

typedef signed char             RK_S8;
typedef signed short            RK_S16;
typedef signed int              RK_S32;
typedef signed __int64          RK_S64;

typedef void                    VOID;

#else

typedef unsigned char           RK_U8;
typedef unsigned short          RK_U16;
typedef unsigned int            RK_U32;
typedef unsigned long long int  RK_U64;


typedef signed char             RK_S8;
typedef signed short            RK_S16;
typedef signed int              RK_S32;
typedef signed long long int    RK_S64;

typedef void                    VOID;

#endif

#endif /*_VPU_TYPE_H*/
