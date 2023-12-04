/*
 * 同时支持任意时刻主副唤醒词打断
 * 支持任意时刻关闭录音设备
 */
#include "dui.h"
#include "os_log.h"
#include "os_stream.h"
#include "os_utils.h"
#include "os_thread.h"
#include "os_queue.h"
#include "os_semaphore.h"
#include "os_event_group.h"
#include "os_mutex.h"
#include "recorder.h"
#include "dui_fsm.h"
#include "os_memory.h"
#include "duilite.h"
#include "dds.h"
#include "dui_msg.h"

#include "player.h"
#include "cJSON.h"

#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#ifdef SAVE_AUDIO
static FILE *input_fd, *output_fd;
#endif

const char *dui_msg_table[] = { 
    "RECORDER_CMD_START",
    "RECORDER_CMD_STOP",
    "PLAYER_CMD_PLAY",
    "PLAYER_INFO_PLAY_END",
    "PLAYER_INFO_PLAY",
    "VAD_CMD_START",
    "VAD_CMD_STOP",
    "VAD_CMD_ABORT",
    "VAD_INFO_TIMEOUT",
    "VAD_INFO_TIMEOUT_REPEAT",
    "VAD_INFO_START",
    "VAD_INFO_END",
    "WAKEUP_CMD_START",
    "WAKEUP_CMD_STOP",
    "WAKEUP_INFO_DOA",
    "WAKEUP_INFO_WAKEUP",
    "WAKEUP_INFO_WAKEUP_MINOR",
    "DDS_INFO_RESULT",
    "DDS_INFO_SPEECH_CONTINUE",
    "DDS_INFO_ERROR"
};

//TODO
static player_handle_t tts_player;
static player_handle_t media_player;

typedef struct {
    struct {
        int bits;
        int samplerate;
        int channels;
        char *device;
    } recorder;
    struct {
        char *device;
    } player;
    struct {
        char *cfg;
        char *param;
        //唤醒词数组
        char **word;
        char **audio;
        int word_count;
    } wakeup;
    struct {
        char *cfg;
        char *param;
        char *start_timeout_prompt;
	char *start_timeout_repeat_prompt;
    } vad;
    struct {
        char *productId;
        char *aliasKey;
        char *server;
        char *deviceProfile;
    } dds;
    user_listen_cb listen;
} dui_library_cfg_t;

static dui_library_cfg_t g_cfg;

//录音线程就绪标志
#define RECORDER_READY_BIT      (1 << 0)
//唤醒线程就绪标志
#define WAKEUP_READY_BIT        (1 << 1)
//VAD线程就绪标志
#define VAD_READY_BIT           (1 << 2)
//DDS线程就绪标志
#define DDS_READY_BIT           (1 << 3)
//播放控制线程就绪标志
#define PLAYER_READY_BIT        (1 << 4)
//用户监听线程就绪标志
#define USER_LISTEN_READY_BIT   (1 << 5)
//处理线程就绪标志
#define PROCESS_READY_BIT       (1 << 6)

//线程就绪事件组
static os_event_group_handle_t task_ready_ev;

//日志结构
os_log_create_module(recorder, OS_LOG_LEVEL_INFO);
os_log_create_module(vad, OS_LOG_LEVEL_INFO);
os_log_create_module(wakeup, OS_LOG_LEVEL_INFO);
os_log_create_module(dds, OS_LOG_LEVEL_INFO);
os_log_create_module(process, OS_LOG_LEVEL_INFO);
os_log_create_module(player, OS_LOG_LEVEL_INFO);
os_log_create_module(user_listen, OS_LOG_LEVEL_INFO);

//录音线程句柄
static os_thread_handle_t recorder_task;
//播放控制线程句柄
static os_thread_handle_t player_task;
//前端信号处理线程句柄
static os_thread_handle_t wakeup_task;
//vad处理线程句柄
static os_thread_handle_t vad_task;
//dds线程句柄
static os_thread_handle_t dds_task;
//处理线程句柄
static os_thread_handle_t process_task;
//用户监听线程句柄
static os_thread_handle_t user_listen_task;

//录音数据写入
os_stream_handle_t wakeup_stream;

//vad截取后的音频数据写入
os_stream_handle_t dds_stream;

//前端信号处理后的音频写入
os_stream_handle_t vad_stream;

//处理线程消息队列
os_queue_handle_t process_queue;

//前端信号处理消息队列
os_queue_handle_t wakeup_queue;

//播放控制线程消息队列
os_queue_handle_t player_queue;

//vad处理线程消息队列
os_queue_handle_t vad_queue;

//录音线程消息队列
os_queue_handle_t recorder_queue;

//用户监听线程消息队列
os_queue_handle_t user_listen_queue;

static void user_listen_run(void *args) {
    int ret;
    dui_msg_t m;
    //置位线程READY标志
    os_event_group_set_bits(task_ready_ev, USER_LISTEN_READY_BIT);
    OS_LOG_I(user_listen, "READY");
    while (1) {
        ret = os_queue_receive(user_listen_queue, &m);
        if (ret == -1) break;
        OS_LOG_I(user_listen, "%s", dui_msg_table[m.type]);
        if (g_cfg.listen) {
            g_cfg.listen(&m);
        }
    }
    OS_LOG_I(user_listen, "EXIT");
}

