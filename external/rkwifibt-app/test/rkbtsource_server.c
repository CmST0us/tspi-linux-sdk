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

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <errno.h>

#include <RkBtBase.h>
#include <RkBtSource.h>
#include <RkBtSpp.h>
#include <RkBle.h>
#include <slog.h>

#include "rkbtsource_common.h"

/* Immediate wifi Service UUID */
#define BLE_UUID_DEVINFO    "0000180A-0000-1000-8000-00805F9B34FB"
#define BLE_UUID_BATTERY    "00002A19-0000-1000-8000-00805F9B34FB"
#define BLE_UUID_STRING     "00002A3D-0000-1000-8000-00805F9B34FB"

#define SCAN_DEVICES_SAVE_COUNT 30

static bool bleconnect = false;

enum {
    RK_BT_IDEL,
    RK_BT_INITING,
    RK_BT_INITED,
    RK_BT_DEINITING,
    RK_BT_DEINITED,
    RK_BT_CONNECTING,
    RK_BT_CONNECTED,
    RK_BT_DISCONNECTING,
    RK_BT_DISCONNECTED,
};

typedef struct {
    int init_state;
    int deinit_state;
    int connect_state;
    int disconnect_state;
} bt_state_t;

static int sockfd = 0;

static int scan_devices_count = 0;
static bool is_recovery_process = false;
static bt_state_t bt_state;
static int scan_count = 0;
static int connect_device_cache_flag = 0;
static int user_connect_device_flag = 0;

int _console_run(char *cmd , char *reply)
{
    FILE   *stream;
    int len = 0;
    char buf[4096];

    if(cmd == NULL || strlen(cmd) == 0)
    {
        pr_info("cmd invalid\n");
        return -1;
    }

    if(cmd[strlen(cmd)-1] == '&') //run cmd in background
    {
        system(cmd);
        if(reply)strcpy(reply , "\0");
    }
    else
    {
        stream = popen( cmd , "r" );
        if(stream)
        {
            len = fread( buf, sizeof(char), sizeof(buf), stream);
            if(len > 0)
            {
                if(buf[len-1] == '\n')
                {
                    buf[len-1] = '\0';
                }
            }
            pclose(stream);
        }
        buf[len] = '\0';
        if(reply)strcpy(reply , buf);

        time_t ts = time(NULL);
        pr_info("[_console_run] %ld cmd %s  resp :%s\n" , ts , cmd , buf);
    }

    return 0;
}

static int rk_bt_send_custom_evt(bt_msg_t *evt)
{
	struct sockaddr_un addr;
	struct msghdr msg;
	struct iovec iov[1];

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, RKBTSOURCE_CLIENT_SOCKET_PATH);

	iov[0].iov_base = evt;
	iov[0].iov_len  = sizeof(bt_msg_t);

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &addr,
	msg.msg_namelen = sizeof(addr),
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if (sendmsg(sockfd, &msg, 0) < 0)
		pr_info("[BT SPP] send fd fail %s\n", strerror(errno));

	return 0;
}

static int rk_bt_send_evt(enum rk_bt_evt val)
{
	struct sockaddr_un addr;
	bt_msg_t evt;
	struct msghdr msg;
	struct iovec iov[1];

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, RKBTSOURCE_CLIENT_SOCKET_PATH);

	memset(&evt, 0, sizeof(bt_msg_t));
	evt.type = RK_BT_EVT;
	evt.id = val;

	iov[0].iov_base = &evt;
	iov[0].iov_len  = sizeof(bt_msg_t);

	memset(&msg, 0, sizeof(msg));

	msg.msg_name = &addr,
	msg.msg_namelen = sizeof(addr),
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if (sendmsg(sockfd, &msg, 0) < 0)
		pr_info("[BT SPP] send fd fail %s\n", strerror(errno));
}

