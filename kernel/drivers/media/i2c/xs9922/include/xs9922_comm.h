/*************************************************
  Copyright (C) 2020-2020 ZheJiang XinSheng Electronic Technology CO.,LTD.
  文件名:    xs9922_commm.h
  作  者:   chen_li(34308)<chen_li3@dahuatech.com>
  版  本:   1.0.0
  日  期： 2020-07-24
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

#ifndef __NI_COMM_H__
#define __NI_COMM_H__

#include "ni_type.h"
#include "ni_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */


#define NI_ID_NI                    (0x22)
#define NI_ERR_NI_INVALID_DEVID     NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_INVALID_DEVID)
#define NI_ERR_NI_INVALID_CHNID     NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_INVALID_CHNID)
#define NI_ERR_NI_ILLEGAL_PARAM     NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_ILLEGAL_PARAM)
#define NI_ERR_NI_NULL_PTR          NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_NULL_PTR)
#define NI_ERR_NI_NOT_CONFIG        NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_NOT_CONFIG)
#define NI_ERR_NI_NOT_SUPPORT       NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_NOT_SUPPORT)
#define NI_ERR_NI_NOT_PERM          NI_DEF_ERR(NI_ID_NI, NI_EN_ERR_LEVEL_ERROR, NI_ERR_NOT_PERM)



#define NI_MAX_CHIPS             (1)
#define NI_MAX_LANE_NUMS	     (4)
#define NI_MAX_CHNS              (4)
#define NI_MAX_LINEIN_CHNS		 (5)

/*定义消息类型*/
typedef enum tagNiMsgType
{
    NI_MSG_TYPE_SILENT,                                     /*静默方式*/
    NI_MSG_TYPE_ERR,                                        /*错误级别*/
    NI_MSG_TYPE_WARN,                                       /*警告级别*/
    NI_MSG_TYPE_INFO,                                       /*信息级别*/
    NI_MSG_TYPE_DEBUG,                                      /*调试级别*/
    
    NI_MSG_TYPE_BUTT
}NI_MSG_TYPE_E;


/*错误码类型*/
typedef enum tagNiErrId
{
    NI_ERRID_SUCCESS = 0,                  /*正常状态*/             
    NI_ERRIF_FAILED,                       /*通用错误*/           
    NI_ERRID_TIMEOUT,                      /*超时错误*/            
	NI_ERRID_INVPARAM,                     /*参数错误*/                 
	NI_ERRID_NOINIT,                       /*未初始化*/                
	NI_ERRID_INVPOINTER,                   /*空指针*/                 
	NI_ERRID_UNSUPPORT,                    /*不支持*/           
    NI_ERRID_UNKNOW,                       /*未知错误*/            
} NI_ERR_ID_E;


typedef enum tagNiVideoFormat
{
    NI_CVI_1280x720_25HZ,      //0
    NI_CVI_1280x720_30HZ,      //1
    NI_CVI_1280x720_50HZ,      //2
    NI_CVI_1280x720_60HZ,      //3
    NI_CVI_1920x1080_25HZ,     //4
    NI_CVI_1920x1080_30HZ,     //5
    NI_CVI_1280x720_30HZ_V20,  //6
    NI_CVI_1280x720_60HZ_V20,  //7
    NI_CVI_1920x1080_30HZ_V20, //8
    
    NI_AHD_1280x720_25HZ,      //9
    NI_AHD_1280x720_30HZ,      //10
    NI_AHD_1280x720_50HZ,      //11
    NI_AHD_1280x720_60HZ,      //12
    NI_AHD_1920x1080_25HZ,     //13
    NI_AHD_1920x1080_30HZ,     //14

    NI_TVI_1280x720_25HZ,      //15
    NI_TVI_1280x720_30HZ,      //16
    NI_TVI_1280x720_50HZ,      //17
    NI_TVI_1280x720_60HZ,      //18
    NI_TVI3_1280x720_25HZ,     //19
    NI_TVI3_1280x720_30HZ,     //20
    NI_TVI_1920x1080_25HZ,     //21
    NI_TVI_1920x1080_30HZ,     //22
    
    NI_SD_NTSC_JM,             //23
    NI_SD_NTSC_443,            //24
    NI_SD_PAL_M,               //25
    NI_SD_PAL_60,              //26
    NI_SD_PAL_CN,              //27
    NI_SD_PAL_BGHID,           //28
    
    NI_VIDEO_FMT_BUTT
}NI_VIDEO_FORMAT_E;


