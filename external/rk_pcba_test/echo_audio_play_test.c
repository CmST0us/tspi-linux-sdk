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
#include <sys/time.h>

#include "audio_test.h"

#define LOG_TAG "audio_play_test"
#include "common.h"
#define AUDIO_EVENT_TIMEOUT -85
#define AUDIO_TIME 5

//*Audio test
void *audio_play_test(void *argv)
{
    char cmd[128];
    memset(cmd, 0, sizeof(cmd));

    fprintf(stderr,"=========function :%s start=============\n",__func__);
    //* 1、先放音
    fprintf(stderr,"Start test audio record and play.\n");
#ifdef PCBA_PX3SE
    sprintf(cmd,"aplay -d %d %s/%s",AUDIO_TIME,PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif

#ifdef PCBA_3308
    sprintf(cmd,"aplay %s/%s",PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif
#ifdef PCBA_3229GVA
    sprintf(cmd,"aplay %s/%s",PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif

#ifdef PCBA_1808
    sprintf(cmd,"aplay -D default:CARD=rockchiprk809co %s/%s",PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif

    if (cmd[0] != '\0')
        system(cmd);
    else {
        fprintf(stderr,"====CMD is null, Pls check PCBA platform====\n",__func__);
        return (void*)-1;
    }
    fprintf(stderr,"=========function :%s finish=============\n",__func__);
    return 0;
}

/*主函数入口*/
int main(int argc, char *argv[])
{
    int delay_t = 0,err_code = 0;
	struct timeval t1, t2;
	char buf[COMMAND_VALUESIZE] = "audio_play_test";
    char result[COMMAND_VALUESIZE] = RESULT_VERIFY;

#ifdef PCBA_3308
    system("amixer cset numid=4 26");
#endif
#ifdef PCBA_3308
    system("amixer set Master Playback 30%");
#endif
#ifdef PCBA_3229GVA
    system("amixer cset numid=4 26");
#endif
	log_info("audio play test start...\n");
	gettimeofday(&t1, NULL);

    int ret;
    while(1)
    {
        ret = (int)audio_play_test(argv[0]);
        if (ret) {
            err_code = AUDIO_PLAY_ERR;
            break;
        }

        gettimeofday(&t2, NULL);
		delay_t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
		if (delay_t > MANUAL_TEST_TIMEOUT) {
			log_warn("audio play test end, timeout 60s\n");
			err_code = AUDIO_EVENT_TIMEOUT;
			break;
		}
    }
    if (err_code)
		strcpy(result, RESULT_FAIL);
    send_msg_to_server(buf, result, err_code);
    return 0;
}
