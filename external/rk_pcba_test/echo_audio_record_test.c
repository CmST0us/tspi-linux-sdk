/*
 *  audio_test.c  --  audio test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Panzhenzhuan Wang <randy.wang@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#include "audio_test.h"

#define LOG_TAG "audio_record_test"
#include "common.h"
#define RECORD_TIME 5
#define AUDIO_EVENT_TIMEOUT -85

//*Audio test
void *audio_record_test(void *argv)
{
    char cmd[128];

    fprintf(stderr,"=========function :%s start=============\n",__func__);
    //指定Card 3，device 0，录音3秒
#ifdef PCBA_3308
    sprintf(cmd,"arecord -D hw:3,0 -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
#endif

#ifdef PCBA_PX3SE
    sprintf(cmd,"arecord -D MainMicCapture -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
#endif

#ifdef PCBA_3229GVA
//TODO:

#endif
    system(cmd);
    fprintf(stderr,"recording\n ");
    //sprintf(cmd,"aplay -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE); //放音
    sprintf(cmd,"aplay -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE); //放音
    system(cmd);
    fprintf(stderr,"playing\n ");
    //remove(cmd);
    sprintf(cmd,"rm %s/%s",TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
    system(cmd);
    fprintf(stderr,"=========Audio record test finished=============\n");
}

//* 信号处理函数，在结束进程前，删除录音的pcm文件
static int del_record_pcm(int sign_no)
{
    char cmd[128];

    printf("====================function : %s start =================\n",__func__);

    sprintf(cmd,"%s/%s",TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
    if(0 == access(cmd,F_OK)){
       remove(cmd);
    }

    printf("====================function : %s finished =================\n",__func__);
    exit(0);
}

/*主函数入口*/
int main(int argc, char *argv[])
{
    int delay_t = 0,err_code = 0;
	struct timeval t1, t2;
	char buf[COMMAND_VALUESIZE] = "audio_record_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;

    system("amixer set Playback 30%");
	log_info("audio record test start...\n");

	//* 注册信号处理函数
	signal(SIGTERM,(__sighandler_t)del_record_pcm);

	gettimeofday(&t1, NULL);
    while(1)
    {
        audio_record_test(argv[0]);
        gettimeofday(&t2, NULL);
		delay_t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
		if (delay_t > MANUAL_TEST_TIMEOUT) {
			log_warn("audio record test end, timeout 60s\n");
			err_code = AUDIO_EVENT_TIMEOUT;
			break;
		}
    }
    if (err_code)
		strcpy(result, RESULT_FAIL);
    send_msg_to_server(buf, result, err_code);
    return 0;
}
