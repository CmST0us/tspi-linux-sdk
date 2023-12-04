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

#include <stdbool.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "uac_log.h"
#include "uac_control.h"
#include <rt_header.h>
#include <rt_metadata.h>
#include <RTUACGraph.h>
#include <RTMediaBuffer.h>
#include <rt_metadata.h>
#include <RTMediaMetaKeys.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "uac"
#endif // LOG_TAG

/*
 * pc datas -> rv1109
 * usb record->xxxx process->speaker playback
 */
#define UAC_USB_RECORD_SPK_PLAY_CONFIG_FILE "/oem/usr/share/uac_app/usb_recode_speaker_playback.json"

/*
 * rv1109 datas -> pc
 * mic record->>xxxx process->usb playback
 */
#define UAC_MIC_RECORD_USB_PLAY_CONFIG_FILE "/oem/usr/share/uac_app/mic_recode_usb_playback.json"

typedef struct _UACStream {
    pthread_mutex_t mutex;
    UACAudioConfig  config;
    RTUACGraph     *uac;
} UACStream;

typedef struct _UACControl {
    UACStream     stream[UAC_STREAM_MAX];
} UACControl;

typedef enum UACConfigCmd {
    UAC_SET_SAMPLE_RATE = 1,
    UAC_SET_VOLUME,
    UAC_SET_PPM,
} UACConfigCmd;

UACControl *gUAControl = NULL;

extern void uac_set_sample_rate(int type, int samplerate);
extern void uac_set_volume(int type, int volume);
extern void uac_set_mute(int type, int  mute);
extern int  uac_set_parameter(RTUACGraph* uac, int type, UACAudioConfig& config, UACConfigCmd cmd);

int uac_control_create() {
    gUAControl = (UACControl*)calloc(1, sizeof(UACControl));
    if (!gUAControl) {
        ALOGE("fail to malloc memory!\n");
        return -1;
    }

    for (int i = 0; i < UAC_STREAM_MAX; i++) {
        pthread_mutex_init(&gUAControl->stream[i].mutex, NULL);
        gUAControl->stream[i].uac = NULL;

        if (UAC_STREAM_PLAYBACK == i) {
            gUAControl->stream[i].config.samplerate = 48000;
        } else {
            gUAControl->stream[i].config.samplerate = 48000;
        }

        gUAControl->stream[i].config.volume = 1.0;
        gUAControl->stream[i].config.mute = 0;
        gUAControl->stream[i].config.ppm = 0;
    }

    return 0;
}

void uac_control_destory() {
    if (gUAControl == NULL)
        return;

    for (int i = 0; i < UAC_STREAM_MAX; i++) {
        if (gUAControl->stream[i].uac != NULL) {
            delete(gUAControl->stream[i].uac);
            gUAControl->stream[i].uac = NULL;
        }
        pthread_mutex_destroy(&gUAControl->stream[i].mutex);
    }

    gUAControl = NULL;
}

void uac_set_sample_rate(int type, int samplerate) {
    if (gUAControl == NULL)
        return;

    if ((type < 0) || (type >= UAC_STREAM_MAX))
        return;

    ALOGD("type = %d, samplerate = %d\n", type, samplerate);
    pthread_mutex_lock(&gUAControl->stream[type].mutex);
    gUAControl->stream[type].config.samplerate = samplerate;
    /*
     * if uac is already start, set samplerate
     */
    RTUACGraph* uac = gUAControl->stream[type].uac;
    if (uac != NULL) {
        uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_SAMPLE_RATE);
    }
    pthread_mutex_unlock(&gUAControl->stream[type].mutex);
}

void uac_set_volume(int type, int volume) {
    if (gUAControl == NULL)
        return;

    if ((type < 0) || (type >= UAC_STREAM_MAX))
        return;

    pthread_mutex_lock(&gUAControl->stream[type].mutex);
    gUAControl->stream[type].config.volume = ((float)volume/100.0);
    ALOGD("type = %d, volume = %f\n", type, gUAControl->stream[type].config.volume);
    /*
     * if uac is already start, set volume to RTUACGraph
     */
    RTUACGraph* uac = gUAControl->stream[type].uac;
    if (uac != NULL) {
        uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_VOLUME);
    }
    pthread_mutex_unlock(&gUAControl->stream[type].mutex);
}

