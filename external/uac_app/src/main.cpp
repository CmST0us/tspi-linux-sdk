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


#include <stdio.h>
#include <unistd.h>

#include "uevent.h"
#include "uac_control.h"
#include "uac_log.h"

int enable_minilog    = 0;
int uac_app_log_level = LOG_LEVEL_DEBUG;
void debug_level_init() {
    char *log_level = getenv("uac_app_log_level");
    if (log_level) {
        uac_app_log_level = atoi(log_level);
    }
}

int main(int argc, char *argv[])
{
    debug_level_init();
    ALOGI("uac uevent version = 1.0\n");
    // create uac control
    int result = uac_control_create();
    if (result < 0) {
        ALOGE("uac_control_create error\n");
        return 0;
    }

    // register uevent monitor
    uevent_monitor_run();

    while(1) {
        usleep(100000);
    }

    uac_control_destory();
    return 0;
}

