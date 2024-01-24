#ifndef __BLUETOOTH_SPP_H__
#define __BLUETOOTH_SPP_H__

#include <RkBtBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RK_BT_SPP_STATE_IDLE = 0,
	RK_BT_SPP_STATE_CONNECT,
	RK_BT_SPP_STATE_CONNECT_FAILED,
	RK_BT_SPP_STATE_DISCONNECT
} RK_BT_SPP_STATE;

typedef void (*RK_BT_SPP_STATUS_CALLBACK)(RK_BT_SPP_STATE status);
typedef void (*RK_BT_SPP_RECV_CALLBACK)(char *data, int len);

int rk_bt_spp_register_status_cb(RK_BT_SPP_STATUS_CALLBACK cb);
int rk_bt_spp_register_recv_cb(RK_BT_SPP_RECV_CALLBACK cb);
int rk_bt_spp_open(char *data);
int rk_bt_spp_close(void);
int rk_bt_spp_get_state(RK_BT_SPP_STATE *pState);
int rk_bt_spp_write(char *data, int len);
int rk_bt_spp_connect(char *address);
int rk_bt_spp_disconnect(char *address);
int rk_bt_spp_listen();

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_SPP_H__ */

