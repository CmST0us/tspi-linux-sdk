/*================================================================
*   Copyright (C) 2018 AISPEECH Ltd. All rights reserved.
*   
*   文件名称：main.c
*   创 建 者：chenjie.gu
*   创建日期：2018年05月16日
*   描    述：
*
================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/select.h>
#include <pthread.h>
#include <time.h>
#include "dds_client.h"
#include "button.h"
#include "busserver.h"
#include "cJSON.h"
#include <stdbool.h>
struct dds_client *dc = NULL;
bool m_is_dialog = false;
int is_enable_wakeup = 1;

extern int music_player_init(char *dev);
extern int music_player_start();
extern bool music_is_playing(void);
extern void play_manager_f(const char *cmd, const char *data, char **user_data);
void handle_doa_result(int doa);
static send_tts_update_topic ();

#define WAIT_WAKEUP_SYSTEM_CMD      "./aispeech_led -m on 4"
#define RUNNING_ASR_SYSTEM_CMD      "./aispeech_led -m scroll -b 4 2 -s 100"
#define WAIT_DM_OUT_SYSTEM_CMD      "./aispeech_led -m scroll -b 4 2 -s 100"
#define RUNNING_TTS_SYSTEM_CMD      "./aispeech_led -m on -b  2 2"
#define DISABLE_WAKEUP_SYSTEM_CMD   "./aispeech_led -m on -b  2 1"
pthread_mutex_t mylock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mycond=PTHREAD_COND_INITIALIZER;
void led_off() {
    printf("%s\n", __func__);
    system("./aispeech_led -m on 4");
}

void clean_silence_frame() {
    system("echo 0 > /sys/module/snd_soc_rockchip_vad/parameters/voice_inactive_frames");
}
void do_system_sleep() {
    system("echo mem > /sys/power/state");
}
void do_wifi_suspend_prepare() {
    system("dhd_priv setsuspendmode 1");
    usleep(200*1000);
}
void do_wifi_resume_prepare() {
    system("dhd_priv setsuspendmode 0");
}
#define VAD_WAKEUP_LEVEL_MIN 0
#define VAD_WAKEUP_LEVEL_MAX 5
#define VAD_WAKEUP_TIME_MAX 10
int m_vad_wakeup_time = 0;
int m_vad_wakeup_level = 0;
void set_vad_level(int level) {
    if(level > VAD_WAKEUP_LEVEL_MAX) {
        m_vad_wakeup_level = VAD_WAKEUP_LEVEL_MAX;
    }

    printf("set vad level:%d\n",level);
    switch(level) {
        case 0:
            system("echo 0x60 0x40ff01C2 > /sys/kernel/debug/vad/reg");
            break;
        case 1:
            system("echo 0x60 0x40ff0200 > /sys/kernel/debug/vad/reg");
            break;
        case 2:
            system("echo 0x60 0x40ff0250 > /sys/kernel/debug/vad/reg");
            break;
        case 3:
            system("echo 0x60 0x40ff0270 > /sys/kernel/debug/vad/reg");
            break;
        case 4:
            system("echo 0x60 0x40ff0300 > /sys/kernel/debug/vad/reg");
            break;
        case 5:
            system("echo 0x60 0x40ff0350 > /sys/kernel/debug/vad/reg");
            break;
    }
}


/*
     * 1. volume.set
     * 2. play.list.update
     * 3. play.list.clear
     * 4. play.list.get
     * 5. status.set
     * 6. change.set
     * 7. mode.set
     * 8. play.choose.update
     * 9. play.collect.choose
     * 10. play.uncollect.choose
     * 11. player.end
     */