static void recorder_run(void *args) {
    int ret;
    dui_msg_t m;
    //置位线程READY标志
    os_event_group_set_bits(task_ready_ev, RECORDER_READY_BIT);
    OS_LOG_I(recorder, "READY");
    while (1) {
        ret = os_queue_receive(recorder_queue, &m);
        if (ret == -1) break;
        OS_LOG_I(recorder, "%s", dui_msg_table[m.type]);
        if (m.type == RECORDER_CMD_START) {
            alsa_open_config_t cfg = {
                .bits = g_cfg.recorder.bits,
                .channels = g_cfg.recorder.channels,
                .samplerate = g_cfg.recorder.samplerate,
                .device = g_cfg.recorder.device
            };  
            recorder_handle_t recorder = recorder_open(&cfg);
            assert(recorder != NULL);
            int block = cfg.bits * cfg.channels / 8;
            char *read_buf = (char *)os_malloc(cfg.period_size * block);
            assert(read_buf != NULL);
            
            system("amixer -c 0 cset name='ADC ALC Group 0 Left Volume' 20");
            system("amixer -c 0 cset name='ADC ALC Group 0 Right Volume' 20");
            recorder_start(recorder);
            OS_LOG_I(recorder, "START: %s", cfg.device);
            os_queue_send(user_listen_queue, &m);
            int frames, read_bytes;
            while (1) {
                frames = recorder_read(recorder, read_buf, cfg.period_size); 
                if (frames > 0) {
                    read_bytes = frames * block;
                    //OS_LOG_I(recorder, "read bytes: %d", read_bytes);
                    ret = os_stream_write(wakeup_stream, read_buf, read_bytes);
                    if (ret == -1) break;   //录音缓冲被终止
                } else {
                    OS_LOG_W(recorder, "read data error");
                }
            }
            recorder_stop(recorder);
            recorder_close(recorder);
            os_free(read_buf);
        } else if (m.type == RECORDER_CMD_STOP) {
            os_queue_send(user_listen_queue, &m);
            OS_LOG_I(recorder, "STOP");
        }
    }
    OS_LOG_W(recorder, "EXIT");
}

static void player_run(void *args) {
    int ret;
    dui_msg_t m;
    //置位线程READY标志
    os_event_group_set_bits(task_ready_ev, PLAYER_READY_BIT);
    OS_LOG_I(player, "READY");

    play_cfg_t item;
    bool cancel = false;
    while (1) {
        memset(&item, 0, sizeof(item));
        ret = os_queue_receive(player_queue, &m);     
        if (ret == -1) break;
        OS_LOG_I(player, "%s", dui_msg_table[m.type]);

        if (m.player.mode == PLAYER_MODE_PROMPT) {
            item.target = m.player.target;
            item.type = PLAY_TYPE_NATIVE;
            item.need_free = m.player.need_free;

            player_play(tts_player, &item);
            ret = player_wait_idle(tts_player);
            if (ret == 0) {
                if (m.player.end_session) {
                    player_resume(media_player);
                } else {
                    memset(&m, 0, sizeof(m));
                    m.type = PLAYER_INFO_PLAY_END;
                    os_queue_send(process_queue, &m);
                }
            }
        } else if (m.player.mode == PLAYER_MODE_TTS) {
            item.type = PLAY_TYPE_NETWORK;
            item.target = m.player.target;
            item.need_free = m.player.need_free;
            player_play(tts_player, &item);
            ret = player_wait_idle(tts_player);
            if (ret == -1) {
                //合成音播放被终止
                cancel = true;
            } else {
                cancel = false;
                if (m.player.end_session == true) {
                    player_resume(media_player);
                } else {
		    //多轮对话告知状态机需要再次进入VAD
                    memset(&m, 0, sizeof(m));
                    m.type = DDS_INFO_SPEECH_CONTINUE;
                    os_queue_send(process_queue, &m);
                }
            }
        } else if (m.player.mode == PLAYER_MODE_AUDIO) {
            item.target = m.player.target;
            item.type = PLAY_TYPE_NETWORK;
            item.need_free = m.player.need_free;
            if (cancel == false) {
                player_stop(media_player);
                player_play(media_player, &item);
            } else {
                cancel = false;
                if (m.player.need_free) {
                    os_free(m.player.target);
                }
            }
        }
    }
    OS_LOG_W(player, "EXIT");
}

static bool is_same_word(const char *s1, const char *s2) {
	while (s1[0] && s2[0]) {
		if (isspace(s1[0])) {
			s1++;
			continue;
		}
		if (isspace(s2[0])) {
			s2++;
			continue;
		}
		if (*s1 != *s2) return false;
		s1++;
		s2++;
	}
	if (strlen(s1)) {
		while (s1[0]) {
			if (!isspace(s1[0])) return false;
			s1++;
		}
	}
	if (strlen(s2)) {
		while (s2[0]) {
			if (!isspace(s2[0])) return false;
			s2++;
		}
	}
	return true; 
}

#ifdef WAKEUP_FESPA
#include "wakeup_fespa.c"
#elif defined(WAKEUP_FESPL)
#include "wakeup_fespl.c"
#elif defined(WAKEUP_AEC)
#include "wakeup_aec.c"
#endif


typedef struct {
    int status;
    bool first;
} vad_info_t;

