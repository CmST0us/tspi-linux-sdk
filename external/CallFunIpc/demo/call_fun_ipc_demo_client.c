// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <inttypes.h>

#include <glib.h>

#include <pthread.h>
#include <gdbus.h>

#include "json-c/json.h"
#include "call_fun_ipc.h"
#include "common.h"

static void main_exit(void)
{
    printf("client %s\n", __func__);
    call_fun_ipc_client_deinit();
}

void signal_crash_handler(int sig)
{
    exit(-1);
}
 
void signal_exit_handler(int sig)
{
    exit(0);
}

static void examples(void)
{
    struct Examples_s para;
    memset(&para, 0, sizeof(struct Examples_s));
    para.r = 1;
    para.g = 2;
    para.b = 3;
    para.ret = -1;
    int ret = call_fun_ipc_call((char *)__func__, &para, sizeof(struct Examples_s), 1);
    printf("%s, ret = %d, para.ret = %d\n", __func__, ret, para.ret);
}

void help_printf(void)
{
    printf("************************\n");
    printf("0.exit\n");
    printf("1.help\n");
    printf("2.1000 examples\n");
    printf("3.examples\n");
    printf("************************\n");
}

int main( int argc , char ** argv)
{
    atexit(main_exit);
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGBUS, signal_crash_handler);
    signal(SIGSEGV, signal_crash_handler);
    signal(SIGFPE, signal_crash_handler);
    signal(SIGABRT, signal_crash_handler);

    call_fun_ipc_client_init(DBUS_NAME, DBUS_IF, DBUS_PATH, SHARE_PATH, 1);
    help_printf();
    while (1) {
        char cmd = 0;
        printf("please enter:");
again:
        scanf("%c", &cmd);
        switch(cmd) {
            case '0':
                return 0;
                break;
            case '1':
                help_printf();
                break;
            case '2':
            {
                int i = 0;
                while (i++ < 1000) {
                    examples();
                    usleep(1000);
                }
            }
                break;
            case '3':
                examples();
                break;
            case '4':
                break;
            case '5':
                break;
            case '6':
                break;
            case 0xa:
                continue;
                break;
        }
        goto again;
    }

    printf("call_fun_ipc_demo_client exit\n");

    return 0;
}
