/*
 *  rtc_test.c  --  rtc test application
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
//RTC测试总程序，包含设置系统时间和获取系统时间
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//time相关头文件
#include <time.h>
#include <sys/time.h>
//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//error相关头文件
#include <errno.h>
//RTC有关头文件
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "rtc_test.h"

#define LOG_TAG "rtc_test"
#include "common.h"
#define RTC_PROC_ERR -95

#define RTC_TIME_DIFF 1

#define MAJOR_RTC_PATH "/dev/rtc"
#define MINOR_RTC_PATH "/dev/rtc0"
#define MAX_TEST 2

//* 获取RTC读取路径
int rtc_xopen(int flags)
{
	int rtc_fd;

	rtc_fd = open(MAJOR_RTC_PATH, flags);
	if (rtc_fd < 0) {
		rtc_fd = open(MINOR_RTC_PATH, flags);
		if (rtc_fd < 0) {
			printf("open %s failed:%s\n", MINOR_RTC_PATH,
			       strerror(errno));
		}
	}

	else {
		printf("open %s\n", MAJOR_RTC_PATH);
	}
	return rtc_fd;
}

//* 设置系统时间
int set_system_time(struct tm *tm_p)
{
    int ret;
    int fd;

    //fd = rtc_xopen(O_RDWR|O_NOCTTY|O_NDELAY);
    fd = open(MINOR_RTC_PATH, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd < 0)
    {
        printf("open %s failed:%s\n", MINOR_RTC_PATH,strerror(errno));
        return -1;
    }

    //使用RTC对应的ioctl函数设置时间
    ret = ioctl(fd, RTC_SET_TIME, tm_p);
    if (ret < 0)
    {
        printf("set rtc failed:%s\n", strerror(errno));
        return -1;
    }

    close(fd);

    return 0;
}

//1、rtc设置系统时间测试
int rtc_set_system_time_test(char *time_set, time_t *local_time)
{
    char *s;
    int day, hour;
    struct tm tm_p;

    printf("==========rtc_set_system_time_test start===========\n");
    s = time_set;
    //设置年、月、日,得到的是格林时间
    day = atoi(s);
    //printf("day value is %d\n",day);
    tm_p.tm_year = day/10000 - 1900;
    tm_p.tm_mon = (day%10000)/100 - 1;
    tm_p.tm_mday = (day%100);

    //循环到达小数点后面的数值，即时分秒值
    while (*s && *s != '.')
        s++;
    if (*s)
        s++;

    //设置时、分、秒
    hour = atoi(s);
    //printf("hour value is %d\n",hour);

    tm_p.tm_hour = hour/10000;
    tm_p.tm_min = (hour%10000)/100;
    tm_p.tm_sec = (hour%100);
    tm_p.tm_isdst = -1;

    //mktime()函数形成本地时间
    *local_time = mktime(&tm_p);

    //struct tm *格式输入，打印设置的本地时间
    //printf("asctime(&tm_p) is %s\n",asctime(&tm_p));
    //time_t *格式输入，打印设置的本地时间
    //printf("ctime(local_time) is %s\n",ctime(local_time));

    /*1、设置系统时间*/
    int ret;
    ret = set_system_time(&tm_p);
    if (ret < 0)
    {
        //printf("test rtc failed:set_system_time failed.\n");
        ret = -1;
    }
    else
    {
        //printf("rtc test: rtc_set_system_time success.\n");
        ret = 0;
    }
    printf("==========rtc_set_system_time_test finish===========\n\n");
    return ret;
}

//2、获取系统时间
int rtc_get_system_time_test(time_t *local_time)
{
    int fd,ret;
    struct tm tm_p;

    //printf("==========rtc_get_system_time_test start===========\n");
    //fd = rtc_xopen(O_RDONLY|O_NOCTTY|O_NDELAY);
    fd = open(MINOR_RTC_PATH, O_RDONLY);
    if (fd < 0)
    {
        printf("open %s failed:%s\n", MINOR_RTC_PATH,strerror(errno));
        return -1;
    }

    //使用RTC对应的ioctl函数获取系统时间
    memset(&tm_p,0,sizeof(tm_p));

    ret = ioctl(fd, RTC_RD_TIME, &tm_p);
    if (ret < 0)
    {
        printf("test rtc failed:get_system_time failed: %s\n",strerror(errno));
        return -1;
    }
	else
    {
        //printf("rtc test: rtc_get_system_time success.\n");
        *local_time = mktime(&tm_p);  //使用mktime函数得到time_t格式的输出

        //printf("test time is: %04d-%02d-%02d %02d:%02d:%02d\n", 1900+tm_p.tm_year,
        //       1 + tm_p.tm_mon, tm_p.tm_mday, tm_p.tm_hour, tm_p.tm_min, tm_p.tm_sec);
    }

    //printf("System time is :%s\n",asctime(&tm_p));
    close(fd);
    //printf("==========rtc_get_system_time_test finish.===========\n");
    return 0;
}

void *rtc_test(void *argv)
{
    char time_set[32] = {"20180101.120000" };   //需要设置的系统时间符号“.”前面是年月日，后面是时分秒
    int ret =0;
    time_t lt_set,lt_get1,lt_get2;
    char cmd[128];
    int count = 0;

    //printf("=================rtc test start=================\n\n");
    //sprintf(cmd,"aplay %s/rtc_test_start.wav",AUDIO_PATH);
    //system(cmd);
    //system("aplay /data/test/rtc_test_start.wav");
    //* 1、首先设置系统时间*/
    ret = rtc_set_system_time_test(time_set,&lt_set);
    if(ret<0)
        goto fail;
    sleep(RTC_TIME_DIFF); //休眠RTC_TIME_DIFF s
    lt_get1 = lt_set;
    //* 2、然后读取系统时间、从而知道RTC是否正常运行*/
    ret = rtc_get_system_time_test(&lt_get2);
    if(ret<0)
        goto fail;
    //printf("lt_get2-lt_get1 = %d\n",lt_get2-lt_get1);
    //printf("lt_set is \t: %s\n",ctime(&lt_set));
    //printf("lt_get2 is \t: %s\n",ctime(&lt_get2));

    //休眠若干s，则RTC走了若干秒钟，如果不是，则测试失败
    if(RTC_TIME_DIFF>(lt_get2%60-lt_get1%60)){
        ret = -1;
        goto fail;
    }
    //printf("=================rtc test success=================\n");

    return (void*)ret;
    fail:
        printf("=================rtc test failed=================\n");
        return (void*)ret;
}


int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "rtc_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    test_flag = (int)rtc_test(argv[0]);
    if(test_flag < 0)
    {
        strcpy(result,RESULT_FAIL);
        err_code = RTC_PROC_ERR;
    }
    send_msg_to_server(buf, result, err_code);
}