typedef struct tagNiImgAdj
{
    NI_BOOL       bUserColor;                                  /*用户调节color使能*/
    NI_U8         u8Brightness;                                /*亮度，取值范围:[0,0xff];默认值:0x00*/
    NI_U8         u8Contrast;                                  /*对比度，取值范围:[0,0xff];默认值:0x80*/
    NI_U8         u8Saturation;                                /*饱和度，取值范围:[0,0xff];默认值:0x80*/
    NI_U8         u8Hue;                                       /*色调，取值范围:[0,0xff];默认值:0x00*/
    NI_U8         u8Gain;                                      /*增益，不开放*/
    NI_U8         u8WhiteBalance;                              /*白电平,不开放*/
    NI_U8         u8Sharpness;                                 /*锐度，取值范围:[0,0x1f];默认值:0x00*/
}NI_IMG_ADJ_S;


typedef struct tagNiImgOffset
{
    NI_BOOL   bUserCofig;                                   /*用户配置或者默认配置选择*/
    NI_U8     u8HorToward;                                  /*水平偏移方向，[0:向左; 1:向右]*/
    NI_U8     u8HorOffset;                                  /*水平偏移量，取值范围:[0,0x7f]*/ 
	NI_U8     u8VerToward;                                  /*垂直偏移方向，[0:向上; 1:向下]*/
    NI_U8     u8VerOffset;                                  /*垂直偏移量，取值范围:[0,0x7f]*/ 
}NI_IMG_OFFSET_S;


typedef struct tagNiImgAttr
{
	NI_IMG_ADJ_S stImgAdj;                               /*调整图像参数*/
	NI_IMG_OFFSET_S stImgOffset;                         /*调整图像偏移*/ 
}NI_IMG_ATTR_S;


typedef enum tagNiCableType
{
    NI_CABLE_TYPE_COAXIAL,                                /*同轴线缆*/
    NI_CABLE_TYPE_UTP_10OHM,                              /*10 ohm阻抗双绞线*/   
    NI_CABLE_TYPE_AVIATION,                               /*航空头车载线缆*/
    
    NI_CABLE_TYPE_BUTT 
} NI_CABLE_TYPE_E;


/*视频状态*/
typedef enum
{
    NI_VIDEO_CONNECT = 0,                               /*视频正常接入*/
    NI_VIDEO_LOST = 1,                                  /*视频丢失*/  
    NI_VIDEO_BUT,
}NI_VIDEO_STATUS_E;


typedef struct tagNiVideoStatusInfo
{
    NI_VIDEO_STATUS_E       enVideoLost;                   /*1视频丢失，0 视频恢复*/
    NI_VIDEO_FORMAT_E       enVideoOutFormat;              /*当前视频输出制式*/
    NI_VIDEO_FORMAT_E       enVideoInFormat;               /*当前检测到的视频制式*/
}NI_VIDEO_STATUS_INFO_S;

typedef enum tagNiNetraMode
{
    NI_NETRA_MODE_DOUBLE,                                  /*双头格式*/
    NI_NETRA_MODE_SINGLE,                                  /*单头格式*/    
    
    NI_NETRA_MODE_BUTT
}NI_NETRA_MODE_E;

typedef enum tagNiChnIdPos
{
    NI_CHN_ID_POS_NOID,                                /*ID号不加*/
    NI_CHN_ID_POS_HBLANK,                              /*ID号加在行消隐区*/   
    NI_CHN_ID_POS_HEADINFO,                            /*ID号加在头信息中*/
    NI_CHN_ID_POS_BOTH,                                /*ID号同时出现在行消隐区和头信息中*/
    
    NI_CHN_ID_POS_BUTT
}NI_CHN_ID_POS_E;

typedef enum tagNiChnIdVal
{
    NI_CHN_ID_VAL_0,                                           /*通道ID 0*/
    NI_CHN_ID_VAL_1,                                           /*通道ID 1*/
    NI_CHN_ID_VAL_2,                                           /*通道ID 2*/
    NI_CHN_ID_VAL_3,                                           /*通道ID 3*/  
    
    NI_CHN_ID_VAL_BUTT,
}NI_CHN_ID_VAL_E;

