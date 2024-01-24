/*
 * Copyright (c) 2021 Rockchip, Inc. All Rights Reserved.
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

#include "main.h"
#include "lvgl/lvgl.h"
#include "lvgl/lv_conf.h"
#include "hal/hal_sdl.h"
#include "hal/hal_drm.h"

static int g_indev_rotation = 0;

static int quit = 0;

#if LV_USE_DEMO_WIDGETS
extern void lv_demo_widgets(void);
#elif LV_USE_DEMO_KEYPAD_AND_ENCODER
extern void lv_demo_keypad_encoder(void);
#elif LV_USE_DEMO_BENCHMARK
extern void lv_demo_benchmark(void);
#elif LV_USE_DEMO_STRESS
extern void lv_demo_stress(void);
#elif LV_USE_DEMO_MUSIC
extern void lv_demo_music(void);
#endif

static void sigterm_handler(int sig) {
    fprintf(stderr, "signal %d\n", sig);
    quit = 1;
}

static void lvgl_init(void)
{
    lv_init();

#ifdef USE_SDL_GPU
    hal_sdl_init(0, 0);
#else
    hal_drm_init(0, 0);
#endif
    lv_port_fs_init();
    lv_port_indev_init(g_indev_rotation);
}

int main(int argc, char **argv)
{
#define FPS     0
#if FPS
    float maxfps = 0.0, minfps = 1000.0;
    float fps;
    float fps0 = 0, fps1 = 0;
    uint32_t st, et;
    uint32_t st0 = 0, et0;
#endif
    signal(SIGINT, sigterm_handler);
    lvgl_init();

#if LV_USE_DEMO_WIDGETS
    lv_demo_widgets();
#elif LV_USE_DEMO_KEYPAD_AND_ENCODER
    lv_demo_keypad_encoder();
#elif LV_USE_DEMO_BENCHMARK
    lv_demo_benchmark();
#elif LV_USE_DEMO_STRESS
    lv_demo_stress();
#elif LV_USE_DEMO_MUSIC
    lv_demo_music();
#endif

    while(!quit) {
#if FPS
        st = clock_ms();
#endif
        lv_task_handler();
#if FPS
        et = clock_ms();
        fps = 1000 / (et - st);
        if (fps != 0.0 && fps < minfps) {
            minfps = fps;
            printf("Update minfps %f\n", minfps);
        }
        if (fps < 60 && fps > maxfps) {
            maxfps = fps;
            printf("Update maxfps %f\n", maxfps);
        }
        if (fps > 0.0 && fps < 60) {
            fps0 = (fps0 + fps) / 2;
            fps1 = (fps0 + fps1) / 2;
        }
        et0 = clock_ms();
        if ((et0 - st0) > 1000) {
            printf("avg:%f\n", fps1);
            st0 = et0;
        }
#endif
        usleep(100);
    }

    return 0;
}
