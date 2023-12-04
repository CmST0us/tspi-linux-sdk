#ifndef __BLUETOOTH_OBEX_H__
#define __BLUETOOTH_OBEX_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	RK_BT_OBEX_CONNECT_FAILED,
	RK_BT_OBEX_CONNECTED,
	RK_BT_OBEX_DISCONNECT_FAILED,
	RK_BT_OBEX_DISCONNECTED,
	RK_BT_OBEX_TRANSFER_ACTIVE,
	RK_BT_OBEX_TRANSFER_COMPLETE,
} RK_BT_OBEX_STATE;

typedef void (*RK_BT_OBEX_STATE_CALLBACK)(const char *bd_addr, RK_BT_OBEX_STATE state);

void rk_bt_obex_register_status_cb(RK_BT_OBEX_STATE_CALLBACK cb);
int rk_bt_obex_init(char *path);
int rk_bt_obex_pbap_init(void);
int rk_bt_obex_pbap_connect(char *btaddr);
int rk_bt_obex_pbap_get_vcf(char *dir_name, char *dir_file);
int rk_bt_obex_pbap_disconnect(char *btaddr);
int rk_bt_obex_pbap_deinit(void);
int rk_bt_obex_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_OBEX_H__ */
