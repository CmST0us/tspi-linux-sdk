#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <alsa/asoundlib.h>
#include <pthread.h>

#include <DeviceIo/DeviceIo.h>
#include <DeviceIo/RkBtBase.h>
#include <DeviceIo/RkBtSink.h>
#include <DeviceIo/RkBtSource.h>
#include <DeviceIo/RkBle.h>
#include <DeviceIo/RkBtSpp.h>
#include <DeviceIo/RkBtHfp.h>
#include <DeviceIo/RkBleClient.h>
#include <DeviceIo/RkBtObex.h>
#include <DeviceIo/RkBtPan.h>

#include "bt_test.h"

/* Immediate wifi Service UUID */
#define BLE_UUID_SERVICE	"0000180A-0000-1000-8000-00805F9B34FB"
#define BLE_UUID_WIFI_CHAR	"00009999-0000-1000-8000-00805F9B34FB"
#define BLE_UUID_PROXIMITY	"7B931104-1810-4CBC-94DA-875C8067F845"
#define BLE_UUID_SEND		"dfd4416e-1810-47f7-8248-eb8be3dc47f9"
#define BLE_UUID_RECV		"9884d812-1810-4a24-94d3-b2c11a851fac"
#define SERVICE_UUID		"00001910-0000-1000-8000-00805f9b34fb"

#define HFP_PCM_CHANNEL_NB	2
#define CVSD_SAMPLE_RATE	8000
#define MSBC_SAMPLE_RATE	16000

#define READ_FRAME_256		256
#define BUFFER_SIZE_1024	1024
#define PERIOD_SIZE_256		READ_FRAME_256

#define READ_FRAME_512		512
#define BUFFER_SIZE_2048	2048
#define PERIOD_SIZE_512		READ_FRAME_512

#define READ_FRAME_1024		1024
#define BUFFER_SIZE_4096	4096
#define PERIOD_SIZE_1024	READ_FRAME_1024

static const char *alsa_playback_device = "default";
static const char *alsa_capture_device = "6mic_loopback"; //"2mic_loopback";
static const char *bt_playback_device = "hw:1,0";
static const char *bt_capture_device = "hw:1,0";

typedef struct {
	unsigned int channels;
	unsigned int sample_rate;
	snd_pcm_uframes_t period_size;
	snd_pcm_uframes_t buffer_size;
} alsa_config_t;

typedef struct {
	bool alsa_duplex_opened;
	bool bt_duplex_opened;
	pthread_t alsa_tid;
	pthread_t bt_tid;
} duplex_info_t;

static duplex_info_t g_duplex_control = {
	false,
	false,
	0,
	0,
};

static RkBtScanedDevice *g_paired_dev_list;
static unsigned int g_mtu = 0;

static void bt_test_ble_recv_data_callback(const char *uuid, char *data, int len);
static void bt_test_ble_request_data_callback(const char *uuid);

/* Must be initialized before using Bluetooth ble */
static RkBtContent bt_content;

static RK_BT_SCO_CODEC_TYPE sco_codec = BT_SCO_CODEC_CVSD;
/******************************************/
/*        BT base server init             */
/******************************************/
static volatile bool gonff = false;
static void bt_test_state_cb(RK_BT_STATE state)
{
	switch(state) {
		case RK_BT_STATE_TURNING_ON:
			printf("++++++++++ RK_BT_STATE_TURNING_ON ++++++++++\n");
			break;
		case RK_BT_STATE_ON:
			printf("++++++++++ RK_BT_STATE_ON ++++++++++\n");
			gonff = true;
			break;
		case RK_BT_STATE_TURNING_OFF:
			printf("++++++++++ RK_BT_STATE_TURNING_OFF ++++++++++\n");
			break;
		case RK_BT_STATE_OFF:
			printf("++++++++++ RK_BT_STATE_OFF ++++++++++\n");
			gonff = false;
			break;
	}
}

static void bt_test_bond_state_cb(const char *bd_addr, const char *name, RK_BT_BOND_STATE state)
{
	switch(state) {
		case RK_BT_BOND_STATE_NONE:
			printf("++++++++++ BT BOND NONE: %s, %s ++++++++++\n", name, bd_addr);
			break;
		case RK_BT_BOND_STATE_BONDING:
			printf("++++++++++ BT BOND BONDING: %s, %s ++++++++++\n", name, bd_addr);
			break;
		case RK_BT_BOND_STATE_BONDED:
			printf("++++++++++ BT BONDED: %s, %s ++++++++++\n", name, bd_addr);
			break;
	}
}

static void bt_test_discovery_status_cb(RK_BT_DISCOVERY_STATE status)
{
	switch(status) {
		case RK_BT_DISC_STARTED:
			printf("++++++++++ RK_BT_DISC_STARTED ++++++++++\n");
			break;
		case RK_BT_DISC_STOPPED_AUTO:
			printf("++++++++++ RK_BT_DISC_STOPPED_AUTO ++++++++++\n");
			break;
		case RK_BT_DISC_START_FAILED:
			printf("++++++++++ RK_BT_DISC_START_FAILED ++++++++++\n");
			break;
		case RK_BT_DISC_STOPPED_BY_USER:
			printf("++++++++++ RK_BT_DISC_STOPPED_BY_USER ++++++++++\n");
			break;
	}

	if(status != RK_BT_DISC_STARTED) {
		printf("+++++++++ deregister scan callback +++++++++\n");
		rk_bt_register_discovery_callback(NULL);
		rk_bt_register_dev_found_callback(NULL);
	}
}

static void bt_test_dev_found_cb(const char *address,const char *name, unsigned int bt_class, int rssi)
{
	printf("++++++++++++ Device is found ++++++++++++\n");
	printf("    address: %s\n", address);
	printf("    name: %s\n", name);
	printf("    class: 0x%x\n", bt_class);
	printf("    rssi: %d\n", rssi);
	printf("+++++++++++++++++++++++++++++++++++++++++\n");
}

static void bt_test_name_change_cb(const char *bd_addr, const char *name)
{
	if(bd_addr)
		printf("+++++ Device Name Change(%s, %s) +++++\n", bd_addr, name);
}


/*
 * The Bluetooth basic service is turned on and the function
 * must be called before using the Bluetooth function.
 */
