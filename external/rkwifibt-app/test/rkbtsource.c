/*
 * Copyright (c) 2017 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <linux/input.h>
#include <linux/rtc.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "rkbtsource_common.h"

static pthread_t rk_bt_recv_t = 0, rk_bt_spp_s_t = 0;
static int spp_is_connected = 0;
static int sockfd = 0, spp_client_fd = 0;
static int bt_server_open_state = 0;
static int bt_server_close_state = 0;

static void bt_cmd_list()
{
	unsigned int i;
	for (i = 1; i < sizeof(bt_command_tbs) / sizeof(bt_commands_t); i++) {
		printf("%02d.  %s \n", i, bt_command_tbs[i].cmd);
	}
}

static int rk_spp_client_recv(void *p)
{
	int bytes_read;
	char buf[64];

wait_client:
	while (spp_is_connected == 0)
		sleep(1);

	while (1) {
		//check spp link
		if (spp_is_connected == 0) {
			close(spp_client_fd);
			spp_client_fd = 0;
			spp_is_connected = 0;
			goto wait_client;
		}

		memset(buf, 0, sizeof(buf));
		bytes_read = read(spp_client_fd, buf, sizeof(buf));
		if (bytes_read < 0) {
			printf("[BT SPP] error: %d:%s!\n", bytes_read, strerror(errno));
			close(spp_client_fd);
			spp_client_fd = 0;
			spp_is_connected = 0;
			goto wait_client;
		}

		printf("spp client recv: [%s] \n", buf);

		//echo
		bytes_read = write(spp_client_fd, buf, sizeof(buf));
	}
}

static int rk_bt_check_server(void)
{
	if (bt_server_open_state && bt_server_close_state) {
		//close socket, exit.
		bt_server_open_state = 0;
		bt_server_close_state = 0;
		close(sockfd);
		return 0;
	}

	return 1;
}

static int rk_bt_recv(void *p)
{
	int ret, status;
	bt_msg_t evt;
	char buff[256] = "";
	struct iovec io;
	struct msghdr msg;
	struct cmsghdr *cmsg;

	while (1) {
		memset(&evt, 0, sizeof(bt_msg_t));
		io.iov_base = &evt,
		io.iov_len = sizeof(bt_msg_t),

		msg.msg_iov = &io,
		msg.msg_iovlen = 1,
		msg.msg_control = buff,
		msg.msg_controllen = sizeof(buff),

		ret = recvmsg(sockfd, &msg, MSG_CMSG_CLOEXEC);
		if (ret <= 0)
			continue;

		switch(evt.id) {
		case RK_BT_EVT_SCAN_ADD_DEV:
			printf("%s: device is found\n", PRINT_FLAG_RKBTSOURCE);
			printf("		address: %s\n", evt.dev.addr);
			printf("		name: %s\n", evt.dev.name);
			printf("		role: 0x%x\n", evt.dev.role);
			printf("		rssi: %d\n", evt.dev.rssi);
			break;
		case RK_BT_EVT_SCAN_REMOVE_DEV:
			printf("%s: device is removed\n", PRINT_FLAG_RKBTSOURCE);
			printf("		address: %s\n", evt.dev.addr);
			printf("		name: %s\n", evt.dev.name);
			break;
		case RK_BT_EVT_SCAN_CHANGE_DEV:
			printf("%s: device is changed\n", PRINT_FLAG_RKBTSOURCE);
			printf("		address: %s\n", evt.dev.addr);
			printf("		name: %s\n", evt.dev.name);
			printf("		rssi: %d\n", evt.dev.rssi);
			break;
		case RK_BT_EVT_PAIRED_DEV:
			printf("%s: device is paired\n", PRINT_FLAG_RKBTSOURCE);
			printf("		address: %s\n", evt.dev.addr);
			printf("		name: %s\n", evt.dev.name);
			printf("		is_connected: %d\n", evt.dev.is_connected);
			break;
		case RK_BT_EVT_SPP_DISCONNECTED:
			printf("RK_BT_EVT_SPP_DISCONNECTED\n");
			spp_is_connected = 0;
			break;
		case RK_BT_EVT_SPP_CONNECT_FAILED:
			printf("RK_BT_EVT_SPP_CONNECT_FAILED\n");
			spp_is_connected = 0;
			break;
		case RK_BT_EVT_SPP_CONNECTED:
			printf("RK_BT_EVT_SPP_CONNECTED\n");
			cmsg = CMSG_FIRSTHDR(&msg);
			if (cmsg->cmsg_level != SOL_SOCKET)
				continue;
			if (cmsg->cmsg_level != SCM_RIGHTS)
				continue;
			spp_client_fd = *((int *)CMSG_DATA(cmsg));
			printf("RK_BT_EVT_SPP_CLIENT_FD: %d\n", spp_client_fd);
			spp_is_connected = 1;
			break;
		case RK_BT_EVT_INIT_OK:
			printf("RK_BT_SERVER OPEN SUCCESS\n");
			bt_server_open_state = 1;
			break;
		case RK_BT_EVT_DEINIT_OK:
			printf("RK_BT_SERVER CLOSED\n");
			bt_server_close_state = 1;
			return 0;
		case RK_BT_EVT_SCAN_ON:
			printf("RK_BT_SERVER SCAN\n");
			break;
		case RK_BT_EVT_SCAN_OFF:
			printf("RK_BT_SERVER SCAN OFF\n");
			break;
		case RK_BT_EVT_SOURCE_OPEN:
			printf("RK_BT_SERVER SOURCE OPEN\n");
			break;
		case RK_BT_EVT_SOURCE_CLOSE:
			printf("RK_BT_SERVER SOURCE OFF\n");
			break;
		case RK_BT_EVT_SOURCE_CONNECTED:
			printf("RK_BT_SERVER SOURCE CONNECTED\n");
			printf("		address: %s\n", evt.dev.addr);
			printf("		name: %s\n", evt.dev.name);
			break;
		case RK_BT_EVT_SOURCE_DISCONNECTED:
			printf("RK_BT_SERVER SOURCE DISCONNECTED\n");
			printf("		address: %s\n", evt.dev.addr);
			printf("		name: %s\n", evt.dev.name);
			break;
		case RK_BT_EVT_PONG:
			printf("RK_BT_SERVER_LIVE\n");
			bt_server_open_state = 1;
			break;
		default:
			printf("[s_t_c: %s]\n", bt_evt_str_table[evt.id].desc);
		}
	}
}

int main(int argc, char *argv[])
{
	int i, item_cnt, ret;
	char cmdbuf[64], cmd[64];
	char *input_start;
	bt_msg_t msg;

	bt_server_open_state = 0;
	bt_server_close_state = 0;

	unlink(RKBTSOURCE_CLIENT_SOCKET_PATH);
	//start rkwifibt_server
	system("killall rkwifibt_server");
	//realpath
	system("rkwifibt_server &");

	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		printf("%s: Socket create failed!\n", PRINT_FLAG_ERR);
		return -1;
	}

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, RKBTSOURCE_CLIENT_SOCKET_PATH);
	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		printf("%s: Bind Local addr failed!\n", PRINT_FLAG_ERR);
		goto OUT;
	}

	struct timeval t = {4, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));

	/* Set server address */
	struct sockaddr_un dst;
	dst.sun_family = AF_UNIX;
	strcpy(dst.sun_path, RKBTSOURCE_SERVER_SOCKET_PATH);

	bt_cmd_list();
	item_cnt = sizeof(bt_command_tbs) / sizeof(bt_commands_t);

	if (rk_bt_recv_t)
		return 0;

	if (pthread_create(&rk_bt_recv_t, NULL, rk_bt_recv, NULL)) {
		printf("%s: Create rk_bt_init_thread thread failed\n", __func__);
		return -1;
	}
	pthread_setname_np(rk_bt_recv_t, "rk_bt_recv_t");
	pthread_detach(rk_bt_recv_t);

	if (pthread_create(&rk_bt_spp_s_t, NULL, rk_spp_client_recv, NULL)) {
		printf("%s: Create rk_bt_init_thread thread failed\n", __func__);
		return -1;
	}
	pthread_setname_np(rk_bt_spp_s_t, "rk_bt_spp_s_t");
	pthread_detach(rk_bt_spp_s_t);

	while (1) {
		if (!rk_bt_check_server())
			return 0;
		memset(cmdbuf, 0, sizeof(cmdbuf));
		memset(&msg, 0, sizeof(bt_msg_t));
		printf("Please input number or help to run: ");

		if (fgets(cmdbuf, 64, stdin) == NULL)
			continue;

		if (!strncmp("help", cmdbuf, 4) || !strncmp("h", cmdbuf, 1))
			bt_cmd_list();

		//remove end space
		cmdbuf[strlen(cmdbuf) - 1] = 0;

		input_start = strstr(cmdbuf, "input");
		if (input_start == NULL) {
			i = atoi(cmdbuf);
			if ((i < 1) && (i > item_cnt))
				continue;

			msg.type = RK_BT_CMD;
			msg.id = bt_command_tbs[i].cmd_id;

		} else {
			memset(cmd, 0, sizeof(cmd));
			strncpy(cmd, cmdbuf, strlen(cmdbuf) - strlen(input_start) - 1);
			i = atoi(cmd);
			if ((i < 1) && (i > item_cnt))
				continue;

			msg.type = RK_BT_CMD;
			msg.id = bt_command_tbs[i].cmd_id;
			if (msg.id == RK_BT_INIT)
				strncpy(msg.data, input_start + strlen("input") + 1, 17); //set bt name
			else
				strncpy(msg.addr, input_start + strlen("input") + 1, 17); //set bt addr
		}

		if (!rk_bt_check_server())
			return 0;

		ret = sendto(sockfd, &msg, sizeof(bt_msg_t), 0, (struct sockaddr *)&dst, sizeof(dst));
		if (ret < 0) {
			printf("%s: Socket send failed! ret = %d\n", PRINT_FLAG_ERR, ret);
			goto OUT;
		}
	}

OUT:
	if(sockfd)
		close(sockfd);
	return ret;
}