void uac_set_mute(int type, int mute) {
    if (gUAControl == NULL)
        return;

    if ((type < 0) || (type >= UAC_STREAM_MAX))
        return;

    ALOGD("type = %d, mute = %d\n", type, mute);
    pthread_mutex_lock(&gUAControl->stream[type].mutex);
    gUAControl->stream[type].config.mute = mute;
    /*
     * if uac is already start, set mute to RTUACGraph
     */
    RTUACGraph* uac = gUAControl->stream[type].uac;
    if (uac != NULL) {
        uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_VOLUME);
    }
    pthread_mutex_unlock(&gUAControl->stream[type].mutex);
}

void uac_set_ppm(int type, int ppm) {
    if (gUAControl == NULL)
        return;

    if ((type < 0) || (type >= UAC_STREAM_MAX))
        return;

    ALOGD("type = %d, ppm = %d\n", type, ppm);
    pthread_mutex_lock(&gUAControl->stream[type].mutex);
    gUAControl->stream[type].config.ppm = ppm;
    /*
     * if uac is already start, set mute to RTUACGraph
     */
    RTUACGraph* uac = gUAControl->stream[type].uac;
    if (uac != NULL) {
        uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_PPM);
    }
    pthread_mutex_unlock(&gUAControl->stream[type].mutex);
}


/*
 * see json file
 */
int uac_set_parameter(RTUACGraph* uac, int type, UACAudioConfig& config, UACConfigCmd cmd) {
    if (uac == NULL)
        return -1;

    switch (cmd) {
        case UAC_SET_SAMPLE_RATE: {
            graph_set_samplerate(uac, type, config);
        } break;
        case UAC_SET_VOLUME: {
			graph_set_volume(uac, type, config);
        } break;
        case UAC_SET_PPM: {
            graph_set_ppm(uac, type, config);
        } break;
        default: {
            ALOGE("cannot find UACConfigCmd = %d.\n", cmd);
        } break;
    }

    return 0;
}

int uac_start(int type) {
    if (gUAControl == NULL)
        return -1;

    if (type >= UAC_STREAM_MAX) {
        ALOGE("error, type = %d\n", type);
        return -1;
    }

    uac_stop(type);
    char* config = (char*)UAC_MIC_RECORD_USB_PLAY_CONFIG_FILE;
    char* name = (char*)"uac_playback";
    if (type == UAC_STREAM_RECORD) {
        name = (char*)"uac_record";
        config = (char*)UAC_USB_RECORD_SPK_PLAY_CONFIG_FILE;
    }
    ALOGD("config = %s\n", config);
    RTUACGraph* uac = new RTUACGraph(name);
    if (uac == NULL) {
        ALOGE("error, malloc fail\n");
        return -1;
    }
    // default configs will be readed in json file
    uac->autoBuild(config);
    uac->prepare();

    uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_VOLUME);
    uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_SAMPLE_RATE);
    uac_set_parameter(uac, type, gUAControl->stream[type].config, UAC_SET_PPM);

    uac->start();

    pthread_mutex_lock(&gUAControl->stream[type].mutex);
    gUAControl->stream[type].uac = uac;
    pthread_mutex_unlock(&gUAControl->stream[type].mutex);

    return 0;
}

void uac_stop(int type) {
    if (gUAControl == NULL)
        return;

    RTUACGraph* uac = NULL;
    if (type >= UAC_STREAM_MAX) {
        ALOGE("error type = %d\n",  type);
        return;
    }
    ALOGD("type = %d\n", type);
    pthread_mutex_lock(&gUAControl->stream[type].mutex);
    uac = gUAControl->stream[type].uac;
    gUAControl->stream[type].uac = NULL;
    pthread_mutex_unlock(&gUAControl->stream[type].mutex);

    if (uac != NULL) {
        uac->stop();
        uac->waitUntilDone();
        delete uac;
    }
    ALOGD("type = %d out\n", type);
}