void dds_cb(const char *topic, const char *topic_data, void *user) {
    static int end_dialog = 0;

    printf("dds cb receive topic: %s\n", topic);
    printf("dds cb receive topic_data: %s\n", topic_data);

    if (!strcmp(topic, "dm.output")) {
        cJSON *root = cJSON_Parse(topic_data);
        assert(root != NULL);
        
        cJSON *dm = cJSON_GetObjectItem(root, "dm");
        cJSON *widget = cJSON_GetObjectItem(dm, "widget");
        if (widget) {
            cJSON *count = cJSON_GetObjectItem(widget, "count");
            if (count && count->valueint > 0) {
                char *out = cJSON_PrintUnformatted(widget);
                play_manager_f("play.list.update", out, NULL);
                free(out);
            }
        }

        cJSON *end_session = cJSON_GetObjectItem(dm, "shouldEndSession");
        if (end_session->valueint) {
            end_dialog = 1;
        }
        else end_dialog = 0;
        
        system(WAIT_WAKEUP_SYSTEM_CMD);
        clean_silence_frame();
        m_is_dialog = false;
        printf("dm.output close dialog\n");     
        cJSON_Delete(root);
    }
	else if (!strcmp(topic, "doa.result")) {
		// doa 结果
		cJSON *root = cJSON_Parse(topic_data);
		assert(root != NULL);
		cJSON *doa = cJSON_GetObjectItem(root, "doa");
		handle_doa_result(doa->valueint);
		cJSON_Delete(root);
    }

    else if (!strcmp(topic, "command://spk.speaker.close")) {
        play_manager_f("play.list.clear", NULL, NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.single")) {
        play_manager_f("mode.set", "single", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.sequence")) {
        play_manager_f("mode.set", "sequence", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.random")) {
        play_manager_f("mode.set", "random", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.loop")) {
        play_manager_f("mode.set", "loop", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.pause")) {
        play_manager_f("status.set", "pause", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.continue")) {
        play_manager_f("status.set", "resume", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.stop")) {
        play_manager_f("play.list.clear", NULL, NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.replay")) {
        play_manager_f("status.set", "replay", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"APICommandResult\":\"success\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.prev")) {
        play_manager_f("change.set", "prev", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"count\":\"more\", \"skillName\":\"speakerChinaPlay\", \"title\":\"\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.next")) {
        play_manager_f("change.set", "next", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"count\":\"more\", \"skillName\":\"speakerChinaPlay\", \"title\":\"\"}}");
    }
    else if (!strcmp(topic, "native://mediacontrol.media.change")) {
        play_manager_f("change.set", "change", NULL);
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"count\":\"more\", \"skillName\":\"speakerChinaPlay\", \"title\":\"\"}}");
    }
    else if (!strcmp(topic, "native://query.music.info")) {
        char *data = NULL;
        play_manager_f("music.info", NULL, &data);
        if (data) {
            cJSON *root = cJSON_Parse(data);
            char resp[512] = {0};
            cJSON *title = cJSON_GetObjectItem(root, "title");
            cJSON *subTitle = cJSON_GetObjectItem(root, "subTitle");
            cJSON *label = cJSON_GetObjectItem(root, "label");
            sprintf(resp, "{\"duiWidget\":\"text\", \"extra\":{\"title\":\"%s\", \"subTitle\":\"%s\", \"label\":\"%s\"}}", title->valuestring, subTitle->valuestring, label->valuestring);

            dds_client_resp_nativeapi(dc, topic, resp);
            free(data);
            cJSON_Delete(root);
        }
        else {
            dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{}}");
        }
    }
    else if (!strcmp(topic, "native://query.story.info")) {
        char *data = NULL;
        play_manager_f("music.info", NULL, &data);
        if (data) {
            cJSON *root = cJSON_Parse(data);
            char resp[512] = {0};
            cJSON *title = cJSON_GetObjectItem(root, "title");
            sprintf(resp, "{\"duiWidget\":\"text\", \"extra\":{\"title\":\"%s\"}}", title->valuestring);
            dds_client_resp_nativeapi(dc, topic, resp);
            free(data);
            cJSON_Delete(root);
        }
        else {
            dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{}}");
        }
    }
    else if (!strcmp(topic, "command://spk.speaker.voice")) {
        cJSON *root = cJSON_Parse(topic_data);
        cJSON *voice = cJSON_GetObjectItem(root, "voice");
        if (voice) {
            play_manager_f("volume.set", voice->valuestring, NULL);
        }
        play_manager_f("status.set", "resume", NULL);
        // speak
        music_player_play("../res/tts/vol.mp3");
        cJSON_Delete(root);
    }

    else if (!strcmp(topic, "native://alarm.set")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"已为您设置闹钟\"}}");
    }
    else if (!strcmp(topic, "native://alarm.list")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"您要查的闹钟不存在哦\"}}");
    }
    else if (!strcmp(topic, "native://alarm.delete")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"已为您删除闹钟\"}}");
    }
    else if (!strcmp(topic, "native://alarm.close")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"已为您关闭闹钟\"}}");
    }

    else if (!strcmp(topic, "native://remind.set")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"已为您设置提醒\"}}");
    }
    else if (!strcmp(topic, "native://remind.list")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"暂时抄不到提醒哦\"}}");
    }
    else if (!strcmp(topic, "native://remind.delete")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"已为您删除提醒\"}}");
    }
    else if (!strcmp(topic, "native://remind.close")) {
        dds_client_resp_nativeapi(dc, topic, "{\"duiWidget\":\"text\", \"extra\":{\"text\":\"已为您关闭提醒\"}}");
    }
 
    else if (!strcmp(topic, "local_wakeup.result")) {
        end_dialog = 0;
        play_manager_f("status.set", "pause", NULL);
        m_vad_wakeup_time = 0;
        m_vad_wakeup_level = VAD_WAKEUP_LEVEL_MIN;
        set_vad_level(m_vad_wakeup_level);
    }
    else if (!strcmp(topic, "sys.dm.end")) {
        // 对话退出
        play_manager_f("play.list.check", NULL, NULL);
        system(WAIT_WAKEUP_SYSTEM_CMD);
        clean_silence_frame();
        m_is_dialog = false;
        if (!is_enable_wakeup) {
            system(DISABLE_WAKEUP_SYSTEM_CMD);
        }
    }
    else if (!strcmp(topic, "sys.tts.begin")) {
        clean_silence_frame();
    }
    else if (!strcmp(topic, "sys.tts.end")) {
    }
    else if (!strcmp(topic, "sys.vad.end")) {
    }
    else if (!strcmp(topic, "sys.asr.begin")) {
        system(RUNNING_ASR_SYSTEM_CMD);
        clean_silence_frame();
        m_is_dialog = true;
    }
}
void handle_doa_result(int doa) {
    printf("handle_doa_result doa is %d\n", doa);
#if DUAL_MIC
    system("./aispeech_led -m on 1");
    return;
#else 
#ifdef RK3308_BOARD_V11
	static char *doa_led_table[12] = {
		"./aispeech_led  -m single -i 2 0",
		"./aispeech_led  -m single -i 3 0",
		"./aispeech_led  -m single -i 4 0",
		"./aispeech_led  -m single -i 5 0",
		"./aispeech_led  -m single -i 6 0",
		"./aispeech_led  -m single -i 7 0",
		"./aispeech_led  -m single -i 8 0",
		"./aispeech_led  -m single -i 9 0",
		"./aispeech_led  -m single -i 10 0",
		"./aispeech_led  -m single -i 11 0",
		"./aispeech_led  -m single -i 0 0",
		"./aispeech_led  -m single -i 1 0"
	};

#elif defined(RK3308_BOARD_V10)
    static char *doa_led_table[12] = { 
        "./aispeech_led  -m single -i 7 0",
        "./aispeech_led  -m single -i 8 0",
        "./aispeech_led  -m single -i 9 0",
        "./aispeech_led  -m single -i 10 0",
        "./aispeech_led  -m single -i 11 0",
        "./aispeech_led  -m single -i 0 0",
        "./aispeech_led  -m single -i 1 0",
        "./aispeech_led  -m single -i 2 0",
        "./aispeech_led  -m single -i 3 0",
        "./aispeech_led  -m single -i 4 0",
        "./aispeech_led  -m single -i 5 0",
        "./aispeech_led  -m single -i 6 0"
    };
#endif

	if (doa >= 0) {
		if (doa >=345 || doa < 15) {
			system(doa_led_table[0]);
		} else if (doa >= 15 && doa < 45) {
			system(doa_led_table[1]);
		} else if (doa >= 45 && doa < 75) {
			system(doa_led_table[2]);
		} else if (doa >= 75 && doa < 105) {
			system(doa_led_table[3]);
		} else if (doa >= 105 && doa < 135) {
			system(doa_led_table[4]);
		} else if (doa >= 135 && doa < 165) {
			system(doa_led_table[5]);
		} else if (doa >= 165 && doa < 195) {
			system(doa_led_table[6]);
		} else if (doa >= 195 && doa < 225) {
			system(doa_led_table[7]);
		} else if (doa >= 225 && doa < 255) {
			system(doa_led_table[8]);
		} else if (doa >= 255 && doa < 285) {
			system(doa_led_table[9]);
		} else if (doa >= 285 && doa < 315) {
			system(doa_led_table[10]);
		} else if (doa >= 315 && doa < 345) {
			system(doa_led_table[11]);
		}
	}
    else printf("=== doa result is wrong\n");

#endif
}


