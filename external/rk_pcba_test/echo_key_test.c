/*
 *  key_test.c  --  key test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Bin Yang <yangbin@rock-chips.com>
 *  Author: Panzhenzhuan Wang <randy.wang@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <unistd.h>

#define LOG_TAG "key_test"
#include "common.h"

#define KEY_INPUT_EVENT "/dev/input/event0"

#define KEY_INPUT_EVENT0 "/dev/input/event0"
#define KEY_INPUT_EVENT1 "/dev/input/event1"
#define KEY_INPUT_EVENT2 "/dev/input/event2"

#define KEY_TIMEOUT_DOWN    60
#define KEY_TIMEOUT_UP      5

typedef struct KEY_BOARD_st {
    char *key_name;
    int key_value;
    int key_press_flag;
    int key_cnt;
} KEY_st;


#ifdef PCBA_PX3SE
#define KEY_VALID_NUM       5    //PX3SE V11

/* key code */
#define KEY_HOME_CODE       102
#define KEY_MENU_CODE       59
#define KEY_ESC_CODE        158
#define KEY_VOLUP_CODE      115
#define KEY_VOLDOWN_CODE    114

KEY_st gkey_test[KEY_VALID_NUM] = {

    {"vol+",    KEY_VOLUMEUP,    0, 0},
    {"vol-",    KEY_VOLUMEDOWN,  0, 0},
    {"menu",    KEY_MENU,     0, 0},
    {"esc",     KEY_F1,    0, 0},
    {"home",    KEY_HOME,    0, 0},
//  {"resv",    212,        0},
};
#endif

#ifdef PCBA_3308
#define KEY_VALID_NUM       6    //3308 evb v11

/* key code */
#define KEY_VOLUP_CODE      115
#define KEY_VOLDN_CODE      114
#define KEY_MUTE_CODE       113
#define KEY_POWER           116
#define KEY_PLAY_CODE       207
#define KEY_MODE_CODE       373

KEY_st gkey_test[KEY_VALID_NUM] = {
    {"VOL+",    KEY_VOLUMEUP,    0},
    {"VOL-",    KEY_VOLUMEDOWN,    0},
    {"MICMUTE", KEY_MICMUTE,    0},
    {"PLAY",    KEY_PLAY,    0},
    {"MODE",    KEY_MODE,    0},
    {"POWER",   KEY_POWER,    0},

};

#endif

#ifdef PCBA_3229GVA
//TODO: According to 3229 real board to add test code.

#endif

#ifdef PCBA_1808
//TODO:According to 1808 real board to add test code.
#define KEY_VALID_NUM       2    //RK_EVB_RK1808_LP3D8P132SD6_V11

/* key code */
#define KEY_VOLUP_CODE      115
#define KEY_VOLDN_CODE      114

KEY_st gkey_test[KEY_VALID_NUM] = {
    {"VOL+",    KEY_VOLUMEUP,    0},
    {"VOL-",    KEY_VOLUMEDOWN,    0},
};

#endif

#ifdef PCBA_PX30
//TODO:According to PX30 real board to add test code.
#define KEY_VALID_NUM       0

#endif

#ifdef PCBA_3288
//TODO:According to 3288 real board to add test code.
#define KEY_VALID_NUM       0

#endif

#ifdef PCBA_3328
//TODO:According to 3328 real board to add test code.
#define KEY_VALID_NUM       0

#endif

#ifdef PCBA_3326
//TODO:According to 3326 real board to add test code.
#define KEY_VALID_NUM       0
#endif

#ifdef PCBA_3399
//TODO:According to 3399 real board to add test code.
#define KEY_VALID_NUM       0
#endif

#ifdef PCBA_3399PRO
//TODO:According to 3399 PRO real board to add test code.
#define KEY_VALID_NUM       0
#endif

#ifdef PCBA_1126_1109
//TODO:According to RV1126 / RV1109 real board to add test code.
#define KEY_VALID_NUM       0
#endif

#ifdef PCBA_356X
//TODO:According to 356X board to add test code.
#define KEY_VALID_NUM       2

KEY_st gkey_test[KEY_VALID_NUM] = {
    {"VOL+",    KEY_VOLUMEUP,    0},
    {"VOL-",    KEY_VOLUMEDOWN,    0},
};
#endif

