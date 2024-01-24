/*
 *  led_test.c  --  led test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author:
 *  Panzhenzhuan Wang <randy.wang@rock-chips.com>
 *  Bin Yang <yangbin@rock-chips.com>
 *
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

#include <string.h>
#include <unistd.h>
#include "led_test.h"

#define LOG_TAG "led_test"
#include "common.h"

#define LED_BRIGHTNESS_FILE "/sys/class/leds/led%d/brightness"
#define LED_TRIGGER_FILE "/sys/class/leds/led%d/trigger"
#define LED_SHOT_FILE "/sys/class/leds/led%d/shot"
#define LED_DELAY_ON_FILE "/sys/class/leds/led%d/delay_on"
#define LED_DELAY_OFF_FILE "/sys/class/leds/led%d/delay_off"

#define LED_NUM 36
#define COLOR_NUM 3
//定义LED开启、关闭
#define LED_ON "128"
#define LED_OFF "0"

#define TRIGGER_NONE 0

#define LOG_TAG "led_test"
#include "common.h"

#define LED_TEST_TIMEOUT -51
#define LED_PROC_ERR -55

//定义LED灯颜色结构体
typedef enum
{
    LED_BLUE = 0,
    LED_GREEN,
    LED_RED,
} LED_colors;

void *led_test(void *argv);    //LED测试程序
int write_int(char const* path,char const* value);  //将值写入对应路径文件

void *led_test(void *argv)
{
    char buf[128];
    char cmd[128];
    int led_num,color_num =1;
    LED_colors test_color;


    printf("===================LED test start======================\n\n");
    fprintf(stderr,"Start testing LED, order is Red, Green, Blue, White.\n Testing:\n");
    while(color_num < 4)
    {
        switch(color_num%3)  //第一轮RED灯、第二轮GREEN灯、第三轮BLUE灯、最后一轮RGB合成白灯
            {
            case 1:
                printf(" The testing color is RED: \n");
                break;
            case 2:
                printf(" The testing color is GREEN: \n");
                break;
            case 0:
                printf(" The testing color is BLUE: \n");
                break;
            default :
                fprintf(stderr,"select color error");
            }
        for(led_num=1;led_num<=LED_NUM;led_num++)
        {
            switch(led_num % 3)
            {
            case LED_RED :
                if(color_num % 3 == 1)
                {
                    sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
                    if(-1==write_int(buf,LED_ON))
                        goto fail;
                }
                else
                {
                    sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
                    if(-1==write_int(buf,LED_OFF))
                        goto fail;
                }
                break;
            case LED_GREEN :
                if(color_num % 3 == 2)
                {
                    sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
                    if(-1==write_int(buf,LED_ON))
                        goto fail;
                }
                else
                {
                    sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
                    if(-1==write_int(buf,LED_OFF))
                        goto fail;
                }
                break;
            case LED_BLUE :
                if(color_num % 3 == 0)
                {
                    sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
                    if(-1==write_int(buf,LED_ON))
                        goto fail;
                }
                else
                {
                    sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
                    if(-1==write_int(buf,LED_OFF))
                        goto fail;
                }
                break;
            default :
                fprintf(stderr,"select color error");
            }
        }
        printf("LED shining 1 seconds:\n");
        sleep(1);  //间隔1秒
        color_num++;
    }

    //RGB三种颜色合在一起形成白灯
    for(led_num=1;led_num<=LED_NUM;led_num++)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        if(-1==write_int(buf,LED_ON))
            goto fail;
    }

    sleep(1);       //暂停1秒

    for(led_num=1;led_num<=LED_NUM;led_num++)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        if(-1==write_int(buf,LED_OFF))
        goto fail;
    }
    fprintf(stderr, "===========LED Test finished=============\n");

    return (void*)0;
    fail:

        return (void*)-1;
}

//* 信号处理函数，在结束进程前，关闭所有灯
static int leds_all_off(int sign_no)
{
    char buf[128];
    int led_num;

    printf("====================function : %s start =================\n",__func__);
    for(led_num=1; led_num<=LED_NUM; led_num++)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        if(-1==write_int(buf,LED_OFF))
            goto fail;
    }

    printf("====================function : %s finished =================\n",__func__);
    exit(0);
    fail:
        exit(-1);
}

/*主函数入口*/
int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "led_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    int delay_t = 0;
	struct timeval t1, t2;

	log_info("led test start...\n");

	//* 注册信号处理函数
	signal(SIGTERM,(__sighandler_t)leds_all_off);

	gettimeofday(&t1, NULL);
    while(1)
    {
        test_flag = (int)led_test(argv[0]);
        if(test_flag < 0)
        {
            err_code = LED_PROC_ERR;
            break;
        }

        gettimeofday(&t2, NULL);
		delay_t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
		if (delay_t > MANUAL_TEST_TIMEOUT) {
			log_warn("led test end, timeout 60s\n");
			err_code = LED_TEST_TIMEOUT;
			break;
		}
    }
    if (err_code)
		strcpy(result, RESULT_FAIL);
    send_msg_to_server(buf, result, err_code);
    return 0;
}
