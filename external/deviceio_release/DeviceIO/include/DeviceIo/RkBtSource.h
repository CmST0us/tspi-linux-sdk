#ifndef __BLUETOOTH_SOURCE_H__
#define __BLUETOOTH_SOURCE_H__

#include <DeviceIo/RkBtBase.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BT_SOURCE_SCAN_DEVICES_CNT	30
typedef struct _bt_device_info {
    char name[128]; // bt name
    char address[17]; // bt address
    bool rssi_valid;
    int rssi;
    /* Audio Sink? Audio Source? Unknown? Audio Sink | Audio Source(Some devices have sink and source attributes) */
    char playrole[25];
} BtDeviceInfo;

/*
 * Specify Bluetooth scan parameters.
 * mseconds: How long is the scan, in milliseconds.
 * item_cnt: Number of devices that have been scanned.
 * device_list: Save scan results.
 */
typedef struct _bt_scan_parameter {
    unsigned int mseconds;
    unsigned char item_cnt;
    BtDeviceInfo devices[BT_SOURCE_SCAN_DEVICES_CNT];
} BtScanParam;

typedef enum {
	BT_SOURCE_EVENT_CONNECT_FAILED,
	BT_SOURCE_EVENT_CONNECTED,
	BT_SOURCE_EVENT_DISCONNECT_FAILED,
	BT_SOURCE_EVENT_DISCONNECTED,
	BT_SOURCE_EVENT_REMOVE_FAILED,
	BT_SOURCE_EVENT_REMOVED,
	BT_SOURCE_EVENT_RC_PLAY,    /* remote control command */
	BT_SOURCE_EVENT_RC_STOP,
	BT_SOURCE_EVENT_RC_PAUSE,
	BT_SOURCE_EVENT_RC_FORWARD,
	BT_SOURCE_EVENT_RC_BACKWARD,
	BT_SOURCE_EVENT_RC_VOL_UP,
	BT_SOURCE_EVENT_RC_VOL_DOWN,
	BT_SOURCE_EVENT_AUTO_RECONNECTING,
} RK_BT_SOURCE_EVENT;

typedef enum {
	BT_SOURCE_STATUS_DISCONNECTED = 0,
	BT_SOURCE_STATUS_CONNECTED,
} RK_BT_SOURCE_STATUS;

typedef void (*RK_BT_SOURCE_CALLBACK)(void *userdata, const char *bd_addr, const char *name, const RK_BT_SOURCE_EVENT event);

int rk_bt_source_register_status_cb(void *userdata, RK_BT_SOURCE_CALLBACK cb);
int rk_bt_source_auto_connect_start(void *userdata, RK_BT_SOURCE_CALLBACK cb);
int rk_bt_source_auto_connect_stop(void);
int rk_bt_source_open(void);
int rk_bt_source_close(void);
int rk_bt_source_get_device_name(char *name, int len);
int rk_bt_source_get_device_addr(char *addr, int len);
int rk_bt_source_get_status(RK_BT_SOURCE_STATUS *pstatus, char *name, int name_len, char *addr, int addr_len);
int rk_bt_source_scan(BtScanParam *data);
int rk_bt_source_connect_by_addr(char *address);
int rk_bt_source_disconnect_by_addr(char *address);
int rk_bt_source_disconnect();
int rk_bt_source_remove(char *address);
int rk_bt_source_resume(void);
int rk_bt_source_stop(void);
int rk_bt_source_pause(void);
int rk_bt_source_vol_up(void);
int rk_bt_source_vol_down(void);
int rk_bt_source_set_vol(int vol);

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_SOURCE_H__ */