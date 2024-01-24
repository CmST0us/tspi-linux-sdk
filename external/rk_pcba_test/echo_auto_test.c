/*
 *  wlan_test.c  --  wlan test application
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
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>


#define LOG_TAG "echo_auto_test"
#include "common.h"

static int pcba_test_result_rw(char *file_name, char *w_buf, char *r_buf, unsigned char rw)
{
	int ret = 0;
	int fd = -1;
	char pcbatest_result_filename[COMMAND_VALUESIZE] = {0};

	snprintf(pcbatest_result_filename, sizeof(pcbatest_result_filename),
		 "%s/%s_result\0", TEST_RESULT_SAVE_PATH, file_name);

	if (rw) {
        log_info("=================fucntion: %s================\n",__func__);
		log_info("write result ** pcbatest_result_filename is :%s\n",pcbatest_result_filename);
        	if(w_buf[0]!='\0'){
            fd = open(pcbatest_result_filename, O_CREAT | O_WRONLY	| O_TRUNC);
            if (fd < 0)
            {
                log_err("open %s fail, errno = %d\n", pcbatest_result_filename, errno);
                return -1;
            }
            		write(fd, w_buf, COMMAND_VALUESIZE);
		}
        else {
            log_info("w_buf is NUll, do nothing\n");
        }
	} else {
		fd = open(pcbatest_result_filename, O_RDONLY);
		if (fd < 0) {
			log_info("can't open %s, errno = %d\n", pcbatest_result_filename, errno);
			return 1;
		}
		ret = read(fd, r_buf, COMMAND_VALUESIZE);
		if (ret <= 0) {
			log_err("read %s fail, errno = %d\n", pcbatest_result_filename, errno);
			ret = -1;
		}
		log_info("\n**********Read file: %s; Result is %s\t*****\n",pcbatest_result_filename,r_buf);
	}
	close(fd);

	return ret;
}

static int run_cmd_to_shell_duplex(char *cmd, char *w_buf, char *r_buf, char *match_str)
{
	int ret = 0;
	int read_len = 0;
	FILE *fp;
	char buf[COMMAND_VALUESIZE] = {0};
	char cmd_msg[COMMAND_VALUESIZE] = {0};

	snprintf(cmd_msg, sizeof(cmd_msg),"%s  %s\0", cmd, w_buf);
	log_info("========cmd_msg is : %s\n",cmd_msg);

	fp = popen(cmd_msg, "r");
	if (fp == NULL) {
		log_err("run_cmd_to_shell_duplex dpopen fail, errno=%d\n", errno);
		return -1;
	}

	if(match_str == NULL){
		read_len = fread(buf, sizeof(char), sizeof(buf), fp);
		if (read_len <= 0)
			ret = -1;
	} else {
		while (fgets(buf, sizeof(buf), fp)) {
			if (strstr(buf, match_str)) {
			    log_info("====================================\n");
                log_info("strstr(buf, match_str) is : %s\n",buf);
				strcpy(r_buf, buf);
				break;   //* ÐÂÌí¼Ó
			} else {
				puts(buf);
			}
		}
	}

EXIT:
	pclose(fp);
	return ret;
}

static int pcba_test_item_process(char *pcba_test_filename, char *args)
{
	int ret = 0;
	char buf[COMMAND_VALUESIZE] = {0};

	chmod(pcba_test_filename, S_IRUSR|S_IWUSR|S_IXUSR);

	ret = run_cmd_to_shell_duplex(pcba_test_filename, args, buf, TESTITEM_SEND_HEAD);
	if (ret) {
		log_err("run_cmd_to_shell_duplex fail, ret=%d \n", ret);
		return TEST_FORK_ERR;
	}

	log_info("pcba_test_result_rw buf is: %s\n", buf);
	ret = pcba_test_result_rw(pcba_test_filename, buf, NULL, 1);
	if (ret)
		return SAVE_RESULE_ERR;

	return ret;
}

int main(int argc, char *argv[])
{
	int i = 0;
	int ret = 0;

	if (argc > 3 || argc < 2) {
		printf("[usage]:\n\techo_auto_test program_name args\n");
		printf("[example]:\n\techo_auto_test sendmsg \"hello world!\"\n");
		return -EINVAL;
	}

	if (argc == 2)
		ret = pcba_test_item_process(argv[1], "");
	else
		ret = pcba_test_item_process(argv[1], argv[2]);

	return ret;
}

