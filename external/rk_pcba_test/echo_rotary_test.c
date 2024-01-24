/*
 *  rotary_test.c  --  rotary test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Bin Yang <yangbin@rock-chips.com>
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
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <unistd.h>

#define LOG_TAG "rotary_test"
#include "common.h"

#define ROTARY_INPUT_EVENT "/dev/input/event1"

#define ROTARY_TIMEOUT_ROTATE 60  //60s超时

static char result[COMMAND_VALUESIZE] = RESULT_FAIL;

static int rotary_wait_event(int maxfd, fd_set *readfds, int time)
{
	int ret;
	struct timeval timeout;

	FD_ZERO(readfds);
	FD_SET(maxfd, readfds);
	timeout.tv_sec = time;
	timeout.tv_usec = 0;
	ret = select(maxfd + 1, readfds, NULL, NULL, &timeout);
	switch (ret) {
		case -1:
			return -1;
		case 0:
			log_err("select timeout(%ds)\n", time);
			return 1;
		default:
			if (FD_ISSET(maxfd, readfds)) {
				FD_CLR (maxfd, readfds);
				return 0;
			}
			break;
	}

	return -1;
}

static int rotary_event_read(int fd, struct input_event *buf)
{
	int read_len = 0;

	read_len = read(fd, buf, sizeof(*buf));
	if (read_len < 0) {
		if ((errno != EINTR) && (errno != EAGAIN))
			return 0;
		return -1;
	}

	if (buf->type)
		return 1;

	return 0;
}

//* 信号处理函数，在结束进程前，为rotary测试返回一个结果；
static int rotary_result_send(int sign_no)
{
    int err_code =0;
    printf("====================function : %s start =================\n",__func__);
    if(!memcmp(result,RESULT_FAIL,strlen(RESULT_FAIL))){
        err_code = ROTARY_QUERY_FAIL;
    }
    send_msg_to_server("rotary_test", result, err_code);

    printf("====================function : %s finished =================\n",__func__);
    exit(0);
}

int main(int argc, char **argv)
{
	int fd;
	int ret = 0;
	int err_code = 0;
	int time = ROTARY_TIMEOUT_ROTATE;
	fd_set rdfds;
	struct input_event rotary_event;
	unsigned short clockwise=0,anticlockwise=0; //标志旋转方向
	char buf[COMMAND_VALUESIZE] = {0};
	//char result[COMMAND_VALUESIZE] = RESULT_VERIFY;

	log_info("rotary test process start...\n");

    //* 注册信号处理函数
	signal(SIGTERM,(__sighandler_t)rotary_result_send);

	fd = open(ROTARY_INPUT_EVENT, O_RDONLY | O_NOCTTY);
	if (fd < 0) {
		log_err("open fail:%s\n", strerror(errno));
		err_code = ROTARY_OPEN_FAIL;
		goto EXIT;
	}

	while (1&&!(clockwise&&anticlockwise)) {
		if (rotary_wait_event(fd, &rdfds, time) == 0) {
			ret = rotary_event_read(fd, &rotary_event);
			if (ret > 0) {
				if (1==rotary_event.value) {
					log_info("rotary(%d) is clockwise\n", rotary_event.code);
					time = ROTARY_TIMEOUT_ROTATE;
					clockwise = 0x1;
				} else if(-1 == rotary_event.value){
					log_info("rotary(%d) is anticlockwise\n", rotary_event.code);
					time = ROTARY_TIMEOUT_ROTATE;
					anticlockwise = 0x1;
				}
			}
		} else {
			log_err("wait rotary event fail, errno=%d\n", errno);
			err_code = ROTARY_EVENT_TIMEOUT;
			goto EXIT;
		}
	}
	snprintf(buf, sizeof(buf), "rotary_code:%d", rotary_event.code);

EXIT:
	if (!err_code)
		strcpy(result, RESULT_VERIFY);
	send_msg_to_server(buf, result, err_code);

	return err_code;
}
