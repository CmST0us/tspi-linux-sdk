#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <linux/prctl.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <DeviceIo/Rk_wifi.h>
#include <DeviceIo/RkBtBase.h>
#include <DeviceIo/RkBle.h>

#include "rk_ble_app.h"

/* Immediate wifi Service UUID */
#define BLE_UUID_SERVICE	"0000180A-0000-1000-8000-00805F9B34FB"
#define BLE_UUID_WIFI_CHAR	"00009999-0000-1000-8000-00805F9B34FB"
#define BLE_UUID_PROXIMITY	"7B931104-1810-4CBC-94DA-875C8067F845"

#define UUID_MAX_LEN 36

typedef enum {
	RK_BLE_WIFI_State_IDLE = 0,
	RK_BLE_WIFI_State_CONNECTTING,
	RK_BLE_WIFI_State_SUCCESS,
	RK_BLE_WIFI_State_FAIL,
	RK_BLE_WIFI_State_WRONGKEY_FAIL,
	RK_BLE_WIFI_State_DISCONNECT
} RK_BLE_WIFI_State_e;

const char *MSG_BLE_WIFI_LIST_FORMAT = "{\"cmd\":\"wifilists\", \"ret\":%s}";

static unsigned char rk_wifi_list_buf[20 * 1024];
static int scanr_len = 0, scanr_len_use = 0;

static pthread_t wificonfig_tid = 0;
static pthread_t wificonfig_scan_tid = 0;

static char wifi_ssid[256];
static char wifi_password[256];

static unsigned int ble_mtu = 0;

struct wifi_config {
	char ssid[512];
	int ssid_len;
	char psk[512];
	int psk_len;
	char key_mgmt[512];
	int key_len;
	bool hide;
	void (*wifi_status_callback)(int status, int reason);
};

static struct wifi_config wifi_cfg;
typedef int (*RK_blewifi_state_callback)(RK_BLE_WIFI_State_e state);

#define RK_BLE_DATA_CMD_LEN      12
#define RK_BLE_DATA_SSID_LEN     32
#define RK_BLE_DATA_PSK_LEN      32

/* 消息开头必须为0x01，结尾必须为0x04 */
typedef struct {
	unsigned char start;               // 段， 01Byte， 固定为 0x01
	char cmd[RK_BLE_DATA_CMD_LEN];     // 段， 12Byte， 值为："wifisetup"、"wifilists"
	char ssid[RK_BLE_DATA_SSID_LEN];   // 段， 32Byte， 需要连接的 WiFi ssid， 长度不足 32 的部分填充 0
	char psk[RK_BLE_DATA_PSK_LEN];     // 段， 32Byte， 需要连接的 wifi password， 长度不足 32 的部分填充 0
	//unsigned char end;               // 段， 01Byte， 固定位 0x04
} RockChipBleData;

typedef struct {
	uint8_t data[6];
} mac_t;

typedef struct {
	uint8_t data[16];
} uuid128_t;

void _rk_ble_status_cb(const char *bd_addr, const char *name, RK_BLE_STATE state)
{
	switch (state) {
		case RK_BLE_STATE_IDLE:
			printf("[RK] ble status: RK_BLE_STATE_IDLE\n");
			break;
		case RK_BLE_STATE_CONNECT:
			printf("[RK] ble status: RK_BLE_STATE_CONNECT\n");
			break;
		case RK_BLE_STATE_DISCONNECT:
			printf("[RK] ble status: RK_BLE_STATE_DISCONNECT\n");
			ble_mtu = 0;
			break;
	}
}

static void _rk_ble_mtu_callback(const char *bd_addr, unsigned int mtu)
{
	printf("=== %s: bd_addr: %s, mtu: %d ===\n", __func__, bd_addr, mtu);
	ble_mtu = mtu;
}

static int rk_blewifi_state_callback(RK_WIFI_RUNNING_State_e state, RK_WIFI_INFO_Connection_s *info)
{
	uint8_t ret = 0;

	printf("[RK] %s state: %d\n", __func__, state);
	switch(state) {
		case RK_WIFI_State_CONNECTED:
			ret = 0x1;
			break;
		case RK_WIFI_State_CONNECTFAILED:
		case RK_WIFI_State_CONNECTFAILED_WRONG_KEY:
			ret = 0x2;
			break;
	}

	if(ret)
		rk_ble_write(BLE_UUID_WIFI_CHAR, &ret, 0x1);

	return 0;
}

static void *rk_config_wifi_thread(void)
{
	printf("[RK] rk_config_wifi_thread\n");

	prctl(PR_SET_NAME,"rk_config_wifi_thread");

	RK_wifi_register_callback(rk_blewifi_state_callback);
	RK_wifi_enable(0);
	RK_wifi_enable(1);
	RK_wifi_connect(wifi_cfg.ssid, wifi_cfg.psk);
	return NULL;
}