static int vad_callback(void *userdata, int type, char *result, int len) {
    dui_msg_t m;
    vad_info_t *info = (vad_info_t *)userdata;
    if (type == DUILITE_MSG_TYPE_JSON) {
        OS_LOG_I(vad, "VAD: %s", result);
        cJSON *js = cJSON_Parse(result);
        cJSON *status_js = cJSON_GetObjectItem(js, "status");
        info->status = status_js->valueint;
        cJSON_Delete(js);
    } else {
        if (info->status == 1) {
            if (info->first) {
                memset(&m, 0, sizeof(m));
                m.type = VAD_INFO_START;
                os_queue_send(process_queue, &m);

                //开始云端请求
                struct dds_msg *msg = dds_msg_new();
                dds_msg_set_type(msg, DDS_EV_IN_SPEECH);
                dds_msg_set_string(msg, "action", "start");
                dds_send(msg);
                dds_msg_delete(msg);
                info->first = false;

                //允许截取的音频写入DDS数据缓冲
                os_stream_start(dds_stream);
            }
            os_stream_write(dds_stream, result, len);
        }
    }
    return 0;
}

bool check_time_expire(struct timeval *now, struct timeval *before, int timeout) {
    long int expire = (now->tv_sec  - before->tv_sec) * 1000 + now->tv_usec / 1000 - before->tv_usec / 1000;
    if (expire >= timeout) return true;
    return false;
}

static void vad_run(void *args) {
    int ret;
    dui_msg_t m;
    vad_info_t info;

    struct duilite_vad *vad_engine = duilite_vad_new(g_cfg.vad.cfg, vad_callback, &info);
    assert(vad_engine != NULL);
    //50ms
    int read_buf_size = g_cfg.recorder.bits / 8 * g_cfg.recorder.samplerate / 20;
    char *read_buf = (char *)os_malloc(read_buf_size);
    assert(read_buf != NULL);

    os_event_group_set_bits(task_ready_ev, VAD_READY_BIT);
    OS_LOG_I(vad, "READY");
    while (1) {
        ret = os_queue_receive(vad_queue, &m);
        if (ret == -1) break;
        OS_LOG_I(vad, "%s", dui_msg_table[m.type]);
        if (m.type == VAD_CMD_START) {
            struct timeval start_time, now_time;
            info.first = true;
            info.status = 0;
            os_stream_start(vad_stream);
            duilite_vad_start(vad_engine, g_cfg.vad.param);
            int read_bytes;

            OS_LOG_I(vad, "START");
            gettimeofday(&start_time, NULL); 
            while (1) {
                read_bytes = os_stream_read(vad_stream, read_buf, read_buf_size);
                if (read_bytes == -1) {
                    //vad数据输入缓冲被终止
                    duilite_vad_cancel(vad_engine);
                    if (info.status == 1) {
                        //终止DDS数据缓冲
                        os_stream_stop(dds_stream);
                        //复位dds状态机
                        struct dds_msg *msg = dds_msg_new();
                        dds_msg_set_type(msg, DDS_EV_IN_RESET);
                        dds_send(msg);
                        dds_msg_delete(msg);
                    }
                    break;
                }

                duilite_vad_feed(vad_engine, read_buf, read_bytes);

                if (info.status == 0) {
                    //判断检测开始说话超时
                    gettimeofday(&now_time, NULL);
                    if (check_time_expire(&now_time, &start_time, 5000)) {
                        //终止VAD输入缓冲写入
                        os_stream_stop(vad_stream);
                        duilite_vad_cancel(vad_engine);
                        memset(&m, 0, sizeof(m));
                        m.type = VAD_INFO_TIMEOUT;
                        os_queue_send(process_queue, &m);
                        break;
                    }
                }

                if (info.status == 2) {
                    //终止VAD输入缓冲写入
                    duilite_vad_stop(vad_engine);
                    
                    //os_stream_finish(dds_stream);
                    os_stream_stop(vad_stream);

                    struct dds_msg *msg = dds_msg_new();
                    dds_msg_set_type(msg, DDS_EV_IN_SPEECH);
                    dds_msg_set_string(msg, "action", "end");
                    dds_send(msg);
                    dds_msg_delete(msg);
                    memset(&m, 0, sizeof(m));
                    m.type = VAD_INFO_END;
                    os_queue_send(process_queue, &m);
                    break;
                }
            }
        } else if (m.type == VAD_CMD_STOP){
            OS_LOG_I(vad, "STOP");
        }
    }
    os_free(read_buf);
    duilite_vad_delete(vad_engine);
    OS_LOG_I(vad, "EXIT");
}

void dui_start_recorder() {
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = RECORDER_CMD_START;
    os_queue_send(process_queue, &m); }

void dui_stop_recorder() {
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = RECORDER_CMD_STOP;
    os_queue_send(process_queue, &m);
}

typedef enum {
    //空闲
    PROCESS_STATE_IDLE = 0,
    //等待唤醒（此时可能正在播放音频）
    PROCESS_STATE_WAIT_FOR_WAKEUP,
    //唤醒提示音播放中
    PROCESS_STATE_WAKEUP_PROMPT,
    //语音起点检测超时提示音播放中
    PROCESS_STATE_VAD_START_TIMEOUT_PROMPT,
    //检测语音起点中
    PROCESS_STATE_WAIT_FOR_SPEECH_START,
    //识别拾音中
    PROCESS_STATE_SPEECH,
    //等待识别结果
    PROCESS_STATE_WAIT_FOR_ASR,
} process_state_t;