typedef struct tagNiVoIdAttr
{
    NI_CHN_ID_POS_E    enChnIdPos;                      /*选择通道ID在数据流中的位置*/
    NI_CHN_ID_VAL_E    enChnId;                         /*视频输出通道对应的ID号码*/
}NI_VO_ID_ATTR_S;

typedef enum tagNiInputChn
{
    NI_INPUT_CHN_0,                                        /*通道0*/
    NI_INPUT_CHN_1,                                        /*通道1*/
    NI_INPUT_CHN_2,                                        /*通道2*/
    NI_INPUT_CHN_3,                                        /*通道3*/ 
    
    NI_INPUT_CHN_BUTT,
}NI_INPUT_CHN_E;


typedef struct tagNiVoTdmInfo
{   
    NI_INPUT_CHN_E  enFirstInputChn;                    /*第一路信号源自*/
    NI_INPUT_CHN_E  enSecondInputChn;                   /*第二路信号源自*/
}NI_VO_TDM_INFO_S;

typedef enum tagNiVoBitMode
{
    NI_VO_MODE_BT1120,                                     /*BT1120*/
    NI_VO_MODE_BT656,                                      /*BT656*/
    
    NI_VO_MODE_BUTT
}NI_VO_BIT_MODE_E;


typedef enum tagNiVoHdGmMode
{
	NI_VO_HD_GM_CLOSE = 0,                            /*消隐区不压缩*/ 
	NI_VO_HD_GM_OPEN,                                 /*消隐区压缩*/
	
    NI_VO_HD_GM_MODE_BUTT,
}NI_VO_HD_GM_MODE_E;


typedef enum tagNiVoSdFmt
{
    NI_VO_SD_MODE_720H,                                /*720H*/
    NI_VO_SD_MODE_960H,                                /*960H*/
    
    NI_VO_SD_MODE_BUTT
}NI_VO_SD_FMT_E;


typedef enum tagNiVoClkEdge
{
    NI_VO_CLK_EDGE_RISING,                             /*上升沿采集*/
    NI_VO_CLK_EDGE_DUAL,                               /*上下边沿采集模式*/   
    
    NI_VO_CLK_EDGE_BUTT
}NI_VO_CLK_EDGE_E;


typedef enum tagNiVoBufMode
{
    NI_VO_BUF_DEFAULT = 0,                           /*默认延时，内部设置*/
    NI_VO_BUF_MANUAL, 								 /*手动配置延时*/
    
    NI_VO_BUF_BUTT                                  
} NI_VO_BUF_MODE_E;


typedef struct tagNiVoBuf
{
    NI_VO_BUF_MODE_E enVoBufMode;                     /*视频输出延时编码模式选择*/
    NI_U16  	     u16VoBufValue;                   /*视频输出设置启动编码的延时时间，取值范围:[0,0x1fff];默认值:0x00*/
}NI_VO_BUF_S;


typedef enum tagNiVoClkMode
{
    NI_CLK_MODE_37_125 = 0,     					/*Half模式输出37.125M时钟*/
    NI_CLK_MODE_74_25 = 1, 						    /*Half模式输出74.25M时钟*/
    NI_CLK_MODE_148_5 = 2,							/*Half模式输出148.5M时钟*/
    NI_CLK_MODE_144 = 3, 							/*Half模式输出144M时钟*/
    NI_CLK_MODE_BUT,
}NI_VO_CLK_MODE_E;


//typedef struct tagNiVoHalfAttr
//{
//    NI_BOOL bHalfEn;								/*视频输出Half模式使能*/
//	NI_VO_CLK_MODE_E enClkMode;					    /*Half模式输出时钟选择*/
//} NI_VO_HALF_ATTR_S;