#ifdef PCBA_3588
//TODO:According to 3588 real board to add test code.
#define KEY_VALID_NUM       2

KEY_st gkey_test[KEY_VALID_NUM] = {
    {"VOL+",    KEY_VOLUMEUP,    0},
    {"VOL-",    KEY_VOLUMEDOWN,    0},
};
#endif

static char result[COMMAND_VALUESIZE] = RESULT_FAIL;

static int isAllKeyPressed(KEY_st *key_array)
{
    int i = 0;

    if (key_array == NULL)
        return 0;

    for (i = 0; i < KEY_VALID_NUM; i++) {
        if (key_array[i].key_press_flag)
            continue;
        else
            break;
    }

    if (i == KEY_VALID_NUM) {
        log_info("########### All key had pressed!!! ######### \n");
        return 1;
    } else
        return 0;
}

static void dumpKeyPressInfo(KEY_st *key_array)
{
    int i = 0;

    if (key_array == NULL)
        return;

    for (i = 0; i < KEY_VALID_NUM; i++) {
        log_info("KEYNAME : %s: \t KEY_press: %d \n", key_array[i].key_name,
                 key_array[i].key_press_flag);
    }
}

static char *assemble_info(char *key_info_buf, int buf_size, int key_index)
{
    int error_code = 0;
    char msg[32] = {0};

    memset(msg, sizeof(msg), 0);

    if (key_info_buf == NULL)
        return NULL;

    memset(key_info_buf, buf_size, 0);

    snprintf(msg, sizeof(msg), "key name:%s", gkey_test[key_index].key_name);
#ifdef PCBA_PX3SE
    snprintf(key_info_buf, buf_size, "x<%s>,<%s>,<%d>", msg, RESULT_KEY_PRESS, \
             gkey_test[key_index].key_cnt);
#else
    snprintf(key_info_buf, buf_size, "x<%s>,<%s>,<%d>", msg, RESULT_KEY_PRESS, error_code);
#endif

    return key_info_buf;
}

static int save_scan_result(char *result_buf)
{
    int fd = -1;
    int ret = 0;
    char *bin_name = "echo_key_test";
    char result_filename[COMMAND_VALUESIZE] = {0};

    snprintf(result_filename, sizeof(result_filename),
             "%s/%s_result",
             TEST_RESULT_SAVE_PATH,
             bin_name);

    fd = open(result_filename, O_CREAT | O_WRONLY   | O_TRUNC);

    if (fd < 0) {
        log_err("open %s fail, errno = %d\n", result_filename, errno);
        ret = errno;
    }

    //assert(strlen(result_buf) <= COMMAND_VALUESIZE);
    int w_len = write(fd, result_buf, strlen(result_buf));

    if (w_len <= 0) {
        log_err("Write %s fail, errno = %d\n", result_filename, errno);
        ret = errno;
    }

    log_info("\t -------key_test_result: %s -------\n", result_buf);
    log_info("########## write to %s len %d ##########\n", result_filename, w_len);

    close(fd);

    return ret;
}


static int key_wait_event(int maxfd, fd_set *readfds, int time)
{
    int ret;
    struct timeval timeout;

    FD_ZERO(readfds);
    FD_SET(maxfd, readfds);
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    ret = select(maxfd + 1, readfds, NULL, NULL, &timeout);
    switch (ret) {
        case -1:
            return -1;
        case 0:
            log_err("select timeout(%ds)\n", time);
            return 1;
        default:
            if (FD_ISSET(maxfd, readfds)) {
                FD_CLR (maxfd, readfds);
                return 0;
            }
            break;
    }

    return -1;
}

static int key_event_read(int fd, struct input_event *buf)
{
    int read_len = 0;

    read_len = read(fd, buf, sizeof(*buf));
    if (read_len < 0) {
        if ((errno != EINTR) && (errno != EAGAIN))
            return 0;
        return -1;
    }

    if (buf->type)
        return 1;

    return 0;
}

static int key_result_send(int sign_no)
{
    int err_code =0;
    printf("====================function : %s start =================\n",__func__);

//    if (!memcmp(result, RESULT_PASS, strlen(RESULT_PASS)))
//        err_code = KEY_PROC_ERR;

    strcpy(result, RESULT_VERIFY);
    send_msg_to_server("key_test", result, err_code);

    printf("====================function : %s finished =================\n",__func__);
    exit(0);
}

