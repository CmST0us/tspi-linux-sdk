/*
 *  sdcard_test.c  --  sdcardtest application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author:
 *  Author:
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	 http://www.apache.org/licenses/LICENSE-2.0
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

#define LOG_TAG "sdcard_test"
#include "common.h"


char result[COMMAND_VALUESIZE] = RESULT_FAIL;


//* 信号处理函数，在结束进程前，为按键测试返回一个结果；
//static int sdcard_result_send(int sign_no)
//{
//    int err_code =0;
//    printf("====================function : %s start =================\n",__func__);
//    if(!memcmp(result,RESULT_FAIL,strlen(RESULT_FAIL))){
//        err_code = KEY_QUERY_FAIL;
//    }
//    send_msg_to_server("sdcard_test", result, err_code);
//
//    printf("====================function : %s finished =================\n",__func__);
//    exit(0);
//}

int sdcard_auto_test()
{
    int ret = 0;
    int status = 0;
    status = system("/data/sdcard_test.sh");
    if (status == -1) {
        log_info("system cmd run error...\n");
        ret = -1;
    } else {
        log_info("exit status value = [0x%x]\n", status);
        if (WIFEXITED(status)) {
            if (0 == WEXITSTATUS(status)) {
                log_info("run shell script successfully.\n");
                ret = 0;
            } else {
                if (1 == WEXITSTATUS(status)) {
                    log_info("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                    ret = 1;
                } else if (2 == WEXITSTATUS(status)) {
                    log_info("run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                    ret = 2;
                }
            }
        } else {
            log_info("exit status = [%d]\n", WEXITSTATUS(status));
            ret = -1;
        }
    }

    if (ret != 0)
        log_info("sdcard test failed. ret = %d\n", ret);

    return ret;
}

int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "sdcard_test";

	log_info("sdcard test process start...\n");
    //* 注册信号处理函数
	//signal(SIGTERM,sdcard_result_send);

    test_flag = sdcard_auto_test();
    if(test_flag == 0) {
        strcpy(result,RESULT_PASS);
        err_code = 0;
        log_info("sdcard test Pass\n");
    } else if (test_flag == 1) {
        strcpy(result,RESULT_FAIL);
        err_code = SDCARD_MOUNT_ERR;
        log_info("sdcard test MountErr...\n");
    } else if (test_flag == 2) {
        strcpy(result,RESULT_FAIL);
        err_code = SDCARD_TIMEOUT_ERR;
        log_info("sdcard test Timgout...\n");
    }

    if (test_flag == 0) {
        int fd = -1;
        double cap;
        char sdcard_size[32] = {0};
        fd = open("/run/sd_capacity", O_RDONLY);
        memset(sdcard_size, 0 ,sizeof(sdcard_size));
        int r_len = read(fd, sdcard_size, sizeof(sdcard_size));
        if (r_len <= 0) {
			log_err("read %s fail, errno = %d\n", "/run/sd_capacity", errno);
		}
        //fgets(sdcard_size, 32, fd);
        cap = strtod(sdcard_size, NULL);
        snprintf(sdcard_size, sizeof(sdcard_size), "capacity:%.4fG ", cap * 1.0 / 1024 / 1024);
        strcat(buf,": ");
        strcat(buf, sdcard_size);
    }
    //strcat(buf, result);
    send_msg_to_server(buf, result, err_code);
    return 0;
}