static void idle_to_wait_for_wakeup(void *args) {
    //打开录音缓冲
    os_stream_start(wakeup_stream);

    //开启唤醒
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_CMD_START;
    os_queue_send(wakeup_queue, &m);

    //开启录音
    memset(&m, 0, sizeof(m));
    m.type = RECORDER_CMD_START;
    os_queue_send(recorder_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_wakeup_to_idle(void *args) {
    //终止录音缓冲（此时唤醒和录音循环被中断）
    os_stream_stop(wakeup_stream);

    //关闭唤醒
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_CMD_STOP;
    os_queue_send(wakeup_queue, &m);

    //关闭录音
    memset(&m, 0, sizeof(m));
    m.type= RECORDER_CMD_STOP;
    os_queue_send(recorder_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_wakeup_to_wakeup_prompt(void *args) {
    //当前可能正在播放合成音或者音乐
    //停止提示音/合成音播放
    player_stop(tts_player);
    //暂停音乐播放
    player_pause(media_player);

    dui_msg_t *msg = (dui_msg_t *)args;

    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;

    m.player.end_session = false;

    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wakeup_prompt_to_wakeup_prompt(void *args) {
    //停止提示音
    player_stop(tts_player);

    dui_msg_t *msg = (dui_msg_t *)args;

    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;

    m.player.end_session = false;

    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wakeup_prompt_to_wait_for_speech_start(void *args) {
    //开启VAD
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = VAD_CMD_START;
    os_queue_send(vad_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_speech_start_to_speech(void *args) {
    OS_LOG_I(process, "=====");
}

static void speech_to_wait_for_asr(void *args) {
    OS_LOG_I(process, "=====");
}

static void wait_for_asr_to_wait_for_wakeup(void *args) {
    OS_LOG_I(process, "=====");
}

static void wait_for_asr_to_wait_for_wakeup3(void *args) {
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = "./audio/network_error.mp3";
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_asr_to_wait_for_wakeup2(void *args) {
    struct dds_msg *dm = dds_msg_new();
    dds_msg_set_type(dm, DDS_EV_IN_RESET);
    dds_send(dm);
    dds_msg_delete(dm);

    dui_msg_t *msg = (dui_msg_t *)args;
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_speech_start_to_wakeup_prompt(void *args) {
    dui_msg_t *msg = (dui_msg_t *)args;
    //终止VAD数据缓冲写入
    os_stream_stop(vad_stream);
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = false;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void speech_to_wakeup_prompt(void *args) {
    dui_msg_t *msg = (dui_msg_t *)args;
    //终止VAD数据缓冲写入
    os_stream_stop(vad_stream);
    os_stream_stop(dds_stream);
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = false;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_speech_start_to_wait_for_wakeup(void *args) {
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.vad.start_timeout_prompt;
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_asr_to_wakeup_prompt(void *args) {
    struct dds_msg *dm = dds_msg_new();
    dds_msg_set_type(dm, DDS_EV_IN_RESET);
    dds_send(dm);
    dds_msg_delete(dm);

    dui_msg_t *msg = (dui_msg_t *)args;

    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void speech_to_wait_for_wakeup(void *args) {
    //终止VAD数据缓冲写入
    os_stream_stop(vad_stream);
    os_stream_stop(dds_stream);
    dui_msg_t m;                                                                                                                        
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = "./audio/network_error.mp3";
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m); 
    OS_LOG_I(process, "=====");
}

static void speech_to_wait_for_wakeup2(void *args) {
    //终止VAD数据缓冲写入
    os_stream_stop(vad_stream);
    os_stream_stop(dds_stream);

    dui_msg_t *msg = (dui_msg_t *)args;

    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_wakeup_to_wait_for_wakeup(void *args) {
    //当前可能正在播放合成音或者音乐
    //停止提示音/合成音播放
    player_stop(tts_player);
    //暂停音乐播放
    player_pause(media_player);

    dui_msg_t *msg = (dui_msg_t *)args;

    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wakeup_prompt_to_wait_for_wakeup(void *args) {
    //停止提示音/合成音播放
    player_stop(tts_player);
    dui_msg_t *msg = (dui_msg_t *)args;

    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_speech_start_to_wait_for_wakeup2(void *args) {
    dui_msg_t *msg = (dui_msg_t *)args;
    //终止VAD数据缓冲写入
    os_stream_stop(vad_stream);
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_PROMPT;
    m.player.target = g_cfg.wakeup.audio[msg->wakeup.index];
    m.player.native = true;
    m.player.need_free = false;
    m.player.end_session = true;
    os_queue_send(player_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wakeup_prompt_to_idle(void *args) {
    //停止提示音
    player_stop(tts_player);

    //终止wakeup输入缓冲
    os_stream_stop(wakeup_stream);

    //关闭唤醒
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_CMD_STOP;
    os_queue_send(wakeup_queue, &m);

    //关闭录音
    memset(&m, 0, sizeof(m));
    m.type = RECORDER_CMD_STOP;
    os_queue_send(recorder_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_speech_start_to_idle(void *args) {
    //终止VAD数据流
    os_stream_stop(vad_stream);

    //终止WAKEUP数据流
    os_stream_stop(wakeup_stream);
    //关闭唤醒
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_CMD_STOP;
    os_queue_send(wakeup_queue, &m);

    //关闭录音
    memset(&m, 0, sizeof(m));
    m.type= RECORDER_CMD_STOP;
    os_queue_send(recorder_queue, &m);
    OS_LOG_I(process, "=====");
}

static void speech_to_idle(void *args) {
    //终止VAD数据缓冲
    os_stream_stop(vad_stream);
    os_stream_stop(dds_stream);

    //终止录音缓冲
    os_stream_stop(wakeup_stream);

    //关闭唤醒
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_CMD_STOP;
    os_queue_send(wakeup_queue, &m);

    //关闭录音
    memset(&m, 0, sizeof(m));
    m.type= RECORDER_CMD_STOP;
    os_queue_send(recorder_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_asr_to_idle(void *args) {

    struct dds_msg *msg = dds_msg_new();
    dds_msg_set_type(msg, DDS_EV_IN_RESET);
    dds_send(msg);
    dds_msg_delete(msg);

    //终止录音缓冲
    os_stream_stop(wakeup_stream);

    //关闭唤醒
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = WAKEUP_CMD_STOP;
    os_queue_send(wakeup_queue, &m);

    //关闭录音
    memset(&m, 0, sizeof(m));
    m.type= RECORDER_CMD_STOP;
    os_queue_send(recorder_queue, &m);
    OS_LOG_I(process, "=====");
}

static void wait_for_wakeup_to_wait_for_speech_start(void *args) {
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = VAD_CMD_START;
    os_queue_send(vad_queue, &m);
    OS_LOG_I(process, "=====");
}

static dui_fsm_transfer_t process_transfer_table[] = {
    //触发事件---当前状态---执行动作---下一个状态
    {RECORDER_CMD_START, PROCESS_STATE_IDLE, idle_to_wait_for_wakeup, PROCESS_STATE_WAIT_FOR_WAKEUP},

    {RECORDER_CMD_STOP, PROCESS_STATE_WAIT_FOR_WAKEUP, wait_for_wakeup_to_idle, PROCESS_STATE_IDLE},
    {WAKEUP_INFO_WAKEUP, PROCESS_STATE_WAIT_FOR_WAKEUP, wait_for_wakeup_to_wakeup_prompt, PROCESS_STATE_WAKEUP_PROMPT},
    {DDS_INFO_SPEECH_CONTINUE, PROCESS_STATE_WAIT_FOR_WAKEUP, wait_for_wakeup_to_wait_for_speech_start, PROCESS_STATE_WAIT_FOR_SPEECH_START},
    {WAKEUP_INFO_WAKEUP_MINOR, PROCESS_STATE_WAIT_FOR_WAKEUP, wait_for_wakeup_to_wait_for_wakeup, PROCESS_STATE_WAIT_FOR_WAKEUP},

    {WAKEUP_INFO_WAKEUP, PROCESS_STATE_WAKEUP_PROMPT, wakeup_prompt_to_wakeup_prompt, PROCESS_STATE_WAKEUP_PROMPT},
    {WAKEUP_INFO_WAKEUP_MINOR, PROCESS_STATE_WAKEUP_PROMPT, wakeup_prompt_to_wait_for_wakeup, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {PLAYER_INFO_PLAY_END, PROCESS_STATE_WAKEUP_PROMPT, wakeup_prompt_to_wait_for_speech_start, PROCESS_STATE_WAIT_FOR_SPEECH_START},
    {RECORDER_CMD_STOP, PROCESS_STATE_WAKEUP_PROMPT, wakeup_prompt_to_idle, PROCESS_STATE_IDLE},

    {WAKEUP_INFO_WAKEUP, PROCESS_STATE_WAIT_FOR_SPEECH_START, wait_for_speech_start_to_wakeup_prompt, PROCESS_STATE_WAKEUP_PROMPT},
    {VAD_INFO_TIMEOUT, PROCESS_STATE_WAIT_FOR_SPEECH_START, wait_for_speech_start_to_wait_for_wakeup, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {WAKEUP_INFO_WAKEUP_MINOR, PROCESS_STATE_WAIT_FOR_SPEECH_START, wait_for_speech_start_to_wait_for_wakeup2, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {VAD_INFO_START, PROCESS_STATE_WAIT_FOR_SPEECH_START, wait_for_speech_start_to_speech, PROCESS_STATE_SPEECH},
    {RECORDER_CMD_STOP, PROCESS_STATE_WAIT_FOR_SPEECH_START, wait_for_speech_start_to_idle, PROCESS_STATE_IDLE},

    {WAKEUP_INFO_WAKEUP, PROCESS_STATE_SPEECH, speech_to_wakeup_prompt, PROCESS_STATE_WAKEUP_PROMPT},
    {VAD_INFO_END, PROCESS_STATE_SPEECH, speech_to_wait_for_asr, PROCESS_STATE_WAIT_FOR_ASR},
    {DDS_INFO_ERROR, PROCESS_STATE_SPEECH, speech_to_wait_for_wakeup, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {WAKEUP_INFO_WAKEUP_MINOR, PROCESS_STATE_SPEECH, speech_to_wait_for_wakeup2, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {RECORDER_CMD_STOP, PROCESS_STATE_SPEECH, speech_to_idle, PROCESS_STATE_IDLE},

    {DDS_INFO_RESULT, PROCESS_STATE_WAIT_FOR_ASR, wait_for_asr_to_wait_for_wakeup, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {DDS_INFO_ERROR, PROCESS_STATE_WAIT_FOR_ASR, wait_for_asr_to_wait_for_wakeup3, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {WAKEUP_INFO_WAKEUP, PROCESS_STATE_WAIT_FOR_ASR, wait_for_asr_to_wakeup_prompt, PROCESS_STATE_WAKEUP_PROMPT},
    {WAKEUP_INFO_WAKEUP_MINOR, PROCESS_STATE_WAIT_FOR_ASR, wait_for_asr_to_wait_for_wakeup2, PROCESS_STATE_WAIT_FOR_WAKEUP},
    {RECORDER_CMD_STOP, PROCESS_STATE_WAIT_FOR_ASR, wait_for_asr_to_idle, PROCESS_STATE_IDLE}
};

static void process_run(void *args) {
    int ret;
    dui_msg_t m;
    dui_fsm_t fsm = {
        .cur_state = PROCESS_STATE_IDLE,
        .t = process_transfer_table,
        .t_size = sizeof(process_transfer_table) / sizeof(process_transfer_table[0])
    };
    os_event_group_set_bits(task_ready_ev, PROCESS_READY_BIT);
    OS_LOG_I(process, "READY");
    while (1) {
        ret = os_queue_receive(process_queue, &m);
        if (ret == -1) break;
        OS_LOG_I(process, "%s", dui_msg_table[m.type]);
        dui_fsm_handle(&fsm, m.type, &m);
    }
    OS_LOG_I(process, "EXIT");
}

static char *msg_type_set[] = {
    "",
    "DDS_EV_OUT_RECORD_AUDIO",
    "DDS_EV_OUT_NATIVE_CALL",
    "DDS_EV_OUT_COMMAND",
    "DDS_EV_OUT_MEDIA",
    "DDS_EV_OUT_STATUS",
    "DDS_EV_OUT_TTS",
    "DDS_EV_OUT_ERROR",
    "DDS_EV_OUT_ASR_RESULT",
    "DDS_EV_OUT_DUI_RESPONSE",
    "DDS_EV_OUT_DUI_LOGIN" 
};

static int process_tts(struct dds_msg *msg) {
    char *str_value = NULL;
    dds_msg_get_string(msg, "speakUrl", &str_value);
    OS_LOG_I(dds, "tts url: %s", str_value);
    char *url = NULL;
    if (strncmp(str_value, "https://", 8) == 0) {
        url = os_strdup(str_value + 8);
    } else {
        url = os_strdup(str_value);
    }
    dui_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = PLAYER_CMD_PLAY;
    m.player.mode = PLAYER_MODE_TTS;
    m.player.native = false;
    m.player.target = url;
    m.player.need_free = true;
    int end_session;
    dds_msg_get_integer(msg, "endSession", &end_session);
    m.player.end_session = (end_session == 1) ? true : false;
    os_queue_send(player_queue, &m);
    return 0;
}

#define HTTP_PREFIX "http://ac.iot.aispeech.com/ogg2mp3/"
static int process_media(struct dds_msg *msg) {
    char *list = NULL;
    char *task = NULL;
    dds_msg_get_string(msg, "list", &list);
    dds_msg_get_string(msg, "task", &task);
    if (list) {
        cJSON *js = cJSON_Parse(list);
        if (js) {
            int items = cJSON_GetArraySize(js);
            if (items > 0) {
                cJSON *first_js = cJSON_GetArrayItem(js, 0);
                cJSON *linkUrl_js = cJSON_GetObjectItem(first_js, "linkUrl");
                int len;
                if (linkUrl_js && (len = strlen(linkUrl_js->valuestring)) > 0) {
                    char *url = strdup(linkUrl_js->valuestring);
                    if ((len > 4) && (0 == strcmp(linkUrl_js->valuestring + len - 4, ".ogg"))) {
                        //如果是ogg音频，需要跳转到解码服务器进行转码
                        url = (char *)os_calloc(len + strlen(HTTP_PREFIX) + 1, sizeof(char));
                        assert(url != NULL);
                        memcpy(url, HTTP_PREFIX, strlen(HTTP_PREFIX));
                        memcpy(url + strlen(HTTP_PREFIX), linkUrl_js->valuestring, len);
                    }
                    if (strncmp(url, "https://", 8) == 0) {
                        memmove(url, url + 8, strlen(url) + 1 - 8);
                    }
                    dui_msg_t m;
                    memset(&m, 0, sizeof(m));
                    m.type = PLAYER_CMD_PLAY;
                    m.player.mode = PLAYER_MODE_AUDIO;
                    m.player.native = false;
                    m.player.target = url;
                    m.player.need_free = true;
                    os_queue_send(player_queue, &m);
                }
            }
            cJSON_Delete(js);
        }
    }
    return 0;
}


static char recorder_data[6400];
static int dds_handler(void *userdata, struct dds_msg *msg) {
    int type;
    int errorId;
    char *str_value;
    dds_msg_get_integer(msg, "type", &type);
    OS_LOG_I(dds, "dds msg type id: %d, type: %s", type, msg_type_set[type]);
    switch (type) {
    case DDS_EV_OUT_RECORD_AUDIO: {
            int read_bytes = os_stream_read2(dds_stream, recorder_data, sizeof(recorder_data));
            if (read_bytes > 0) {
                msg = dds_msg_new();
                assert(msg != NULL);
                dds_msg_set_type(msg, DDS_EV_IN_AUDIO_STREAM);
                dds_msg_set_bin(msg, "audio", recorder_data, read_bytes);
                dds_send(msg);
                dds_msg_delete(msg);
                struct timeval tv;
                gettimeofday(&tv, NULL);
            }
            //OS_LOG_I(dds, "read audio length: %d, now: %.3lf\n", read_bytes, tv.tv_sec + tv.tv_usec / 1000000.0);
            break;
        }
    case DDS_EV_OUT_NATIVE_CALL: {
            //process_native(msg);
            break;
        }
    case DDS_EV_OUT_COMMAND: {
            //process_command(msg);
            break;
        }
    case DDS_EV_OUT_MEDIA: {
            process_media(msg);
            break;
        }
    case DDS_EV_OUT_STATUS: {
            dds_msg_get_string(msg, "status", &str_value);
            OS_LOG_I(dds, "dds status: %s", str_value);
            if (0 == strcmp(str_value, "idle")) {

            } else if (0 == strcmp(str_value, "listening")) {
            } else if (0 == strcmp(str_value, "understanding")) {
            }
            break;
        }
    case DDS_EV_OUT_TTS: {
	        process_tts(msg);
            break;
        }
    case DDS_EV_OUT_ERROR: {
            dui_msg_t m;
            memset(&m, 0, sizeof(m));
            m.type = DDS_INFO_ERROR;
            os_queue_send(process_queue, &m);
            dds_msg_get_string(msg, "error", &str_value);
            dds_msg_get_integer(msg, "errorId", &errorId);
            OS_LOG_I(dds, "error: %s, errorId: %d\n", str_value, errorId);
            break;
        }
    case DDS_EV_OUT_ASR_RESULT: {
            if (!dds_msg_get_string(msg, "var", &str_value)) {
                OS_LOG_I(dds,"var: %s\n", str_value);
            }
            if (!dds_msg_get_string(msg, "text", &str_value)) {
                OS_LOG_I(dds, "text: %s\n", str_value);
            }
            dui_msg_t m;
            memset(&m, 0, sizeof(m));
            m.type = DDS_INFO_RESULT;
            os_queue_send(process_queue, &m);
            break;
        }
    case DDS_EV_OUT_DUI_RESPONSE: {
            dds_msg_get_string(msg, "response", &str_value);
            OS_LOG_I(dds, "response: %s\n", str_value);
            break;
        }
    default:
        break;
    }
    return 0;
}

static void dds_run(void *args) {
    struct dds_msg *msg = dds_msg_new();
    dds_msg_set_string(msg, "productId", g_cfg.dds.productId);
    dds_msg_set_string(msg, "aliasKey", g_cfg.dds.aliasKey);
    dds_msg_set_string(msg, "server", g_cfg.dds.server);
    dds_msg_set_string(msg, "deviceProfile", g_cfg.dds.deviceProfile);
    struct dds_opt opt = {
        ._handler = dds_handler
    };
    os_event_group_set_bits(task_ready_ev, DDS_READY_BIT);
    OS_LOG_I(dds, "READY");
    int ret = dds_start(msg, &opt);
    if (ret != 0) {
        OS_LOG_I(dds, "dds start error");
    }
    dds_msg_delete(msg);
    OS_LOG_I(dds, "EXIT");
}


static int dui_parse_cfg(const char *cfg) {
    cJSON_Hooks js_hook = {
        .malloc_fn = os_malloc,
        .free_fn = os_free
    };
    cJSON_InitHooks(&js_hook);

    cJSON *js = cJSON_Parse(cfg);
    if (!js) return -1;
    //auth
    cJSON *auth_js = cJSON_GetObjectItem(js, "auth");
    char *auth = cJSON_Print(auth_js);
    duilite_library_load(auth); 
    free(auth);

    //recorder
    cJSON *recorder_js = cJSON_GetObjectItem(js, "recorder");
    cJSON *recorder_device_js = cJSON_GetObjectItem(recorder_js, "device");
    cJSON *recorder_bits_js = cJSON_GetObjectItem(recorder_js, "bits");
    cJSON *recorder_channels_js = cJSON_GetObjectItem(recorder_js, "channels");
    cJSON *recorder_samplerate_js = cJSON_GetObjectItem(recorder_js, "samplerate");

    g_cfg.recorder.device = os_strdup(recorder_device_js->valuestring);
    g_cfg.recorder.bits = recorder_bits_js->valueint;
    g_cfg.recorder.samplerate = recorder_samplerate_js->valueint;
    g_cfg.recorder.channels = recorder_channels_js->valueint;

    //player
    cJSON *player_js = cJSON_GetObjectItem(js, "player");
    cJSON *player_device_js = cJSON_GetObjectItem(player_js, "device");
    g_cfg.player.device = os_strdup(player_device_js->valuestring);

    //wakeup
    cJSON *wakeup_js = cJSON_GetObjectItem(js, "wakeup");
    cJSON *wakeup_cfg_js = cJSON_GetObjectItem(wakeup_js, "cfg");

    cJSON *wakeup_word_js = cJSON_GetObjectItem(wakeup_js, "wakeupWord");
    cJSON *wakeup_audio_js = cJSON_GetObjectItem(wakeup_js, "wakeupAudio");
    g_cfg.wakeup.word_count = cJSON_GetArraySize(wakeup_word_js);

    if (g_cfg.wakeup.word_count > 0) {
        g_cfg.wakeup.word = (char **)os_malloc(g_cfg.wakeup.word_count * sizeof(char *));
        g_cfg.wakeup.audio = (char **)os_malloc(g_cfg.wakeup.word_count * sizeof(char *));
        int i;
        for (i = 0; i < g_cfg.wakeup.word_count; i++) {
            g_cfg.wakeup.word[i] = os_strdup(cJSON_GetArrayItem(wakeup_word_js, i)->valuestring);
            g_cfg.wakeup.audio[i] = os_strdup(cJSON_GetArrayItem(wakeup_audio_js, i)->valuestring);
        }
    }
    g_cfg.wakeup.cfg = cJSON_Print(wakeup_cfg_js);

    //vad
    cJSON *vad_js = cJSON_GetObjectItem(js, "vad");
    cJSON *vad_cfg_js = cJSON_GetObjectItem(vad_js, "cfg");
    cJSON *vad_start_timeout_prompt_js = cJSON_GetObjectItem(vad_js, "startTimeoutPrompt");
    g_cfg.vad.cfg = cJSON_Print(vad_cfg_js);
    g_cfg.vad.start_timeout_prompt = os_strdup(vad_start_timeout_prompt_js->valuestring);


    //dds
    cJSON *dds_js = cJSON_GetObjectItem(js, "dds");
    cJSON *productId_js = cJSON_GetObjectItem(dds_js, "productId");
    cJSON *aliasKey_js = cJSON_GetObjectItem(dds_js, "aliasKey");
    cJSON *server_js = cJSON_GetObjectItem(dds_js, "server");
    cJSON *deviceProfile_js = cJSON_GetObjectItem(dds_js, "deviceProfile");

    g_cfg.dds.productId = os_strdup(productId_js->valuestring);
    g_cfg.dds.aliasKey = os_strdup(aliasKey_js->valuestring);
    g_cfg.dds.server = os_strdup(server_js->valuestring);
    g_cfg.dds.deviceProfile = os_strdup(deviceProfile_js->valuestring);

    cJSON_Delete(js);
    return 0;
}

int dui_library_init(const char *cfg, user_listen_cb listen) {
#ifdef SAVE_AUDIO
    input_fd = fopen("/tmp/1.pcm", "wb");
    output_fd = fopen("/tmp/2.pcm", "wb");
#endif
    if (0 != dui_parse_cfg(cfg)) return -1;
    os_log_init(NULL);

    player_init();
    player_cfg_t tts_cfg = {
        .preprocess_buf_size = 10240,
        .decode_buf_size = 1024 * 5,
        .name = "tts"
    };
    tts_player = player_create(&tts_cfg);
    assert(tts_player != NULL);

    player_cfg_t media_cfg = {
        .preprocess_buf_size = 1024 * 100,
        .decode_buf_size = 1024 * 10,
        .name = "media"
    };
    media_player = player_create(&media_cfg);
    assert(media_player != NULL);

    g_cfg.listen = listen;

    recorder_queue = os_queue_create(5, sizeof(dui_msg_t));
    wakeup_queue = os_queue_create(5, sizeof(dui_msg_t));
    vad_queue = os_queue_create(5, sizeof(dui_msg_t));
    process_queue = os_queue_create(10, sizeof(dui_msg_t));
    player_queue = os_queue_create(5, sizeof(dui_msg_t));
    user_listen_queue = os_queue_create(10, sizeof(dui_msg_t));

    task_ready_ev = os_event_group_create();

    //500ms缓冲
    int recorder_stream_size = 5 * g_cfg.recorder.channels * (g_cfg.recorder.bits / 8) * (g_cfg.recorder.samplerate / 10);
    wakeup_stream = os_stream_create(recorder_stream_size);
    
    //200ms缓冲
    int vad_stream_size = 2 * (g_cfg.recorder.bits / 8) * (g_cfg.recorder.samplerate / 10);
    vad_stream = os_stream_create(vad_stream_size);

    //500ms缓冲
    //由于VAD在检测到语音起始点时数据较大
    int dds_stream_size = 5 * (g_cfg.recorder.bits / 8) * (g_cfg.recorder.samplerate / 10);
    dds_stream = os_stream_create(dds_stream_size);

    //录音线程
    os_thread_cfg_t c = {
        .run = recorder_run
    };
    recorder_task = os_thread_create(&c);

    //唤醒线程
    c.run = wakeup_run;
    wakeup_task = os_thread_create(&c);
    
    //vad线程
    c.run = vad_run;
    vad_task = os_thread_create(&c);
    
    //处理线程
    c.run = process_run;
    process_task = os_thread_create(&c);

    //dds线程
    c.run = dds_run;
    dds_task = os_thread_create(&c);

    //播放线程
    c.run = player_run;
    player_task = os_thread_create(&c);

    c.run = user_listen_run;
    user_listen_task = os_thread_create(&c);

    os_event_group_wait_bits(task_ready_ev, RECORDER_READY_BIT | WAKEUP_READY_BIT | VAD_READY_BIT | PLAYER_READY_BIT | DDS_READY_BIT | USER_LISTEN_READY_BIT | PROCESS_READY_BIT,
            true, true);
    return 0;
}

void dui_library_cleanup(void) {
    //TODO
    os_thread_exit(recorder_task);
    os_thread_exit(wakeup_task);
    os_thread_exit(vad_task);
    os_thread_exit(process_task);
    os_thread_exit(dds_task);
    os_thread_exit(player_task);
    os_thread_exit(user_listen_task);

    os_stream_destroy(wakeup_stream);
    os_stream_destroy(vad_stream);
    os_stream_destroy(dds_stream);
    os_event_group_destroy(task_ready_ev);
    os_queue_destroy(process_queue);
    os_queue_destroy(wakeup_queue);
    os_queue_destroy(vad_queue);
    os_queue_destroy(recorder_queue);
    os_queue_destroy(player_queue);
    os_queue_destroy(user_listen_queue);
    os_log_deinit();

    os_free(g_cfg.recorder.device);
    os_free(g_cfg.player.device);
    os_free(g_cfg.wakeup.cfg);
    os_free(g_cfg.wakeup.param);
    int i;
    for (i = 0; i < g_cfg.wakeup.word_count; i++) {
        os_free(g_cfg.wakeup.word[i]);
        os_free(g_cfg.wakeup.audio[i]);
    }
    os_free(g_cfg.wakeup.word);
    os_free(g_cfg.wakeup.audio);
    os_free(g_cfg.vad.cfg);
    os_free(g_cfg.vad.param);
    os_free(g_cfg.vad.start_timeout_prompt);

    os_free(g_cfg.dds.productId);
    os_free(g_cfg.dds.aliasKey);
    os_free(g_cfg.dds.server);
    os_free(g_cfg.dds.deviceProfile);
    memset(&g_cfg, 0, sizeof(g_cfg));
}
