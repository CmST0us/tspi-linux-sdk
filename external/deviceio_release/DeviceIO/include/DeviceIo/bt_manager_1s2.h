#ifndef __BT_MANAGER_1S2_H__
#define __BT_MANAGER_1S2_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if __cplusplus
extern "C" {
#endif

#define BTMGVERSION "Version:2.0.3.I161ca661"

#define BT_LAST_CONNECT_FILE "/data/lib/bluetooth/last_connected"

#ifndef  CONFIG_FILE_PATH
#define CONFIG_FILE_PATH "/etc/bluetooth/aw_bluetooth"
#endif

/*log devel in control of bt_manager*/
typedef enum btmg_log_level_t {
	BTMG_LOG_LEVEL_NONE = 0,
	BTMG_LOG_LEVEL_ERROR,
	BTMG_LOG_LEVEL_WARNG,
	BTMG_LOG_LEVEL_INFO,
	BTMG_LOG_LEVEL_DEBUG,
} btmg_log_level_t;

/*BT state*/
typedef enum {
	BTMG_STATE_OFF,
	BTMG_STATE_ON,
	BTMG_STATE_TURNING_ON,
	BTMG_STATE_TURNING_OFF,
} btmg_state_t;

/*BT discovery state*/
typedef enum {
	BTMG_DISC_STARTED,
	BTMG_DISC_STOPPED_AUTO,
	BTMG_DISC_START_FAILED,
	BTMG_DISC_STOPPED_BY_USER,
} btmg_discovery_state_t;

/*BT discovery mode*/
typedef enum {
	BTMG_SCAN_MODE_NONE,
	BTMG_SCAN_MODE_CONNECTABLE,
	BTMG_SCAN_MODE_CONNECTABLE_DISCOVERABLE,
} btmg_discovery_mode_t;

/*BT bond state*/
typedef enum {
	BTMG_BOND_STATE_NONE,
	BTMG_BOND_STATE_BONDING,
	BTMG_BOND_STATE_BONDED,
} btmg_bond_state_t;

/*AVRCP commands*/
typedef enum {
	BTMG_AVRCP_PLAY,
	BTMG_AVRCP_PAUSE,
	BTMG_AVRCP_STOP,
	BTMG_AVRCP_FORWARD,
	BTMG_AVRCP_BACKWARD,
} btmg_avrcp_command_t;

/*A2DP_SINK connection state for callback*/
typedef enum {
	BTMG_A2DP_SINK_DISCONNECTED,
	BTMG_A2DP_SINK_CONNECTING,
	BTMG_A2DP_SINK_CONNECTED,
	BTMG_A2DP_SINK_DISCONNECTING,
} btmg_a2dp_sink_connection_state_t;

/*A2DP_SINK audio state for callback*/
typedef enum {
	BTMG_A2DP_SINK_AUDIO_SUSPENDED,
	BTMG_A2DP_SINK_AUDIO_STOPPED,
	BTMG_A2DP_SINK_AUDIO_STARTED,
} btmg_a2dp_sink_audio_state_t;

/*AVRCP play state for callback*/
/*The A2DP_SINK audio state may not be updated in time due to the BT stack implementation
* of different smartphone, while the AVRCP play state is always updated in time. So it is
*recommended to take the reported ARVCP state to judge the playing status of BT music*/
typedef enum {
	BTMG_AVRCP_PLAYSTATE_STOPPED,
	BTMG_AVRCP_PLAYSTATE_PLAYING,
	BTMG_AVRCP_PLAYSTATE_PAUSED,
	BTMG_AVRCP_PLAYSTATE_ERROR,
} btmg_avrcp_play_state_t;

typedef struct btmg_track_info_t {
	char title[256];
	char artist[256];
	char album[256];
	char track_num[64];
	char num_tracks[64];
	char genre[256];
	char playing_time[256];
} btmg_track_info_t;

struct paired_dev {
	char *remote_address;
	char *remote_name;
	bool is_connected;
	struct paired_dev *next;
};
typedef struct paired_dev bt_paried_device;

#define MAX_BT_NAME_LEN 248
#define MAX_BT_ADDR_LEN 17

/*callback functions for GAP profile*/
typedef void (*bt_gap_status_cb)(btmg_state_t status);
typedef void (*bt_gap_bond_state_cb)(btmg_bond_state_t state,const char *bd_addr,const char *name);
typedef void (*bt_gap_discovery_status_cb)(btmg_discovery_state_t status);
typedef void (*bt_gap_dev_found_cb)(const char *address,const char *name, unsigned int bt_class, int rssi);
/*gap callback*/
typedef struct btmg_gap_callback_t {
	bt_gap_status_cb gap_status_cb; /*used for return results of bt_manager_enable and status of BT*/
	bt_gap_bond_state_cb gap_bond_state_cb; /*used for bond state event*/
	bt_gap_discovery_status_cb gap_disc_status_cb; /*used for return discovery status of BT*/
	bt_gap_dev_found_cb gap_dev_found_cb; /*used for device found event*/
} btmg_gap_callback_t;

/*callback functions for a2dp_sink profile*/
typedef void (*bt_a2dp_sink_connection_state_cb)(const char *bd_addr, btmg_a2dp_sink_connection_state_t state);
typedef void (*bt_a2dp_sink_audio_state_cb)(const char *bd_addr, btmg_a2dp_sink_audio_state_t state);
typedef void (*bt_a2dp_sink_audio_underrun_cb)(void);

/*a2dp_sink callback*/
typedef struct btmg_a2dp_sink_callback_t {
	bt_a2dp_sink_connection_state_cb a2dp_sink_connection_state_cb;/*used to report the a2dp_sink connection state*/
	bt_a2dp_sink_audio_state_cb a2dp_sink_audio_state_cb;/*used to report the a2dp_sink audio state, not recommended as mentioned before*/
	bt_a2dp_sink_audio_underrun_cb a2dp_sink_audio_underrun_cb;/*underrun callabck*/
} btmg_a2dp_sink_callback_t;

/*callback functions for avrcp profile*/
typedef void (*bt_avrcp_play_state_cb)(const char *bd_addr, btmg_avrcp_play_state_t state); /*used to report play state of avrcp, recommended*/
typedef void (*bt_avrcp_track_changed_cb)(const char *bd_addr, btmg_track_info_t track_info); /*used to report track information*/
typedef void (*bt_avrcp_play_position_cb)(const char *bd_addr, int song_len, int song_pos);/*used to report the progress of playing music*/

/*avrcp callback*/
typedef struct btmg_avrcp_callback_t {
	bt_avrcp_play_state_cb avrcp_play_state_cb;
	bt_avrcp_track_changed_cb avrcp_track_changed_cb;
	bt_avrcp_play_position_cb avrcp_play_position_cb;
} btmg_avrcp_callback_t;

/*bt_manager callback struct to be registered when calling bt_manager_init to report various event*/
typedef struct btmg_callback_t {
	btmg_gap_callback_t btmg_gap_cb;
	btmg_a2dp_sink_callback_t btmg_a2dp_sink_cb;
	btmg_avrcp_callback_t btmg_avrcp_cb;
}btmg_callback_t;

/*bt_manager APIs*/
/* set the bt_manager printing level*/
int bt_manager_set_loglevel(btmg_log_level_t log_level);
/* get the bt_manager printing level*/
btmg_log_level_t bt_manager_get_loglevel(void);
int bt_manager_debug_open_file(const char *path);
void bt_manager_debug_close_file(void);
void bt_manager_debug_open_syslog(void);
void bt_manager_debug_close_syslog(void);

/*preinit function, to allocate room for callback struct, which will be free by bt_manager_deinit*/
int bt_manager_preinit(btmg_callback_t **btmg_cb);
/*init function, the callback functions will be registered*/
int bt_manager_init(btmg_callback_t *btmg_cb);
/*deinit function, must be called before exit*/
int bt_manager_deinit(btmg_callback_t *btmg_cb);

/*GAP APIs*/
/*set BT discovery mode*/
int bt_manager_set_discovery_mode(btmg_discovery_mode_t mode);
/*enable BT*/

int bt_manager_enable(bool enable);
/*return BT state, is enabled or not*/
bool bt_manager_is_enabled(void);
/*start discovery, will return immediately*/
int bt_manager_start_discovery(unsigned int mseconds);
/*cancel discovery, will return immediately*/
int bt_manager_cancel_discovery(void);
/*judge the discovery is in process or not*/
bool bt_manager_is_discovering();
/*pair*/
int bt_manager_pair(char *addr);
/*unpair*/
int bt_manager_unpair(char *addr);
/*get bt state*/
btmg_state_t bt_manager_get_state();
/*get BT name*/
int bt_manager_get_name(char *name, int size);
/*set BT name*/
int bt_manager_set_name(const char *name);
/*get local device address*/
int bt_manager_get_address(char *addr, int size);
/*a2dp sink APIs*/
/*request a2dp_sink connection*/
int bt_manager_a2dp_sink_connect(char *addr);
/*request a2dp_sink disconnection*/
int bt_manager_a2dp_sink_disconnect(char *addr);
/*used to send avrcp command, refer to the struct btmg_avrcp_command_t for the supported commands*/

int bt_manager_disconnect(char *addr);

int bt_manager_avrcp_command(char *addr, btmg_avrcp_command_t command);

/* Get the paired device,need to call <bt_manager_free_paired_devices>	to free data*/
int bt_manager_get_paired_devices(bt_paried_device **dev_list,int *count);

/* free paird device data resource*/
int bt_manager_free_paired_devices(bt_paried_device *dev_list);

/*send GetPlayStatus cmd*/
int bt_manager_send_get_play_status(void);

/*if support avrcp EVENT_PLAYBACK_POS_CHANGED,*/
bool bt_manager_is_support_pos_changed();

int bt_manager_switch_throughput(bool sw_to_wlan);
#if __cplusplus
};	// extern "C"
#endif

#endif
