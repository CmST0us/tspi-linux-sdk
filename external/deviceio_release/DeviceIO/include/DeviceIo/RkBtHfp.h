#ifndef __BLUETOOTH_HANDSFREE_H__
#define __BLUETOOTH_HANDSFREE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BT_SCO_CODEC_CVSD = 1,
    BT_SCO_CODEC_MSBC,
} RK_BT_SCO_CODEC_TYPE;

typedef enum {
    RK_BT_HFP_CONNECT_EVT,              /* HFP connection open */
    RK_BT_HFP_DISCONNECT_EVT,           /* HFP connection closed */
    RK_BT_HFP_RING_EVT,                 /* RING alert from AG */
    RK_BT_HFP_AUDIO_OPEN_EVT,           /* Audio connection open */
    RK_BT_HFP_AUDIO_CLOSE_EVT,          /* Audio connection closed */
    RK_BT_HFP_PICKUP_EVT,               /* Call has been picked up */
    RK_BT_HFP_HANGUP_EVT,               /* Call has been hung up */
    RK_BT_HFP_VOLUME_EVT,               /* Speaker volume change */
    RK_BT_HFP_BCS_EVT,                  /* Codec selection from AG */
    RK_BT_HFP_CLIP_EVT,                 /* Calling subscriber information from AG */
    RK_BT_HFP_OUTGOING_CALL_DIAL_EVT,   /* Outgoing call setup in dialing state */
    RK_BT_HFP_OUTGOING_CALL_RING_EVT,   /* Outgoing call setup in alerting state */
    RK_BT_HFP_CLCC_EVT,                 /* Current active call list info */
} RK_BT_HFP_EVENT;

typedef int (*RK_BT_HFP_CALLBACK)(const char *bd_addr, RK_BT_HFP_EVENT event, void *data);

void rk_bt_hfp_register_callback(RK_BT_HFP_CALLBACK cb);
int rk_bt_hfp_sink_open(void);
int rk_bt_hfp_open(void);
int rk_bt_hfp_close(void);
int rk_bt_hfp_pickup(void);
int rk_bt_hfp_hangup(void);
int rk_bt_hfp_redial(void);
int rk_bt_hfp_dial_number(char *number);
int rk_bt_hfp_report_battery(int value);
int rk_bt_hfp_set_volume(int volume);
void rk_bt_hfp_enable_cvsd(void);
void rk_bt_hfp_disable_cvsd(void);
int rk_bt_hfp_disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* __BLUETOOTH_HANDSFREE_H__ */
