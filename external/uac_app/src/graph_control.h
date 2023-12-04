/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef _ROCKIT_GRAPH_CONTROL_H_
#define _ROCKIT_GRAPH_CONTROL_H_

#include "uac_control.h"
#include <rt_header.h>
#include <rt_metadata.h>
#include <RTUACGraph.h>
#include <RTMediaBuffer.h>
#include <rt_metadata.h>
#include <RTMediaMetaKeys.h>

enum UACStreamType {
    // our device record datas from usb, pc/remote->our device
    UAC_STREAM_RECORD = 0,
    // play datas to usb, our device->pc/remote
    UAC_STREAM_PLAYBACK,
    UAC_STREAM_MAX
};

typedef struct _UACAudioConfig {
    int samplerate;
    float volume;
    int mute;
    int ppm;
} UACAudioConfig;

void graph_set_samplerate(RTUACGraph* uac, int type, UACAudioConfig& config);
void graph_set_volume(RTUACGraph* uac, int type, UACAudioConfig& config);
void graph_set_ppm(RTUACGraph* uac, int type, UACAudioConfig& config);

#endif  // _ROCKIT_GRAPH_CONTROL_H_