typedef struct tagNiVoAttr
{                 
    NI_BOOL            bDataSeqInv;					/*VO输出数据按bit位反转*/
    NI_BOOL            bDataSwapEn;					/*VO输出数据按Byt位反转*/
    NI_NETRA_MODE_E    enNetraMode;                 /*高清视频输入头格式选择*/
    NI_VO_ID_ATTR_S    stChnIdAttr;                 /*通道ID配置*/	
    NI_VO_TDM_INFO_S   stVoTdmInfo;					/*视频输出在2x模式下通道信号源选择*/
	NI_VO_BIT_MODE_E   enVoBitMode; 			    /*输出模式选择*/
	NI_VO_HD_GM_MODE_E enGmMode;					/*当前VO通道GM模式配置*/
	NI_VO_SD_FMT_E     enSdFormat;					/*VO标清输出分辨率配置*/
    NI_VO_CLK_EDGE_E   enVoClkEdge;                 /*选择时钟边沿采模式*/
    NI_VO_BUF_S        stVoBuffer;					/*当前VO通道延时编码配置*/
    NI_INPUT_CHN_E     enChnDataSrc;                /*当前VO通道映射的输入数据端口*/
    NI_BOOL 		   bHalfEn;						/*视频输出Half模式使能*/
	NI_VO_CLK_MODE_E   enClkMode;					/*Half模式输出时钟选择*/
	
}NI_VO_ATTR_S;

typedef enum tagNiFreeRunColor
{
    NI_FREERUN_COLOR_WHITE,                             /*白色*/
    NI_FREERUN_COLOR_YELLOW,                            /*黄色*/  
    NI_FREERUN_COLOR_CYAN,                              /*青色*/ 
    NI_FREERUN_COLOR_GREEN,                             /*绿色*/    
    NI_FREERUN_COLOR_MAGENTA,                           /*洋红色*/ 
    NI_FREERUN_COLOR_RED,                               /*红色*/
    NI_FREERUN_COLOR_BLUE,                              /*蓝色*/
    NI_FREERUN_COLOR_BLACK,                             /*黑色*/
    
    NI_FREERUN_COLOR_BUTT
}NI_FREERUN_COLOR_E;

typedef struct tagNiFreeRunAttr
{
    NI_FREERUN_COLOR_E    enFreeRunColor;				/*Freerun单色品颜色*/
    NI_VIDEO_FORMAT_E     enFreeRunFormat;				/*Freerun单色品制式*/
}NI_FREERUN_ATTR_S;


typedef struct tagNiVideoAttr
{
    NI_VO_ATTR_S    	stVoAttr;						/*VO参数配置*/
    NI_FREERUN_ATTR_S   stFreeRunAttr;					/*Freerun配置*/
}NI_VIDEO_ATTR_S;


typedef enum tagNiCoAudMode
{
    NI_COAUD_MODE_OLD,									/*同轴音频老方案*/
    NI_COAUD_MODE_NEW,									/*同轴音频新方案*/
    
    NI_COAUD_MODE_BUTT
}NI_COAUD_MODE_E;
	

typedef enum tagNiAudSampleRate
{
    NI_AUD_SAMPLERATE_8K,                       /*8K采样频率*/
    NI_AUD_SAMPLERATE_16K,                      /*16K采样频率*/
    NI_AUD_SAMPLERATE_32K,                      /*32K采样频率*/
    NI_AUD_SAMPLERATE_48K,                      /*48K采样频率*/
    NI_AUD_SAMPLERATE_44_1K,                    /*44.1K采样频率*/

    NI_AUD_SAMPLERATE_BUTT
}NI_AUD_SAMPLERATE_E;


typedef enum tagNiAudClkMode
{
	NI_AUD_CLK_MODE_MASTER, 						   /*主模式*/
    NI_AUD_CLK_MODE_SLAVE,                             /*从模式*/
    
    NI_AUD_MODE_BUTT
}NI_AUD_CLK_MODE_E;


typedef enum tagNiAudIntfMode
{
    NI_AUD_INTF_MODE_IIS,							/*I2S模式*/
    NI_AUD_INTF_MODE_DSP,							/*DSP模式*/
    
    NI_AUD_INTF_MODE_BUTT
}NI_AUD_INTF_MODE_E;


typedef enum tagNiAudEncSrc
{
    NI_AUD_SRC_LINEIN,								/*模拟音频输入*/
    NI_AUD_SRC_COAXIAL,								/*同轴音频输入*/

    NI_AUD_SRC_BUTT
}NI_AUD_ENC_SRC_E;


