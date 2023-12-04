#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <stdbool.h>

#include "DeviceIo/Rk_wifi.h"
#include "DeviceIo/Rk_softap.h"

struct wifi_info {
	int ssid_len;
	char ssid[512];
	int psk_len;
	char psk[512];
};

static void printf_system_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	printf("--- time: %lld ms ---\n", tv.tv_sec * 1000 + tv.tv_usec/1000 + tv.tv_usec%1000);
}

static void ping_test()
{
	char line[2048];

	while(1) {
		sleep(1);
#if 0
		if (RK_wifi_ping("www.baidu.com")) {
			printf("ping ok\n");
			printf_system_time();
			rk_wifi_getConnectionInfo(NULL);
			break;
		}
//#else
		memset(line, 0, sizeof(line));
		RK_shell_exec("ping www.baidu.com -c 1", line, sizeof(line));
		//RK_shell_exec("ping 8.8.8.8 -c 1", line, sizeof(line));
		//usleep(100000);
		printf("line: %s\n", line);
		if (strstr(line, "PING www.baidu.com") && strstr(line, "bytes from")) {
		//if (strstr(line, "PING 8.8.8.8") && strstr(line, "bytes from")) {
			printf("ping ok\n");
			printf_system_time();
			rk_wifi_getConnectionInfo(NULL);
			break;
		}
#endif
		usleep(100000);
	}
}

static void printf_connect_info(RK_WIFI_INFO_Connection_s *info)
{
	if(!info)
		return;

	printf("	id: %d\n", info->id);
	printf("	bssid: %s\n", info->bssid);
	printf("	ssid: %s\n", info->ssid);
	printf("	freq: %d\n", info->freq);
	printf("	mode: %s\n", info->mode);
	printf("	wpa_state: %s\n", info->wpa_state);
	printf("	ip_address: %s\n", info->ip_address);
	printf("	mac_address: %s\n", info->mac_address);
}

/*****************************************************************
 *                     wifi config                               *
 *****************************************************************/
static volatile bool rkwifi_gonff = false;
static RK_WIFI_RUNNING_State_e wifi_state = 0;
static int rk_wifi_state_callback(RK_WIFI_RUNNING_State_e state, RK_WIFI_INFO_Connection_s *info)
{
	printf("%s state: %d\n", __func__, state);
	printf_connect_info(info);

	wifi_state = state;
	if (state == RK_WIFI_State_CONNECTED) {
		printf("RK_WIFI_State_CONNECTED\n");
		//ping_test();
		//RK_wifi_get_connected_ap_rssi();
	} else if (state == RK_WIFI_State_CONNECTFAILED) {
		printf("RK_WIFI_State_CONNECTFAILED\n");
	} else if (state == RK_WIFI_State_CONNECTFAILED_WRONG_KEY) {
		printf("RK_WIFI_State_CONNECTFAILED_WRONG_KEY\n");
	} else if (state == RK_WIFI_State_OPEN) {
		rkwifi_gonff = true;
		printf("RK_WIFI_State_OPEN\n");
	} else if (state == RK_WIFI_State_OFF) {
		rkwifi_gonff = false;
		printf("RK_WIFI_State_OFF\n");
	} else if (state == RK_WIFI_State_DISCONNECTED) {
		printf("RK_WIFI_State_DISCONNECTED\n");
	} else if (state == RK_WIFI_State_SCAN_RESULTS) {
		char *scan_r;
		printf("RK_WIFI_State_SCAN_RESULTS\n");
		scan_r = RK_wifi_scan_r();
		//system("cat /tmp/scan_r.tmp");
		//printf("%s\n", scan_r);
		free(scan_r);
	}

	return 0;
}

static void *rk_wifi_config_thread(void *arg)
{
	struct wifi_info *info;

	printf("rk_config_wifi_thread\n");

	prctl(PR_SET_NAME,"rk_config_wifi_thread");

	wifi_state = 0;

	info = (struct wifi_info *) arg;
	RK_wifi_register_callback(rk_wifi_state_callback);
	RK_wifi_connect(info->ssid, info->psk);

	printf("Exit wifi config thread\n");
	return NULL;
}

/*****************************************************************
 *                     airkiss wifi config test                  *
 *****************************************************************/
void rk_wifi_airkiss_start(void *data)
{
	int err  = 0;
	struct wifi_info info;
	pthread_t tid = 0;

	memset(&info, 0, sizeof(struct wifi_info));

	printf("===== %s =====\n", __func__);

	//if(RK_wifi_airkiss_start(info.ssid, info.psk) < 0)
	//	return;

	wifi_state = 0;

	err = pthread_create(&tid, NULL, rk_wifi_config_thread, &info);
	if (err) {
		printf("Error - pthread_create() return code: %d\n", err);
		return;
	}

	while (!wifi_state)
		sleep(1);
}

void rk_wifi_airkiss_stop(void *data)
{
	//RK_wifi_airkiss_stop();
}

/*****************************************************************
 *                     softap wifi config test                   *
 *****************************************************************/
static int rk_wifi_softap_state_callback(RK_SOFTAP_STATE state, const char* data)
{
	switch (state) {
	case RK_SOFTAP_STATE_CONNECTTING:
		printf("RK_SOFTAP_STATE_CONNECTTING\n");
		break;
	case RK_SOFTAP_STATE_DISCONNECT:
		printf("RK_SOFTAP_STATE_DISCONNECT\n");
		break;
	case RK_SOFTAP_STATE_FAIL:
		printf("RK_SOFTAP_STATE_FAIL\n");
		break;
	case RK_SOFTAP_STATE_SUCCESS:
		printf("RK_SOFTAP_STATE_SUCCESS\n");
		break;
	default:
		break;
	}

	return 0;
}

