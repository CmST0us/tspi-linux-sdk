/*
 * Copyright (C) 2007 The Android Open Source Project
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
 */

#include <linux/input.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "minui/minui.h"
#include "recovery_ui.h"

void ui_init(void)
{
}

void ui_set_background(int icon)
{
}

void ui_show_indeterminate_progress()
{
}

void ui_show_progress(float portion, int seconds)
{
}

void ui_set_progress(float fraction)
{
}

void ui_reset_progress()
{
}

void ui_print(const char *fmt, ...)
{
}

void ui_start_menu(char** headers, char** items, int initial_selection) {
}

int ui_menu_select(int sel) {
    return 0;
}

void ui_end_menu() {
}

int ui_text_visible()
{
    return 0;
}

void ui_show_text(int visible)
{
}

int ui_wait_key()
{
    return 0;
}

int ui_key_pressed(int key)
{
    return 0;
}

void ui_clear_key_queue() {
}
