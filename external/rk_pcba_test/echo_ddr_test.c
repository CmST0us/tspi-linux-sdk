/*
 *  ddr_test.c  --  ddr test application
 *
 *  Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *  Author: Panzhenzhuan Wang <randy.wang@rock-chips.com>
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
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "ddr_test.h"

#define LOG_TAG "ddr_test"
#include "common.h"

#define EMMCPATH "/sys/bus/mmc/devices/mmc0:0001/block/mmcblk0/size"
#define READ_DDR_COMMAND "cat /proc/zoneinfo | busybox grep present | \
				busybox awk 'BEGIN{a=0}{a+=$2}END{print a}'"

/* for ddr  */
int ddr_exec(const char *cmd, char *ddrsize_char, unsigned int length)
{
    FILE *pp = popen(cmd, "r");

    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("errno=%d\n",errno);
        char * mesg = strerror(errno);  //使用strerror（）翻译错误代码
        printf("Mesg:%s\n",mesg);
        return -1;
    }
	if (fgets(ddrsize_char, length, pp) == NULL) {
		printf("popen read from %s is NULL!\n",cmd);
		pclose(pp);
		return -1;
	}
	pclose(pp);
	return 0;
}

/* 内存是每页4KB，echo样机DDR内存为128M*16 DDR3 SDRAM即2Gb也就是256MB*/
void *ddr_test(void *argv)
{
	int ddr_ret = 0;
	char ddrsize_char[20];
	int ddr_size = 0;
	char cmd[128];

    printf("=======  ddr test starting   ========\n");
    //sprintf(cmd,"aplay %s/ddr_test_start.wav",AUDIO_PATH);
    //system(cmd);
    //system("aplay /data/test/ddr_test_start.wav");
    /* For ddr */
    memset(ddrsize_char, 0, sizeof(ddrsize_char));
    ddr_ret = ddr_exec(READ_DDR_COMMAND,ddrsize_char, sizeof(ddrsize_char));
    if (ddr_ret >= 0)
    {
        printf("======%s value is %s=====.\n",READ_DDR_COMMAND,ddrsize_char);
        ddr_size = (int)(atoi(ddrsize_char)*4/1024);
        printf("=========== ddr_size is : %dMB ==========\n",ddr_size);
        if(DDR_CAPACITY != ddr_size)
            goto fail;
    }
    else
    {
        goto fail;
    }
    printf("=========== ddr test success ==========\n");

	return (void*)ddr_ret;
fail:
    printf("=========== ddr test failed ==========\n");

    return (void*)ddr_ret;
}

//主函数启动emmc_test
int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "ddr_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    test_flag = (int)ddr_test(argv[0]);
    if(test_flag < 0)
    {
        strcpy(result,RESULT_FAIL);
        err_code = DDR_PROC_ERR;
    } else {
        char ddrSize[32] = {0};
        snprintf(ddrSize, sizeof(ddrSize), "size:%dMB",  DDR_CAPACITY);
        strcat(buf, ": ");
        strcat(buf, ddrSize);
    }

    send_msg_to_server(buf, result, err_code);
}
