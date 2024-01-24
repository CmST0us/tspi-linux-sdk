/*
 *  echo_ir_test.c  --  Infrared ray test application
 *
 *  Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 *  Author: chad.ma <chad.ma@rock-chips.com>
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <unistd.h>

#define LOG_TAG "IR_test"
#include "common.h"

#define IR_INPUT_EVENT "/dev/input/event0"

#define IR_TIMEOUT_DOWN    60
#define IR_TIMEOUT_UP      5


/* TV Remote Controler
 * here accord kernel ir drivers keymap,./driver/media/rc/keymaps/rc-360.c
 * just defien 10 keys.
*/
#define IR_VALID_NUM              10

typedef struct IR_CONTRALOR_st {
    char *key_name;
    int key_value;
    int key_press_flag;
} IR_st;


IR_st gIR_test[IR_VALID_NUM] = {
    {"NUM_0",    KEY_0,    0},
    {"NUM_1",    KEY_1,    0},
    {"NUM_2",    KEY_2,    0},
    {"NUM_3",    KEY_3,    0},
    {"NUM_4",    KEY_4,    0},
    {"NUM_5",    KEY_5,    0},
    {"NUM_6",    KEY_6,    0},
    {"NUM_7",    KEY_7,    0},
    {"NUM_8",    KEY_8,    0},
    {"NUM_9",    KEY_9,    0},

};
static int isAllIRKeyPressed(IR_st *key_array)
{
    int i = 0;

    if (key_array == NULL)
        return 0;

    for (i = 0; i < IR_VALID_NUM; i++) {
        if (key_array[i].key_press_flag)
            continue;
        else
            break;
    }

    if (i == IR_VALID_NUM) {
        log_info("########### All IR key had pressed!!! ######### \n");
        return 1;
    } else
        return 0;
}

static void dumpKeyPressInfo(IR_st *key_array)
{
    int i = 0;

    if (key_array == NULL)
        return;

    for (i = 0; i < IR_VALID_NUM; i++) {
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

    snprintf(msg, sizeof(msg), "ir-key name:%s", gIR_test[key_index].key_name);
    snprintf(key_info_buf, buf_size, "x<%s>,<%s>,<%d>", msg, RESULT_KEY_PRESS, error_code);

    return key_info_buf;
}
static int save_scan_result(char *result_buf)
{
    int fd = -1;
    int ret = 0;
    char *bin_name = "echo_ir_test";
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

    assert(strlen(result_buf) <= COMMAND_VALUESIZE);
    int w_len = write(fd, result_buf, strlen(result_buf));

    if (w_len <= 0) {
        log_err("Write %s fail, errno = %d\n", result_filename, errno);
        ret = errno;
    }

    log_info("\t -------IR_test_result: %s -------\n", result_buf);
    log_info("########## write to %s len %d ##########\n", result_filename, w_len);

    close(fd);

    return ret;
}

static char result[COMMAND_VALUESIZE] = RESULT_FAIL;

static int ir_wait_event(int maxfd, fd_set *readfds, int time)
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

static int ir_event_read(int fd, struct input_event *buf)
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

//* 信号处理函数，在结束进程前，为按键测试返回一个结果；
static int ir_result_send(int sign_no)
{
    int err_code = 0;
    printf("====================function : %s start =================\n", __func__);

    if (!memcmp(result, RESULT_VERIFY, strlen(RESULT_VERIFY)))
        err_code = IR_QUERY_FAIL;

    send_msg_to_server("key_test", result, err_code);

    printf("====================function : %s finished =================\n", __func__);
    exit(0);
}

#define MAX_INPUT_COUNT (4)
int main(int argc, char **argv)
{
    int fd;
    int ret = 0;
    int err_code = 0;
    int time = IR_TIMEOUT_DOWN;
    fd_set rdfds;
    struct input_event key_event;
    int modifier;
    char buf[COMMAND_VALUESIZE] = "ir_test";
    int idx = 0;
    int fds[MAX_INPUT_COUNT] = {0};
    char path[64];
    int fdcount = 0;
    int max_fd = 0;
    int i, k;
    struct timeval sel_timeout_tv;

    log_info("key test process start...\n");
    //* 注册信号处理函数
    signal(SIGTERM, (__sighandler_t)ir_result_send);

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
        err_code = IR_OPEN_FAIL;
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

        if (isAllIRKeyPressed(gIR_test))
            break;

        ret = select(max_fd + 1, &rdfds, NULL, NULL, &sel_timeout_tv);
        if (ret > 0) {
            k = 0;

            while (k < fdcount) {
                int fd = fds[k++];

                if (FD_ISSET(fd, &rdfds)) {
                    ret = ir_event_read(fd, &key_event);

                    if (ret > 0) {
                        if (key_event.value) {
                            log_info("key(%d) is down\n", key_event.code);
                            time = IR_TIMEOUT_UP;
                        } else {
                            log_info("key(%d) is up\n", key_event.code);
                            time = IR_TIMEOUT_DOWN;

                            for (idx = 0; idx < IR_VALID_NUM; idx ++) {
                                if (key_event.code == gIR_test[idx].key_value) {
                                    if (gIR_test[idx].key_press_flag == 0)
                                        gIR_test[idx].key_press_flag = 1;

                                    assemble_info(buf, COMMAND_VALUESIZE, idx);
                                    save_scan_result(buf);
                                } else
                                    continue;
                            }

                            if (isAllIRKeyPressed(gIR_test)) {
                                //int i =0;
                                log_info(" ======== key test is over ========= \n");
                                strcpy(result, RESULT_VERIFY);
                                break;
                            }
                        }
                    }
                }
            }
        } else if (ret == 0) {
            log_err("wait key event fail, errno=%d\n", errno);

            if (idx != IR_VALID_NUM) {
                int i = 0;

                for (i  = 0; i < IR_VALID_NUM; ++i)  {
                    if (gIR_test[i].key_press_flag == 0) {
                        strcat(buf, gIR_test[i].key_name);
                        strcat(buf, " ");
                    }
                }

                strcat(buf, " NOT PRESS!");
            }

            err_code = IR_EVENT_TIMEOUT;
            goto EXIT;
        }
    }

    //snprintf(buf, sizeof(buf), "key_code:%d", key_event.code);
    dumpKeyPressInfo(gIR_test);
EXIT:
    sleep(1);
    memset(buf, 0 , sizeof(buf));

    if (!err_code)
        strcpy(result, RESULT_VERIFY);

    send_msg_to_server(buf, result, err_code);

    return err_code;
}


