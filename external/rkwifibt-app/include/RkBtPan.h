#ifndef __BLUETOOTH_PAN_H__
#define __BLUETOOTH_PAN_H__

#include <RkBtBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RK_BT_PAN_CONNECT_FAILED,
	RK_BT_PAN_CONNECT,
	RK_BT_PAN_DISCONNECT
} RK_BT_PAN_EVENT;

typedef void (*RK_BT_PAN_EVENT_CALLBACK)(RK_BT_PAN_EVENT event, char *bd_addr);

void rk_bt_pan_register_event_cb(RK_BT_PAN_EVENT_CALLBACK cb);
int rk_bt_pan_open();
int rk_bt_pan_close();
int rk_bt_pan_connect(char *address);
int rk_bt_pan_disconnect(char *address);

#ifdef __cplusplus
}
#endif

#endif
