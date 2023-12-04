/*
 *  audio_play_test.c  --  audio play test application
 *
 *  Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 *  Author: chad.ma <chad.ma@rock-chips.com>
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
#include <pthread.h>

#include <linux/input.h>
#include <fcntl.h>
#include <dirent.h>
#include "common.h"
#include "key_test.h"
#include "test_case.h"
#include "language.h"

#include <unistd.h>
#include "audio_play_test.h"

#define LOG_TAG	    "PCBA [audio play]: "
#define LOG(x...)	printf(LOG_TAG x)

#define PCBA_TEST_PATH "/data/"
#define AUDIO_TIME 5

void *audio_play_test(void *argv)
{
    int ret, y;
    char cmd[128];
	struct testcase_info *tc_info = (struct testcase_info *)argv;

	if (tc_info->y <= 0)
		tc_info->y = get_cur_print_y();
	y = tc_info->y;

    LOG("=========function :%s start=============\n",__func__);
	ui_print_xy_rgba(0, y, 255, 255, 0, 255, "%s:[%s..]\n", PCBA_CODEC_PLAY,
			 PCBA_TESTING);

#ifdef PCBA_PX3SE
    sprintf(cmd,"aplay -d %d %s%s",AUDIO_TIME,PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif

#ifdef PCBA_3308
    sprintf(cmd,"aplay %s/%s",PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif
#ifdef PCBA_3229GVA
    sprintf(cmd,"aplay %s/%s",PCBA_TEST_PATH,AUDIO_PLAY_FILE);
#endif

    ret = run_test_item_cmd(cmd);

    if(ret == 0) {
       ui_print_xy_rgba(0, y, 0, 255, 0, 255, "%s:[%s] { %s }\n", PCBA_CODEC_PLAY,
                PCBA_SECCESS, "play test success.");
       tc_info->result = 0;
       LOG("audio play success.\n");
   }else  {
       //ret < 0
       ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s] { %s }\n", PCBA_CODEC_PLAY,
                PCBA_FAILED, "play test fail....... ");
       tc_info->result = -1;
       LOG("audio play failed.\n");
       return NULL;
   }

    LOG("=========function :%s finish=============\n",__func__);
    return argv;
}


