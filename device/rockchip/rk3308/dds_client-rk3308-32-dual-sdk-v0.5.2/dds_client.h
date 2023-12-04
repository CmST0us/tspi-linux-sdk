/*================================================================
 * *   Copyright (C) 2018 AISpeech Ltd. All rights reserved.
 * *
 * *   文件名称：dds_client.h
 * *   创建日期：2018年04月06日
 * *   描    述：
 * *
 * ================================================================*/


#ifndef _DDS_CLIENT_H
#define _DDS_CLIENT_H
#ifdef __cplusplus
extern "C"
{
#endif

#define DDS_CLIENT_VERSION           "DDS_CLIENT 0.5.2"

#define DDS_CLIENT_TTS_ZHILING      "zhilingf"      // 甜美女神
#define DDS_CLIENT_TTS_GDG          "gdgm"          // 沉稳纲叔
#define DDS_CLIENT_TTS_GEYOU        "geyou"         // 淡定葛爷
#define DDS_CLIENT_TTS_HYANIF       "hyanif"        // 邻家女声
#define DDS_CLIENT_TTS_XIJUNM       "xijunm"        // 标准男声
#define DDS_CLIENT_TTS_QIANRAN      "qianranf"      // 可爱童声
#define DDS_CLIENT_TTS_LUCYF        "lucyf"         // 标准女声

#define DDS_CLIENT_USER_EV_BASE		        1000
#define DDS_CLIENT_USER_DEVICE_MODE         1001
#define DDS_CLIENT_USER_EXTERNAL_WAKEUP     1002

struct dds_client;

typedef void (*ddsLintener)(const char *topic, const char *topic_data, void *user);

struct dds_client *dds_client_init (const char *config_json);

int dds_client_start(struct dds_client *, ddsLintener cb, void *user);

void dds_client_release(struct dds_client *);

// 发送事件给 sdk
int dds_client_publish(struct dds_client *ds, int ev, const char *data);

/*
 * 对 nativeAPI 命令做出查询回应的接口，其中 native_api_data_json 的格式如下:
 * duiWidget 字段表示 dui 控件的类型，当前仅支持 "text"。
 * extra 字段用于返回用户的数据。 
 * {
 *  "duiWidget":"text",
 *  "extra": {
 *      "xx": "11"
 *  }
 * }
 * 出错时返回值为 -1。
 */
int dds_client_resp_nativeapi(struct dds_client *ds, const char *native_api,
                                const char *native_api_data_json);
/*
 * 录音机接口
 */
int dds_client_feed_audio(struct dds_client *ds, char *data, int len);

/*
 * 对话的接口
 */
int dds_client_stop_dialog(struct dds_client *ds);
int dds_client_trigger_intent(struct dds_client *ds, char *skill, char *task, 
                                char *intent, char *slots);

/*
 * tts 的相关接口
 */
int dds_client_speak(struct dds_client *ds, const char *text);
char *dds_client_get_speaker(struct dds_client *ds);
float dds_client_get_speed(struct dds_client *ds);
int dds_client_get_volume(struct dds_client *ds);

int dds_client_set_speaker(struct dds_client *ds, char *speaker);
int dds_client_set_speed(struct dds_client *ds, float speed);
int dds_client_set_volume(struct dds_client *ds, int vol);

/*
 * 唤醒的相关设置
 */
int dds_client_disable_wakeup(struct dds_client *ds);
int dds_client_enable_wakeup(struct dds_client *ds);
int dds_client_update_customword(struct dds_client *ds, const char *word);
char* dds_client_get_wakeupwords(struct dds_client *ds);


/*
 * 声纹相关
 */
char *dds_client_vprint_get_detail(struct dds_client *ds);
int dds_client_vprint_regist(struct dds_client *ds, char *name);
int dds_client_vprint_unregist(struct dds_client *ds, char *name);

/*
 * 能量接口
 */

int dds_client_energy_estimate(struct dds_client* ds, int second);

/*
 * 设备端接口
 */
int dds_client_upload_location(struct dds_client* ds, char *city);


#ifdef __cplusplus
}
#endif
#endif //DDS_CLIENT_H

