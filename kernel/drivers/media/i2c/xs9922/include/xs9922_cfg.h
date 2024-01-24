/*************************************************
  Copyright (C) 2020-2020 ZheJiang XinSheng Electronic Technology CO.,LTD.
  文件名:    xs9910_cfg.h
  作  者:   chen_li(34308)<chen_li3@dahuatech.com>
  版  本:   1.0.0
  日  期： 2020-07-24
  描  述:   提供各自编译的默认编译平台
  
            1、使用说明
            xx
           
            2、局限性
            xx
  
  修订历史:
  1. 日    期:
     修订版本:
     作    者:
     修订备注:
     
  2. 日    期:
     修订版本:
     作    者:
     修订备注:
*************************************************/

#ifndef __NI_CFG_H__
#define __NI_CFG_H__

/* 可开关编译宏，为了兼容各个编译器，只是用简单的
    ifdef进行判断
*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/* Win */
#undef NI_NISDK_WIN

/* Linux 驱动层 */
//#define NI_NISDK_LINUX_DRV
#undef NI_NISDK_LINUX_DRV

/* Linux 应用层 */
#define NI_NISDK_LINUX_LIB
//#undef NI_NISDK_LINUX_LIB

/* MDK编译器 */
#undef NI_NISDK_MDK

/* IAR编译器 */
#undef NI_NISDK_IAR

#ifdef MODULE_NAME
#undef MODULE_NAME
#endif

/* 
    根据编译开关确定头文件包含,并对平台区分
    进行宏定义
*/

/* Linux库 */
#ifdef NI_NISDK_LINUX_LIB

	//#include <stdio.h>
	//#include <pthread.h>
	//#include <string.h>
	//#include <sys/time.h>
	//#include <sys/types.h>
	//#include <unistd.h>
	
    /* 定义导出函数声明 */
    #define NI_DEF_API
    
    /* 定义本地函数声明 */
    #define NI_DEF_LOCAL               static
    
    /* 定义函数符号导出 */
    #define NI_DEF_EXP_SYMBOL(x)
    
    /* 定义inline */
    #define NI_DEF_INLINE              inline
    
    /* 定义全局变量是否使用static或者const */
    #define NI_DEF_GLOBAL_VAR          const
    
    /* 使用系统C库 */
    //#define NI_DEF_DEFAULTC
    #undef NI_DEF_DEFAULTC

    #define NI_DEF_XDATA
    
    
    /* 定义模块名 */
    #define MODULE_NAME                 "xs9922_sdk.a"

#endif

#ifdef NI_NISDK_MDK
	
    /* 定义导出函数声明 */
    #define NI_DEF_API
    
    /* 定义本地函数声明 */
    #define NI_DEF_LOCAL					static               
    
    /* 定义函数符号导出 */
    #define NI_DEF_EXP_SYMBOL(x)
    
    /* 定义inline */
    #define NI_DEF_INLINE
    
    /* 定义全局变量是否使用static或者const */
    #define NI_DEF_GLOBAL_VAR			static

	#define NI_DEF_XDATA
    
    /* 使用系统C库 */
    #define NI_DEF_DEFAULTC
	//#undef NI_DEF_DEFAULTC
    
    
    /* 定义模块名 */
    #define MODULE_NAME                 "xs9922_sdk.a"
		
		#pragma anon_unions

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */



#endif /* __NI_CFG_H__ */