typedef enum tagNiAudDataWidth
{
    NI_AUD_DATA_16BIT,								/*16bit位宽*/
    NI_AUD_DATA_20BIT,								/*20bit位宽*/
    NI_AUD_DATA_24BIT,								/*24bit位宽*/

    NI_AUD_DATA_BUTT
}NI_AUD_DATA_WIDTH_E;


typedef enum tagNiAudDecFreq
{
    NI_AUD_DEC_FREQ_32,                       /*位同步与帧同步频率的倍数关系:32*/
    NI_AUD_DEC_FREQ_64,                       /*位同步与帧同步频率的倍数关系:64*/
    NI_AUD_DEC_FREQ_128,                      /*位同步与帧同步频率的倍数关系:128*/
    NI_AUD_DEC_FREQ_256,                      /*位同步与帧同步频率的倍数关系:256*/

    NI_AUD_DEC_FREQ_BUTT
}NI_AUD_DEC_FREQ_E;


typedef enum tagNiAudDacSrc
{
    NI_AUD_DAC_SRC_IIS,							/*音频数据来自I2S*/
    NI_AUD_DAC_SRC_ADC1,						/*音频数据来自LINE_IN1*/
    NI_AUD_DAC_SRC_ADC2,						/*音频数据来自LINE_IN2*/
    NI_AUD_DAC_SRC_ADC3,						/*音频数据来自LINE_IN3*/
    NI_AUD_DAC_SRC_ADC4,						/*音频数据来自LINE_IN4*/
    NI_AUD_DAC_SRC_ADC5,						/*音频数据来自LINE_IN5*/
    
    NI_AUD_DAC_SRC_BUTT
}NI_AUD_DAC_SRC_E;


typedef enum tagNiAudAdcInputSelMode
{
    NI_AUD_ADC_SEL_MODE_LINEIN,						/*通道选择LINEIN输入*/
    NI_AUD_ADC_SEL_MODE_MIC,						/*通道选择MIC输入*/
    
    NI_AUD_ADC_SEL_MODE_BUTT
}NI_AUD_ADC_INPUT_SEL_MODE_E;


typedef enum tagNiAudAdcInputSelLine
{
    NI_AUD_ADC_SEL_LINE_1,						/*音频数据来自LINE_1*/
    NI_AUD_ADC_SEL_LINE_2,						/*音频数据来自LINE_2*/
	NI_AUD_ADC_SEL_LINE_3,						/*音频数据来自LINE_3*/
	
    NI_AUD_ADC_SEL_LINE_BUTT
}NI_AUD_ADC_INPUT_SEL_LINE_E;


typedef struct tagNiAudCasMode
{
    NI_U8 u8CasNum;                                    /*级联数目*/
    NI_U8 u8CasPos;                                    /*级联位置*/
}NI_AUD_CAS_MODE_S;


typedef struct tagNiCoaudAttr
{
    NI_BOOL bRecEn;										/*同轴音频接收使能*/
	NI_BOOL bCheckEn;									/*同轴新音频校验使能*/
	NI_U8   u8AudioGain;								/*同轴音频数字增益，取值范围:[0,0xff];默认值:0x80*/
	NI_COAUD_MODE_E enCoaudMode;						/*同轴音频新老方案选择*/
} NI_COAUD_ATTR_S;
	

typedef struct tagNiAudEncAttr
{
    NI_BOOL                 bEncEn;                           /*编码使能*/
	NI_AUD_DATA_WIDTH_E 	enEncDataWidth;  				  /*编码数据位宽*/
    NI_AUD_CLK_MODE_E       enEncClkMode;                     /*编码时钟主从模式选择，应与enDecClkMode一致*/
    NI_AUD_INTF_MODE_E      enEncIntfMode;                    /*音频接口模式*/
    NI_AUD_ENC_SRC_E       	aenEncSrc[NI_MAX_CHNS];       	  /*数据源选择*/   
}NI_AUD_ENC_ATTR_S;


typedef struct tagNiAudDecAttr
{
    NI_BOOL                 bDecEn;                      /*解码模块使能*/
	NI_U8 					u8DecOutpSel;				 /*解码通道输入源选择*/
	NI_U8					u8DecChsel;					 /*编码输出左右通道选择0：右，1：左*/
    NI_AUD_CLK_MODE_E       enDecClkMode;                /*解码时钟主从模式选择，应与enEncClkMode一致*/
    NI_AUD_INTF_MODE_E      enDecIntfMode;               /*音频接口模式*/
    NI_AUD_DATA_WIDTH_E     enDecDataWidth;  			 /*解码数据位宽*/
}NI_AUD_DEC_ATTR_S;


