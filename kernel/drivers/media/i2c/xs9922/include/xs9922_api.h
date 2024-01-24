/*************************************************
  Copyright (C) 2020-2020 ZheJiang XinSheng Electronic Technology CO.,LTD.
  文件名:    xs9922_api.h
  作  者:   chen_li(34308)<chen_li3@dahuatech.com>
  版  本:   1.0.0
  日  期：   2020-07-24
  描  述:   此处为文件具体描述，包括主要功能
  
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

#ifndef _NIC_NI_API_H_ 
#define _NIC_NI_API_H_

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /*__cplusplus*/ 


/*******************************************************************************
 * 函数名  : NI_SDK_RecvCo485Buf
 * 描  述    : 接收同轴485数据
 * 输  入  :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   pstCo485Buf     同轴485数据信息指针，
                               长度最大64字节，返回
                               实际接收到的字节数
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_RecvCo485Buf(NI_U8 u8DevId, NI_U8 u8ChnId, NI_CO485_BUF_S *pstCo485Buf);


/*******************************************************************************
 * 函数名  : NI_SDK_SendCo485Buf
 * 描  述  :   发送同轴485数据
 * 输  入  :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   pstCo485Buf     同轴485数据信息指针
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_SendCo485Buf(NI_U8 u8DevId, NI_U8 u8ChnId, NI_CO485_BUF_S *pstCo485Buf);


/*******************************************************************************
 * 函数名  : NI_SDK_SetEqLevel
 * 描  述  :   适应EQ等级 
 * 输  入  :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   bAutoEn         是否自适应EQ等级
 *         :   u8EqLevel       手动配置时的EQ等级[0 - 40]
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_AdaptEqLevel(NI_U8 u8DevId, NI_U8 u8ChnId, NI_BOOL bAutoEn, NI_U8 u8EqLevel);


/*******************************************************************************
 * 函数名  : NI_SDK_GetEqLevel
 * 描  述    : 获取EQ等级 
 * 输  入  :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   pu8EqLevel      当前EQ等级
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_GetEqLevel(NI_U8 u8DevId, NI_U8 u8ChnId, NI_U8 *pu8EqLevel);


/*******************************************************************************
 * 函数名  : NI_SDK_SetFreeRun
 * 描  述    : 设置视频FreeRun输出
 * 输  入  :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   pstFreeRunAttr  FreeRun属性
 *         :   bEnable         FreeRun使能开关
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_SetFreeRun(NI_U8 u8DevId, NI_U8 u8ChnId, NI_FREERUN_ATTR_S *pstFreeRunAttr, NI_BOOL bEnable);


/*******************************************************************************
 * 函数名  : NI_SDK_SetFreeRun
 * 描  述    : 调节视频颜色属性 
 * 输  入    :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   pstImgAttr      图像参数
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_SetImgAttr(NI_U8 u8DevId, NI_U8 u8ChnId, NI_IMG_ATTR_S *pstImgAttr);


/*******************************************************************************
 * 函数名  : NI_SDK_GetImgAttr
 * 描  述  :   获取视频颜色属性
 * 输  入  :  参数        描述
 *         :   u8DevId         设备ID
 *         :   u8ChnId         通道ID
 *         :   bUserCofig      图像参数获取选择（0：获取默认推荐参数；1：获取用户配置的参数）
 *         :   pstImgAttr      图像参数
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_GetImgAttr(NI_U8 u8DevId, NI_U8 u8ChnId, NI_BOOL bUserCofig, NI_IMG_ATTR_S *pstImgAttr);


/*******************************************************************************
 * 函数名  : NI_SDK_GetVideoStatus
 * 描  述   :  获取当前视频状态信息
 * 输  入  :  参数        描述
 *         :   u8DevId             设备ID
 *         :   u8ChnId             通道ID
 *         :   pstVideoStatusInfo  当前视频状态信息
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_GetVideoStatus(NI_U8 u8DevId, NI_U8 u8ChnId, NI_VIDEO_STATUS_INFO_S *pstVideoStatusInfo);


/*******************************************************************************
 * 函数名  : NI_SDK_GetDevAttr
 * 描  述    : 获取设备属性 
 * 输  入  :  参数        描述
 *         :   u8DevId                  设备ID
 *         :   pstNiDevAttr         设备属性
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_GetDevAttr(NI_U8 u8DevId, NI_DEV_ATTR_S *pstNiDevAttr);


/*******************************************************************************
 * 函数名  : NI_SDK_SetDevAttr
 * 描  述    : 设置设备属性 
 * 输  入  :  参数        描述
 *         :   u8DevId                  设备ID
 *         :   pstNiDevAttr         设备属性
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_SetDevAttr(NI_U8 u8DevId, NI_DEV_ATTR_S *pstNiDevAttr);


/*******************************************************************************
 * 函数名  : NI_SDK_SetMsgLevel
 * 描  述    : 设置信息输出等级 
 * 输  入  :  参数        描述
 *         :   enMsgType         信息等级
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_SetMsgLevel(NI_MSG_TYPE_E enMsgType);


/*******************************************************************************
 * 函数名  : NI_SDK_DetectThr
 * 描  述    : 通道检测，监测输入信号，并根据输入的改变自动配置输出
             用户需开启线程对该函数进行循环调用
 * 输  入  :  参数        描述
 *         :   u8DevId              设备ID
 *         :   u8ChnId              通道ID
 *         :   pstDetectOpt         检测配置属性
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_DetectThr(NI_U8 u8DevId, NI_U8 u8ChnId, NI_DETECT_OPT_S *pstDetectOpt);


/*******************************************************************************
 * 函数名  : NI_SDK_Register
 * 描  述    : 设备注册 
 * 输  入  :  参数        描述
 *         :   pstRegInfo         注册信息
 * 输  出  : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_Register(NI_REG_INFO_S *pstRegInfo);


/*******************************************************************************
 * 函数名  : NI_SDK_UnRegister
 * 描  述    : 设备去注册 
 * 输  入    : 参数        描述
 *         : 无         无
 * 输  出    : 无
 * 返回值  : NI_SUCCESS: 成功
 *           NI_FAILURE: 失败
 *******************************************************************************/
NI_S32 NI_SDK_UnRegister(NI_VOID);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /*_NIC_NI_API_H_*/