void button_cb(button_event_t ev, void *userdata) {
    const char *info[] = {
        "BUTTON_EVENT_VOLUME_ADD",
        "BUTTON_EVENT_VOLUME_SUB",
        "BUTTON_EVENT_PREV",
        "BUTTON_EVENT_NEXT",
        "BUTTON_EVENT_PLAY_PAUSE",
        "BUTTON_EVENT_PLAY_PAUSE_LONG",
        "BUTTON_EVENT_MUTE_UNMUTE",
        "BUTTON_EVENT_MUTE_UNMUTE_LONG",
        "BUTTON_EVENT_MODE_WIFI",
        "BUTTON_EVENT_MODE_NORMAL"
    };

    printf("%s\n", info[ev]);

    if (ev == BUTTON_EVENT_VOLUME_ADD) {
        //短按触发
        play_manager_f("volume.set", "+", NULL);
    }
    else if (ev == BUTTON_EVENT_VOLUME_SUB) {
        //短按触发
        play_manager_f("volume.set", "-", NULL);
    }
    else if (ev == BUTTON_EVENT_PREV) {
        //长按每隔1.5秒触发一次
        play_manager_f("change.set", "prev", NULL);
        play_manager_f("play.list.check", NULL, NULL);
    }
    else if (ev == BUTTON_EVENT_NEXT) {
        //长按每隔1.5秒触发一次
        play_manager_f("change.set", "next", NULL);
        play_manager_f("play.list.check", NULL, NULL);
    }
    else if (ev == BUTTON_EVENT_PLAY_PAUSE) {
        //短按触发
        play_manager_f("status.set", "step", NULL);
    }
    else if (ev == BUTTON_EVENT_MUTE_UNMUTE) {
        // mute 
        if (is_enable_wakeup) {
            is_enable_wakeup = 0;
            dds_client_stop_dialog(dc);
            dds_client_disable_wakeup(dc);
            system(DISABLE_WAKEUP_SYSTEM_CMD);
        }
        else {
            is_enable_wakeup = 1;
            dds_client_enable_wakeup(dc);
            system(WAIT_WAKEUP_SYSTEM_CMD);
        }
    }
}

