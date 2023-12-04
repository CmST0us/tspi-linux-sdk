#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/rtc.h>
#include <unistd.h>
#include <ctype.h>

#include "common.h"
#include "rtc_test.h"
#include "script.h"
#include "test_case.h"
#include "Language/language.h"


#define TAG	"[PCBA,RTC]: "
#define LOG(x...)	printf(TAG x)


int rtc_xopen(int flags)
{
	int rtc;
	char major_rtc[] = "/dev/rtc";
	char minor_rtc[] = "/dev/rtc0";

	rtc = open(major_rtc, flags);
	if (rtc < 0) {
		rtc = open(minor_rtc, flags);
		if (rtc < 0) {
			printf("open %s failed:%s\n", minor_rtc,
			       strerror(errno));
		}
	}

	else {
		printf("open %s\n", major_rtc);
	}
	return rtc;
}

int rtc_read_tm(struct tm *ptm, int fd)
{
	int ret;

	memset(ptm, 0, sizeof(*ptm));

	ret = ioctl(fd, RTC_RD_TIME, ptm);
	if (ret < 0)
		printf("read rtc failed:%s\n", strerror(errno));
	else
		ptm->tm_isdst = -1;	/* "not known" */

	return ret;
}

static int read_rtc(time_t *time_p)
{
	int fd;
	int ret;
	struct tm tm_time;

	//fd = rtc_xopen(O_RDONLY);
	fd = open("/dev/rtc0", O_RDONLY);
	if (fd < 0)
		return fd;
	else
		ret = rtc_read_tm(&tm_time, fd);

	close(fd);

	if (ret < 0)
		return ret;

	else
		*time_p = mktime(&tm_time);

	return 0;
}

