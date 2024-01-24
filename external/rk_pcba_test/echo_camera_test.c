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
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>


#define LOG_TAG "camera_test"
#include "common.h"

/*主函数入口*/
int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "camera_test";
    char result[COMMAND_VALUESIZE] = RESULT_FAIL;
    int ret;
    int exit_status;


	log_info("=============== camera test start...============\n");

    pid_t fpid;

    fpid = fork();
    if (fpid < 0) {
        printf("error in fork!\n");
        return fpid;
    } else if (fpid == 0) {
        pid_t child_pid;

        child_pid = fork();
        if (child_pid < 0) {
            printf("error in fork,fork error in child pid!\n");
            return child_pid;
        } else if (child_pid == 0) {
            //child process, capture one frame camera data,save to /tmp/camera_data.
            char *argv0[]={"./v4l2_test", NULL};
            char *envp0[]={"/data",NULL}; //传递给执行文件新的环境变量数组
            ret = execve("/data/v4l2_test", argv0, envp0);
            if(ret == -1) {
                printf("######## v4l2_test run error : %s ###### \n", strerror(errno));
            } else {
                printf("######## v4l2_test run... ######\n");
            }
        } else {
            //father process
            int status;

            ret = wait(&status);
            if (ret < 0) {
                log_err("ERR: wait child failed: (%d)\n", ret);
                return ret;
            }

            log_info("process(%d) wait child(%d) status: %d\n",
                   getpid(), ret, status);

            if (WIFEXITED(status)) {
                exit_status = WEXITSTATUS(status);
                log_info(" v412_test run success and exit. exit_status = %d \n",exit_status);
            } else {
                log_info(" v412_test run seem no good....\n");
                return -1;
            }

            /* nv12 transform to bmp */
            char *argv1[]={"./nv12_to_bmp_main", "/tmp/camera_data", "720", "576", NULL};
            char *envp1[]={"/data",NULL}; //传递给执行文件新的环境变量数组
            ret = execve("/data/nv12_to_bmp_main", argv1, envp1);
            if (ret == -1) {
                printf("######## nv12_to_bmp_main run error : %s ###### \n", strerror(errno));
            } else {
                printf("######## nv12_to_bmp_main run... ######\n");
            }
        }
    } else {
        //father process
        int status;

        ret = wait(&status);
        if (ret < 0) {
            log_err("ERR: wait child failed: (%d)\n", ret);
            return ret;
        }

        log_info("process(%d) wait child(%d) status: %d\n",
               getpid(), ret, status);

        if (WIFEXITED(status)) {
            exit_status = WEXITSTATUS(status);
            log_info(" All run success and exit. exit_status = %d \n",exit_status);
        } else {
            log_info(" Somethings run seem no good....\n");
            err_code = CAMERA_PROC_ERR;
        }

        if (err_code)
	        strcpy(result, RESULT_FAIL);
        else
            strcpy(result, RESULT_PASS);

        printf("######## send msg to server... ######\n");
        send_msg_to_server(buf, result, err_code);
    }

    return 0;
}