#if 0
void *bt_test_bluetooth_init_thread(void *arg)
{
	printf("%s: BT BLUETOOTH INIT\n", __func__);
	memset(&bt_content, 0, sizeof(RkBtContent));
	bt_content.bt_name = "ROCKCHIP_TEST";
	//bt_content.bt_addr = "11:22:33:44:55:66";

	bt_content.ble_content.ble_name = "ROCKCHIP_AUDIO BLE";

#if 0
	//user-defined ble address test, if not set, the default random address
	bt_content.ble_content.ble_addr[0] = 0x11;
	bt_content.ble_content.ble_addr[1] = 0x22;
	bt_content.ble_content.ble_addr[2] = 0x33;
	bt_content.ble_content.ble_addr[3] = 0x44;
	bt_content.ble_content.ble_addr[4] = 0x55;
	bt_content.ble_content.ble_addr[5] = 0x66;
	//Clear two most significant bits
	bt_content.ble_content.ble_addr[5] &= 0x3f;
	//Set second most significant bit, Private resolvable
	bt_content.ble_content.ble_addr[5] |= 0x40;
	printf("ble_addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
			bt_content.ble_content.ble_addr[5], bt_content.ble_content.ble_addr[4],
			bt_content.ble_content.ble_addr[3], bt_content.ble_content.ble_addr[2],
			bt_content.ble_content.ble_addr[1], bt_content.ble_content.ble_addr[0]);
#endif

	bt_content.ble_content.server_uuid.uuid = BLE_UUID_SERVICE;
	bt_content.ble_content.server_uuid.len = UUID_128;
	bt_content.ble_content.chr_uuid[0].uuid = BLE_UUID_WIFI_CHAR;
	bt_content.ble_content.chr_uuid[0].len = UUID_128;
	bt_content.ble_content.chr_uuid[1].uuid = BLE_UUID_SEND;
	bt_content.ble_content.chr_uuid[1].len = UUID_128;
	bt_content.ble_content.chr_uuid[2].uuid = BLE_UUID_RECV;
	bt_content.ble_content.chr_uuid[2].len = UUID_128;
	bt_content.ble_content.chr_cnt = 3;
	bt_content.ble_content.advDataType = BLE_ADVDATA_TYPE_SYSTEM;
	bt_content.ble_content.cb_ble_recv_fun = bt_test_ble_recv_data_callback;
	bt_content.ble_content.cb_ble_request_data = bt_test_ble_request_data_callback;

	rk_bt_register_state_callback(bt_test_state_cb);
	rk_bt_register_bond_callback(bt_test_bond_state_cb);
	rk_bt_register_name_change_callback(bt_test_name_change_cb);
	rk_bt_init(&bt_content);

	return NULL;
}
#else
//BLE_ADVDATA_TYPE_USER demo
void *bt_test_bluetooth_init_thread(void *arg)
{
	printf("%s: BT BLUETOOTH INIT\n", __func__);

	int len, ble_name_len, remain_len;

	memset(&bt_content, 0, sizeof(RkBtContent));
	bt_content.bt_name = "ROCKCHIP_AUDIO";
	//bt_content.bt_addr = "11:22:33:44:55:66";

	bt_content.ble_content.ble_name = "ROCKCHIP_AUDIO_BLE";
	bt_content.ble_content.server_uuid.uuid = SERVICE_UUID;
	bt_content.ble_content.server_uuid.len = UUID_128;
	bt_content.ble_content.chr_uuid[0].uuid = BLE_UUID_SEND;
	bt_content.ble_content.chr_uuid[0].len = UUID_128;
	bt_content.ble_content.chr_uuid[1].uuid = BLE_UUID_RECV;
	bt_content.ble_content.chr_uuid[1].len = UUID_128;
	bt_content.ble_content.chr_cnt = 2;

	bt_content.ble_content.advDataType = BLE_ADVDATA_TYPE_USER;

	//标识设备 LE 物理连接的功能
	bt_content.ble_content.advData[1] = 0x02;
	bt_content.ble_content.advData[2] = 0x01;
	bt_content.ble_content.advData[3] = 0x02;

	//service uuid(SERVICE_UUID)
	bt_content.ble_content.advData[4] = 0x03;
	bt_content.ble_content.advData[5] = 0x03;
	bt_content.ble_content.advData[6] = 0x10;
	bt_content.ble_content.advData[7] = 0x19;

	//ble name
	printf("ble_name_len: %s(%d)\n", bt_content.ble_content.ble_name, strlen(bt_content.ble_content.ble_name));
	ble_name_len = strlen(bt_content.ble_content.ble_name);
	remain_len = 31 - (bt_content.ble_content.advData[1] + 1)
					- (bt_content.ble_content.advData[4] + 1);
	len = ble_name_len > remain_len ? remain_len : ble_name_len;
	bt_content.ble_content.advData[8] = len + 1;
	bt_content.ble_content.advData[9] = 0x09;
	memcpy(&bt_content.ble_content.advData[10], bt_content.ble_content.ble_name, len);

	bt_content.ble_content.advData[0] = bt_content.ble_content.advData[1] + 1
										+ bt_content.ble_content.advData[4] + 1
										+ bt_content.ble_content.advData[8] + 1;
	bt_content.ble_content.advDataLen = bt_content.ble_content.advData[0] + 1;

	//==========================rsp======================
	bt_content.ble_content.respData[1] = 0x16;  //长度
	bt_content.ble_content.respData[2] = 0xFF;  //字段类型

	/*厂商编码*/
	bt_content.ble_content.respData[3] = 0x46;
	bt_content.ble_content.respData[4] = 0x00;

	bt_content.ble_content.respData[5] = 0x02;  //项目代号长度

	/*项目代号*/
	bt_content.ble_content.respData[6] = 0x1c;
	bt_content.ble_content.respData[7] = 0x02;

	bt_content.ble_content.respData[8] = 0x04;  //版本号长度
	bt_content.ble_content.respData[9] = 'T';   //版本号类型
	/*版本号*/
	bt_content.ble_content.respData[10] = 0x01;
	bt_content.ble_content.respData[11] = 0x00;
	bt_content.ble_content.respData[12] = 0x00;

	bt_content.ble_content.respData[13] = 0x08;	// SN长度
	/*SN号*/
	bt_content.ble_content.respData[14] = 0x54;
	bt_content.ble_content.respData[15] = 0x00;
	bt_content.ble_content.respData[16] = 0x00;
	bt_content.ble_content.respData[17] = 0x00;
	bt_content.ble_content.respData[18] = 0x00;
	bt_content.ble_content.respData[19] = 0x00;
	bt_content.ble_content.respData[20] = 0x00;
	bt_content.ble_content.respData[21] = 0x36;

	bt_content.ble_content.respData[22] = 0x01;	//绑定信息长度
	bt_content.ble_content.respData[23] = 0x00;	//绑定信息

	bt_content.ble_content.respData[0] = bt_content.ble_content.respData[1] + 1;  //长度
	bt_content.ble_content.respDataLen = bt_content.ble_content.respData[0] + 1;

	bt_content.ble_content.cb_ble_recv_fun = bt_test_ble_recv_data_callback;
	bt_content.ble_content.cb_ble_request_data = bt_test_ble_request_data_callback;
	rk_bt_register_state_callback(bt_test_state_cb);
	rk_bt_register_bond_callback(bt_test_bond_state_cb);
	rk_bt_init(&bt_content);

	return NULL;
}

#endif

static int bt_onff_cnt = 0;
void *bt_test_onff_thread(void *arg)
{
	while (1) {
		bt_test_bluetooth_init_thread(NULL);
		bt_test_source_open(NULL);
		while (gonff == false) {
			sleep(1);
			printf("%s: BT BLUETOOTH TURNING ON ...\n", __func__);
		}

		printf("%s: BT BLUETOOTH DEINIT\n", __func__);
		bt_test_source_close(NULL);
		rk_bt_deinit();
		while (gonff == true) {
			sleep(1);
			printf("%s: BT BLUETOOTH TURNING OFF ...\n", __func__);
		}
		printf("%s: BT BLUETOOTH TURN ONOFF CNT: [%d] \n", __func__, bt_onff_cnt++);
	}
}

static pthread_t bt_init_onoff_thread = 0;
void bt_test_bluetooth_onff_init(char *data)
{
	printf("%s: ", __func__);

	if (bt_init_onoff_thread) {
		printf("bt init thread already exist\n");
		return;
	}

	if (pthread_create(&bt_init_onoff_thread, NULL, bt_test_onff_thread, NULL)) {
		printf("Create bt init pthread failed\n");
		return;
	}
}

static pthread_t bt_init_thread = 0;
void bt_test_bluetooth_init(char *data)
{
	printf("%s: ", __func__);

	if (bt_init_thread) {
		printf("bt init thread already exist\n");
		return;
	}

	if (pthread_create(&bt_init_thread, NULL, bt_test_bluetooth_init_thread, NULL)) {
		printf("Create bt init pthread failed\n");
		return;
	}
}

void bt_test_bluetooth_deinit(char *data)
{
	int ret;

	printf("%s: BT BLUETOOTH DEINIT\n", __func__);
	rk_bt_deinit();

	if(bt_init_thread) {
		ret = pthread_join(bt_init_thread, NULL);
		if (ret) {
			printf("%s: bt init thread exit failed!\n", __func__);
		} else {
			printf("%s: bt init thread exit ok\n", __func__);
		}
		bt_init_thread = 0;
	}

}

void bt_test_set_class(char *data)
{
	rk_bt_set_class(0x240404);
}

void bt_test_enable_reconnect(char *data)
{
	rk_bt_enable_reconnect(1);
}

void bt_test_disable_reconnect(char *data)
{
	rk_bt_enable_reconnect(0);
}

void bt_test_get_device_name(char *data)
{
	char name[256];
	memset(name, 0, 256);
	rk_bt_get_device_name(name, 256);
	printf("bt device name: %s\n", name);
}

void bt_test_get_device_addr(char *data)
{
	char addr[18];
	memset(addr, 0, 18);
	rk_bt_get_device_addr(addr, 18);
	printf("bt device addr: %s\n", addr);
}

void bt_test_set_device_name(char *data)
{
	printf("%s: device name: %s\n", __func__, data);
	rk_bt_set_device_name(data);
}

void bt_test_pair_by_addr(char *data)
{
	rk_bt_pair_by_addr(data);
}

void bt_test_unpair_by_addr(char *data)
{
	rk_bt_unpair_by_addr(data);
}

void bt_test_get_paired_devices(char *data)
{
	int i, count;
	RkBtScanedDevice *dev_tmp = NULL;

	if(g_paired_dev_list)
		bt_test_free_paired_devices(NULL);

	rk_bt_get_paired_devices(&g_paired_dev_list, &count);

	printf("%s: current paired devices count: %d\n", __func__, count);
	dev_tmp = g_paired_dev_list;
	for(i = 0; i < count; i++) {
		printf("device %d\n", i);
		printf("	remote_address: %s\n", dev_tmp->remote_address);
		printf("	remote_name: %s\n", dev_tmp->remote_name);
		printf("	is_connected: %d\n", dev_tmp->is_connected);
		dev_tmp = dev_tmp->next;
	}
}

void bt_test_free_paired_devices(char *data)
{
	rk_bt_free_paired_devices(g_paired_dev_list);
	g_paired_dev_list = NULL;
}

void bt_test_start_discovery(char *data)
{
	int time;

	if(data == NULL) {
		printf("Please enter the scan time\n");
		return;
	}

	time = atoi(data);
	if(time < 10000) {
		printf("Scan time is too short(%d), reset to 10000ms\n", time);
		time = 10000;
	}

	rk_bt_register_discovery_callback(bt_test_discovery_status_cb);
	rk_bt_register_dev_found_callback(bt_test_dev_found_cb);
	rk_bt_start_discovery(time, SCAN_TYPE_AUTO);
}

void bt_test_start_discovery_bredr(char *data)
{
	int time;

	if(data == NULL) {
		printf("Please enter the scan time\n");
		return;
	}

	time = atoi(data);
	if(time < 10000) {
		printf("Scan time is too short(%d), reset to 10000ms\n", time);
		time = 10000;
	}

	rk_bt_register_discovery_callback(bt_test_discovery_status_cb);
	rk_bt_register_dev_found_callback(bt_test_dev_found_cb);
	rk_bt_start_discovery(time, SCAN_TYPE_BREDR);
}

