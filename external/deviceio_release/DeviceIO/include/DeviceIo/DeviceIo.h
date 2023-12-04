/*
 * Copyright (c) 2017 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef DEVICEIO_FRAMEWORK_DEVICEIO_H_
#define DEVICEIO_FRAMEWORK_DEVICEIO_H_

#include <pthread.h>
#include <stdint.h>
#include <string.h>

#include <DeviceIo/RkBtBase.h>

//typedef _Bool bool;

#define ture 1
#define false 0

#define PLAYBACK_DEVICE_NUM 1

/* bt control cmd */
enum BtControl {
    BT_OPEN,
    BT_CLOSE,

    BT_SOURCE_OPEN,
    BT_SOURCE_SCAN,
    BT_SOURCE_CONNECT,
    BT_SOURCE_DISCONNECT, //5
    BT_SOURCE_STATUS,
    BT_SOURCE_REMOVE,
    BT_SOURCE_CLOSE,
    BT_SOURCE_IS_OPENED,

    BT_SINK_OPEN, //10
    BT_SINK_CLOSE,
    BT_SINK_RECONNECT,
    BT_SINK_IS_OPENED,

    BT_IS_CONNECTED,
    BT_UNPAIR,

    BT_PLAY,
    BT_PAUSE_PLAY,
    BT_RESUME_PLAY,
    BT_VOLUME_UP,
    BT_VOLUME_DOWN, //20
    BT_AVRCP_FWD,
    BT_AVRCP_BWD,
    BT_AVRCP_STOP,
    BT_HFP_RECORD,

    BT_BLE_OPEN, //25
    BT_BLE_COLSE,
    BT_BLE_IS_OPENED,
    BT_BLE_WRITE,
    BT_BLE_READ,
    BT_VISIBILITY,
    BT_BLE_DISCONNECT,

    BT_HFP_OPEN,
    BT_HFP_SINK_OPEN,

    GET_BT_MAC,
};

/* wifi control cmd */
enum WifiControl {
    WIFI_OPEN,
    WIFI_CLOSE,
    WIFI_CONNECT,
    WIFI_DISCONNECT,
    WIFI_IS_OPENED,
    WIFI_IS_CONNECTED,
    WIFI_SCAN = 6,
    WIFI_IS_FIRST_CONFIG,
    WIFI_OPEN_AP_MODE,
    WIFI_CLOSE_AP_MODE,
    GET_WIFI_MAC,
    GET_WIFI_IP,
    GET_WIFI_SSID = 12,
    GET_WIFI_BSSID,
    GET_LOCAL_NAME,
    WIFI_RECOVERY,
	WIFI_GET_DEVICE_CONTEXT
};

enum BT_Device_Class {
	BT_SINK_DEVICE,
	BT_SOURCE_DEVICE,
	BT_BLE_DEVICE,
	BT_IDLE,
};

/* input event */
enum BtEvent {
	//蓝牙等待配对
	BT_WAIT_PAIR,
	//蓝牙配对中
	BT_PAIR_FAILED_PAIRED,
	//配对失败,请重试
	BT_PAIR_FAILED_OTHER,
	//蓝牙配对成功
	BT_PAIR_SUCCESS, //20
	BT_CONNECT,
	//手机关闭蓝牙蓝牙断开
	BT_DISCONNECT,
	BLE_CLIENT_DISCONNECT,
	BLE_SERVER_RECV,
	//BT CALL
	BT_HFP_AUDIO_CONNECT,
	BT_HFP_AUDIO_DISCONNECT, //30
	BT_POWER_OFF,

	//BT EVENT
	BT_SINK_ENV_CONNECT,
	BT_SINK_ENV_DISCONNECT,
	BT_SINK_ENV_CONNECT_FAIL,
	BT_SRC_ENV_CONNECT,
	BT_SRC_ENV_DISCONNECT,
	BT_SRC_ENV_CONNECT_FAIL,
	BT_BLE_ENV_CONNECT,
	BT_BLE_ENV_DISCONNECT,
	BT_BLE_ENV_CONNECT_FAIL,

	BT_EVENT_START_PLAY,
	BT_EVENT_PAUSE_PLAY,
	BT_EVENT_STOP_PLAY,

	//BT spp event
	SPP_CLIENT_CONNECT,
	SPP_CLIENT_DISCONNECT,

	//WIFI
	WIFI_ENV_CONNECT,
	WIFI_ENV_DISCONNECT,
	WIFI_ENV_CONNECT_FAIL,
};

struct wifi_config {
	char ssid[512];
	int ssid_len;
	char psk[512];
	int psk_len;
	char key_mgmt[512];
	int key_len;
	bool hide;
	void (*wifi_status_callback)(int status, int reason);
};

#endif // DEVICEIO_FRAMEWORK_DEVICEIO_H_