static void rk_ble_send_data(void)
{
	int len, send_max_len = BT_ATT_DEFAULT_LE_MTU;
	uint8_t *data;

	if (scanr_len == 0) {
		scanr_len_use = 0;
		printf("[RK] NO WIFI SCAN_R OR READ END!!!\n");
		return;
	}

	prctl(PR_SET_NAME,"rk_ble_send_data");

	if(ble_mtu > BT_ATT_HEADER_LEN)
		send_max_len = ble_mtu;

	send_max_len -= BT_ATT_HEADER_LEN;
	if(send_max_len > BT_ATT_MAX_VALUE_LEN)
		send_max_len = BT_ATT_MAX_VALUE_LEN;

	while (scanr_len) {
		printf("[RK] %s: wifi use: %d, remain len: %d\n", __func__, scanr_len_use, scanr_len);
		len = (scanr_len > send_max_len) ? send_max_len : scanr_len;
		data = rk_wifi_list_buf + scanr_len_use;
		usleep(100000);
		rk_ble_write(BLE_UUID_WIFI_CHAR, data, len);
		scanr_len -= len;
		scanr_len_use += len;
	}
}

void _rk_ble_recv_data_cb(const char *uuid, char *data, int len)
{
	if (!strcmp(uuid, BLE_UUID_WIFI_CHAR)) {
		uint8_t str[512];
		RockChipBleData *ble_data;
		char *wifilist;
		unsigned char end_flag = 0;

		memset(str, 0, 512);
		memcpy(str, data, len);
		str[len] = '\0';
		end_flag = str[len - 1];

		printf("[RK] chr_write_value: %p, %d, data: %s\n", data, len, data);

		for (int i = 0; i < len; i++) {
			if (!( i % 8))
				printf("\n");
			printf("0x%02x ", str[i]);
		}
		printf("\n");

		ble_data = (RockChipBleData *)str;

		/* Msg is valid? */
		printf("[RK] ble_data.cmd: %s, ble_data.start: %d, ble_data.end: %d\n",
			ble_data->cmd, ble_data->start, end_flag);
		if ((ble_data->start != 0x1) || (end_flag != 0x4)) {
			printf("[RK] BLE RECV DATA ERROR !!!\n");
			return;
		}

		if (strncmp(ble_data->cmd, "wifilists", 9) == 0) {
scan_retry:
			printf("[RK] RK_wifi_scan ...\n");
			RK_wifi_scan();
			usleep(800000);

			printf("[RK] RK_wifi_scan_r_sec ...\n");
			wifilist = RK_wifi_scan_r_sec(0x14);
			printf("[RK] RK_wifi_scan_r_sec end wifilist: %p\n", wifilist);

			if (wifilist == NULL)
				goto scan_retry;
			if (wifilist && (strlen(wifilist) < 3)) {
				free(wifilist);
				goto scan_retry;
			}

			memset(rk_wifi_list_buf, 0, sizeof(rk_wifi_list_buf));
			snprintf(rk_wifi_list_buf, sizeof(rk_wifi_list_buf), MSG_BLE_WIFI_LIST_FORMAT, wifilist);
			scanr_len = strlen(rk_wifi_list_buf);
			scanr_len_use = 0;
			printf("[RK] wifi scan_r: %s, len: %d\n", rk_wifi_list_buf, scanr_len);

			free(wifilist);
			pthread_create(&wificonfig_scan_tid, NULL, rk_ble_send_data, NULL);
		} else if (strncmp(ble_data->cmd, "wifisetup", 9) == 0) {
			strcpy(wifi_ssid, ble_data->ssid); // str + 20);
			strcpy(wifi_password, ble_data->psk); // str + 52);

			printf("[RK] wifi ssid is %s\n", wifi_ssid);
			printf("[RK] wifi psk is %s\n", wifi_password);

			strcpy(wifi_cfg.ssid, wifi_ssid);
			strcpy(wifi_cfg.psk, wifi_password);
			//wifi_cfg.wifi_status_callback = wifi_status_callback;
			pthread_create(&wificonfig_tid, NULL, rk_config_wifi_thread, NULL);
		}
	}
}

void _rk_ble_request_data_cb(const char *uuid)
{
	if (!strcmp(uuid, BLE_UUID_WIFI_CHAR)) {
		printf("Call ble wifi request callback!\n");
	}
}

void rk_ble_wifi_init(void *data)
{
	RkBtContent bt_content;

	printf("===== %s =====\n", __func__);

	//MUST TO SET 0
	memset(&bt_content, 0, sizeof(RkBtContent));
	bt_content.bt_name = strdup("RockChip");
	bt_content.ble_content.ble_name = strdup("RockChipBle");
	bt_content.ble_content.server_uuid.uuid = BLE_UUID_SERVICE;
	bt_content.ble_content.server_uuid.len = UUID_128;
	bt_content.ble_content.chr_uuid[0].uuid = BLE_UUID_WIFI_CHAR;
	bt_content.ble_content.chr_uuid[0].len = UUID_128;
	bt_content.ble_content.chr_cnt = 1;
	bt_content.ble_content.cb_ble_recv_fun = _rk_ble_recv_data_cb;
	bt_content.ble_content.cb_ble_request_data = _rk_ble_request_data_cb;
	bt_content.ble_content.advDataType = BLE_ADVDATA_TYPE_SYSTEM;

	//bsa ble must register data recv callback, can't delete
	rk_ble_register_recv_callback(_rk_ble_recv_data_cb);
	rk_ble_register_status_callback(_rk_ble_status_cb);
	rk_bt_init(&bt_content);

	sleep(3);
	printf(">>>>> Start ble ....\n");
	rk_ble_register_mtu_callback(_rk_ble_mtu_callback);
	rk_ble_start(&bt_content.ble_content);
}

void rk_ble_wifi_deinit(void *data)
{
	ble_mtu = 0;
	rk_ble_stop();
	sleep(3);
	rk_bt_deinit();
}
