#ifndef __BT_TEST_1S2_H__
#define __BT_TEST_1S2_H__

#ifdef __cplusplus
extern "C" {
#endif

void btmg_init_test(char *data);
void btmg_deinit_test(char *data);

void btmg_set_scan_none_test(char *data);
void btmg_connectable_test(char *data);
void btmg_connectable_discoverable_test(char *data);
void btmg_set_device_name_test(char *data);
void btmg_get_device_name_test(char *data);
void btmg_get_device_addr_test(char *data);
void btmg_pair_by_addr_test(char *data);
void btmg_unpair_by_addr_test(char *data);
void btmg_get_paired_devices_test(char *data);
void btmg_free_paired_devices_test(char *data);

//sink test
void btmg_enable_reconnect_test(char *data);
void btmg_disable_reconnect_test(char *data);
void btmg_sink_connect_by_addr_test(char *data);
void btmg_sink_disconnect_by_addr_test(char *data);
void btmg_sink_avrcp_play_test(char *data);
void btmg_sink_avrcp_pause_test(char *data);
void btmg_sink_avrcp_stop_test(char *data);
void btmg_sink_avrcp_next_test(char *data);
void btmg_sink_avrcp_previous_test(char *data);
void btmg_sink_get_play_status_test(char *data);
void btmg_sink_get_poschange_test(char *data);

//rk api, for test pair and unpair
void btmg_start_discovery_test(char *data);
void btmg_cancel_discovery_test(char *data);
void btmg_is_discovering_test(char *data);
void btmg_display_devices_test(char *data);
void btmg_display_paired_devices_test(char *data);


#ifdef __cplusplus
}
#endif

#endif /* __BT_TEST_H__ */