#define MAX_INPUT_COUNT (4)

int main(int argc, char **argv)
{
    int fd;
    int ret = 0;
    int err_code = 0;
    int time = KEY_TIMEOUT_DOWN;
    fd_set rdfds;
    struct input_event key_event;
    int modifier;
    char buf[COMMAND_VALUESIZE] = "key_test";
    int idx = 0;
    int fds[MAX_INPUT_COUNT] = {0};
    char path[64];
    int fdcount = 0;
    int max_fd = 0;
    int i, k;
    struct timeval sel_timeout_tv;

    log_info("key test process start...\n");
    signal(SIGTERM, (__sighandler_t)key_result_send);

    for (i = 0; i < MAX_INPUT_COUNT; i++) {
        sprintf(path, "/dev/input/event%d", i);
        fd = open(path, O_RDONLY | O_NOCTTY);

        if (fd < 0) {
            log_err("open fail:%s\n", strerror(errno));
            continue;
        }

        fds[fdcount++] = fd;
    }

    if (fdcount < 1) {
        err_code = KEY_OPEN_FAIL;
        goto EXIT;
    }

    for (i = 0 ; i < fdcount; i++)
        if (max_fd < fds[i])
            max_fd = fds[i];

    while (1) {
        FD_ZERO(&rdfds);
        sel_timeout_tv.tv_sec = time;
        sel_timeout_tv.tv_usec = 0;

        for (i = 0 ; i < fdcount; i++)
            FD_SET(fds[i], &rdfds);

        if (isAllKeyPressed(gkey_test))
            break;

        ret = select(max_fd + 1, &rdfds, NULL, NULL, &sel_timeout_tv);
        if (ret > 0) {
            k = 0;

            while (k < fdcount) {
                int fd = fds[k++];

                if (FD_ISSET(fd, &rdfds)) {
                    ret = key_event_read(fd, &key_event);

                    if (ret > 0) {
                        if (key_event.value) {
                            log_info("key(%d) is down\n", key_event.code);
                            time = KEY_TIMEOUT_UP;
                        } else {
                            log_info("key(%d) is up\n", key_event.code);
                            time = KEY_TIMEOUT_DOWN;

                            for (idx = 0; idx < KEY_VALID_NUM; idx ++) {
                                if (key_event.code == gkey_test[idx].key_value) {
                                    if (gkey_test[idx].key_press_flag == 0) {
                                        gkey_test[idx].key_press_flag = 1;
                                        #ifdef PCBA_PX3SE
                                        gkey_test[idx].key_cnt = 1;
                                        #endif
                                    } else {
                                        #ifdef PCBA_PX3SE
                                        gkey_test[idx].key_cnt++;
                                        printf("\t\t %s : %d \n\n", gkey_test[idx].key_name,gkey_test[idx].key_cnt);
                                        #endif
                                    }

                                    assemble_info(buf, COMMAND_VALUESIZE, idx);
                                    save_scan_result(buf);
                                } else
                                    continue;
                            }
#ifndef PCBA_PX3SE
                            if (isAllKeyPressed(gkey_test)) {
                                log_info(" ======== key test is over ========= \n");
                                strcpy(result, RESULT_VERIFY);
                                break;
                            }
#endif
                        }
                    }
                }
            }
        } else if (ret == 0) {
            log_err("wait key event fail, errno=%d\n", errno);

            if (idx != KEY_VALID_NUM) {
                int i = 0;

                for (i  = 0; i < KEY_VALID_NUM; ++i)  {
                    if (gkey_test[i].key_press_flag == 0) {
                        strcat(buf, gkey_test[i].key_name);
                        strcat(buf, " ");
                    }
                }
                strcat(buf, " NOT PRESS!");
            }

            err_code = KEY_EVENT_TIMEOUT;
            goto EXIT;
        }
    }

    //snprintf(buf, sizeof(buf), "key_code:%d", key_event.code);
    dumpKeyPressInfo(gkey_test);
EXIT:
    sleep(1);
    memset(buf, 0 , sizeof(buf));

    if (!err_code)
        strcpy(result, RESULT_VERIFY);

    send_msg_to_server(buf, result, err_code);

    return err_code;
}