void bt_test_start_discovery_le(char *data)
{
	int time;

	if(data == NULL) {
		printf("Please enter the scan time\n");
		return;
	}

	time = atoi(data);
	if(time < 10000) {
		printf("Scan time is too short(%d), reset to 10000ms\n", time);
		time = 10000;
	}

	rk_bt_register_discovery_callback(bt_test_discovery_status_cb);
	rk_bt_register_dev_found_callback(bt_test_dev_found_cb);
	rk_bt_start_discovery(time, SCAN_TYPE_LE);
}

void bt_test_start_discovery_pan(char *data)
{
	int time;

	if(data == NULL) {
		printf("Please enter the scan time\n");
		return;
	}

	time = atoi(data);
	if(time < 10000) {
		printf("Scan time is too short(%d), reset to 10000ms\n", time);
		time = 10000;
	}

	rk_bt_register_discovery_callback(bt_test_discovery_status_cb);
	rk_bt_register_dev_found_callback(bt_test_dev_found_cb);
	rk_bt_start_discovery(time, SCAN_TYPE_PAN);
}

void bt_test_start_discovery_spp(char *data)
{
	int time;

	if(data == NULL) {
		printf("Please enter the scan time\n");
		return;
	}

	time = atoi(data);
	if(time < 10000) {
		printf("Scan time is too short(%d), reset to 10000ms\n", time);
		time = 10000;
	}

	rk_bt_register_discovery_callback(bt_test_discovery_status_cb);
	rk_bt_register_dev_found_callback(bt_test_dev_found_cb);
	rk_bt_start_discovery(time, SCAN_TYPE_SPP);
}

void bt_test_cancel_discovery(char *data)
{
	rk_bt_cancel_discovery();
}

void bt_test_is_discovering(char *data)
{
	bool ret = rk_bt_is_discovering();
	printf("the device discovery procedure is active? %s\n", (ret == true) ? "yes" : "no");
}

void bt_test_get_scaned_devices(char *data)
{
	int i, count;
	RkBtScanedDevice *dev_tmp = NULL;
	RkBtScanedDevice *scaned_dev_list = NULL;

	rk_bt_get_scaned_devices(&scaned_dev_list, &count);

	printf("%s: current scaned devices count: %d\n", __func__, count);
	dev_tmp = scaned_dev_list;
	for(i = 0; i < count; i++) {
		printf("device %d\n", i);
		printf("	remote_address: %s\n", dev_tmp->remote_address);
		printf("	remote_name: %s\n", dev_tmp->remote_name);
		printf("	is_connected: %d\n", dev_tmp->is_connected);
		printf("	class_of_device: 0x%x\n", dev_tmp->cod);
		dev_tmp = dev_tmp->next;
	}

	rk_bt_free_scaned_devices(scaned_dev_list);
}

void bt_test_display_devices(char *data)
{
	rk_bt_display_devices();
}

void bt_test_display_paired_devices(char *data)
{
	rk_bt_display_paired_devices();
}

void bt_test_get_connected_properties(char *data)
{
	bool is_connected = false;
	is_connected = rk_bt_get_connected_properties(data);
	printf("the device connected properties is %s\n", (is_connected == true) ? "yes" : "no");
}

void bt_test_read_remote_device_name(char *data)
{
	rk_bt_read_remote_device_name(data, RK_BT_TRANSPORT_UNKNOWN);
}

