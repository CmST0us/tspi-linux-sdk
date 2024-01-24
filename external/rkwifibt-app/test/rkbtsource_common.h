#ifndef __RK_BT_SOURCE_COMMON__
#define __RK_BT_SOURCE_COMMON__

#ifndef msleep
#define msleep(x) usleep(x * 1000)
#endif

#define PRINT_FLAG_RKBTSOURCE "[RK_BT_RKBTSOURCE]"
#define PRINT_FLAG_SCAN "[RK_BT_SCAN]"
#define PRINT_FLAG_ERR "[RK_BT_ERROR]"
#define PRINT_FLAG_SUCESS "[RK_BT_SUCESS]"

#define RKBTSOURCE_SERVER_SOCKET_PATH "/tmp/rockchip_btsource_server"
#define RKBTSOURCE_CLIENT_SOCKET_PATH "/tmp/rockchip_btsource_client"

/* The order must correspond to bt_command_table */
enum rk_bt_source_cmd {
	RK_BT_INIT = 1,
	RK_BT_DEINIT,
	RK_BT_SCAN_ON,
	RK_BT_SCAN_OFF,
	RK_BT_SOURCE_OPEN,
	RK_BT_SOURCE_CLOSE,
	RK_BT_SOURCE_CONNECT,
	RK_BT_SOURCE_DISCONNECT,
	RK_BT_SOURCE_REMOVE,
	RK_BT_GET_PAIRED_DEVICES,
	RK_BT_SPP_LISTEN,
	RK_BT_SPP_CONNECT,
	RK_BT_SPP_WRITE,
	RK_BT_SPP_CLOSE,
	RK_BT_PING,
};

enum rk_bt_evt {
	RK_BT_EVT_INIT_OK = 1,
	RK_BT_EVT_DEINIT_OK,
	RK_BT_EVT_SCAN_ON,
	RK_BT_EVT_SCANNING,
	RK_BT_EVT_SCAN_OFF,
	RK_BT_EVT_SCAN_ADD_DEV,
	RK_BT_EVT_SCAN_CHANGE_DEV,
	RK_BT_EVT_SCAN_REMOVE_DEV,
	RK_BT_EVT_SOURCE_OPEN,
	RK_BT_EVT_SOURCE_CLOSE,
	RK_BT_EVT_SOURCE_CONNECTED,
	RK_BT_EVT_SOURCE_DISCONNECTED,
	RK_BT_EVT_SOURCE_CONNECT_FAILED,
	RK_BT_EVT_SOURCE_REMOVED,
	RK_BT_EVT_SPP_CONNECTED,
	RK_BT_EVT_SPP_DISCONNECTED,
	RK_BT_EVT_SPP_CONNECT_FAILED,
	RK_BT_EVT_SPP_CLIENT_FD,
	RK_BT_EVT_PAIRED_DEV,
	RK_BT_EVT_PONG,
};

typedef struct {
	enum rk_bt_evt evt_id;
	const char *desc;
} bt_evt_str_t;

static bt_evt_str_t bt_evt_str_table[] = {
	{0, "rkbt null"},
	{RK_BT_EVT_INIT_OK, "rkbt init ok"},
	{RK_BT_EVT_DEINIT_OK, "rkbt deinit ok"},
	{RK_BT_EVT_SCAN_ON, "rkbt scan on ok"},
	{RK_BT_EVT_SCANNING, "rkbt scaning"},
	{RK_BT_EVT_SCAN_OFF, "rkbt scan off"},
	{RK_BT_EVT_SCAN_ADD_DEV, "rkbt scan found dev"},
	{RK_BT_EVT_SCAN_REMOVE_DEV, "rkbt scan disapper dev"},
	{RK_BT_EVT_SOURCE_OPEN, "rkbt a2dp source open"},
	{RK_BT_EVT_SOURCE_CONNECTED, "rkbt connect ok"},
	{RK_BT_EVT_SOURCE_DISCONNECTED, "rkbt disconnect ok"},
	{RK_BT_EVT_SOURCE_REMOVED, "rkbt remove ok"},
};

#define RKBTSOURCE_MSG_INIT_OK "rkbt init ok"
#define RKBTSOURCE_MSG_DEINIT_OK "rkbt deinit ok"
#define RKBTSOURCE_MSG_SCAN_OFF "rkbt scan off"
#define RKBTSOURCE_MSG_CONNECT_OK "rkbt connect ok"
#define RKBTSOURCE_MSG_CONNECT_ERR "rkbt connect err"
#define RKBTSOURCE_MSG_DISCONNECT_OK "rkbt disconnect ok"
#define RKBTSOURCE_MSG_REMOVE_OK "rkbt remove ok"
#define RKBTSOURCE_MSG_ERR "rkbt exec err"

typedef struct {
	char name[128];
	char addr[18];
	char role;
	char change;
	int rssi;
	int is_connected;
} scan_device_t;

enum rk_bt_msg_type {
	RK_BT_CMD = 1,
	RK_BT_EVT,
};

typedef struct {
	enum rk_bt_msg_type type;
	char id;
	char addr[24];
	char data[24];
	scan_device_t dev;
} bt_msg_t;

typedef struct {
	const char *cmd;
	int cmd_id;
} bt_commands_t;

static bt_commands_t bt_command_tbs[] = {
	{"", NULL},
	{"init bluetooth", RK_BT_INIT},
	{"deinit bluetooth", RK_BT_DEINIT},
	{"open a2dp source", RK_BT_SOURCE_OPEN},
	{"close a2dp source", RK_BT_SOURCE_OPEN},
	{"scan on", RK_BT_SCAN_ON},
	{"scan off", RK_BT_SCAN_OFF},
	{"connect [address]", RK_BT_SOURCE_CONNECT},
	{"disconnect [address]", RK_BT_SOURCE_DISCONNECT},
	{"remove [address]", RK_BT_SOURCE_REMOVE},
	{"get paired devices", RK_BT_GET_PAIRED_DEVICES},
	{"spp listen", RK_BT_SPP_LISTEN},
	{"spp connect", RK_BT_SPP_CONNECT},
	{"spp write", RK_BT_SPP_WRITE},
	{"spp close", RK_BT_SPP_CLOSE},
	{"ping", RK_BT_PING},
};

#define BT_SHARE_MSG_MAGIC 0x42544D41
#define BT_CMD_CONNECT 1
#define BT_CMD_DISCONNECT 2
#define BT_CMD_SCAN_DEV 3
#define BT_CMD_SCAN_START 4
#define BT_CMD_SCAN_STOP 5

typedef struct{
	char dev_name[128];
	char addr[18];
	int signal_qua;
	char online;
	char connect;
} bluetooch_dev_t;

#endif