int get_system_time(char *dt)
{
	int ret;
	int fd;
	time_t t;
	time_t timep;
	struct tm *p;

	ret = read_rtc(&timep);
	if (ret < 0)
		return ret;

	else
		p = localtime(&timep);

	sprintf(dt, "%04d-%02d-%02d %02d:%02d:%02d", (1900 + p->tm_year),
		(1 + p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

	return 0;
}

int set_system_time(struct timeval *tv)
{
	int ret;
	int fd;

	fd = open("/dev/rtc0", O_RDWR);
	if (fd < 0) {
		printf("open /dev/rtc0 failed:%s\n", strerror(errno));
		return -1;
	}
	ret = ioctl(fd, RTC_SET_TIME, tv);
	//ret = ioctl(fd, ANDROID_ALARM_SET_RTC, tv);
	if (ret < 0) {
		printf("set rtc failed:%s\n", strerror(errno));
		return -1;
	}

  close(fd);

	return 0;
}


void *rtc_test(void *argc)
{
	struct testcase_info *tc_info = (struct testcase_info *)argc;
	char dt[32] = { "20120926.132600" };
	int ret, y;
	struct tm tm;
	struct timeval tv;
	char *s;
	int day, hour;
	time_t t;
	struct tm *p;
	struct timespec ts;
	struct rtc_time rtc;

	/* remind ddr test */
	if (tc_info->y <= 0)
		tc_info->y = get_cur_print_y();
	y = tc_info->y;

	ui_print_xy_rgba(0, y, 255, 255, 0, 255, "%s:[%s..]\n", PCBA_RTC,
			 PCBA_TESTING);

#if 0
	s = malloc(32);
	if (script_fetch("rtc", "module_args", (int *)dt, 8) == 0)
		strncpy(s, dt, 32);
	day = atoi(s);
	while (*s && *s != '.')
		s++;
	if (*s)
		s++;
	hour = atoi(s);

	tm.tm_year = day / 10000 - 1900;
	tm.tm_mon = (day % 10000) / 100 - 1;
	tm.tm_mday = (day % 100);
	tm.tm_hour = hour / 10000;
	tm.tm_min = (hour % 10000) / 100;
	tm.tm_sec = (hour % 100);
	tm.tm_isdst = -1;
	tv.tv_sec = mktime(&tm);
	tv.tv_usec = 0;
	printf("set rtc time :%lu\n", tv.tv_sec);
	//ret = set_system_time(&tv);
	ret = set_system_time(&tm);
	if (ret < 0) {
		printf("test rtc failed:set_system_time failed\n");
		ret = -1;
	}

	else {
		sleep(1);

		while (1) {
			t = get_system_time(dt);
			if (t < 0) {
				ret = -1;
				break;
			}
			p = localtime(&t);

			ui_display_sync(0, y, 0, 255, 0, 255,
					"%s:[%s] { %04d/%02d/%02d %02d:%02d:%02d }\n",
					PCBA_RTC, PCBA_SECCESS,
					(1900 + p->tm_year),
					(1 + p->tm_mon), p->tm_mday,
					p->tm_hour, p->tm_min, p->tm_sec);
			sleep(1);
		}
	}
#else
{
    int rst;
    char result_filename[100] = {0};

    LOG("=======pcba_core :start rtc test========\n");
    rst = run_test_item_cmd("echo_auto_test echo_rtc_test");

    if(rst == 0) {
        snprintf(result_filename, sizeof(result_filename),
		        "%s/echo_%s_test_result", "/tmp",tc_info->base_info->name);
        ret = parse_test_result(result_filename, "rtc_test", NULL);
    } else {
        return NULL;
    }

    #define MINOR_RTC_PATH "/dev/rtc0"

    //show time
    if (ret == 0) {
        int fd,ret;
        struct tm tm_p;

        //LOG("==========rtc_get_system_time_test start===========\n");
        //fd = rtc_xopen(O_RDONLY|O_NOCTTY|O_NDELAY);
        fd = open(MINOR_RTC_PATH, O_RDONLY);
        if (fd < 0) {
            LOG("open %s failed:%s\n", MINOR_RTC_PATH,strerror(errno));
            return argc;
        }

        memset(&tm_p,0,sizeof(tm_p));
        ret = ioctl(fd, RTC_RD_TIME, &tm_p);
        if (ret < 0) {
            LOG("test rtc failed:get_system_time failed: %s\n",strerror(errno));
    		tc_info->result = -1;
    		ui_print_xy_rgba(0, y, 255, 0, 0, 255, "%s:[%s]\n",
                            PCBA_RTC, PCBA_FAILED);
            return NULL;
        }
    	else {
            //LOG("rtc test: rtc_get_system_time success.\n");
            mktime(&tm_p);  //使用mktime函数得到time_t格式的输出

            //LOG("test time is: %04d-%02d-%02d %02d:%02d:%02d\n", 1900+tm_p.tm_year,
            //       1 + tm_p.tm_mon, tm_p.tm_mday, tm_p.tm_hour, tm_p.tm_min, tm_p.tm_sec);

            tc_info->result = 0;
			ui_display_sync(0, y, 0, 255, 0, 255,
        					"%s:[%s] { %04d/%02d/%02d %02d:%02d:%02d }\n",
        					PCBA_RTC, PCBA_SECCESS,
        					1900+tm_p.tm_year, 1 + tm_p.tm_mon, tm_p.tm_mday,
        					tm_p.tm_hour, tm_p.tm_min, tm_p.tm_sec);
        }

        //LOG("System time is :%s\n",asctime(&tm_p));
        close(fd);
        //LOG("==========rtc_get_system_time_test finish.===========\n");
        return 0;
    } else {
		tc_info->result = -1;
		ui_print_xy_rgba(0, y, 255, 0, 0, 255, "%s:[%s]\n",
                        PCBA_RTC, PCBA_FAILED);
        return argc;
    }
}
#endif
	if (ret == 0) {
		tc_info->result = 0;
		ui_print_xy_rgba(0, y, 0, 255, 0, 255, "%s:[%s]\n",
				        PCBA_RTC, PCBA_SECCESS);
	} else {
		tc_info->result = -1;
		ui_print_xy_rgba(0, y, 255, 0, 0, 255, "%s:[%s]\n",
                        PCBA_RTC,PCBA_FAILED);
	}
	return argc;
}
