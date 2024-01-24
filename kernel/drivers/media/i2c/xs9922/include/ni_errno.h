/******************************************************************************

 Copyright (C) 2020-2020 ZheJiang XinSheng Electronic Technology CO.,LTD.

 ******************************************************************************
  File Name     : ni_errno.h
  Version       : Initial Draft
  Author        : Lv Zhuqing<lv_zhuqing@dahuatech.com>
  Created       : 2014.3.13
  Last Modified :
  Description   : define the format of error code
  Function List :
  NIstory       :
  1.Date        : 2014/3/13
    Author      : 24497
    Modification: Create
******************************************************************************/

#ifndef __NI_ERRNO_H__
#define __NI_ERRNO_H__

#include "ni_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#define NI_ERR_APPID  (0xE0000000L)

typedef enum tagErrorLevel
{
    NI_EN_ERR_LEVEL_DEBUG = 0,  /* debug-level */
    NI_EN_ERR_LEVEL_INFO,       /* informational */
    NI_EN_ERR_LEVEL_NOTICE,     /* normal but significant condition */
    NI_EN_ERR_LEVEL_WARNING,    /* warning conditions */
    NI_EN_ERR_LEVEL_ERROR,      /* error conditions */
    NI_EN_ERR_LEVEL_CRIT,       /* critical conditions */
    NI_EN_ERR_LEVEL_ALERT,      /* action must be taken immediately */
    
    NI_EN_ERR_LEVEL_BUTT
}NI_ERR_LEVEL_E;

/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define NI_DEF_ERR( module, level, errid) \
    ((NI_S32)((NI_ERR_APPID) | ((module) << 16 ) | ((level)<<13) | (errid)))

/* NOTE! the following defined all common error code,
** all module must reserved 0~63 for their common error code
*/
typedef enum tagErrorCode
{
    NI_ERR_SUCCESS        = 0,   /* for C# and VC */
    NI_ERR_INVALID_DEVID  = 1,   /* invalid device ID  */        
    NI_ERR_INVALID_GRPID  = 2,   /* invalid grp ID */
    NI_ERR_INVALID_CHNID  = 3,   /* invalid channel ID */  
    
    NI_ERR_ILLEGAL_PARAM  = 4,   /* at lease one parameter is illegal eg, an illegal enumeration value */
    NI_ERR_EXIST          = 5,   /* resource exists */
    NI_ERR_UNEXIST        = 6,   /* resource unexists  */
    
    NI_ERR_NULL_PTR       = 7,   /* using a NULL pointer */
    
    NI_ERR_NOT_CONFIG     = 8,   /* try to enable device or channel, before configing attribute */

    NI_ERR_NOT_SUPPORT    = 9,   /* operation or type is not supported */
    NI_ERR_NOT_PERM       = 10,  /* operation is not permitted eg. */

    NI_ERR_NOMEM          = 11,  /* failure caused by malloc memory */
    NI_ERR_NOBUF          = 12,  /* failure caused by malloc buffer */

    NI_ERR_BUF_EMPTY      = 13,  /* no data in buffer */
    NI_ERR_BUF_FULL       = 14,  /* no buffer for new data */

    NI_ERR_SYS_NOTREADY   = 15,  /* System is not ready,maybe not initialized or  loaded. Returning the error code when opening
                                  * a device file failed. */

    NI_ERR_INVALID_ADDR   = 16,  /* bad address, eg. used for copy_from_user & copy_to_user  */

    NI_ERR_BUSY           = 17,  /* resource is busy, eg. */

    NI_ERR_REQ_IRQ_FAIL   = 18,  /* request irq failure */

    NI_ERR_NOT_ENABLE     = 19,  /* not enable */
    NI_ERR_NOT_DISABLE    = 20,  /* not disable */

    NI_ERR_TIME_OUT       = 21,  /* timeout */
    
    NI_ERR_BUTT           = 63,  /* maxium code, private error code of all modules must be greater than it  */
}NI_ERR_CODE_E;


/* 
** following is an example for defining error code of VENC module
** #define NI_ERR_VENC_INVALID_STRMID NI_DEF_ERR(NI_ID_VENC, ERR_LEVEL_ERROR, ERR_INVALID_CHNID)
**
*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __NI_ERRNO_H__ */