void mqtt_cb(const char *topic, const char *topic_data, int data_len, void *user) {
    printf("mqtt_cb receive: %s: %.*s\n", topic, data_len, topic_data);

    cJSON *root = cJSON_Parse(topic_data);
    assert(root != NULL);
    cJSON *volume, *music, *tts;
    volume = cJSON_GetObjectItem(root, "volume");
    music = cJSON_GetObjectItem(root, "music");
    tts = cJSON_GetObjectItem(root, "tts");
    if (volume) {
        // 音量设置
        play_manager_f("volume.set", volume->valuestring, NULL);
    }
    if (music) {
        // 音乐设置
        cJSON *data;
        cJSON *change, *status, *mode, *currentIndex;
        data = cJSON_Parse(music->valuestring);
        assert(data != NULL);
        change = cJSON_GetObjectItem(data, "change");
        if (change) {
            static char change_cmd[3][10] = {
                {"nothing"},
                {"prev"},
                {"next"}
            };
            play_manager_f("change.set", change_cmd[change->valueint], NULL);
            play_manager_f("play.list.check", NULL, NULL);
        }
        status = cJSON_GetObjectItem(data, "status");
        if (status) {
            static char status_cmd[3][10] = {
                {"nothing"},
                {"resume"},
                {"pause"}
            };
            play_manager_f("status.set", status_cmd[status->valueint], NULL);
        }
        mode = cJSON_GetObjectItem(data, "mode");
        if (mode) {
            static char mode_cmd[5][10] = {
                {"nothing"},
                {"sequence"},
                {"random"},
                {"single"},
                {"loop"}
            };
            play_manager_f("mode.set", mode_cmd[mode->valueint], NULL);
        }

        currentIndex = cJSON_GetObjectItem(data, "currentIndex");
        if (currentIndex) {
            char index[8];
            sprintf(index, "%d", currentIndex->valueint);
            play_manager_f("play.choose.update", index, NULL);
        }

        cJSON_Delete(data);
    }
    if (tts) {
        cJSON *data;
        cJSON *current;
        data = cJSON_Parse(data->valuestring);
        assert(data != NULL);
        current = cJSON_GetObjectItem(data, "current");
        if (current) {
            cJSON *voice = cJSON_GetObjectItem(current, "voiceId");
            dds_client_set_speaker(dc, voice->valuestring);
            dds_client_speak(dc, "该轮到我上场了");
            send_tts_update_topic();
        }
        cJSON_Delete(data);
    }
    cJSON_Delete(root);
}

