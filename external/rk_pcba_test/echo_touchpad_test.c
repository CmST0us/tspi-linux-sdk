/*
 * Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>

#define LOG_TAG "echo_touchpad_test"
#include "common.h"

#define TOUCHPAD_NODE "sys/devices/11050000.i2c/i2c-0/0-0020/cy8ctouchread"

int char_to_value(char *ch)
{
	if ((*ch >= 'A') && (*ch <= 'F'))
		return 10 + *ch - 'A';
	else if ((*ch >= 'a') && (*ch <= 'f'))
		return 10 + *ch - 'a';
	else if ((*ch >= '0') && (*ch <= '9'))
		return *ch - '0';
	else
		return 0;
}

int main()
{
	int delay_t = 0,err_code = 0;
	struct timeval t1, t2;
	char buf[COMMAND_VALUESIZE] = "touchpad_test";
    char result[COMMAND_VALUESIZE] = RESULT_VERIFY;
	int fd = 0, ret = 0, key = 0;
	char buff[40], *start = NULL;
	int flag_pre = 0, flag_play = 0, flag_nxt = 0;

	fd = open(TOUCHPAD_NODE, O_RDONLY);
	if (fd <= 0) {
		log_err("open touchpad node file failed(%s).\n",
				strerror(errno));
		err_code = -ENODEV;
		goto OUT;
	}

	gettimeofday(&t1, NULL);
    while(1) {
		lseek(fd, 0, SEEK_SET);
		ret = read(fd, buff, sizeof(buff));
		if (ret < 0) {
			log_err("read touchpad node file failed(%s).\n",
					strerror(errno));
			err_code = -ENODEV;
			break;
		}
		start = strstr(buff, "0x");
		key = char_to_value(start + 2)*16 + char_to_value(start + 3);

		if ((key >= 0x40) && (key <= 0x44)) {
			log_info("--> <pause/play> key pressed!\n");
			flag_play = 1;
		} else if ((key >= 0x59) && (key <= 0x5d)) {
			log_info("--> <previous> key pressed!\n");
			flag_pre = 1;
		} else if ((key >= 0x28) && (key <= 0x2c)) {
			log_info("--> <next> key pressed!\n");
			flag_nxt = 1;
		}

		if (flag_nxt && flag_play && flag_pre)
			break;

        gettimeofday(&t2, NULL);
		delay_t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
		if (delay_t > MANUAL_TEST_TIMEOUT) {
			log_warn("timeout(60s)\n");
			err_code = -ETIME;
			break;
		}

		usleep(2000);
    }

OUT:
    if (err_code)
		strcpy(result, RESULT_FAIL);
    send_msg_to_server(buf, result, err_code);

    return 0;
}
