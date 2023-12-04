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
#include "audio_record_test.h"

#define LOG_TAG	    "PCBA [audio record]: "
#define LOG(x...)	printf(LOG_TAG x)

#define TEST_RESULT_SAVE_PATH "/tmp"
#define RECORD_TIME 5

void *audio_record_test(void *argv)
{
    int ret, y;
    char cmd[128];
	struct testcase_info *tc_info = (struct testcase_info *)argv;

	if (tc_info->y <= 0)
		tc_info->y = get_cur_print_y();
	y = tc_info->y;

    LOG("=========function :%s start=============\n",__func__);
	ui_print_xy_rgba(0, y, 255, 255, 0, 255, "%s:[%s..]\n", PCBA_CODEC_RECORD,
			 PCBA_TESTING);

    //指定Card 3，device 0，录音3秒
#ifdef PCBA_3308
    sprintf(cmd,"arecord -D hw:3,0 -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
#endif

#ifdef PCBA_PX3SE
    sprintf(cmd,"arecord -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
#endif

#ifdef PCBA_3229GVA
//TODO:

#endif

    //arecord file
    ret = run_test_item_cmd(cmd);
    if (ret != 0) {//run cmd fail, do'nt go ahead ,return.
        ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s] { %s }\n", PCBA_CODEC_RECORD,
                 PCBA_FAILED, "record test fail....... ");
        tc_info->result = -1;
        LOG("audio record failed.\n");
        return NULL;
    }
    LOG("\trecording\n ");

    //aplay file
    //sprintf(cmd,"aplay -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE); //放音
    sprintf(cmd,"aplay -f S16_LE -c 2 -r 16000 -d %d %s/%s",RECORD_TIME,TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE); //放音
    ret = run_test_item_cmd(cmd);
    if (ret != 0) {//run cmd fail, do'nt go ahead ,return.
        ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s] { %s }\n", PCBA_CODEC_RECORD,
                 PCBA_FAILED, "record test fail....... ");
        tc_info->result = -1;
        LOG("audio record failed.\n");
        return NULL;
    }
    LOG("\tplaying\n ");

    //remove(cmd);
    sprintf(cmd,"rm %s/%s",TEST_RESULT_SAVE_PATH,AUDIO_RECORD_FILE);
    ret = run_test_item_cmd(cmd);

    if(ret == 0) {
        ui_print_xy_rgba(0, y, 0, 255, 0, 255, "%s:[%s] { %s }\n", PCBA_CODEC_RECORD,
                 PCBA_SECCESS, "record test success.");
        tc_info->result = 0;
        LOG("audio record success.\n");
    }else  {
        //ret < 0
        ui_print_xy_rgba(0, y, 225, 0, 0, 255, "%s:[%s] { %s }\n", PCBA_CODEC_RECORD,
                 PCBA_FAILED, "record test fail....... ");
        tc_info->result = -1;
        LOG("audio record failed.\n");
        return NULL;
    }

    LOG("=========Audio record test finished=============\n");
}