static send_tts_update_topic () {
	static char *tts_label[8] = {"甜美女生",
		"沉稳纲叔",
		"淡定葛爷",
		"邻家女声",
		"标准男声",
		"可爱童声",
		"标准女声",
		NULL};

	static char *tts_voiceId[8] = {"zhilingf", "gdgm", "geyou", "hyanif", "xijunm",
		"qianranf", "lucyf", NULL};
    
    cJSON *root, *root2, *array, *temp, *current;
    root = cJSON_CreateObject();
    root2 = cJSON_CreateObject();
    
    array = cJSON_CreateArray();
    for (int i = 0; tts_voiceId[i]; i++) {
        temp = cJSON_CreateObject();
        cJSON_AddStringToObject(temp, "voiceId", tts_voiceId[i]);
        cJSON_AddStringToObject(temp, "label", tts_label[i]);
        cJSON_AddItemToArray(array, temp);
    }
    cJSON_AddItemToObject(root, "list", array);
    
    current = cJSON_CreateObject();
    char *speaker = dds_client_get_speaker(dc);
    int volume = dds_client_get_volume(dc);
    float speed = dds_client_get_speed(dc);

    cJSON_AddStringToObject(current, "voiceId", speaker);
    cJSON_AddNumberToObject(current, "volume", volume);
    cJSON_AddNumberToObject(current, "speed", speed);

    cJSON_AddItemToObject(root, "current", current);

    char *tts = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    cJSON_AddStringToObject(root2, "tts", tts);
    char *out = cJSON_PrintUnformatted(root2);
    
    printf("send_tts_update_topic is %s\n", out);
    busserver_send_msg("ui.control.topics.response", out);

    free(tts);
    free(out);
}

void *button_routine(void *user) {
    button_config_t config = {
        .dev = "/dev/input/event0", // v11: event0 | v10: event1
        .cb = button_cb
    };
    button_handle_t button = button_create(&config);
    while (1) {
        button_run(button);
    }
    button_destroy(button);
    return (void *)0;
}

void *busserver_routine(void *user) {
    busserver_run("0.0.0.0:50001", mqtt_cb, NULL);
    return (void *)0;
}
const unsigned int voice_inactive_max_count = 16000 * 5; //16k, 3 seconds
unsigned int read_voice_inactive_frames(void)
{
	FILE *fp;
	char buf[100];
	unsigned int frames = 0;

	fp = popen("cat /sys/module/snd_soc_rockchip_vad/parameters/voice_inactive_frames", "r");
	if(!fp) {
		perror("popen");
		exit(EXIT_FAILURE);
	}
	memset(buf, 0, sizeof(buf));
	if (fgets(buf, sizeof(buf) - 1, fp) != 0 ) {
		sscanf(buf, "%ul", &frames);
		//printf("%s frames %lu\n", buf, frames);
	}
	pclose(fp);
	return frames;
}

bool sleep_check(void) {
    unsigned int inactive_frames = read_voice_inactive_frames();
    bool music_playing = music_is_playing();

    printf("inactive frames %d, max %d, dialog %d, music %d\n",
            inactive_frames, voice_inactive_max_count, m_is_dialog, music_playing);
    if (music_playing) {
        clean_silence_frame();
        m_vad_wakeup_time = 0;
    } else {
        m_vad_wakeup_time++;
        if(m_vad_wakeup_time > VAD_WAKEUP_TIME_MAX) {
            m_vad_wakeup_time = 0;
            set_vad_level(m_vad_wakeup_level++);
        }
    }
    if ((inactive_frames > voice_inactive_max_count) \
        && !m_is_dialog && !music_playing)
        return true;
    return false;
}

