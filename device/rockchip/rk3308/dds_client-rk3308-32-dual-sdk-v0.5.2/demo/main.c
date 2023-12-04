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
#include "cJSON.h"

struct dds_client *dc = NULL;

extern int music_player_init(char *dev);
extern int music_player_start();
extern void play_manager_f(const char *cmd, const char *data, char **user_data);

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


static char cmd_word_hint_map[69][64] = {
    {"command://qi ke kong tiao"},
    {"command://da kai kong tiao"},
    {"command://guan bi kong tiao"},
    {"command://da kai sheng yin"},
    {"command://zeng da yin liang"},
    {"command://jian xiao yin liang"},
    {"command://guan bi sheng yin"},
    {"command://qie huan nan sheng"},
    {"command://qie huan nv sheng"},
    {"command://sheng wen cai ji"},
    {"command://qing chu cai ji xin xi"},
    {"command://zhi leng mo shi"},
    {"command://zhi re mo shi"},
    {"command://nuan feng mo shi"},
    {"command://song feng mo shi"},
    {"command://pi ei mu wei mo shi"},
    {"command://kong qi qing xin"},
    {"command://chu shi mo shi"},
    {"command://kong wen chu shi"},
    {"command://jie neng mo shi"},
    {"command://guan bi jie neng"},
    {"command://dian jia re"},
    {"command://guan bi dian jia re"},
    {"command://feng sui sheng dong"},
    {"command://guan bi feng sui sheng dong"},
    {"command://feng ni sheng dong"},
    {"command://guan bi feng ni sheng dong"},
    {"command://ge xing mo shi"},
    {"command://guan bi ge xing mo shi"},
    {"command://ting zhi bai dong"},
    {"command://zuo you bai dong"},
    {"command://shang xia bai dong"},
    {"command://zui xiao feng"},
    {"command://zhong deng feng"},
    {"command://zui da feng"},
    {"command://zeng da feng su"},
    {"command://jian xiao feng su"},
    {"command://zi dong feng"},
    {"command://shang shu shi feng"},
    {"command://xia shu shi feng"},
    {"command://quan shu shi feng"},
    {"command://sheng gao wen du"},
    {"command://jiang di wen du"},
    {"command://shi liu du"},
    {"command://shi qi du"},
    {"command://shi ba du"},
    {"command://shi jiu du"},
    {"command://er shi du"},
    {"command://er shi yi du"},
    {"command://er shi er du"},
    {"command://er shi san du"},
    {"command://er shi si du"},
    {"command://er shi wu du"},
    {"command://er shi liu du"},
    {"command://er shi qi du"},
    {"command://er shi ba du"},
    {"command://er shi jiu du"},
    {"command://san shi du"},
    {"command://san shi yi du"},
    {"command://san shi er du"},
    {"command://kai ji"},
    {"command://guan ji"},
    {"command://shu shi mo shi"},
    {"command://yi shei ou"},
    {"command://guan bi yi shei ou"},
    {"command://feng da dian"},
    {"command://feng xiao dian"},
    {"command://tai re le"},
    {"command://tai leng le"}
};

