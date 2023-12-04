/*================================================================
*   Copyright (C) 2018 AISPEECH Ltd. All rights reserved.
*   
*   文件名称：music.c
*   创 建 者：chenjie.gu
*   创建日期：2018年05月24日
*   描    述：
*
================================================================*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "audio_player.h"
#include <time.h>
#include "cJSON.h"
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

audio_player_t *aplayer = NULL;
float vol_multiplier = 0.5;
int vol_system = 70;
int player_is_end = 0;
pthread_mutex_t music_mutex;

int play_judge_f(int index, int count, int mode);

static int g_player_ev = AUDIO_PLAYER_EV_END;
void play_manager_f(const char *cmd, const char *data, char **user_data);
bool music_is_playing(void) {
    if (g_player_ev == AUDIO_PLAYER_EV_END ||
        g_player_ev == AUDIO_PLAYER_EV_ERROR ||
        g_player_ev == AUDIO_PLAYER_EV_STOPPED)
        return false;
    else
        return true;
}

static int play_callback(void *userdata, int ev) {
	printf("++++++%s: ev %d\n", __func__, ev);
    if (ev == AUDIO_PLAYER_EV_END) {
        player_is_end = 1;
    }
	g_player_ev = ev;
    return 0;
}

void *player_routine(void *user) {
    while (1) {
        if (player_is_end) {
            player_is_end = 0;
            play_manager_f("player.end", NULL, NULL);
        }
        usleep(100 * 1000);
    }
    return (void *)0;
}

int music_player_init(char *dev) {
    aplayer = audio_player_new(play_callback, NULL);
    audio_player_set_device(aplayer, dev);
    audio_player_set_channel_volume(aplayer, vol_multiplier);
    pthread_mutex_init(&music_mutex, NULL);
    system("amixer cset name='Master Playback Volume' 70");
    return 0;
}

int music_player_play(char *path) {
    audio_player_play(aplayer, path);
}

int music_player_start() {
    int ret;
    pthread_t tid;
    pthread_create(&tid, NULL, player_routine, NULL);

    return 0;
}

static void send_vol_update_topic (int vol) {
    printf("send_vol_update_topic vol is %d\n", vol);
    char out[128] = {0};
    sprintf(out, "{\"volume\":\"%d\"}", vol);
    printf("send_vol_update_topic is %s\n", out);
    busserver_send_msg("ui.control.topics.response", out);
}

static void send_music_update_topic(int change, int status, int mode, int index, cJSON *list) {
    printf("send_music_update_topic %d %d %d %d %p\n", change, status, mode, index, list);
    char *out;
    cJSON *root, *root2;
    char *music;

    root = cJSON_CreateObject();
    root2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "change", change);
    cJSON_AddNumberToObject(root, "status", status);
    cJSON_AddNumberToObject(root, "mode", mode);
    cJSON_AddNumberToObject(root, "currentIndex", index);
    if (list) {
        cJSON *tmp = cJSON_GetObjectItem(list, "content");
        cJSON_AddItemReferenceToObject(root, "list", tmp);
    }
    
    music = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    cJSON_AddStringToObject(root2, "music", music);
    out = cJSON_PrintUnformatted(root2);
    cJSON_Delete(root2);

    printf("send_music_update_topic is %s\n", out);
    busserver_send_msg("ui.control.topics.response", out);
    free(music);
    free(out);
}

void play_manager_f(const char *cmd, const char *data, char **user_data) {

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
	 * 12. music.info
     */

    enum PLAY_MODE {
        sequence, random, single, loop
    };
    
    enum PLAY_STATUS {
        idle, playing, pause
    };

    static enum PLAY_MODE mode = sequence;
    static enum PLAY_STATUS status = idle;
    static int count = 0;
    static int index = 0;
    static int old_index = 0;
    static cJSON *root = NULL;

    printf("play_manager_f cmd: %s\tdata: %s\n", cmd, data);

    pthread_mutex_lock(&music_mutex);

    if (!strcmp(cmd, "volume.set")) {
        // 设置音量
        if (!strcmp(data, "+")) {
            vol_system += 10;
            if (vol_system > 100) vol_system = 100;
        }

        else if (!strcmp(data, "-")) {
            vol_system -= 10;
            if (vol_system < 0) vol_system = 0;
        }
        else {
            vol_system = atoi(data);
        }

        char cmd[64] = {0};
        sprintf(cmd, "amixer cset name='Master Playback Volume' %d", vol_system);
        system(cmd);
        printf("set vol to %d\n", vol_system);
        send_vol_update_topic(vol_system);
    }

    else if (!strcmp(cmd, "play.list.clear")) {
        // 清空播放列表
        printf("clear the play list info\n");
        audio_player_stop(aplayer);
        cJSON_Delete(root);
        root = NULL;
        index = 0;
        old_index = 0;
        count = 0;
        status = idle;
        send_music_update_topic(0, status, mode, index, NULL);
    }

    else if (!strcmp(cmd, "play.list.get")) {
    }
    else if (!strcmp(cmd, "music.info")) {
        if (root) {
            cJSON *temp, *music;
            music = cJSON_GetObjectItem(root, "content");
            temp = cJSON_GetArrayItem(music, index);
            *user_data = cJSON_Print(temp);
        }
        else *user_data = NULL;
    }
    else if (!strcmp(cmd, "play.list.check")) {
        // 开始真正播放
        cJSON *temp, *music;
        if (root) {
            if (status == idle) {
                temp = cJSON_GetObjectItem(root, "count");
                count = temp->valueint;
                if (count > 0) {
                    status = playing;
                    music = cJSON_GetObjectItem(root, "content");
                    temp = cJSON_GetArrayItem(music, index);
                    temp = cJSON_GetObjectItem(temp, "linkUrl");
                    printf("ready to play url is %s\n", temp->valuestring);
                    audio_player_play(aplayer, temp->valuestring);
                }
            }
            else if (status == pause) {
                status = playing;
                audio_player_resume(aplayer);
            }
            send_music_update_topic(0, status, mode, index, root);
        }
        else send_music_update_topic(0, status, mode, index, NULL);
    }
    else if (!strcmp(cmd, "play.list.update")) {
        // 更新播放列表
        // TODO: update
        printf("update the play list info\n");
        audio_player_stop(aplayer);
        if (root) cJSON_Delete(root);
        root = NULL;
        index = 0;
        old_index = 0;
        count = 0;
        status = idle;
        root = cJSON_Parse(data);
    }
    else if (!strcmp(cmd, "status.set")) {
        // 设置播放状态
        printf("status.set data is %s status is %d\n", data, status);
        if (!strcmp(data, "pause") && status == playing) {
            status = pause;
            audio_player_pause(aplayer);
        }
        else if (!strcmp(data, "resume") && status == pause) {
            status = playing;
            audio_player_resume(aplayer);
        }
        else if (!strcmp(data, "replay") && status == pause) {
            status = playing;
        }
        else if (!strcmp(data, "step")) {
            if (status == playing) {
                status = pause;
                audio_player_pause(aplayer);
            }
            else if (status == pause) {
                status = playing;
                audio_player_resume(aplayer);
            }
        }
        send_music_update_topic(0, status, mode, index, root);
    }
    else if (!strcmp(cmd, "mode.set")) {
        // 播放模式
        if (!strcmp(data, "sequence")) mode = sequence;
        else if (!strcmp(data, "random")) mode = random;
        else if (!strcmp(data, "single")) mode = single;
        else if (!strcmp(data, "loop")) mode = loop;
        send_music_update_topic(0, status, mode, index, root);
    }
    else if (!strcmp(cmd, "change.set")) {
        // 歌曲切换
        // TODO: update
        if (!strcmp(data, "prev")) {
            // 上一首
            index = old_index;
        }
        else if (!strcmp(data, "next")) {
            // 下一首
            old_index = index;
            index = play_judge_f(index, count, mode);
        }
        else if (!strcmp(data, "change")) {
            // 换一首
            old_index = index;
            index = play_judge_f(index, count, mode);
        }

        if (index == -1) {
            // 播放结束
            cJSON_Delete(root);
            audio_player_stop(aplayer);
            root = NULL;
            index = 0;
            old_index = 0;
            count = 0;
            status = idle;
        }
        else {
            // 待播放指定的音频
            status = idle;
        }
    }
    else if (!strcmp(cmd, "play.choose.update")) {
        // 播放特定歌曲
        old_index = index;
        index = atoi(data);
        cJSON *temp, *music;
        music = cJSON_GetObjectItem(root, "content");
        temp = cJSON_GetArrayItem(music, index);
        temp = cJSON_GetObjectItem(temp, "linkUrl");
        printf("ready to play url is %s\n", temp->valuestring);
        audio_player_stop(aplayer);
        audio_player_play(aplayer, temp->valuestring);
        send_music_update_topic(0, status, mode, index, root);
    }
    else if (!strcmp(cmd, "play.collect.choose")) {
        // 收藏歌曲
    }

    else if (!strcmp(cmd, "play.uncollect.choose")) {
        // 取消收藏
    }

    else if (!strcmp(cmd, "player.end") && root) {
        // 播放器播放结束
        // TODO: update
        old_index = index;
        index = play_judge_f(index, count, mode);
        printf("play_judge_f index is %d\n", index);
        if (index == -1) {
            // 播放结束
            audio_player_stop(aplayer);
            cJSON_Delete(root);
            root = NULL;
            index = 0;
            old_index = 0;
            count = 0;
            status = idle;
            send_music_update_topic(0, status, mode, index, NULL);
        }
        else {
            // 播放指定的音频
            cJSON *temp, *music;
            music = cJSON_GetObjectItem(root, "content");
            temp = cJSON_GetArrayItem(music, index);
            temp = cJSON_GetObjectItem(temp, "linkUrl");
            printf("ready to play url is %s\n", temp->valuestring);
            audio_player_stop(aplayer);
            audio_player_play(aplayer, temp->valuestring);
            send_music_update_topic(0, status, mode, index, root);
        }
    }

    pthread_mutex_unlock(&music_mutex);
}

int play_judge_f(int index, int count, int mode) {
    // sequence, random, single, loop
    if (mode == 0) {
        // 顺序播放
        if (index + 1 == count) return -1;
        else return index + 1;
    }
    else if (mode == 1) {
        // 随机播放
        srand(time(0));
        return rand() % count;
    }
    else if (mode == 2) {
        // 单曲循环
        return index;
    }
    else if (mode == 3) {
        // 循环播放
        return (index + 1) % count;
    }
    
    else return -1;
}