typedef struct tagNiAudDacAttr
{
    NI_BOOL           bDacEn;                                   /*模拟音频输出使能*/
    NI_U8             u8DacGain;                                /*模拟音频输出增益，取值范围:[0,0x1f];默认值:0x1b*/
    NI_AUD_DAC_SRC_E  enAudDacSrc;                              /*模拟音频输出数据源选择*/
}NI_AUD_DAC_ATTR_S;
 

typedef struct tagNiAudAdcAttr
{
	NI_BOOL				 		 		bAdcEn;					/*输入使能*/
	NI_U8                 				u8PgaGain;              /*PGA 增益，取值范围:[0,0xf];默认值:0x8*/
    NI_U8                 				u8DigtalGain;           /*Digital 增益，取值范围:[0,0xf];默认值:0x8*/
	NI_AUD_ADC_INPUT_SEL_MODE_E			enInputSelMode;			/*CHN0-3模拟音频输入模式选择*/
    NI_AUD_ADC_INPUT_SEL_LINE_E			enInputSelLine;         /*CHN4-5模拟音频输入LINE选择*/
 }NI_AUD_ADC_ATTR_S;


typedef struct tagNiAudioAttr
{
	NI_AUD_SAMPLERATE_E enAudSamplerate;						    /*音频采样率*/
	NI_AUD_CAS_MODE_S stAudCasMode;									/*音频级联模式*/
	NI_COAUD_ATTR_S stCoAudAttr[NI_MAX_CHNS];						/*同轴音频配置*/
	NI_AUD_ENC_ATTR_S stAudEncAttr; 								/*音频编码配置*/			 
	NI_AUD_DEC_ATTR_S stAudDecAttr;  								/*音频解码配置*/
	NI_AUD_DAC_ATTR_S stAudDacAttr;									/*模拟音频输出配置*/
	NI_AUD_ADC_ATTR_S stAudAdcAttr[NI_MAX_LINEIN_CHNS];				/*模拟音频输入配置*/
}NI_AUDIO_ATTR_S;


typedef enum tagNiWorkMode
{
    NI_WORK_MODE_1Multiplex,                           /*1x模式*/
    NI_WORK_MODE_2Multiplex,                           /*2x模式*/
    NI_WORK_MODE_4Multiplex,                           /*4x模式*/  
    NI_WORK_MODE_BUTT
}NI_WORK_MODE_E;


typedef struct tagNiCo485Attr
{
    NI_BOOL bSendEn; 									/*反向485数据发送使能*/
	NI_BOOL bSendCheckEn;								/*向485数据奇偶校验位使能*/
	NI_BOOL bRecEn; 									/*正向485数据接收使能*/
	NI_BOOL bUtoEn;										/*正向传输超时使能*/
	NI_U16  u16UtoTime;									/*正向传输，设定超时目标时间(秒)*/
	NI_U16  u8IntLevel;									/*正向传输，设定中断水位（数据长度）*/
} NI_CO485_ATTR_S;
	

typedef struct tagNiCo485Buf
{
    NI_U8 *pu8485Buf;                                          /*485数据指针*/
    NI_U8 u8BufLength;                                         /*数据长度*/
}NI_CO485_BUF_S;


typedef struct tagNiDevAttr
{
    NI_WORK_MODE_E          enWorkMode;                     /*Video Out多通道复用模式选择*/
	NI_VIDEO_ATTR_S         astVideoAttr[NI_MAX_CHNS];     	/*视频输出属性设置*/
    NI_AUDIO_ATTR_S         stAudioAttr;                    /*音频属性设置*/
	NI_CO485_ATTR_S         astCo485Attr[NI_MAX_CHNS];		/*CO485配置*/
}NI_DEV_ATTR_S;

