/*
 *  pcbatest_server.h
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Bin Yang <yangbin@rock-chips.com>
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

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__


#define TCP_QUEUE_LINE 10
#define SOURCE_PORT 6666
#define RECV_BUFFER_SIZE 200
#define SEND_BUFFER_SIZE 200

typedef enum _PCBA_COMMAND_FORMAT {
	INDEX_TYPE = 0,
	INDEX_TEST_ITEM = 1,
	INDEX_CMD = 2,
	INDEX_RES = 2,
	INDEX_MSG = 3,
	INDEX_STATUS = 4,
	INDEX_RESULT = 5,
	INDEX_ERRCODE = 6,
}PCBA_COMMAND_FORMAT;

typedef struct _PCBA_SINGLE_PARA {
	char name[COMMAND_NAMESIZE];
	char valuestr[COMMAND_VALUESIZE];
	int value;
	bool opt;
} PCBA_SINGLE_PARA;

PCBA_SINGLE_PARA recv_cmd_target[] = {
	[INDEX_TYPE] = {
		.name = "TYPE",
		.opt = false,
	},
	[INDEX_TEST_ITEM] = {
		.name = "TEST_ITEM",
		.opt = true,
	},
	[INDEX_CMD] = {
		.name = "CMD",
		.opt = false,
	},
	[INDEX_MSG] = {
		.name = "MSG",
		.opt = true,
	},
};

PCBA_SINGLE_PARA send_cmd_target[] = {
	[INDEX_TYPE] = {
		.name = "TYPE",
	},
	[INDEX_TEST_ITEM] = {
		.name = "TEST_ITEM",
	},
	[INDEX_RES] = {
		.name = "RES",
	},
	[INDEX_MSG] = {
		.name = "MSG",
	},
	[INDEX_STATUS] = {
		.name = "STATUS",
	},
	[INDEX_RESULT] = {
		.name = "RESULT",
	},
	[INDEX_ERRCODE] = {
		.name = "ERR_CODE",
	},
};

#define RECV_COMMAND_PARANUM (sizeof(recv_cmd_target) / sizeof(PCBA_SINGLE_PARA))
#define SEND_COMMAND_PARANUM (sizeof(send_cmd_target) / sizeof(PCBA_SINGLE_PARA))

typedef struct _PCBA_COMMAND_PARA {
	PCBA_SINGLE_PARA recv_paras[RECV_COMMAND_PARANUM];
	PCBA_SINGLE_PARA send_paras[SEND_COMMAND_PARANUM];
} PCBA_COMMAND_PARA;

typedef enum _PCBA_CMD_NUM {
	ENTER_CMD = 0,
	EXIT_CMD = 1,
	START_CMD = 2,
	STOP_CMD = 3,
	QUERY_CMD = 4,
	HEARBEAT_CMD = 5,
	CAPTURE_CMD = 6,
}PCBA_CMD_NUM;

typedef struct _PCBA_CMD_TYPE {
	char name[COMMAND_NAMESIZE];
} PCBA_CMD_TYPE;

PCBA_CMD_TYPE recv_cmd_type[] = {
	[ENTER_CMD] = {
		.name = "ENTER",
	},
	[EXIT_CMD] = {
		.name = "EXIT",
	},
	[START_CMD] = {
		.name = "START",
	},
	[STOP_CMD] = {
		.name = "STOP",
	},
	[QUERY_CMD] = {
		.name = "QUERY",
	},
	[HEARBEAT_CMD] = {
		.name = "HEARBEAT",
	},
	[CAPTURE_CMD] = {
		.name = "CAPTURE_IMAGE",
	},
};

#define RECV_CMD_NUM (sizeof(recv_cmd_type) / sizeof(PCBA_CMD_TYPE))
#define RECV_TYPE_NAME "CMD"
#define STORAGE_TESTITEM "write_storage"
#define NAK_STA "NAK"
#define ACK_STA "ACK"
#define DEV_WDT_NAME "/dev/watchdog"

#endif
