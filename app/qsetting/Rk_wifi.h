#ifndef __RK_WIFI_H__
#define __RK_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RK_WIFI_VERSION "V1.1"

#define RK_WIFI_SAVED_INFO_MAX 10
#define SSID_BUF_LEN 64
#define BSSID_BUF_LEN 20
#define STATE_BUF_LEN 20

typedef enum {
	RK_WIFI_State_IDLE = 0,
	RK_WIFI_State_CONNECTING,
	RK_WIFI_State_CONNECTFAILED,
	RK_WIFI_State_CONNECTFAILED_WRONG_KEY,
	RK_WIFI_State_CONNECTED,
	RK_WIFI_State_DISCONNECTED,
	RK_WIFI_State_OPEN,
	RK_WIFI_State_OFF,
	RK_WIFI_State_SCAN_RESULTS,
	RK_WIFI_State_DHCP_OK,
} RK_WIFI_RUNNING_State_e;

typedef enum {
	NONE = 0,
	WPA,
	WEP
} RK_WIFI_CONNECTION_Encryp_e;

typedef struct {
	int id;
	char bssid[BSSID_BUF_LEN];
	char ssid[SSID_BUF_LEN];
	int freq;
	char mode[20];
	char wpa_state[20];
	char ip_address[20];
	char mac_address[20];
	int reason;
} RK_WIFI_INFO_Connection_s;

typedef struct {
	int id;
	char bssid[BSSID_BUF_LEN];
	char ssid[SSID_BUF_LEN];
	char state[STATE_BUF_LEN];
} RK_WIFI_SAVED_INFO_s;

typedef struct {
	int count;
	RK_WIFI_SAVED_INFO_s save_info[RK_WIFI_SAVED_INFO_MAX];
} RK_WIFI_SAVED_INFO;

typedef int(*RK_wifi_state_callback)(RK_WIFI_RUNNING_State_e state, RK_WIFI_INFO_Connection_s *info);

int RK_wifi_register_callback(RK_wifi_state_callback cb);
int RK_wifi_running_getState(RK_WIFI_RUNNING_State_e* pState);
int RK_wifi_running_getConnectionInfo(RK_WIFI_INFO_Connection_s* pInfo);
int RK_wifi_enable(int enable);
int RK_wifi_scan(void);
char* RK_wifi_scan_r(void);
char* RK_wifi_scan_r_sec(const unsigned int cols);
int RK_wifi_connect(char* ssid, const char* psk);
int RK_wifi_connect_wep(char* ssid, const char* psk);
int RK_wifi_connect1(char* ssid, const char* psk, const RK_WIFI_CONNECTION_Encryp_e encryp, const int hide);
int RK_wifi_disconnect_network(void);
int RK_wifi_restart_network(void);
int RK_wifi_set_hostname(const char* name);
int RK_wifi_get_hostname(char* name, int len);
int RK_wifi_get_mac(char *wifi_mac);
int RK_wifi_has_config(void);
int RK_wifi_ping(char *address);
int RK_wifi_recovery(void);
int RK_wifi_airkiss_start(char *ssid, char *password);
void RK_wifi_airkiss_stop(void);
int RK_wifi_forget_with_ssid(char *ssid);
int RK_wifi_cancel(void);
int RK_wifi_getSavedInfo(RK_WIFI_SAVED_INFO_s **pInfo, int *ap_cnt);
int RK_wifi_connect_with_ssid(char* ssid);
int RK_wifi_reset(void);
int RK_wifi_get_connected_ap_rssi(void);
char *RK_wifi_scan_for_softap(void);

#ifdef __cplusplus
}
#endif

#endif
