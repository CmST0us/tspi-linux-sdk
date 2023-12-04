#ifndef DUI_MSG_H
#define DUI_MSG_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/time.h>

typedef enum {
    //recorder
    RECORDER_CMD_START = 0,
    RECORDER_CMD_STOP,

    //以下消息内部使用

    //player
    PLAYER_CMD_PLAY,
    //播放结束消息
    PLAYER_INFO_PLAY_END,
    //播放开始
    PLAYER_INFO_PLAY,
    //vad
    VAD_CMD_START,
    VAD_CMD_STOP,
    VAD_CMD_ABORT,
    //检测语音起点超时
    VAD_INFO_TIMEOUT,
    //检测语音起点多次超时
    VAD_INFO_TIMEOUT_REPEAT,
    //检测到语音起点
    VAD_INFO_START,
    //检测到语音结束
    VAD_INFO_END,
    //wakeup
    WAKEUP_CMD_START,
    WAKEUP_CMD_STOP,
    //唤醒角度
    WAKEUP_INFO_DOA,
    //主唤醒词唤醒
    WAKEUP_INFO_WAKEUP,
    //快捷唤醒词唤醒
    WAKEUP_INFO_WAKEUP_MINOR,
    //dds
    //收到云端识别结果
    DDS_INFO_RESULT,
    //多轮对话
    DDS_INFO_SPEECH_CONTINUE,
    //错误
    DDS_INFO_ERROR,
} dui_msg_type_t;

typedef enum {
    //提示音
    PLAYER_MODE_PROMPT = 0,
    //合成音
    PLAYER_MODE_TTS,
    //音频
    PLAYER_MODE_AUDIO,
} player_mode_t;

typedef struct {
    dui_msg_type_t type;
    union {
        struct {
            int dummy;
        } recorder;
        struct {
            player_mode_t mode;
            char *target;
            bool need_free;
            bool native;
            bool end_session;
        } player;
        struct {
            int doa;
            bool major;
            int index; 

            //记录上一次唤醒时间
            struct timeval last_major_time;
            struct timeval last_minor_time;

            //记录每次唤醒的时间
            struct timeval cur_major_time;
            struct timeval cur_minor_time;
        } wakeup;
        struct {
            //判断如果VAD检测起点超时情况发生时，是否需要重复播放提示音
            bool timeout_need_prompt;
            //允许循环检测超时的次数
            int timeout_prompt_count;
        } vad;
    };
} dui_msg_t;

#ifdef __cplusplus
}
#endif
#endif
