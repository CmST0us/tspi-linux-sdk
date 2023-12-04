#ifndef __BLUETOOTH_BLE_CLIENT_H__
#define __BLUETOOTH_BLE_CLIENT_H__

#include <DeviceIo/RkBtBase.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DESCRIBE_BUG_LEN	50
#define UUID_BUF_LEN		40
#define PATH_BUF_LEN		70
#define MAX_ATTR_VAL_LEN	512

#define SERVICE_COUNT_MAX	10
#define CHRC_COUNT_MAX		20
#define DESC_COUNT_MAX		5

/*
 * ATT attribute permission bitfield values. Permissions are grouped as
 * "Access", "Encryption", "Authentication", and "Authorization". A bitmask of
 * permissions is a byte that encodes a combination of these.
 */
#define BT_ATT_PERM_READ		0x01
#define BT_ATT_PERM_WRITE		0x02
#define BT_ATT_PERM_READ_ENCRYPT	0x04
#define BT_ATT_PERM_WRITE_ENCRYPT	0x08
#define BT_ATT_PERM_ENCRYPT		(BT_ATT_PERM_READ_ENCRYPT | \
					BT_ATT_PERM_WRITE_ENCRYPT)
#define BT_ATT_PERM_READ_AUTHEN		0x10
#define BT_ATT_PERM_WRITE_AUTHEN	0x20
#define BT_ATT_PERM_AUTHEN		(BT_ATT_PERM_READ_AUTHEN | \
					BT_ATT_PERM_WRITE_AUTHEN)
#define BT_ATT_PERM_AUTHOR		0x40
#define BT_ATT_PERM_NONE		0x80
#define BT_ATT_PERM_READ_SECURE		0x0100
#define BT_ATT_PERM_WRITE_SECURE	0x0200
#define BT_ATT_PERM_SECURE		(BT_ATT_PERM_READ_SECURE | \
					BT_ATT_PERM_WRITE_SECURE)

/* GATT Characteristic Properties Bitfield values */
#define BT_GATT_CHRC_PROP_BROADCAST			0x01 /* broadcast */
#define BT_GATT_CHRC_PROP_READ				0x02 /* read */
#define BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP		0x04 /* write-without-response */
#define BT_GATT_CHRC_PROP_WRITE				0x08 /* write */
#define BT_GATT_CHRC_PROP_NOTIFY			0x10 /* notify */
#define BT_GATT_CHRC_PROP_INDICATE			0x20 /* indicate */
#define BT_GATT_CHRC_PROP_AUTH				0x40 /* authenticated-signed-writes */
#define BT_GATT_CHRC_PROP_EXT_PROP			0x80 /* extended-properties */

/* GATT Characteristic Extended Properties Bitfield values */
#define BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE		0x01 /* reliable-write */
#define BT_GATT_CHRC_EXT_PROP_WRITABLE_AUX		0x02 /* writable-auxiliaries */
#define BT_GATT_CHRC_EXT_PROP_ENC_READ			0x04 /* encrypt-read */
#define BT_GATT_CHRC_EXT_PROP_ENC_WRITE			0x08 /* encrypt-write */
#define BT_GATT_CHRC_EXT_PROP_ENC	(BT_GATT_CHRC_EXT_PROP_ENC_READ | \
					BT_GATT_CHRC_EXT_PROP_ENC_WRITE)
#define BT_GATT_CHRC_EXT_PROP_AUTH_READ			0x10 /* encrypt-authenticated-read */
#define BT_GATT_CHRC_EXT_PROP_AUTH_WRITE		0x20 /* encrypt-authenticated-write */
#define BT_GATT_CHRC_EXT_PROP_AUTH	(BT_GATT_CHRC_EXT_PROP_AUTH_READ | \
					BT_GATT_CHRC_EXT_PROP_AUTH_WRITE)

typedef enum {
	RK_BLE_CLIENT_STATE_IDLE = 0,
	RK_BLE_CLIENT_STATE_CONNECT,
	RK_BLE_CLIENT_STATE_DISCONNECT,
	RK_BLE_CLIENT_WRITE_SUCCESS,
	RK_BLE_CLIENT_WRITE_ERROR,
} RK_BLE_CLIENT_STATE;

typedef struct {
	char describe[DESCRIBE_BUG_LEN];
	char path[PATH_BUF_LEN];
	char uuid[UUID_BUF_LEN];
} RK_BLE_CLIENT_DESC;

typedef struct {
	char describe[DESCRIBE_BUG_LEN];
	char path[PATH_BUF_LEN];
	char uuid[UUID_BUF_LEN];
	unsigned int props;
	unsigned int ext_props;
	unsigned int perm;
	bool notifying;
	int desc_cnt;
	RK_BLE_CLIENT_DESC desc[DESC_COUNT_MAX];
} RK_BLE_CLIENT_CHRC;

typedef struct {
	char describe[DESCRIBE_BUG_LEN];
	char path[PATH_BUF_LEN];
	char uuid[UUID_BUF_LEN];
	int chrc_cnt;
	RK_BLE_CLIENT_CHRC chrc[CHRC_COUNT_MAX];
} RK_BLE_CLIENT_SERVICE;

typedef struct {
	int service_cnt;
	RK_BLE_CLIENT_SERVICE service[SERVICE_COUNT_MAX];
} RK_BLE_CLIENT_SERVICE_INFO;

typedef void (*RK_BLE_CLIENT_STATE_CALLBACK)(const char *bd_addr, const char *name, RK_BLE_CLIENT_STATE state);
typedef void (*RK_BLE_CLIENT_RECV_CALLBACK)(const char *uuid, char *data, int len);

void rk_ble_client_register_state_callback(RK_BLE_CLIENT_STATE_CALLBACK cb);
void rk_ble_client_register_recv_callback(RK_BLE_CLIENT_RECV_CALLBACK cb);
void rk_ble_client_register_mtu_callback(RK_BT_MTU_CALLBACK cb);
int rk_ble_client_open(bool mtu_change);
void rk_ble_client_close(void);
RK_BLE_CLIENT_STATE rk_ble_client_get_state();
int rk_ble_client_get_service_info(char *address, RK_BLE_CLIENT_SERVICE_INFO *info);
int rk_ble_client_write(const char *uuid, char *data, int data_len);
int rk_ble_client_read(const char *uuid);
int rk_ble_client_connect(char *address);
int rk_ble_client_disconnect(char *address);
bool rk_ble_client_is_notifying(const char *uuid);

//is_indicate: only for bsa
int rk_ble_client_notify(const char *uuid, bool is_indicate, bool enable);

//get broadcast of the remote device
int rk_ble_client_get_eir_data(char *address, char *eir_data, int len);

//only for bsa, hci le write suggested default data length(27 byte)
int rk_ble_client_default_data_length();

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_BLE_CLIENT_H__ */
