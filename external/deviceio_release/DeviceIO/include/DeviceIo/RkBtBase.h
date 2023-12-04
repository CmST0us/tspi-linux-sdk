#ifndef __BT_BASE_H__
#define __BT_BASE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MXA_ADV_DATA_LEN 32
#define DEVICE_ADDR_LEN  6

#define BT_ATT_DEFAULT_LE_MTU    23
#define BT_ATT_MAX_LE_MTU        517
#define BT_ATT_MAX_VALUE_LEN     512
#define BT_ATT_HEADER_LEN        3

#define RK_BT_TRANSPORT_UNKNOWN   0
#define RK_BT_TRANSPORT_BR_EDR    1
#define RK_BT_TRANSPORT_LE        2

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct {
#define UUID_16     2
#define UUID_32     4
#define UUID_128    16

	uint16_t len; //byte
	const char *uuid;
} Ble_Uuid_Type_t;

enum {
	BLE_ADVDATA_TYPE_USER = 0,
	BLE_ADVDATA_TYPE_SYSTEM
};

typedef enum {
	PLAYROLE_TYPE_UNKNOWN,
	PLAYROLE_TYPE_SOURCE,
	PLAYROLE_TYPE_SINK,
} RK_BT_PLAYROLE_TYPE;

/*BT state*/
typedef enum {
	RK_BT_STATE_OFF,
	RK_BT_STATE_ON,
	RK_BT_STATE_TURNING_ON,
	RK_BT_STATE_TURNING_OFF,
} RK_BT_STATE;

typedef enum {
	RK_BT_BOND_STATE_NONE,
	RK_BT_BOND_STATE_BONDING,
	RK_BT_BOND_STATE_BONDED,
} RK_BT_BOND_STATE;

typedef enum {
	SCAN_TYPE_AUTO, //LE, BR/EDR, or both
	SCAN_TYPE_BREDR,
	SCAN_TYPE_LE,
	SCAN_TYPE_PAN,
	SCAN_TYPE_SPP
} RK_BT_SCAN_TYPE;

/*BT discovery state*/
typedef enum {
	RK_BT_DISC_STARTED,
	RK_BT_DISC_START_FAILED,
	RK_BT_DISC_STOPPED_AUTO,
	RK_BT_DISC_STOPPED_BY_USER,
} RK_BT_DISCOVERY_STATE;

typedef enum {
	RK_BT_DEV_PLATFORM_UNKNOWN = 0, /* unknown platform */
	RK_BT_DEV_PLATFORM_IOS,         /* Apple iOS */
} RK_BT_DEV_PLATFORM_TYPE;

typedef struct {
	Ble_Uuid_Type_t server_uuid;
	Ble_Uuid_Type_t chr_uuid[12];
	uint8_t chr_cnt;
	const char *ble_name;
	uint8_t ble_addr[DEVICE_ADDR_LEN];
	uint8_t advData[MXA_ADV_DATA_LEN];
	uint8_t advDataLen;
	uint8_t respData[MXA_ADV_DATA_LEN];
	uint8_t respDataLen;
	uint8_t advDataType;
	/* recevice data */
	void (*cb_ble_recv_fun)(const char *uuid, char *data, int len);
	/* full data */
	void (*cb_ble_request_data)(const char *uuid);
} RkBleContent;

typedef struct {
	RkBleContent ble_content;
	const char *bt_name;
	const char *bt_addr;
} RkBtContent;

typedef struct scaned_dev {
	char *remote_address;
	char *remote_name;
	unsigned int cod; //class of device
	bool is_connected;
	struct scaned_dev *next;
} RkBtScanedDevice;

typedef void (*RK_BT_STATE_CALLBACK)(RK_BT_STATE state);
typedef void (*RK_BT_BOND_CALLBACK)(const char *bd_addr, const char *name, RK_BT_BOND_STATE state);
typedef void (*RK_BT_DISCOVERY_CALLBACK)(RK_BT_DISCOVERY_STATE state);
typedef void (*RK_BT_DEV_FOUND_CALLBACK)(const char *address, const char *name, unsigned int bt_class, int rssi);
typedef void (*RK_BT_NAME_CHANGE_CALLBACK)(const char *bd_addr, const char *name);
typedef void (*RK_BT_MTU_CALLBACK)(const char *bd_addr, unsigned int mtu);

void rk_bt_register_state_callback(RK_BT_STATE_CALLBACK cb);
void rk_bt_register_bond_callback(RK_BT_BOND_CALLBACK cb);
void rk_bt_register_discovery_callback(RK_BT_DISCOVERY_CALLBACK cb);
void rk_bt_register_dev_found_callback(RK_BT_DEV_FOUND_CALLBACK cb);
void rk_bt_register_name_change_callback(RK_BT_NAME_CHANGE_CALLBACK cb);
int rk_bt_init(RkBtContent *p_bt_content);
int rk_bt_deinit(void);
int rk_bt_is_connected(void);
int rk_bt_set_class(int value);
int rk_bt_set_sleep_mode(void);
int rk_bt_enable_reconnect(int value);
int rk_bt_start_discovery(unsigned int mseconds, RK_BT_SCAN_TYPE scan_type);
int rk_bt_cancel_discovery();
bool rk_bt_is_discovering();
int rk_bt_get_scaned_devices(RkBtScanedDevice **dev_list, int *count);
int rk_bt_free_scaned_devices(RkBtScanedDevice *dev_list);
void rk_bt_display_devices();
int rk_bt_pair_by_addr(char *addr);
int rk_bt_unpair_by_addr(char *addr);
int rk_bt_set_device_name(char *name);
int rk_bt_get_device_name(char *name, int len);
int rk_bt_get_device_addr(char *addr, int len);
int rk_bt_get_paired_devices(RkBtScanedDevice **dev_list, int *count);
int rk_bt_free_paired_devices(RkBtScanedDevice *dev_list);
void rk_bt_display_paired_devices();
int rk_bt_set_visibility(const int visiable, const int connectable);
bool rk_bt_get_connected_properties(char *addr);
RK_BT_PLAYROLE_TYPE rk_bt_get_playrole_by_addr(char *addr);
RK_BT_DEV_PLATFORM_TYPE rk_bt_get_dev_platform(char *addr);
int rk_bt_get_eir_data(char *address, char *eir_data, int len);

//for bsa, default /usr/bin/bsa_server.sh
void rk_bt_set_bsa_server_path(char *path);

//for bsa, bluez don't support
//0: TRANSPORT_UNKNOWN, 1: TRANSPORT_BR_EDR, 2: TRANSPORT_LE
int rk_bt_read_remote_device_name(char *addr, int transport);

#ifdef __cplusplus
}
#endif

#endif /* __BT_BASE_H__ */