typedef enum tagNiVideoProtocol
{
    NI_VIDEO_PROTOCOL_AUTO,                                /*自动模式*/
    NI_VIDEO_PROTOCOL_CVI,                                 /*CVI协议*/
    NI_VIDEO_PROTOCOL_AHD,                                 /*AHD协议*/
    NI_VIDEO_PROTOCOL_TVI,                                 /*TVI协议*/
    NI_VIDEO_PROTOCOL_SD,                                  /*标清*/
    
    NI_VIDEO_PROTOCOL_BUTT  
}NI_VIDEO_PROTOCOL_E;


typedef enum tagNiDetectMode
{
    NI_DETECT_NORMAL_MODE,                                /*正常识别模式，识别速度一般*/
    NI_DETECT_FAST_MODE,                                  /*快速识别模式，识别速度较快*/
    
    NI_DETECT_BUTT  
}NI_DETECT_MODE_E;


typedef struct tagNiDetectOpt
{
    NI_BOOL                     bUseDetectOptEn;				/*选择使用用户配置使能*/
	NI_DETECT_MODE_E            enDetectMode;					/*自适应模式选择*/	
    NI_CABLE_TYPE_E             enCableType;					/*传输线缆选择*/
    NI_VIDEO_FORMAT_E           enVideoFormat;					/*强制配置视频制式*/
    NI_VIDEO_PROTOCOL_E         enVideoProtocol;				/*强制配置视频协议*/
    NI_BOOL                     bReLockEn;						/*检测时进行重新锁定*/
    NI_BOOL                     bEqEn;							/*EQ自适应使能*/
}NI_DETECT_OPT_S;


typedef struct tagNiDevInfo
{
    NI_U8              u8I2cDev;      /*iic设备号*/   
    NI_U8              u8DevAddr;     /*器件地址*/  
}NI_DEV_INFO_S;


typedef struct tagNiRegFunc
{
    NI_S32  (*NI_WriteByte)(NI_U8 u8I2cDev, NI_U8 u8DevAddr, NI_U16 u16RegAddr, NI_U8 u8RegValue);
    NI_S32  (*NI_ReadByte)(NI_U8 u8I2cDev, NI_U8 u8DevAddr, NI_U16 u16RegAddr, NI_U8 *pu8RegValue);
    NI_VOID (*NI_MSleep)(NI_U8 u8MsDly);
    NI_VOID (*NI_Printf)(NI_CHAR *pszStr);
    NI_VOID (*NI_GetLock)(NI_VOID);
    NI_VOID (*NI_ReleaseLock)(NI_VOID);

}NI_REG_FUNC_S;

typedef enum tagNiMipiLaneNum
{
    NI_MIPI_LANE_MODE_1,											/*lane_num=1*/
    NI_MIPI_LANE_MODE_2,											/*lane_num=2*/
	NI_MIPI_LANE_MODE_4,											/*lane_num=4*/
	
    NI_MIPI_LANE_MODE_BUTT
}NI_MIPI_LANE_NUM_E;


typedef struct tagNiMipiLaneAttr
{
    NI_MIPI_LANE_NUM_E     stMipiLaneNum;      					/*Mipi lane num*/
	NI_BOOL   			   bMipiLanePnSwap[NI_MAX_LANE_NUMS];	/*Mipi lane swap pn*/
    NI_U16                 u16MipiLaneFreq;    					/*Mipi lane freq(Mbps) 必须为100的倍数 默认为1.5G*/  
}NI_MIPI_LANE_ATTR_S;


typedef enum tagNiChipPack
{
    NI_CHIP_PACK_BT,												/*BT输出*/
    NI_CHIP_PACK_MIPI,												/*MIPI输出*/

    NI_CHIP_PACK_BUTT
}NI_CHIP_PACK_E;

typedef struct tagNiRegInfo
{
    NI_CHIP_PACK_E         enChipPack[NI_MAX_CHIPS];           /*芯片类型*/
	NI_MIPI_LANE_ATTR_S    stMipiLaneAttr;                     /*Mipi lane属性*/
    NI_BOOL                bCheckChipId;                       /*效验芯片的ID*/ 
    NI_U8                  u8AdCount;                          /*芯片个数*/
    NI_DEV_INFO_S          astNiDevInfo[NI_MAX_CHIPS];         /*记录每个芯片的I2C信息*/
    NI_REG_FUNC_S          stNiRegFunc;                        /*用户函数注册*/
}NI_REG_INFO_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */



#endif /*__NI_COMM_H__*/