void dds_cb(const char *topic, const char *topic_data, void *user) {
    static int end_dialog = 0;
    static int num = 0;

    printf("dds cb receive topic: %s\n", topic);
    printf("dds cb receive topic_data: %s\n", topic_data);

    if (!strncmp(topic, "command", 7)) {
        for (int i = 0; i < 69; i++) {
            if (!strcmp(topic, cmd_word_hint_map[i])) {
                // do the speak 
                char wav_name[16] = {0};
                sprintf(wav_name, "./audio/%d.wav", i + 1);
                char play_cmd[32] = {0};
                sprintf(play_cmd, "aplay %s", wav_name);
                system(play_cmd);
                break;
            }
        }
    }

    if (!strcmp(topic, "command://sheng wen cai ji")) {
        // 注册声纹
        char name[64] = {0};
        sprintf(name, "{\"name\":\"%s%d\"}", "test", ++num);

        dds_client_vprint_regist(dc, name);
    }
    
    else if (!strcmp(topic, "command://qing chu cai ji xin xi")) {
        // 删除声纹
        //dds_client_vprint_unregist(dc, "test1");
    }

    else if (!strcmp(topic, "vprint.regist.result")) {
        // 收到了声纹注册的消息
        cJSON *root = cJSON_Parse(topic_data);
        cJSON *operation = cJSON_GetObjectItem(root, "operation");

        if (!strcmp(operation->valuestring, "start")) {
            printf("====================== 开始注册声纹\n");
            dds_client_speak(dc, "请说唤醒词，开始注册声纹");
        }

        if (!strcmp(operation->valuestring, "nameRepeat")) {
            printf("====================== 注册姓名重复\n");
            dds_client_speak(dc, "注册姓名重复");
        }
        else if (!strcmp(operation->valuestring, "vNumLimit")) {
            printf("============= 注册数量已满\n");
            dds_client_speak(dc, "注册数量已满");
        }

        else if (!strcmp(operation->valuestring, "unavailable")) {
            printf("============= 信噪比不符合要求\n");
            dds_client_speak(dc, "信噪比不符合要求");
        }

        else if (!strcmp(operation->valuestring, "continue")) {
            printf("============= 请继续注册声纹\n");
            dds_client_speak(dc, "请继续注册声纹");
        }

        else if (!strcmp(operation->valuestring, "success")) {
            printf("============ 声纹注册成功\n");
            dds_client_speak(dc, "声纹注册成功");
        }
        cJSON_Delete(root);
    }

    else if (!strcmp(topic, "vprint.unregist.result")) {
        // 收到了声纹删除的消息
        
        cJSON *root = cJSON_Parse(topic_data);
        cJSON *operation = cJSON_GetObjectItem(root, "operation");

        if (!strcmp(operation->valuestring, "success")) {
            printf("==================== 声纹删除成功");
            //dds_client_speak(dc, "声纹删除成功");
        }
        else if (!strcmp(operation->valuestring, "noSpeaker")) {
            printf("==================== 该用户尚未注册");
            //dds_client_speak(dc, "该用户尚未注册");
        }

        cJSON_Delete(root);
    }
    else if (!strcmp(topic, "vprint.test.result")) {
        // 收到了声纹计算的结果，有以下三种情况:
        /*
         * 1. {"register":"nothing"} // 目前还没有声纹模型
         *
         * 
         * // 收到了匹配的声纹计算结果
         * 2. {"score":23.286682,"word":"qi ke kong tiao","register":"test0","time":179.679932,"speech":0.880000,"RTF":0.204188} 
         *
         *
         * // 收到了不匹配的声纹计算结果
         *
         * 3. {"score":23.286682,"word":"qi ke kong tiao","register":"others","time":179.679932,"speech":0.880000,"RTF":0.204188} 
         *
         */

        printf("vprint test receive result is %s\n", topic_data);
        
        cJSON *root = cJSON_Parse(topic_data);
        cJSON *reg = cJSON_GetObjectItem(root, "register");

        if (!strcmp("nothing", reg->valuestring)) {
            play_manager_f("status.set", "pause", NULL);
            char *hint = "{\"nlg\":\"path:../res/tts/help.mp3\"}";
            dds_client_publish(dc, DDS_CLIENT_USER_EXTERNAL_WAKEUP, hint);
        }
        else {
            play_manager_f("status.set", "pause", NULL);
            char hint[256] = {0};
            snprintf(hint, 256, "{\"nlg\":\"path:../res/tts/help.mp3\"}", reg->valuestring);
            dds_client_publish(dc, DDS_CLIENT_USER_EXTERNAL_WAKEUP, hint);
        }

        cJSON_Delete(root);
    }

    else if (!strcmp(topic, "dm.output")) {
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
        
        cJSON_Delete(root);
    }
    else if (!strcmp(topic, "doa.result")) {
        // doa 结果
    }

    else if (!strcmp(topic, "command://sys.tts.setspeaker")) {
        static char speakers[6][20] = {
            {"zhilingf"}, 
            {"gdgm"}, 
            {"geyou"}, 
            {"hyanif"}, 
            {"xijunm"}, 
            {"qianranf"}
        };
        srand(time(0));
        int choose = rand() % 6;
        dds_client_set_speaker(dc, speakers[choose]);
        //dds_client_speak(dc, "已经为您切换");
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
    }
    else if (!strcmp(topic, "sys.dm.end")) {
        // 对话退出
        play_manager_f("play.list.check", NULL, NULL);
    }
    else if (!strcmp(topic, "sys.tts.begin")) {
    }
    else if (!strcmp(topic, "sys.tts.end")) {
    }
    else if (!strcmp(topic, "sys.vad.end")) {
    }
    else if (!strcmp(topic, "sys.asr.begin")) {
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

    // 3. run the dds client
    dc = dds_client_init(config);
    assert(dc != NULL);

    ret = dds_client_start(dc, dds_cb, NULL);
    assert(ret != -1);

    select(0, 0, 0, 0, 0);

    dds_client_release(dc);

    return 0;
}