void rk_wifi_softap_start(void *data)
{
	RK_softap_register_callback(rk_wifi_softap_state_callback);
	if (0 != RK_softap_start("Rockchip-SoftAp", RK_SOFTAP_TCP_SERVER)) {
		return;
	}
}

void rk_wifi_softap_stop(void *data)
{
	RK_softap_stop();
}

void _rk_wifi_open(void *data)
{
	RK_wifi_register_callback(rk_wifi_state_callback);

	if (RK_wifi_enable(1) < 0)
		printf("RK_wifi_enable 1 fail!\n");
}

void rk_wifi_close(void *data)
{
	if (RK_wifi_enable(0) < 0)
		printf("RK_wifi_enable 0 fail!\n");
}

static int wifi_cnt = 0;
void *wifi_test_onff_thread(void *arg)
{
	while (1) {
		//open
		RK_wifi_register_callback(rk_wifi_state_callback);
		if (RK_wifi_enable(1) < 0)
			printf("RK_wifi_enable 1 fail!\n");

		while (rkwifi_gonff == false) {
			sleep(1);
			printf("%s: RKWIFI TURNING ON ...\n", __func__);
		}

		//scan
		RK_wifi_scan();
		sleep(1);

		//close
		printf("%s: RKWIFI DEINIT\n", __func__);
		if (RK_wifi_enable(0) < 0)
			printf("RK_wifi_enable 0 fail!\n");

		while (rkwifi_gonff == true) {
			sleep(1);
			printf("%s: RKWIFI TURNING OFF ...\n", __func__);
		}
		printf("%s: RKWIFI TURN ONOFF CNT: [%d] \n", __func__, wifi_cnt++);
	}
}

static pthread_t rkwifi_init_thread = 0;
void rk_wifi_openoff_test(char *data)
{
	printf("%s: ", __func__);

	if (rkwifi_init_thread) {
		printf("rkwifi_init_thread already exist\n");
		return;
	}

	if (pthread_create(&rkwifi_init_thread, NULL, wifi_test_onff_thread, NULL)) {
		printf("Createrkwifi_init_thread failed\n");
		return;
	}
}

void rk_wifi_open(char *data)
{
	printf("%s: ", __func__);

	RK_wifi_register_callback(rk_wifi_state_callback);
	if (RK_wifi_enable(1) < 0)
		printf("RK_wifi_enable 1 fail!\n");
}

//9 input fish1:rk12345678
void rk_wifi_connect(void *data)
{
	char *ssid = NULL, *psk = NULL;

	if(data == NULL) {
		printf("%s: invalid input\n", __func__);
		return;
	}

	ssid = strtok(data, ":");
	if(ssid)
		psk = strtok(NULL, ":");

	if (RK_wifi_connect(ssid, psk) < 0)
		printf("RK_wifi_connect1 fail!\n");
}

void rk_wifi_connect1(void *data)
{
	char *ssid = NULL, *psk = NULL;

	if (RK_wifi_connect1("1a~!@#%^*()=+]['\\\\\\\":\\/?.>,<", "12345678", WPA, 1) < 0)
		printf("RK_wifi_connect1 fail!\n");

	return;

	if(data == NULL) {
		printf("%s: invalid input\n", __func__);
		return;
	}

	ssid = strtok(data, ":");
	if(ssid)
		psk = strtok(NULL, ":");

	if (RK_wifi_connect(ssid, psk) < 0)
		printf("RK_wifi_connect1 fail!\n");
}

void rk_wifi_ping(void *data)
{
	if (!RK_wifi_ping("www.baidu.com"))
		printf("RK_wifi_ping fail!\n");
}

void rk_wifi_scan(void *data)
{
	if (RK_wifi_scan() < 0)
		printf("RK_wifi_scan fail!\n");
}

void rk_wifi_getSavedInfo(void *data)
{
	RK_WIFI_SAVED_INFO_s *wsi;
	int ap_cnt = 0;

	RK_wifi_getSavedInfo(&wsi, &ap_cnt);
	for (int i = 0; i < ap_cnt; i++) {
		printf("id: %d, name: %s, bssid: %s, state: %s\n",
					wsi[i].id,
					wsi[i].ssid,
					wsi[i].bssid,
					wsi[i].state);
	}

	free(wsi);
}

void rk_wifi_getConnectionInfo(void *data)
{
	RK_WIFI_INFO_Connection_s info;

	if(!RK_wifi_running_getConnectionInfo(&info))
		printf_connect_info(&info);
}

void rk_wifi_connect_with_ssid(void *data)
{
	if(data == NULL) {
		printf("%s: ssid is null\n", __func__);
		return;
	}

	if (RK_wifi_connect_with_ssid(data) < 0)
		printf("RK_wifi_connect_with_ssid fail!\n");
}

void rk_wifi_cancel(void *data)
{
	if (RK_wifi_cancel() < 0)
		printf("RK_wifi_cancel fail!\n");
}

void rk_wifi_forget_with_ssid(void *data)
{
	if(data == NULL) {
		printf("%s: ssid is null\n", __func__);
		return;
	}

	if (RK_wifi_forget_with_ssid(data) < 0) {
		printf("rk_wifi_forget_with_ssid fail!\n");
	}
}

void rk_wifi_disconnect(void *data)
{
	RK_wifi_disconnect_network();
}