static void rk_bt_send_spp_client_fd(void)
{
	struct sockaddr_un addr;
	bt_msg_t evt;
	struct iovec iov[1];
	union {
		char buf[CMSG_SPACE(sizeof(int))];
		struct cmsghdr _align;
	} control_un;

	memset(&evt, 0, sizeof(bt_msg_t));
	evt.type = RK_BT_EVT;
	evt.id = RK_BT_EVT_SPP_CONNECTED;

	iov[0].iov_base = &evt;
	iov[0].iov_len  = sizeof(bt_msg_t);

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, RKBTSOURCE_CLIENT_SOCKET_PATH);

	struct msghdr msg = {
		.msg_name = &addr,
		.msg_namelen = sizeof(addr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = control_un.buf,
		.msg_controllen = sizeof(control_un.buf),
	};

	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	int *fdptr = (int *)CMSG_DATA(cmsg);
	*fdptr = _rk_bt_spp_get_client_fd();
	pr_info("[BT SPP] send fd %d\n", _rk_bt_spp_get_client_fd());

	if (sendmsg(sockfd, &msg, 0) == -1)
		pr_info("[BT SPP] send fd fail %s\n", strerror(errno));
}

static void rk_bt_state_cb(RK_BT_STATE state)
{
	switch(state) {
	case RK_BT_STATE_TURNING_ON:
		pr_info("%s: RK_BT_STATE_TURNING_ON\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case RK_BT_STATE_ON:
		pr_info("%s: RK_BT_STATE_ON\n", PRINT_FLAG_RKBTSOURCE);
		rk_bt_send_evt(RK_BT_EVT_INIT_OK);
		break;
	case RK_BT_STATE_TURNING_OFF:
		pr_info("%s: RK_BT_STATE_TURNING_OFF\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case RK_BT_STATE_OFF:
		pr_info("%s: RK_BT_STATE_OFF\n", PRINT_FLAG_RKBTSOURCE);
		rk_bt_send_evt(RK_BT_EVT_DEINIT_OK);
		break;
	}
}

static void rk_bt_bond_state_cb(const char *bd_addr, const char *name, RK_BT_BOND_STATE state)
{
	switch(state) {
	case RK_BT_BOND_STATE_NONE:
		pr_info("%s: BT BOND NONE: %s, %s\n", PRINT_FLAG_RKBTSOURCE, name, bd_addr);
		break;
	case RK_BT_BOND_STATE_BONDING:
		pr_info("%s: BT BOND BONDING: %s, %s\n", PRINT_FLAG_RKBTSOURCE, name, bd_addr);
		break;
	case RK_BT_BOND_STATE_BONDED:
		pr_info("%s: BT BONDED: %s, %s\n", PRINT_FLAG_RKBTSOURCE, name, bd_addr);
		break;
	}
}

static void rk_ble_status_cb(const char *bd_addr, const char *name, RK_BLE_STATE state)
{
	switch (state) {
	case RK_BLE_STATE_IDLE:
		pr_info("+++++ RK_BLE_STATE_IDLE +++++\n");
		break;
	case RK_BLE_STATE_CONNECT:
		pr_info("+++++ RK_BLE_STATE_CONNECT: %s, %s +++++\n", name, bd_addr);
		bleconnect = true;
		break;
	case RK_BLE_STATE_DISCONNECT:
		pr_info("+++++ RK_BLE_STATE_DISCONNECT: %s, %s +++++\n", name, bd_addr);
		bleconnect = false;
		break;
	}
}

static void rk_bt_discovery_status_cb(RK_BT_DISCOVERY_STATE status)
{
	switch(status) {
	case RK_BT_DISC_STARTED:
		pr_info("%s: RK_BT_DISC_STARTED\n", PRINT_FLAG_RKBTSOURCE);
		rk_bt_send_evt(RK_BT_EVT_SCAN_ON);
		break;
	case RK_BT_DISC_START_FAILED:
		pr_info("%s: RK_BT_DISC_START_FAILED\n", PRINT_FLAG_RKBTSOURCE);
		rk_bt_send_evt(RK_BT_EVT_SCAN_OFF);
		break;
	case RK_BT_DISC_STOPPED_BY_USER:
		pr_info("%s: RK_BT_DISC_STOPPED_BY_USER\n", PRINT_FLAG_RKBTSOURCE);
		rk_bt_send_evt(RK_BT_EVT_SCAN_OFF);
		break;
	}
}

static void rk_bt_send_devinfo_evt(enum rk_bt_evt val, const char *name, const char *address)
{
	int rc;
	/* INVALID = 0, SOURCE = 1, SINK = 2 */
	int i, name_len, scan_msg_len = 0, scan_device_exist = 0;
	char buff[2048];
	char *offset;
	bt_msg_t evt;

	evt.type = RK_BT_EVT;
	evt.id = val;
	evt.dev.role = rk_bt_get_playrole_by_addr(address);

	memcpy(evt.dev.addr, address, 17);
	evt.dev.addr[17] = '\0';

	name_len = strlen(name);
	memcpy(evt.dev.name, name, name_len);
	evt.dev.name[name_len] = '\0';

	pr_info("%s: device is %s\n", PRINT_FLAG_RKBTSOURCE,
		val == RK_BT_EVT_SOURCE_CONNECTED ? "connected" : "disconnected");
	pr_info("        address: %s\n", address);
	pr_info("        name: %s\n", name);

	rk_bt_send_custom_evt(&evt);
}

static void rk_bt_dev_found_cb(const char *address, const char *name, unsigned int bt_class, int rssi, int change)
{
	int rc;
	/* INVALID = 0, SOURCE = 1, SINK = 2 */
	int i, name_len, scan_msg_len = 0, scan_device_exist = 0;
	char buff[2048];
	char *offset;
	bt_msg_t evt;

	if (change)
		evt.id = RK_BT_EVT_SCAN_ADD_DEV;
	else
		evt.id = RK_BT_EVT_SCAN_REMOVE_DEV;

	if (change == 2)
		evt.id = RK_BT_EVT_SCAN_CHANGE_DEV;
	
	evt.dev.role = rk_bt_get_playrole_by_addr(address);
	evt.dev.change = 1;
	evt.dev.rssi = rssi;

	memcpy(evt.dev.addr, address, 17);
	evt.dev.addr[17] = '\0';

	name_len = strlen(name);
	memcpy(evt.dev.name, name, name_len);
	evt.dev.name[name_len] = '\0';

	evt.dev.change = change;

	pr_info("%s: device is found\n", PRINT_FLAG_RKBTSOURCE);
	pr_info("        address: %s\n", address);
	pr_info("        name: %s\n", name);
	pr_info("        class: 0x%x\n", bt_class);
	pr_info("        rssi: %d\n", rssi);

	rk_bt_send_custom_evt(&evt);
}

static void bt_test_get_paired_devices(void)
{
	int i, count;
	RkBtScanedDevice *dev_tmp = NULL;

	rk_bt_get_paired_devices(&dev_tmp, &count);

	pr_info("%s: current paired devices count: %d\n", __func__, count);
	for(i = 0; i < count; i++) {

		bt_msg_t evt;
		int name_len;

		evt.type = RK_BT_EVT;
		evt.id = RK_BT_EVT_PAIRED_DEV;
		evt.dev.role = rk_bt_get_playrole_by_addr(dev_tmp->remote_address);

		memcpy(evt.dev.addr, dev_tmp->remote_address, 17);
		evt.dev.addr[17] = '\0';

		name_len = strlen(dev_tmp->remote_name);
		memcpy(evt.dev.name, dev_tmp->remote_name, name_len);
		evt.dev.name[name_len] = '\0';
		evt.dev.is_connected = dev_tmp->is_connected;

		pr_info("device %d\n", i);
		pr_info("	remote_address: %s\n", dev_tmp->remote_address);
		pr_info("	remote_name: %s\n", dev_tmp->remote_name);
		pr_info("	is_connected: %d\n", dev_tmp->is_connected);
		dev_tmp = dev_tmp->next;
		rk_bt_send_custom_evt(&evt);
	}
	rk_bt_free_paired_devices(dev_tmp);
}

void rk_bt_source_status_callback(void *userdata, const char *remote_addr,
		const char *remote_name, const RK_BT_SOURCE_EVENT enEvent)
{
	char address[18] = {0}, name[100] = {0};
	char cmd[128];

	switch(enEvent) {
	case BT_SOURCE_EVENT_CONNECT_FAILED:
		bt_state.connect_state = RK_BT_IDEL;
		rk_bt_send_evt(RK_BT_EVT_SOURCE_CONNECT_FAILED);
		pr_info("%s: BT_SOURCE_EVENT_CONNECT_FAILED, %s, %s\n", PRINT_FLAG_RKBTSOURCE, remote_name, remote_addr);
		break;
	case BT_SOURCE_EVENT_CONNECTED:
	{
		bt_state.connect_state = RK_BT_CONNECTED;
		bt_state.disconnect_state = RK_BT_IDEL;
		pr_info("%s: BT_SOURCE_EVENT_CONNECTED, %s, %s\n", PRINT_FLAG_RKBTSOURCE, remote_name, remote_addr);
		rk_bt_send_devinfo_evt(RK_BT_EVT_SOURCE_CONNECTED, remote_name, remote_addr);
		break;
	}
	case BT_SOURCE_EVENT_DISCONNECTED:
	{
		bt_state.connect_state = RK_BT_IDEL;
		bt_state.disconnect_state = RK_BT_DISCONNECTED;
		rk_bt_send_devinfo_evt(RK_BT_EVT_SOURCE_DISCONNECTED, remote_name, remote_addr);
		pr_info("%s: BT_SOURCE_EVENT_DISCONNECTED, %s, %s\n", PRINT_FLAG_RKBTSOURCE, remote_name, remote_addr);
		break;
	}
	case BT_SOURCE_EVENT_RC_PLAY:
		pr_info("%s: BT_SOURCE_EVENT_RC_PLAY\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_RC_STOP:
		pr_info("%s: BT_SOURCE_EVENT_RC_STOP\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_RC_PAUSE:
		pr_info("%s: BT_SOURCE_EVENT_RC_PAUSE\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_RC_FORWARD:
		pr_info("%s: BT_SOURCE_EVENT_RC_FORWARD\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_RC_BACKWARD:
		pr_info("%s: BT_SOURCE_EVENT_RC_BACKWARD\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_RC_VOL_UP:
		pr_info("%s: BT_SOURCE_EVENT_RC_VOL_UP\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_RC_VOL_DOWN:
		pr_info("%s: BT_SOURCE_EVENT_RC_VOL_DOWN\n", PRINT_FLAG_RKBTSOURCE);
		break;
	case BT_SOURCE_EVENT_REMOVED:
		bt_state.connect_state = RK_BT_IDEL;
		bt_state.disconnect_state = RK_BT_IDEL;
		pr_info("%s: BT_SOURCE_EVENT_REMOVED\n", PRINT_FLAG_RKBTSOURCE);
		break;
	default:
		pr_info("%s: Source event: %d\n", PRINT_FLAG_RKBTSOURCE, enEvent);
		break;
	}
}

static void rk_bt_spp_status_callback(RK_BT_SPP_STATE type)
{
	switch(type) {
	case RK_BT_SPP_STATE_IDLE:
		pr_info("+++++++ RK_BT_SPP_STATE_IDLE +++++\n");
		break;
	case RK_BT_SPP_STATE_CONNECT:
		pr_info("+++++++ RK_BT_SPP_EVENT_CONNECT +++++\n");
		//rk_bt_send_evt(RK_BT_EVT_SPP_CONNECTED);
		rk_bt_send_spp_client_fd();
		break;
	case RK_BT_SPP_STATE_DISCONNECT:
		pr_info("+++++++ RK_BT_SPP_EVENT_DISCONNECT +++++\n");
		rk_bt_send_evt(RK_BT_EVT_SPP_DISCONNECTED);
		break;
	case RK_BT_SPP_STATE_CONNECT_FAILED:
		pr_info("+++++++ RK_BT_SPP_EVENT_CONNECT_FAILED +++++\n");
		rk_bt_send_evt(RK_BT_EVT_SPP_CONNECT_FAILED);
		break;
	default:
		pr_info("+++++++ BT SPP NOT SUPPORT TYPE! +++++\n");
		break;
	}
}

static void rk_ble_recv_data_cb(const char *uuid, char *data, int len)
{
	pr_info("=== %s uuid: %s===\n", __func__, uuid);
	for (int i = 0 ; i < len; i++) {
		pr_info("%02x ", data[i]);
	}
	pr_info("\n");
}

static void rk_ble_request_data_cb(const char *uuid)
{
	pr_info("=== %s: uuid = %s ===\n", __func__, uuid);

	if (strcmp(uuid, BLE_UUID_BATTERY) == 0) {
		rk_ble_write(BLE_UUID_BATTERY, "Hello Rockchip", strlen("Hello Rockchip"));
	}
}

static void rk_ble_mtu_cb(const char *bd_addr, unsigned int mtu)
{
	pr_info("=== %s: bd_addr: %s, mtu: %d ===\n", __func__, bd_addr, mtu);
}

static char name[128];
static RkBtContent bt_content;

static int rk_bt_server_init(char *bt_name)
{
	pr_info("----- rk_bt_server_init(YDPen) -----\n");
	memset(&bt_content, 0, sizeof(bt_content));

	//sprintf(name, "YDPen:%s", "test");

	bt_content.bt_name = bt_name;
	bt_content.ble_content.ble_name = bt_name;
	bt_content.ble_content.server_uuid.uuid = BLE_UUID_DEVINFO;
	bt_content.ble_content.server_uuid.len = UUID_128;
	bt_content.ble_content.chr_uuid[0].uuid = BLE_UUID_BATTERY;
	bt_content.ble_content.chr_uuid[0].len = UUID_128;
	bt_content.ble_content.chr_uuid[1].uuid = BLE_UUID_STRING;
	bt_content.ble_content.chr_uuid[1].len = UUID_128;
	bt_content.ble_content.chr_cnt = 2;
	bt_content.ble_content.advDataType = BLE_ADVDATA_TYPE_SYSTEM;
	bt_content.ble_content.cb_ble_recv_fun = NULL;
	bt_content.ble_content.cb_ble_request_data = rk_ble_request_data_cb;

	rk_bt_register_state_callback(rk_bt_state_cb);
	rk_bt_register_bond_callback(rk_bt_bond_state_cb);
	rk_bt_register_discovery_callback(rk_bt_discovery_status_cb);
	rk_bt_register_dev_found_callback(rk_bt_dev_found_cb);

	return rk_bt_init(&bt_content);
}

static int rk_bt_server_init_ble()
{
	rk_ble_register_mtu_callback(rk_ble_mtu_cb);

	return rk_ble_start(&bt_content.ble_content);
}

static int rk_bt_server_deinit()
{
	pr_info("%s: BT SERVER DEINIT\n", PRINT_FLAG_RKBTSOURCE);
	return _rk_bt_deinit(NULL);
	//return rk_bt_deinit();
}

int main(int argc, char *argv[])
{
	int ret;
	char buff[256] = {0};

	unlink(RKBTSOURCE_SERVER_SOCKET_PATH);

	pr_info("----- rkbtsource_server -----\n");

	sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		pr_info("%s: Create socket failed!\n", PRINT_FLAG_ERR);
		return -1;
	}

	struct sockaddr_un addr;
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, RKBTSOURCE_SERVER_SOCKET_PATH);
	ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		pr_info("%s: Bind Local addr failed!\n", PRINT_FLAG_ERR);
		goto fail;
	}

	struct timeval t = {4, 0};
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof(t));

	memset(&bt_state, 0, sizeof(bt_state_t));

	while (1) {
		memset(buff, 0, sizeof(buff));
		ret = recvfrom(sockfd, buff, sizeof(buff), 0, NULL, NULL);
		if (ret <= 0)
			continue;

		bt_msg_t *msg = (bt_msg_t *)buff;
		pr_info("recv cmd: type: %d, id: %d\n", msg->type, msg->id);

		if (msg->type != RK_BT_CMD)
			continue;

		switch (msg->id) {
		case RK_BT_INIT:
			if (bt_state.init_state == RK_BT_IDEL &&
				bt_state.deinit_state != RK_BT_DEINITING) {

				bt_state.init_state = RK_BT_INITING;

				if (rk_bt_server_init(msg->data) < 0) {
					pr_info("%s: bt server init failed!\n", PRINT_FLAG_ERR);
					goto fail;
				}

				/*
				if (rk_bt_server_init_ble() < 0) {
					pr_info("%s: ble init failed!\n", PRINT_FLAG_ERR);
					goto fail;
				}
				*/

				bt_state.init_state = RK_BT_INITED;
				bt_state.deinit_state = RK_BT_IDEL;
				pr_info("%s: bt server init sucessful!\n", PRINT_FLAG_SUCESS);
			} else {
				pr_info("%s: [INIT] init_state = %d, deinit_state = %d\n", PRINT_FLAG_RKBTSOURCE, bt_state.init_state, bt_state.deinit_state);
			}
			break;

		case RK_BT_SOURCE_OPEN:
			exec_command_system("hciconfig hci0 piscan");
			rk_bt_source_register_status_cb(NULL, rk_bt_source_status_callback);
			if (rk_bt_source_open() < 0) {
				pr_info("%s: bt source open failed!\n", PRINT_FLAG_ERR);
				goto fail;
			}
			rk_bt_send_evt(RK_BT_EVT_SOURCE_OPEN);
			break;
		case RK_BT_SOURCE_CLOSE:
			//rk_bt_source_register_status_cb(NULL, rk_bt_source_status_callback);
			if (rk_bt_source_close() < 0) {
				pr_info("%s: bt source close failed!\n", PRINT_FLAG_ERR);
				goto fail;
			}
			rk_bt_send_evt(RK_BT_EVT_SOURCE_CLOSE);
			break;
		case RK_BT_SOURCE_CONNECT:
			if (is_recovery_process) {
				pr_info("%s: [CONNECT] recovering process\n", PRINT_FLAG_RKBTSOURCE);
				break;
			}

			if (bt_state.init_state != RK_BT_INITED) {
				pr_info("%s: [CONNECT] bt don't init, init_state = %d\n", PRINT_FLAG_ERR, bt_state.init_state);
				break;
			}

			if (bt_state.connect_state == RK_BT_IDEL &&
				bt_state.disconnect_state != RK_BT_DISCONNECTING) {

				bt_state.connect_state = RK_BT_CONNECTING;
				user_connect_device_flag = 1;
				if (rk_bt_source_connect_by_addr(msg->addr) < 0) {
					bt_state.connect_state = RK_BT_IDEL;
					user_connect_device_flag = 0;
					pr_info("%s: rk_bt_source_connect %s failed!\n", PRINT_FLAG_ERR, msg->addr);
				}
			} else {
				pr_info("%s: [CONNECT] connect_state = %d, disconnect_state = %d\n", PRINT_FLAG_RKBTSOURCE, bt_state.connect_state, bt_state.disconnect_state);
			}
			break;

		case RK_BT_SCAN_ON:
			if (is_recovery_process) {
				pr_info("%s: [SCAN_ON] recovering process\n", PRINT_FLAG_RKBTSOURCE);
				rk_bt_send_evt(RK_BT_EVT_SCANNING);
				break;
			}

			if (bt_state.init_state != RK_BT_INITED) {
				pr_info("%s: [SCAN_ON] bt don't init, init_state = %d\n", PRINT_FLAG_ERR, bt_state.init_state);
				rk_bt_send_evt(RK_BT_EVT_SCAN_OFF);
				break;
			}

			/* Scan bluetooth devices, 10s for default */
			if (rk_bt_start_discovery(10000, SCAN_TYPE_BREDR) < 0)
			{
				pr_info("%s: rk_bt_start_discovery failed\n", PRINT_FLAG_ERR);
				rk_bt_send_evt(RK_BT_EVT_SCAN_OFF);
			}
			else
			{
				//BT_CMD_SCAN_START
			}
			break;

		case RK_BT_SCAN_OFF:
			if (is_recovery_process) {
				pr_info("%s: [SCAN_OFF] recovering process\n", PRINT_FLAG_RKBTSOURCE);
				break;
			}

			if (bt_state.init_state != RK_BT_INITED) {
				pr_info("%s: [SCAN_OFF] bt don't init, init_state = %d\n", PRINT_FLAG_ERR, bt_state.init_state);
				break;
			}

			rk_bt_cancel_discovery();
			break;

		case RK_BT_SOURCE_DISCONNECT:
			if (is_recovery_process) {
				pr_info("%s: [DISCONNECT] recovering process\n", PRINT_FLAG_RKBTSOURCE);
				break;
			}

			if (bt_state.init_state != RK_BT_INITED) {
				pr_info("%s: [DISCONNECT] bt don't init, init_state = %d\n", PRINT_FLAG_ERR, bt_state.init_state);
				break;
			}

			if (bt_state.disconnect_state == RK_BT_IDEL && bt_state.connect_state == RK_BT_CONNECTED) {
				bt_state.disconnect_state = RK_BT_DISCONNECTING;
				user_connect_device_flag = 0;
				if (rk_bt_source_disconnect_by_addr(msg->addr) < 0) {
					bt_state.disconnect_state = RK_BT_IDEL;
					user_connect_device_flag = 1;
					pr_info("%s: rk_bt_source_disconnect failed!\n", PRINT_FLAG_ERR);
				}
			} else {
				pr_info("%s: [DISCONNECT] connect_state = %d, disconnect_state = %d\n", PRINT_FLAG_RKBTSOURCE, bt_state.connect_state, bt_state.disconnect_state);
			}
			break;

		case RK_BT_SOURCE_REMOVE:
			if (is_recovery_process) {
				pr_info("%s: [REMOVE] recovering process\n", PRINT_FLAG_RKBTSOURCE);
				break;
			}

			if (bt_state.init_state != RK_BT_INITED) {
				pr_info("%s: [REMOVE] bt don't init, init_state = %d\n", PRINT_FLAG_ERR, bt_state.init_state);
				break;
			}

			if (rk_bt_source_remove(msg->addr) < 0)
				pr_info("%s: remove failed!\n", PRINT_FLAG_ERR);
			else
				pr_info("%s: remove sucess!\n", PRINT_FLAG_SUCESS);
			break;
		case RK_BT_SPP_LISTEN:
			rk_bt_spp_open(NULL);
			rk_bt_spp_register_status_cb(rk_bt_spp_status_callback);
			break;
		case RK_BT_SPP_CONNECT:
			rk_bt_spp_connect(msg->addr);
			rk_bt_spp_register_status_cb(rk_bt_spp_status_callback);
			break;
		case RK_BT_SPP_CLOSE:
			rk_bt_spp_close();
			rk_bt_spp_register_status_cb(NULL);
			break;
		case RK_BT_DEINIT:
			if(bt_state.init_state == RK_BT_INITED && bt_state.deinit_state == RK_BT_IDEL) {
				bt_state.deinit_state = RK_BT_DEINITING;

				rk_bt_server_deinit();
				rk_bt_send_evt(RK_BT_EVT_DEINIT_OK);
				if(sockfd) {
					close(sockfd);
					pr_info("%s: close server socket\n", PRINT_FLAG_RKBTSOURCE);
					sockfd = 0;
				}

				bt_state.deinit_state = RK_BT_DEINITED;
				bt_state.init_state = RK_BT_IDEL;
				pr_info("%s:deinit bt server sucess!\n", PRINT_FLAG_SUCESS);
				return 0;
			} else {
				pr_info("%s: [DEINIT] init_state = %d, deinit_state = %d\n", PRINT_FLAG_RKBTSOURCE, bt_state.init_state, bt_state.deinit_state);
			}
			break;
		case RK_BT_GET_PAIRED_DEVICES:
			bt_test_get_paired_devices();
			break;
		case RK_BT_PING:
			rk_bt_send_evt(RK_BT_EVT_PONG);
			break;
		default:
			break;
		}
	}

fail:
	pr_info("%s: rkbtsource_server failed and exit\n", PRINT_FLAG_ERR);
	if(sockfd) {
		close(sockfd);
		pr_info("%s: close server socket\n", PRINT_FLAG_RKBTSOURCE);
		sockfd = 0;
	}

	rk_bt_server_deinit();
	return -1;
}