void wait_device_mode_timeout_ms(int microseconds)
{
    struct timeval tv;
    long long absmsec;
    struct timespec abstime;

    gettimeofday(&tv, NULL);
    absmsec = tv.tv_sec * 1000ll + tv.tv_usec / 1000ll;
    absmsec += microseconds;

    abstime.tv_sec = absmsec / 1000ll;
    abstime.tv_nsec = absmsec % 1000ll * 1000000ll;

    printf("#### public sleep mode ####");
    pthread_mutex_lock(&mylock);
    pthread_cond_timedwait(&mycond, &mylock, &abstime);
    pthread_mutex_unlock(&mylock);
    printf("#### return sleep mode succeed ####");
}

void *vad_detect_func(void* arg) {
    clean_silence_frame();
    while(true) {
        if (sleep_check()) {
            m_vad_wakeup_time = 0;
            fprintf(stderr,"voice inactive timeout,go to sleep\n");
            dds_client_publish(dc, DDS_CLIENT_USER_DEVICE_MODE, "{\"mode\":\"sleep\"}");
            wait_device_mode_timeout_ms(30);
            printf("pause >>>>\n");
            clean_silence_frame();

            do_wifi_suspend_prepare();
            do_system_sleep();
            do_wifi_resume_prepare();

            printf("resume >>>>\n");
            dds_client_publish(dc, DDS_CLIENT_USER_DEVICE_MODE, "{\"mode\":\"normal\"}");
        }
        usleep(1000*1000);
    }
}

int main () {
    int ret;
    char config[1024 * 5];
    FILE *fp = NULL;
    fp = fopen("./config.json", "r");
    if (fp) {
        fread(config, 1, 1024 * 5, fp);
        fclose(fp);
    }
    else return -1;

    // 1. init music player
    cJSON *root = cJSON_Parse(config);
    cJSON *player = cJSON_GetObjectItem(root, "player");
    if (player) {
        player = cJSON_GetObjectItem(player, "device");
        if (player) music_player_init(player->valuestring);
        else music_player_init("default");
    }
    else music_player_init("default");

    // 2. start the music player
    music_player_start();

    // 3. run the busserver
    pthread_t tid;
    ret = pthread_create(&tid, NULL, busserver_routine, NULL);
    assert(ret != -1);

    // 3. run the dds client
    dc = dds_client_init(config);
    assert(dc != NULL);

    ret = dds_client_start(dc, dds_cb, NULL);
    assert(ret != -1);

    // 4. button thread
    pthread_t tid2;
    pthread_create(&tid2, NULL, button_routine, NULL);

    // 5. system init
    system("amixer cset name='Master Playback Volume' 70");
    system("chmod +x aispeech_led");
    pthread_t softvad_detect;
    pthread_create(&softvad_detect,NULL,vad_detect_func,NULL);
#if 0    
#ifdef PDM_MIC
    system("amixer -c 0 cset name='ADC ALC Group 0 Left Volume' 20");
    system("amixer -c 0 cset name='ADC ALC Group 0 Right Volume' 20");
#else
    system("amixer -c 0 cset name='ADC ALC Group 0 Left Volume' 28");
    system("amixer -c 0 cset name='ADC ALC Group 0 Right Volume' 28");
    system("amixer -c 0 cset name='ADC ALC Group 1 Left Volume' 28");
    system("amixer -c 0 cset name='ADC ALC Group 1 Right Volume' 28");
    system("amixer -c 0 cset name='ADC ALC Group 2 Left Volume' 28");
    system("amixer -c 0 cset name='ADC ALC Group 2 Right Volume' 28");
    system("amixer -c 0 cset name='ADC ALC Group 3 Left Volume' 18");                                                                   
    system("amixer -c 0 cset name='ADC ALC Group 3 Right Volume' 18");
#endif
#endif
    //send_tts_update_topic();
    led_off();//led init
    select(0, 0, 0, 0, 0);

    dds_client_release(dc);

    return 0;
}