/******************************************/
/*               A2DP SINK                */
/******************************************/
int bt_sink_callback(RK_BT_SINK_STATE state)
{
	switch(state) {
		case RK_BT_SINK_STATE_IDLE:
			printf("++++++++++++ BT SINK EVENT: idle ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_CONNECT:
			printf("++++++++++++ BT SINK EVENT: connect sucess ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_DISCONNECT:
			printf("++++++++++++ BT SINK EVENT: disconnected ++++++++++\n");
			//system("amixer set bt 255");
			break;
		//avrcp
		case RK_BT_SINK_STATE_PLAY:
			printf("++++++++++++ BT SINK EVENT: playing ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_PAUSE:
			printf("++++++++++++ BT SINK EVENT: paused ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_STOP:
			printf("++++++++++++ BT SINK EVENT: stoped ++++++++++\n");
			break;
		//avdtp(a2dp)
		case RK_BT_A2DP_SINK_STARTED:
			printf("++++++++++++ BT A2DP SINK STATE: started ++++++++++\n");
			break;
		case RK_BT_A2DP_SINK_SUSPENDED:
			printf("++++++++++++ BT A2DP SINK STATE: suspended ++++++++++\n");
			break;
		case RK_BT_A2DP_SINK_STOPPED:
			printf("++++++++++++ BT A2DP SINK STATE: stoped ++++++++++\n");
			break;
	}

	return 0;
}

void bt_sink_volume_callback(int volume)
{
	printf("++++++++ bt sink volume change, volume: %d ++++++++\n", volume);

	/* Change the code below based on which interface audio is going out to. */
#if 0
	char buffer[100];
	sprintf(buffer, "amixer set bt %d", volume * 255 / 127);
	if (-1 == system(buffer))
		printf("set volume error: %d, volume: %d\n", errno, volume);
#endif
}

void bt_sink_track_change_callback(const char *bd_addr, BtTrackInfo track_info)
{
#if 0
	printf("++++++++ bt sink track change ++++++++\n");
	printf("    remote device address: %s\n", bd_addr);
	printf("    title: %s\n", track_info.title);
	printf("    artist: %s\n", track_info.artist);
	printf("    album: %s\n", track_info.album);
	printf("    genre: %s\n", track_info.genre);
	printf("    num_tracks: %s\n", track_info.num_tracks);
	printf("    track_num: %s\n", track_info.track_num);
	printf("    playing_time: %s\n", track_info.playing_time);
#endif
}

void bt_sink_position_change_callback(const char *bd_addr, int song_len, int song_pos)
{
#if 0
	printf("++++++++ bt sink position change ++++++++\n");
	printf("    remote device address: %s\n", bd_addr);
	printf("    song_len: %d, song_pos: %d\n", song_len, song_pos);
#endif
}

void bt_test_sink_open(char *data)
{
	rk_bt_sink_register_volume_callback(bt_sink_volume_callback);
	rk_bt_sink_register_track_callback(bt_sink_track_change_callback);
	rk_bt_sink_register_position_callback(bt_sink_position_change_callback);
	rk_bt_sink_register_callback(bt_sink_callback);
	rk_bt_sink_open();
	//rk_bt_sink_set_alsa_device("bt");
}

void bt_test_sink_visibility00(char *data)
{
	rk_bt_set_visibility(0, 0);
}

void bt_test_sink_visibility01(char *data)
{
	rk_bt_set_visibility(0, 1);
}

void bt_test_sink_visibility10(char *data)
{
	rk_bt_set_visibility(1, 0);
}

void bt_test_sink_visibility11(char *data)
{
	rk_bt_set_visibility(1, 1);
}

void bt_test_ble_visibility00(char *data)
{
	rk_bt_set_visibility(0, 0);
}

void bt_test_ble_visibility11(char *data)
{
	rk_bt_set_visibility(1, 1);
}

void bt_test_sink_status(char *data)
{
	RK_BT_SINK_STATE pState;

	rk_bt_sink_get_state(&pState);
	switch(pState) {
		case RK_BT_SINK_STATE_IDLE:
			printf("++++++++++++ BT SINK STATUS: idle ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_CONNECT:
			printf("++++++++++++ BT SINK STATUS: connect sucess ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_PLAY:
		case RK_BT_A2DP_SINK_STARTED:
			printf("++++++++++++ BT SINK STATUS: playing ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_PAUSE:
		case RK_BT_A2DP_SINK_SUSPENDED:
			printf("++++++++++++ BT SINK STATUS: paused ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_STOP:
		case RK_BT_A2DP_SINK_STOPPED:
			printf("++++++++++++ BT SINK STATUS: stoped ++++++++++\n");
			break;
		case RK_BT_SINK_STATE_DISCONNECT:
			printf("++++++++++++ BT SINK STATUS: disconnected ++++++++++\n");
			break;
	}
}

void bt_test_sink_music_play(char *data)
{
	rk_bt_sink_play();
}

void bt_test_sink_music_pause(char *data)
{
	rk_bt_sink_pause();
}

void bt_test_sink_music_next(char *data)
{
	rk_bt_sink_next();
}

void bt_test_sink_music_previous(char *data)
{
	rk_bt_sink_prev();
}

void bt_test_sink_music_stop(char *data)
{
	rk_bt_sink_stop();
}

void bt_test_sink_disconnect(char *data)
{
	rk_bt_sink_disconnect();
}

void bt_test_sink_close(char *data)
{
	rk_bt_sink_close();
}

void bt_test_sink_set_volume(char *data)
{
	int i = 0;

	printf("===== A2DP SINK Set Volume:100 =====\n");
	rk_bt_sink_set_volume(127);
	sleep(2);
	printf("===== A2DP SINK Set Volume:64 =====\n");
	rk_bt_sink_set_volume(64);
	sleep(2);
	printf("===== A2DP SINK Set Volume:0 =====\n");
	rk_bt_sink_set_volume(0);
	sleep(2);

	for (; i < 17; i++) {
		printf("===== A2DP SINK Set Volume UP =====\n");
		rk_bt_sink_volume_up();
		sleep(2);
	}

	for (i = 0; i < 17; i++) {
		printf("===== A2DP SINK Set Volume DOWN =====\n");
		rk_bt_sink_volume_down();
		sleep(2);
	}
}

void bt_test_sink_connect_by_addr(char *data)
{
	rk_bt_sink_connect_by_addr(data);
}

void bt_test_sink_disconnect_by_addr(char *data)
{
	rk_bt_sink_disconnect_by_addr(data);
}

void bt_test_sink_get_play_status(char *data)
{
	rk_bt_sink_get_play_status();
}

void bt_test_sink_get_poschange(char *data)
{
	bool pos_change = rk_bt_sink_get_poschange();
	printf("support position change: %s\n", pos_change ? "yes" : "no");
}

/******************************************/
/*              A2DP SOURCE               */
/******************************************/
void bt_test_source_status_callback(void *userdata, const char *bd_addr,
			const char *name, const RK_BT_SOURCE_EVENT enEvent)
{
	switch(enEvent)
	{
		case BT_SOURCE_EVENT_CONNECT_FAILED:
			printf("+++++ BT_SOURCE_EVENT_CONNECT_FAILED: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_CONNECTED:
			printf("+++++ BT_SOURCE_EVENT_CONNECTED: %s, %s +++++\n", name, bd_addr);
			printf("+++++ device playrole: %d\n", rk_bt_get_playrole_by_addr(bd_addr));
			break;
		case BT_SOURCE_EVENT_AUTO_RECONNECTING:
			printf("+++++ BT_SOURCE_EVENT_AUTO_RECONNECTING: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_DISCONNECT_FAILED:
			printf("+++++ BT_SOURCE_EVENT_DISCONNECT_FAILED: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_DISCONNECTED:
			printf("+++++ BT_SOURCE_EVENT_DISCONNECTED: %s, %s +++++\n", name, bd_addr);
			break;
#if 0
		case BT_SOURCE_EVENT_REMOVE_FAILED:
			printf("+++++ BT_SOURCE_EVENT_REMOVE_FAILED +++++\n");
			break;
		case BT_SOURCE_EVENT_REMOVED:
			printf("+++++ BT_SOURCE_EVENT_REMOVED +++++\n");
			break;
#endif
		case BT_SOURCE_EVENT_RC_PLAY:
			printf("+++++ BT_SOURCE_EVENT_RC_PLAY: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_RC_STOP:
			printf("+++++ BT_SOURCE_EVENT_RC_STOP: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_RC_PAUSE:
			printf("+++++ BT_SOURCE_EVENT_RC_PAUSE: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_RC_FORWARD:
			printf("+++++ BT_SOURCE_EVENT_RC_FORWARD: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_RC_BACKWARD:
			printf("+++++ BT_SOURCE_EVENT_RC_BACKWARD: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_RC_VOL_UP:
			printf("+++++ BT_SOURCE_EVENT_RC_VOL_UP: %s, %s +++++\n", name, bd_addr);
			break;
		case BT_SOURCE_EVENT_RC_VOL_DOWN:
			printf("+++++ BT_SOURCE_EVENT_RC_VOL_DOWN: %s, %s +++++\n", name, bd_addr);
			break;
	}
}

void bt_test_source_auto_start(char *data)
{
	rk_bt_source_auto_connect_start(NULL, bt_test_source_status_callback);
}

void bt_test_source_auto_stop(char *data)
{
	rk_bt_source_auto_connect_stop();
}

void bt_test_source_connect_status(char *data)
{
	RK_BT_SOURCE_STATUS status;
	char name[256], address[256];

	rk_bt_source_get_status(&status, name, 256, address, 256);
	if (status == BT_SOURCE_STATUS_CONNECTED) {
		printf("++++++++++++ BT SOURCE STATUS: connected ++++++++++++\n");
		printf("\t name:%s, address:%s\n", name, address);
	} else
		printf("++++++++++++ BT SOURCE STATUS: disconnected ++++++++++++\n");
}

void bt_test_source_open(char *data)
{
	rk_bt_source_register_status_cb(NULL, bt_test_source_status_callback);
	rk_bt_source_open();
}

void bt_test_source_close(char *data)
{
	rk_bt_source_close();
}

void bt_test_source_connect_by_addr(char *data)
{
	rk_bt_source_connect_by_addr(data);
}

void bt_test_source_disconnect_by_addr(char *data)
{
	rk_bt_source_disconnect_by_addr(data);
}

void bt_test_source_remove_by_addr(char *data)
{
	rk_bt_source_remove(data);
}

void bt_test_source_disconnect(char *data)
{
	rk_bt_source_disconnect();
}

void bt_test_source_set_vol(char *data)
{
	int vol;

	vol = atoi(data);
	printf("+++++ bt_test_source_set_vol: %s, %d +++++\n", data, vol);
	rk_bt_source_set_vol(vol);
}

/******************************************/
/*                  BLE                   */
/******************************************/
static void ble_status_callback_test(const char *bd_addr, const char *name, RK_BLE_STATE state)
{
	printf("%s: status: %d.\n", __func__, state);

	switch (state) {
		case RK_BLE_STATE_IDLE:
			printf("+++++ RK_BLE_STATE_IDLE +++++\n");
			break;
		case RK_BLE_STATE_CONNECT:
			printf("+++++ RK_BLE_STATE_CONNECT: %s, %s +++++\n", name, bd_addr);
			break;
		case RK_BLE_STATE_DISCONNECT:
			printf("+++++ RK_BLE_STATE_DISCONNECT: %s, %s +++++\n", name, bd_addr);
			g_mtu = 0;
			break;
	}
}

static void bt_test_ble_recv_data_callback(const char *uuid, char *data, int len)
{
	char data_t[512];
	char reply_buf[512] = {"My name is rockchip"};

	printf("=== %s uuid: %s===\n", __func__, uuid);
	memcpy(data_t, data, len);
	for (int i = 0 ; i < len; i++) {
		printf("%02x ", data_t[i]);
	}
	printf("\n");

	if (strstr(data_t, "Hello RockChip") || strstr(data_t, "HelloRockChip") ||
		strstr(data_t, "HELLO ROCKCHIP") || strstr(data_t, "HELLOROCKCHIP") ||
		strstr(data_t, "hello rockchip") || strstr(data_t, "hellorockchip")) {
		printf("=== %s Reply:%s ===\n", __func__, reply_buf);
		rk_ble_write(uuid, reply_buf, 17);
	}
}

void *_send_data(void *data)
{
	char *uuid = (char *)data;

	rk_ble_write(uuid, "abcd", 4);
	usleep(100000);
	rk_ble_write(uuid, "wwww", 4);
	usleep(100000);
	rk_ble_write(uuid, "zzzz", 4);

	return ((void *)0);
}

void send_data(char *uuid)
{
	pthread_t tid = 0;
	if (pthread_create(&tid, NULL, _send_data, (void *)uuid)) {
		printf("Create _send_data pthread failed\n");
		return;
	}
}

static void bt_test_ble_request_data_callback(const char *uuid)
{
	printf("=== %s uuid: %s===\n", __func__, uuid);
	//rk_ble_write(uuid, "Hello Rockchip", strlen("Hello Rockchip"));

	send_data(uuid);
}

static void bt_test_mtu_callback(const char *bd_addr, unsigned int mtu)
{
	printf("=== %s: bd_addr: %s, mtu: %d ===\n", __func__, bd_addr, mtu);
	g_mtu = mtu;
}

void bt_test_ble_start(char *data)
{
	rk_ble_register_status_callback(ble_status_callback_test);
	rk_ble_register_recv_callback(bt_test_ble_recv_data_callback);
	rk_ble_register_request_data_callback(bt_test_ble_request_data_callback);
	rk_ble_register_mtu_callback(bt_test_mtu_callback);
	rk_ble_start(&bt_content.ble_content);
}

void bt_test_ble_set_address(char *data)
{
	//user-defined ble address test, if not set, the default random address
	char ble_addr[DEVICE_ADDR_LEN];

	if(!data || strlen(data) < 17)
		return;

	printf("ble address: %s\n", data);
	if (sscanf(data, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&ble_addr[5], &ble_addr[4], &ble_addr[3],
			&ble_addr[2], &ble_addr[1], &ble_addr[0]) != 6) {
		return;
	}

	//Clear two most significant bits
	ble_addr[5] &= 0x3f;
	//Set second most significant bit, Private resolvable
	ble_addr[5] |= 0x40;

	printf("ble_addr: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
			ble_addr[5], ble_addr[4], ble_addr[3],
			ble_addr[2], ble_addr[1], ble_addr[0]);

	rk_ble_set_address(ble_addr);
}

void bt_test_ble_set_adv_interval(char *data)
{
	//default 100ms, test: 20ms(32 * 0.625) ~ 100ms(160 * 0.625)
	rk_ble_set_adv_interval(32, 160);
}

void bt_test_ble_write(char *data)
{
	int i = 0, write_len = BT_ATT_DEFAULT_LE_MTU;
	char *write_buf;

	if(g_mtu > BT_ATT_HEADER_LEN)
		write_len = g_mtu;

	write_len -= BT_ATT_HEADER_LEN;
	if(write_len > BT_ATT_MAX_VALUE_LEN)
		write_len = BT_ATT_MAX_VALUE_LEN;

	write_buf = (char *)malloc(write_len);
	for (i = 0; i < (write_len - 1); i++)
		write_buf[i] = '0' + i % 10;
	write_buf[write_len - 1] = '\0';

	rk_ble_write(BLE_UUID_SEND, write_buf, write_len);
	//rk_ble_write(BLE_UUID_WIFI_CHAR, write_buf, write_len);
	free(write_buf);
}

void bt_test_ble_get_status(char *data)
{
	RK_BLE_STATE state;

	printf("RK_ble_status_test: ");
	rk_ble_get_state(&state);
	switch (state) {
		case RK_BLE_STATE_IDLE:
			printf("RK_BLE_STATE_IDLE.\n");
			break;
		case RK_BLE_STATE_CONNECT:
			printf("RK_BLE_STATE_CONNECT.\n");
			break;
		case RK_BLE_STATE_DISCONNECT:
			printf("RK_BLE_STATE_DISCONNECT.\n");
			break;
	}
}

void bt_test_ble_stop(char *data) {
	g_mtu = 0;
	rk_ble_stop();
}

void bt_test_ble_disconnect(char *data) {
	rk_ble_disconnect();
}


/******************************************/
/*               BLE CLIENT               */
/******************************************/
void ble_client_test_state_callback(const char *bd_addr, const char *name, RK_BLE_CLIENT_STATE state)
{
	switch(state)
	{
		case RK_BLE_CLIENT_STATE_IDLE:
			printf("+++++ RK_BLE_CLIENT_STATE_IDLE +++++\n");
			break;
		case RK_BLE_CLIENT_STATE_CONNECT:
			printf("+++++ RK_BLE_CLIENT_STATE_CONNECT(%s, %s) +++++\n", bd_addr, name);
			break;
		case RK_BLE_CLIENT_STATE_DISCONNECT:
			printf("+++++ RK_BLE_CLIENT_STATE_DISCONNECT(%s, %s) +++++\n", bd_addr, name);
			g_mtu = 0;
			break;
		case RK_BLE_CLIENT_WRITE_SUCCESS:
			printf("+++++ RK_BLE_CLIENT_WRITE_SUCCESS(%s, %s) +++++\n", bd_addr, name);
			break;
		case RK_BLE_CLIENT_WRITE_ERROR:
			printf("+++++ RK_BLE_CLIENT_WRITE_ERROR(%s, %s) +++++\n", bd_addr, name);
			break;
	}
}

static void bt_test_ble_client_recv_data_callback(const char *uuid, char *data, int len)
{
	printf("+++++ recv data +++++\n");
	printf("	uuid: %s\n", uuid);
	printf("	data len: %d\n	", len);
	for (int i = 0 ; i < len; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
}


void bt_test_ble_client_open(char *data)
{
	rk_ble_client_register_state_callback(ble_client_test_state_callback);
	rk_ble_client_register_recv_callback(bt_test_ble_client_recv_data_callback);
	rk_ble_client_register_mtu_callback(bt_test_mtu_callback);
	rk_ble_client_open(true);
}

void bt_test_ble_client_close(char *data)
{
	g_mtu = 0;
	rk_ble_client_close();
}

void bt_test_ble_client_get_status(char *data)
{
	RK_BLE_CLIENT_STATE state;

	state = rk_ble_client_get_state();
	switch (state) {
		case RK_BLE_CLIENT_STATE_IDLE:
			printf("RK_BLE_CLIENT_STATE_IDLE\n");
			break;
		case RK_BLE_CLIENT_STATE_CONNECT:
			printf("RK_BLE_CLIENT_STATE_CONNECT\n");
			break;
		case RK_BLE_CLIENT_STATE_DISCONNECT:
			printf("RK_BLE_CLIENT_STATE_DISCONNECT\n");
			break;
	}
}

void bt_test_ble_client_connect(char *data)
{
	rk_ble_client_connect(data);
}

void bt_test_ble_client_disconnect(char *data)
{
	rk_ble_client_disconnect(data);
}

void bt_test_ble_client_get_service_info(char *data)
{
	int i, j, k;
	RK_BLE_CLIENT_SERVICE_INFO info;

	if(!rk_ble_client_get_service_info(data, &info)) {
		printf("+++++ get device(%s) service info +++++\n", data);
		for(i = 0; i < info.service_cnt; i++) {
			printf("service[%d]:\n", i);
			printf("	describe: %s\n", info.service[i].describe);
			printf("	path: %s\n", info.service[i].path);
			printf("	uuid: %s\n", info.service[i].uuid);

			for(j = 0; j < info.service[i].chrc_cnt; j++) {
				printf("	characteristic[%d]:\n", j);
				printf("		describe: %s\n", info.service[i].chrc[j].describe);
				printf("		path: %s\n", info.service[i].chrc[j].path);
				printf("		uuid: %s\n", info.service[i].chrc[j].uuid);
				printf("		props: 0x%x\n", info.service[i].chrc[j].props);
				printf("		ext_props: 0x%x\n", info.service[i].chrc[j].ext_props);
				printf("		perm: 0x%x\n", info.service[i].chrc[j].perm);
				printf("		notifying: %d\n", info.service[i].chrc[j].notifying);

				for(k = 0; k < info.service[i].chrc[j].desc_cnt; k++) {
					printf("		descriptor[%d]:\n", k);

					printf("			describe: %s\n", info.service[i].chrc[j].desc[k].describe);
					printf("			path: %s\n", info.service[i].chrc[j].desc[k].path);
					printf("			uuid: %s\n", info.service[i].chrc[j].desc[k].uuid);
				}
			}
		}
	}
}

void bt_test_ble_client_read(char *data)
{
	rk_ble_client_read(data);
}

void bt_test_ble_client_write(char *data)
{
#if 0
	//write data len no more than (g_mtu - BT_ATT_HEADER_LEN)
	rk_ble_client_write(data, "rockchip ble client write test", strlen("rockchip ble client write test"));
#else
	int i = 0, write_len = BT_ATT_DEFAULT_LE_MTU;
	char *write_buf;

	if(g_mtu > BT_ATT_HEADER_LEN)
		write_len = g_mtu;

	write_len -= BT_ATT_HEADER_LEN;
	if(write_len > BT_ATT_MAX_VALUE_LEN)
		write_len = BT_ATT_MAX_VALUE_LEN;

	write_buf = (char *)malloc(write_len);
	for (i = 0; i < (write_len - 1); i++)
		write_buf[i] = '0' + i % 10;
	write_buf[write_len - 1] = '\0';

	rk_ble_client_write(data, write_buf, strlen(write_buf));
	free(write_buf);
#endif
}

void bt_test_ble_client_is_notify(char *data)
{
	bool notifying;

	notifying = rk_ble_client_is_notifying(data);
	printf("%s notifying %s\n", data, notifying ? "yes" : "no");
}

void bt_test_ble_client_notify_on(char *data)
{
	rk_ble_client_notify(data, false, true);
}

void bt_test_ble_client_notify_off(char *data)
{
	rk_ble_client_notify(data, false, false);
}

void bt_test_ble_client_indicate_on(char *data)
{
	rk_ble_client_notify(data, true, true);
}

void bt_test_ble_client_indicate_off(char *data)
{
	rk_ble_client_notify(data, true, false);
}

void bt_test_ble_client_get_eir_data(char *data)
{
	char eir_data[300];

	rk_ble_client_get_eir_data(data, eir_data, 300);
#if 1
	int i;
	for(i = 0; i < 300; i++) {
		printf("%02x ", eir_data[i]);
		if((i != 0) && (i % 20 == 0))
			printf("\n");
	}
	printf("\n");
#endif
}

void bt_test_get_eir_data(char *data)
{
	char eir_data[300];
	int len;

	len = rk_bt_get_eir_data(data, eir_data, 300);
#if 1
	int i;
	printf("\n");
	for (i = 0; i < len; i++) {
		if((i != 0) && (i % 16 == 0))
			printf("\n");
		printf("%02x ", eir_data[i]);
	}
	printf("\n");
#endif
}

/******************************************/
/*                  SPP                   */
/******************************************/
void _btspp_status_callback(RK_BT_SPP_STATE type)
{
	switch(type) {
		case RK_BT_SPP_STATE_IDLE:
			printf("+++++++ RK_BT_SPP_STATE_IDLE +++++\n");
			break;
		case RK_BT_SPP_STATE_CONNECT:
			printf("+++++++ RK_BT_SPP_EVENT_CONNECT +++++\n");
			break;
		case RK_BT_SPP_STATE_DISCONNECT:
			printf("+++++++ RK_BT_SPP_EVENT_DISCONNECT +++++\n");
			break;
		default:
			printf("+++++++ BT SPP NOT SUPPORT TYPE! +++++\n");
			break;
	}
}

void _btspp_recv_callback(char *data, int len)
{
	if (len) {
		printf("+++++++ RK BT SPP RECV DATA: +++++\n");
		printf("\tRECVED(%d):%s\n", len, data);
	}
}

void bt_test_spp_open(char *data)
{
	rk_bt_spp_open();
	rk_bt_spp_register_status_cb(_btspp_status_callback);
	rk_bt_spp_register_recv_cb(_btspp_recv_callback);
}

void bt_test_spp_write(char *data)
{
	unsigned int ret = 0;
	char buff[100] = {"This is a message from rockchip board!"};

	ret = rk_bt_spp_write(buff, strlen(buff));
	if (ret < 0) {
		printf("%s failed\n", __func__);
	}
}

void bt_test_spp_connect(char *data)
{
	rk_bt_spp_connect(data);
}

void bt_test_spp_disconnect(char *data)
{
	rk_bt_spp_disconnect(data);
}

void bt_test_spp_listen(char *data)
{
    rk_bt_spp_listen();
}

void bt_test_spp_close(char *data)
{
	rk_bt_spp_close();
}

void bt_test_spp_status(char *data)
{
	RK_BT_SPP_STATE status;

	rk_bt_spp_get_state(&status);
	switch(status) {
		case RK_BT_SPP_STATE_IDLE:
			printf("+++++++ RK_BT_SPP_STATE_IDLE +++++\n");
			break;
		case RK_BT_SPP_STATE_CONNECT:
			printf("+++++++ RK_BT_SPP_STATE_CONNECT +++++\n");
			break;
		case RK_BT_SPP_STATE_DISCONNECT:
			printf("+++++++ RK_BT_SPP_STATE_DISCONNECT +++++\n");
			break;
		default:
			printf("+++++++ BTSPP NO STATUS SUPPORT! +++++\n");
			break;
	}
}

/******************************************/
/*                  HFP                   */
/******************************************/
static int hfp_set_sw_params(snd_pcm_t *pcm, snd_pcm_uframes_t buffer_size,
				snd_pcm_uframes_t period_size, char **msg)
{
	int err;
	snd_pcm_sw_params_t *params;

	snd_pcm_sw_params_malloc(&params);
	if ((err = snd_pcm_sw_params_current(pcm, params)) != 0) {
		printf("Get current params: %s\n", snd_strerror(err));
		return -1;
	}

	/* start the transfer when the buffer is full (or almost full) */
	snd_pcm_uframes_t threshold = (buffer_size / period_size) * period_size;
	if ((err = snd_pcm_sw_params_set_start_threshold(pcm, params, threshold)) != 0) {
		printf("Set start threshold: %s: %lu\n", snd_strerror(err), threshold);
		return -1;
	}

	/* allow the transfer when at least period_size samples can be processed */
	if ((err = snd_pcm_sw_params_set_avail_min(pcm, params, period_size)) != 0) {
		printf("Set avail min: %s: %lu\n", snd_strerror(err), period_size);
		return -1;
	}

	if ((err = snd_pcm_sw_params(pcm, params)) != 0) {
		printf("snd_pcm_sw_params: %s\n", snd_strerror(err));
		return -1;
	}

	if(params)
		snd_pcm_sw_params_free(params);

	return 0;
}

static int hfp_playback_device_open(snd_pcm_t** playback_handle,
		const char* device_name, alsa_config_t alsa_config)
{
	int err;
	snd_pcm_hw_params_t *hw_params;
	unsigned int rate = alsa_config.sample_rate;
	snd_pcm_uframes_t period_size = alsa_config.period_size;
	snd_pcm_uframes_t buffer_size = alsa_config.buffer_size;

	err = snd_pcm_open(playback_handle, device_name, SND_PCM_STREAM_PLAYBACK, 0);
	if (err) {
		printf( "Unable to open playback PCM device: %s\n", device_name);
		return -1;
	}
	printf("Open playback PCM device: %s\n", device_name);

	err = snd_pcm_hw_params_malloc(&hw_params);
	if (err) {
		printf("cannot malloc hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_any(*playback_handle, hw_params);
	if (err) {
		printf("cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_set_access(*playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err) {
		printf("Error setting interleaved mode: %s\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_set_format(*playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (err) {
		printf("Error setting format: %s\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_set_channels(*playback_handle, hw_params, alsa_config.channels);
	if (err) {
		printf( "Error setting channels: %s\n", snd_strerror(err));
		return -1;
	}
	printf("setting channels (%d)\n", alsa_config.channels);

	printf("WANT-RATE = %d\n", rate);
	err = snd_pcm_hw_params_set_rate_near(*playback_handle, hw_params, &rate, 0);
	if (err) {
		printf("Error setting sampling rate (%d): %s\n", rate, snd_strerror(err));
		return -1;
	}
	printf("set sampling rate (%d)\n", rate);

	err = snd_pcm_hw_params_set_period_size_near(*playback_handle, hw_params, &period_size, 0);
	if (err) {
		printf("Error setting period size (%ld): %s\n", period_size, snd_strerror(err));
		return -1;
	}
	printf("period_size = %d\n", (int)period_size);

	err = snd_pcm_hw_params_set_buffer_size_near(*playback_handle, hw_params, &buffer_size);
	if (err) {
		printf("Error setting buffer size (%ld): %s\n", buffer_size, snd_strerror(err));
		return -1;
	}
	printf("buffer_size = %d\n", (int)buffer_size);

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(*playback_handle, hw_params);
	if (err < 0) {
		printf( "Unable to set HW parameters: %s\n", snd_strerror(err));
		return -1;
	}

	printf("Open playback device is successful: %s\n", device_name);

	hfp_set_sw_params(*playback_handle, buffer_size, period_size, NULL);
	if (hw_params)
		snd_pcm_hw_params_free(hw_params);

	return 0;
}

static int hfp_capture_device_open(snd_pcm_t** capture_handle,
		const char* device_name, alsa_config_t alsa_config)
{
	int err;
	snd_pcm_hw_params_t *hw_params;
	unsigned int rate = alsa_config.sample_rate;
	snd_pcm_uframes_t period_size = alsa_config.period_size;
	snd_pcm_uframes_t buffer_size = alsa_config.buffer_size;

	err = snd_pcm_open(capture_handle, device_name, SND_PCM_STREAM_CAPTURE, 0);
	if (err) {
		printf( "Unable to open capture PCM device: %s\n", device_name);
		return -1;
	}
	printf("Open capture PCM device: %s\n", device_name);

	err = snd_pcm_hw_params_malloc(&hw_params);
	if (err) {
		printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_any(*capture_handle, hw_params);
	if (err) {
		printf("cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_set_access(*capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err) {
		printf("Error setting interleaved mode: %s\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_set_format(*capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
	if (err) {
		printf("Error setting format: %s\n", snd_strerror(err));
		return -1;
	}

	err = snd_pcm_hw_params_set_channels(*capture_handle, hw_params, alsa_config.channels);
	if (err) {
		printf( "Error setting channels: %s, channels = %d\n", snd_strerror(err), alsa_config.channels);
		return -1;
	}
	printf("setting channels (%d)\n", alsa_config.channels);

	printf("WANT-RATE = %d\n", rate);
	err = snd_pcm_hw_params_set_rate_near(*capture_handle, hw_params, &rate, 0);
	if (err) {
		printf("Error setting sampling rate (%d): %s\n", rate, snd_strerror(err));
		return -1;
	}
	printf("set sampling rate (%d)\n", rate);

	err = snd_pcm_hw_params_set_period_size_near(*capture_handle, hw_params, &period_size, 0);
	if (err) {
		printf("Error setting period size (%d): %s\n", (int)period_size, snd_strerror(err));
		return -1;
	}
	printf("period_size = %d\n", (int)period_size);

	err = snd_pcm_hw_params_set_buffer_size_near(*capture_handle, hw_params, &buffer_size);
	if (err) {
		printf("Error setting buffer size (%d): %s\n", (int)buffer_size, snd_strerror(err));
		return -1;
	}
	printf("buffer_size = %d\n", (int)buffer_size);

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(*capture_handle, hw_params);
	if (err < 0) {
		 printf( "Unable to set HW parameters: %s\n", snd_strerror(err));
		 return -1;
	 }

	printf("Open capture device is successful: %s\n", device_name);
	if (hw_params)
		snd_pcm_hw_params_free(hw_params);

	return 0;
}

static void hfp_pcm_close(snd_pcm_t *handle)
{
	if(handle)
		snd_pcm_close(handle);
}

static void hfp_tinymix_set(int group, int volume)
{
	char cmd[50] = {0};

	sprintf(cmd, "tinymix set 'ADC MIC Group %d Left Volume' %d", group, volume);
	if (-1 == system(cmd))
		printf("tinymix set ADC MIC Group %d Left Volume failed\n", group);

	memset(cmd, 0, 50);
	sprintf(cmd, "tinymix set 'ADC MIC Group %d Right Volume' %d", group, volume);
	if (-1 == system(cmd))
		printf("tinymix set ADC MIC Group %d Right Volume failed\n", group);
}

static void *hfp_alsa_playback(void *arg)
{
	int err, ret = -1;
	snd_pcm_t *capture_handle = NULL;
	snd_pcm_t *playbcak_handle = NULL;
	short *buffer;
	int read_frame, buffer_size;
	alsa_config_t alsa_config;

	switch(sco_codec) {
		case BT_SCO_CODEC_CVSD:
			read_frame = READ_FRAME_512;
			alsa_config.sample_rate = CVSD_SAMPLE_RATE;
			alsa_config.period_size = PERIOD_SIZE_512;
			alsa_config.buffer_size = BUFFER_SIZE_2048;
			break;
		case BT_SCO_CODEC_MSBC:
			read_frame = READ_FRAME_1024;
			alsa_config.sample_rate = MSBC_SAMPLE_RATE;
			alsa_config.period_size = PERIOD_SIZE_1024;
			alsa_config.buffer_size = BUFFER_SIZE_4096;
			break;
		default:
			printf("%s: invalid sco codec type: %d\n", __func__, sco_codec);
			return NULL;
	}
	alsa_config.channels = HFP_PCM_CHANNEL_NB;
	buffer_size = read_frame * HFP_PCM_CHANNEL_NB * sizeof(short);
	buffer = (short *)malloc(buffer_size);
	memset((char *)buffer, 0, buffer_size);

device_open:
	printf("==========bt capture, alsa playback============\n");
	ret = hfp_capture_device_open(&capture_handle, bt_capture_device, alsa_config);
	if (ret == -1) {
		printf("capture device open failed: %s\n", bt_capture_device);
		goto exit;
	}

	ret = hfp_playback_device_open(&playbcak_handle, alsa_playback_device, alsa_config);
	if (ret == -1) {
		printf("playback device open failed: %s\n", alsa_playback_device);
		goto exit;
	}

	g_duplex_control.alsa_duplex_opened = true;

	while (g_duplex_control.alsa_duplex_opened) {
		err = snd_pcm_readi(capture_handle, buffer , read_frame);
		if (!g_duplex_control.alsa_duplex_opened)
			goto exit;

		if (err != read_frame)
			printf("=====read frame error = %d=====\n", err);

		if (err == -EPIPE)
			printf("Overrun occurred: %d\n", err);

		if (err < 0) {
			err = snd_pcm_recover(capture_handle, err, 0);
			// Still an error, need to exit.
			if (err < 0) {
				printf( "Error occured while recording: %s\n", snd_strerror(err));
				usleep(100 * 1000);
				hfp_pcm_close(capture_handle);
				hfp_pcm_close(playbcak_handle);
				goto device_open;
			}
		}

		err = snd_pcm_writei(playbcak_handle, buffer, read_frame);
		if (!g_duplex_control.alsa_duplex_opened)
			goto exit;

		if (err != read_frame)
			printf("=====write frame error = %d=====\n", err);

		if (err == -EPIPE)
			printf("Underrun occurred from write: %d\n", err);

		if (err < 0) {
			err = snd_pcm_recover(playbcak_handle, err, 0);
			// Still an error, need to exit.
			if (err < 0) {
				printf( "Error occured while writing: %s\n", snd_strerror(err));
				usleep(100 * 1000);
				hfp_pcm_close(capture_handle);
				hfp_pcm_close(playbcak_handle);
				goto device_open;
			}
		}
	}

exit:
	hfp_pcm_close(capture_handle);
	hfp_pcm_close(playbcak_handle);
	free(buffer);

	printf("Exit app hs alsa playback thread\n");
	pthread_exit(0);
}

static void *hfp_bt_playback(void *arg)
{
	int err, ret = -1;
	snd_pcm_t *capture_handle = NULL;
	snd_pcm_t *playbcak_handle = NULL;
	short buffer[READ_FRAME_256 * HFP_PCM_CHANNEL_NB] = {0};
	alsa_config_t alsa_config;

	alsa_config.channels = HFP_PCM_CHANNEL_NB;
	alsa_config.period_size = PERIOD_SIZE_256;
	alsa_config.buffer_size = BUFFER_SIZE_1024;
	switch(sco_codec) {
		case BT_SCO_CODEC_CVSD:
			alsa_config.sample_rate = CVSD_SAMPLE_RATE;
			break;
		case BT_SCO_CODEC_MSBC:
			alsa_config.sample_rate = MSBC_SAMPLE_RATE;
			break;
		default:
			printf("%s: invalid sco codec type: %d\n", __func__, sco_codec);
			return NULL;
	}

device_open:
	printf("==========mic capture, bt playback============\n");
	ret = hfp_capture_device_open(&capture_handle, alsa_capture_device, alsa_config);
	if (ret == -1) {
		printf("capture device open failed: %s\n", alsa_capture_device);
		goto exit;
	}

	hfp_tinymix_set(1, 3);

	ret = hfp_playback_device_open(&playbcak_handle, bt_playback_device, alsa_config);
	if (ret == -1) {
		printf("playback device open failed: %s\n", bt_playback_device);
		goto exit;
	}

	g_duplex_control.bt_duplex_opened = true;

	while (g_duplex_control.bt_duplex_opened) {
		err = snd_pcm_readi(capture_handle, buffer , READ_FRAME_256);
		if (!g_duplex_control.bt_duplex_opened)
			goto exit;

		if (err != READ_FRAME_256)
			printf("=====read frame error = %d=====\n", err);

		if (err == -EPIPE)
			printf("Overrun occurred: %d\n", err);

		if (err < 0) {
			err = snd_pcm_recover(capture_handle, err, 0);
			// Still an error, need to exit.
			if (err < 0) {
				printf( "Error occured while recording: %s\n", snd_strerror(err));
				usleep(100 * 1000);
				hfp_pcm_close(capture_handle);
				hfp_pcm_close(playbcak_handle);
				goto device_open;
			}
		}

		err = snd_pcm_writei(playbcak_handle, buffer, READ_FRAME_256);
		if (!g_duplex_control.bt_duplex_opened)
			goto exit;

		if (err != READ_FRAME_256)
			printf("====write frame error = %d===\n",err);

		if (err == -EPIPE)
			printf("Underrun occurred from write: %d\n", err);

		if (err < 0) {
			err = snd_pcm_recover(playbcak_handle, err, 0);
			// Still an error, need to exit.
			if (err < 0) {
				printf( "Error occured while writing: %s\n", snd_strerror(err));
				usleep(100 * 1000);
				hfp_pcm_close(capture_handle);
				hfp_pcm_close(playbcak_handle);
				goto device_open;
			}
		}
	}

exit:
	hfp_pcm_close(capture_handle);
	hfp_pcm_close(playbcak_handle);

	printf("Exit app hs bt pcm playback thread\n");
	pthread_exit(0);
}

static int hfp_open_alsa_duplex()
{
	if (!g_duplex_control.alsa_duplex_opened) {
		if (pthread_create(&g_duplex_control.alsa_tid, NULL, hfp_alsa_playback, NULL)) {
			printf("Create alsa duplex thread failed\n");
			return -1;
		}
	} else {
		printf("hfp_open_alsa_duplex: alsa duplex already open\n");
	}

	return 0;
}

static void hfp_close_alsa_duplex(void)
{
	printf("app_hs_close_alsa_duplex start\n");
	g_duplex_control.alsa_duplex_opened = false;
	if (g_duplex_control.alsa_tid) {
		pthread_join(g_duplex_control.alsa_tid, NULL);
		g_duplex_control.alsa_tid = 0;
	}

	printf("app_hs_close_alsa_duplex end\n");
}

static int hfp_open_bt_duplex()
{
	if (!g_duplex_control.bt_duplex_opened) {
		if (pthread_create(&g_duplex_control.bt_tid, NULL, hfp_bt_playback, NULL)) {
			printf("Create bt pcm duplex thread failed\n");
			return -1;
		}
	} else {
		printf("hfp_open_bt_duplex: bt duplex already open\n");
	}

	return 0;
}

static void hfp_close_bt_duplex(void)
{
	printf("app_hs_close_bt_duplex start\n");
	g_duplex_control.bt_duplex_opened = false;
	if (g_duplex_control.bt_tid) {
		pthread_join(g_duplex_control.bt_tid, NULL);
		g_duplex_control.bt_tid = 0;
	}

	printf("app_hs_close_bt_duplex end\n");
}

static int hfp_open_audio_duplex()
{
	if(sco_codec != BT_SCO_CODEC_CVSD && sco_codec != BT_SCO_CODEC_MSBC) {
		printf("%s: invalid sco codec type: %d\n", __func__, sco_codec);
		return -1;
	}

	if(hfp_open_alsa_duplex() < 0)
		return -1;

	if(hfp_open_bt_duplex() < 0)
		return -1;

	return 0;
}

static void hfp_close_audio_duplex()
{
	hfp_close_alsa_duplex();
	hfp_close_bt_duplex();
}

static void get_call_info(char *str)
{
	char number[20];
	char name[256];
	int  i = 0;
	char *p0=str;
	char *p1=number;
	char *p2=name;

	if(!str)
		return;

	while(*p0!='\0')
	{

		if(*p0=='\"')
		i++;
		if(i==1)
		{
			if(*p0!='\"')
			{*p1=*p0;p1++;}
		}
		if(i==3)
		{
			if(*p0!='\"')
			{*p2=*p0;p2++;}
		}
		p0++;
	}
	printf("Call info: %s\n", str);
	if(i>0)
	printf("Call number: %s\n", number);
	if(i>2)
	printf("Call name: %s\n", name);
}

int bt_test_hfp_hp_cb(const char *bd_addr, RK_BT_HFP_EVENT event, void *data)
{
	switch(event) {
		case  RK_BT_HFP_CONNECT_EVT:
			printf("+++++ BT HFP HP CONNECT(%s) +++++\n", bd_addr);
			printf("device platform is %s\n", rk_bt_get_dev_platform((char *)bd_addr) == RK_BT_DEV_PLATFORM_UNKNOWN ?
				"Unknown Platform" : "Apple IOS");
			break;
		case RK_BT_HFP_DISCONNECT_EVT:
			printf("+++++ BT HFP HP DISCONNECT(%s) +++++\n", bd_addr);
			break;
		case RK_BT_HFP_RING_EVT:
			printf("+++++ BT HFP HP RING(%s) +++++\n", bd_addr);
			break;
		case RK_BT_HFP_AUDIO_OPEN_EVT:
			printf("+++++ BT HFP AUDIO OPEN(%s) +++++\n", bd_addr);
			hfp_open_audio_duplex();
			break;
		case RK_BT_HFP_AUDIO_CLOSE_EVT:
			printf("+++++ BT HFP AUDIO CLOSE(%s) +++++\n", bd_addr);
			hfp_close_audio_duplex();
			break;
		case RK_BT_HFP_PICKUP_EVT:
			printf("+++++ BT HFP PICKUP(%s) +++++\n", bd_addr);
			break;
		case RK_BT_HFP_HANGUP_EVT:
			printf("+++++ BT HFP HANGUP(%s) +++++\n", bd_addr);
			break;
		case RK_BT_HFP_VOLUME_EVT:
		{
			unsigned short volume = *(unsigned short*)data;
			printf("+++++ BT HFP VOLUME CHANGE, volume: %d +++++\n", volume);
			break;
		}
		case RK_BT_HFP_BCS_EVT:
		{
			unsigned short codec_type = *(unsigned short*)data;
			printf("+++++ BT HFP BCS EVENT: %d(%s) +++++\n", codec_type,
				(codec_type == BT_SCO_CODEC_MSBC) ? "mSBC":"CVSD");
			sco_codec = (RK_BT_SCO_CODEC_TYPE)codec_type;
			break;
		}
		case RK_BT_HFP_CLIP_EVT:
		{
			printf("+++++ BT HFP CLIP EVENT(%s) +++++\n", bd_addr);
			get_call_info((char *)data);
			break;
		}

		case RK_BT_HFP_OUTGOING_CALL_DIAL_EVT:
			printf("+++++ BT HFP OUTGOING CALL DIAL EVT(%s) +++++\n", bd_addr);
			break;

		case RK_BT_HFP_OUTGOING_CALL_RING_EVT:
			printf("+++++ BT HFP OUTGOING CALL RING EVT(%s) +++++\n", bd_addr);
			break;

		case RK_BT_HFP_CLCC_EVT:
		{
			printf("+++++ BT HFP CLCC EVENT(%s) +++++\n", bd_addr);
			get_call_info((char *)data);
		}
		default:
			break;
	}

	return 0;
}

void bt_test_hfp_hp_open(char *data)
{
	int ret = 0;

	/* must be placed before rk_bt_hfp_open */
	rk_bt_hfp_register_callback(bt_test_hfp_hp_cb);

	/* only bsa: if enable cvsd, sco_codec must be set to BT_SCO_CODEC_CVSD */
	rk_bt_hfp_enable_cvsd();
	ret = rk_bt_hfp_open();
	if (ret < 0)
		printf("%s hfp open failed!\n", __func__);
}

void bt_test_hfp_hp_accept(char *data)
{
	int ret = 0;

	ret = rk_bt_hfp_pickup();
	if (ret < 0)
		printf("%s hfp accept failed!\n", __func__);
}

void bt_test_hfp_hp_hungup(char *data)
{
	int ret = 0;

	ret = rk_bt_hfp_hangup();
	if (ret < 0)
		printf("%s hfp hungup failed!\n", __func__);
}

void bt_test_hfp_hp_redial(char *data)
{
	int ret = 0;

	ret = rk_bt_hfp_redial();
	if (ret < 0)
		printf("%s hfp redial failed!\n", __func__);
}

void bt_test_hfp_hp_dial_number(char *data)
{
	rk_bt_hfp_dial_number(data);
}


void bt_test_hfp_hp_report_battery(char *data)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < 10; i++) {
		ret = rk_bt_hfp_report_battery(i);
		if (ret < 0) {
			printf("%s hfp report battery(%d) failed!\n", __func__, i);
			break;
		}

		sleep(1);
	}
}

void bt_test_hfp_hp_set_volume(char *data)
{
	int i;

	for(i = 0; i <= 15; i++) {
		if (rk_bt_hfp_set_volume(i) < 0) {
			printf("%s hfp set volume(%d) failed!\n", __func__, i);
			break;
		}
		sleep(2);
	}
}

void bt_test_hfp_hp_close(char *data)
{
	rk_bt_hfp_close();
}

void bt_test_hfp_hp_disconnect(char *data)
{
	rk_bt_hfp_disconnect();
}

void bt_test_hfp_sink_open(char *data)
{
	rk_bt_sink_register_volume_callback(bt_sink_volume_callback);
	rk_bt_sink_register_track_callback(bt_sink_track_change_callback);
	rk_bt_sink_register_position_callback(bt_sink_position_change_callback);
	rk_bt_sink_register_callback(bt_sink_callback);
	rk_bt_hfp_register_callback(bt_test_hfp_hp_cb);
	rk_bt_hfp_sink_open();
}


/******************************************/
/*                 OBEX                   */
/******************************************/
static void  obex_pbap_event_cb(const char *bd_addr, RK_BT_OBEX_STATE state)
{
	switch(state) {
		case RK_BT_OBEX_CONNECT_FAILED:
			printf("----- RK_BT_OBEX_CONNECT_FAILED(%s) -----\n", bd_addr);
			break;

		case RK_BT_OBEX_CONNECTED:
			printf("----- RK_BT_OBEX_CONNECTED(%s) -----\n", bd_addr);
			break;

		case RK_BT_OBEX_DISCONNECT_FAILED:
			printf("----- RK_BT_OBEX_DISCONNECT_FAILED(%s) -----\n", bd_addr);
			break;

		case RK_BT_OBEX_DISCONNECTED:
			printf("----- RK_BT_OBEX_DISCONNECTED(%s) -----\n", bd_addr);
			break;

		case RK_BT_OBEX_TRANSFER_ACTIVE:
			printf("----- RK_BT_OBEX_TRANSFER_ACTIVE(%s) -----\n", bd_addr);
			break;

		case RK_BT_OBEX_TRANSFER_COMPLETE:
			printf("----- RK_BT_OBEX_TRANSFER_COMPLETE(%s) -----\n", bd_addr);
			break;
	}
}

void bt_test_obex_init(char *data)
{
	rk_bt_obex_init(data);
}

void bt_test_obex_pbap_init(char *data)
{
	rk_bt_obex_register_status_cb(obex_pbap_event_cb);
	rk_bt_obex_pbap_init();
}

void bt_test_obex_pbap_connect(char *data)
{
	rk_bt_obex_pbap_connect(data);
}

void bt_test_obex_pbap_get_pb_vcf(char *data)
{
	rk_bt_obex_pbap_get_vcf("pb", "/data/pb.vcf");
}

void bt_test_obex_pbap_get_ich_vcf(char *data)
{
	rk_bt_obex_pbap_get_vcf("ich", "/data/ich.vcf");
}

void bt_test_obex_pbap_get_och_vcf(char *data)
{
	rk_bt_obex_pbap_get_vcf("och", "/data/och.vcf");
}

void bt_test_obex_pbap_get_mch_vcf(char *data)
{
	rk_bt_obex_pbap_get_vcf("mch", "/data/mch.vcf");
}

void bt_test_obex_pbap_get_spd_vcf(char *data)
{
	rk_bt_obex_pbap_get_vcf("spd", "/data/spd.vcf");
}

void bt_test_obex_pbap_get_fav_vcf(char *data)
{
	rk_bt_obex_pbap_get_vcf("fav", "/data/fav.vcf");
}

void bt_test_obex_pbap_disconnect(char *data)
{
	rk_bt_obex_pbap_disconnect(data);
}

void bt_test_obex_pbap_deinit(char *data)
{
	rk_bt_obex_pbap_deinit();
}

void bt_test_obex_deinit(char *data)
{
	rk_bt_obex_deinit();
}

/******************************************/
/*                 PAN                    */
/******************************************/
static void pan_event_cb(RK_BT_PAN_EVENT event, char *bd_addr)
{
	switch(event) {
		case RK_BT_PAN_CONNECT_FAILED:
			printf("----- RK_BT_PAN_CONNECT_FAILED(%s) -----\n", bd_addr);
			break;

		case RK_BT_PAN_CONNECT:
			printf("----- RK_BT_PAN_CONNECT(%s) -----\n", bd_addr);
			break;

		case RK_BT_PAN_DISCONNECT:
			printf("----- RK_BT_PAN_DISCONNECT(%s) -----\n", bd_addr);
			break;
	}
}

void bt_test_pan_init(char *data)
{
	rk_bt_pan_register_event_cb(pan_event_cb);
	rk_bt_pan_open();
}

void bt_test_pan_deinit(char *data)
{
	rk_bt_pan_close();
}

void bt_test_pan_connect(char *data)
{
	rk_bt_pan_connect(data);
}

void bt_test_pan_disconnect(char *data)
{
	rk_bt_pan_disconnect(data);
}
