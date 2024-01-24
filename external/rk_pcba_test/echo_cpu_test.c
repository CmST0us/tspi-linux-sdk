/*
 *  cpu_test.c  --  cpu test application
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

/* Get CPU frequency */
#include <stdio.h>

/* error 处理函数所需头文件*/
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cpu_test.h"

#define LOG_TAG "cpu_test"
#include "common.h"
#define CPU_PROC_ERR -55

#define _CPU_FREQ_TABLE_PATH "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies"
#define _CPU_FREQ_TXT "/data/cfg/rk_pcba_test/cpu%d_freq_table.txt"
#define _CPU_FREQ_GET "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq"
#define _CPU_FREQ_SET "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed"

int cpu_num0 = 0;
CPU_FREQ cpu_0;

int cpu_num1 = 0;
CPU_FREQ cpu_1;

int cpu_num2 = 0;
CPU_FREQ cpu_2;

int cpu_num3 = 0;
CPU_FREQ cpu_3;

int cpu_space = 0;

//第一项：CPU frequency test=========================
//===================================================
/* 将CPU可用频率加入频率链表*/
void Insert_list(CPU_FREQ *head, CPU_FREQ *item)
{
	CPU_FREQ *p1 = NULL, *p0 = NULL;

	p1 = head;
	p0 = item;
	p0->next = NULL;
	if (head == NULL) {
		head = p0;
	} else {
		while (p1 && p1->next)
			p1 = p1->next;

		if (p1->freq == 0){
            p1->freq = p0->freq;
            free(p0);  //需要释放p0，不然item内存泄漏
        }
		else
			p1->next = p0;
	}
}

/* 1、获取CPU支持的frequency table, 并返回可用CPU频率个数*/
int get_cpu_freq_table(int cpu_num, CPU_FREQ *cpu_freq_head)
{

    char freq_table_path[512];
    char freq_table_file[128];

    printf("\n=================function :%s start \tcpu number: %d==================\n",__func__,cpu_num);
    /* 获取CPU number 对应的frequency table 路径*/
	memset(freq_table_path, 0, sizeof(freq_table_path));
    sprintf(freq_table_path, _CPU_FREQ_TABLE_PATH, cpu_num);
    printf("cpu: %d; freq_table_path is: %s\n",cpu_num,freq_table_path);

    /* 将CPU frequency table导入到文件中 */
    char cmd[1024];
	memset(cmd, 0, sizeof(cmd));
	memset(freq_table_file, 0,sizeof(freq_table_file));
	sprintf(freq_table_file, _CPU_FREQ_TXT, cpu_num);
	printf("cpu: %d; freq_table_file is: %s\n",cpu_num,freq_table_file);
	sprintf(cmd, "busybox cat %s > %s", freq_table_path, freq_table_file);
	if(system(cmd)<0)
    {
        perror("system(cmd)");
        return -1;
    }
	/* 将所有可能的CPU频率加入到链表中, 并打印出可用的频率个数*/
    FILE *fp_read = NULL;
    char buf[64];
	CPU_FREQ *new_freq;

    fp_read = fopen(freq_table_file, "r");
	if (NULL==fp_read) {
		printf("%s open err:%s\r\n", freq_table_file,strerror(errno));
		return -1;
	}
    //读取文件中各频率值，并插入到以cpu_freq_head为头的链表中
    int freq_num = 0;  //可用频率个数
    while(!feof(fp_read)&&(fp_read!=NULL))
    {
        fscanf(fp_read,"%s",buf);
        new_freq = calloc(1,sizeof(CPU_FREQ));
        if (new_freq == NULL)
        {
            printf("%s:calloc err\r\n", __func__);
            return -1;
            break;
        }
        new_freq->freq = atoi(buf);
        printf("cpu frequency num:%d,frequency is %d MHz\r\n",freq_num,new_freq->freq/1000);
        Insert_list(cpu_freq_head, new_freq);
        freq_num++;
    }
    fclose(fp_read);
    remove(freq_table_file);
    printf("cpu %d has %d availble frequencies\n",cpu_num,freq_num);
    printf("\n=================function :%s finish \tcpu number: %d==================\n",__func__,cpu_num);
    return freq_num;
}

/* 2、获取当前前CPU运行频率，打印出来并返回 */
int get_curl_freq(int cpu_num)
{
	FILE *pp;
	char cmd[512];
	char cpufreq[64];
	int cpu_curl_freq =0;

    printf("\n=================function :%s start \tcpu number: %d==================\n",__func__,cpu_num);
    /* 获取CPU 0/1/2/3 对应的当前运行frequency路径*/
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "busybox cat %s", _CPU_FREQ_GET);
	sprintf(cmd, cmd, cpu_num);
	printf("cpu: %d; get current frequency cmd is: %s\n",cpu_num,cmd);
	pp = popen(cmd, "r");

	//如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s open err:%s\r\n", __func__,strerror(errno)); //使用strerror（）翻译错误代码
        return -1;
    }
	if (fgets(cpufreq, sizeof(cpufreq), pp) == NULL) {
		printf("popen read from %s is NULL!\n",cmd);
		pclose(pp);
		return -1;
	}
	pclose(pp);
	cpu_curl_freq = atoi(cpufreq);
	printf("cpu：%d ; current frequency is: %d MHz\n",cpu_num,cpu_curl_freq/1000);

	printf("\n=================function :%s finish \tcpu number: %d==================\n",__func__,cpu_num);
	return cpu_curl_freq;
}

/* 3、设置CPU一个支持的CPU frequency */
//设置CPU 模式
int cpu_set_mode(char *mode)
{
	char cmd[512];
	printf("\n=================== function :%s start======================\n",__func__);

	if (cpu_num0 > 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "echo " "%s" " > %s", mode,_CPU_0_FREQ_GOVERNOR);
		system(cmd);
	}

	if (cpu_num1 > 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "echo " "%s" " > %s", mode,_CPU_1_FREQ_GOVERNOR);
		system(cmd);
	}

	if (cpu_num2 > 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "echo " "%s" " > %s", mode,_CPU_2_FREQ_GOVERNOR);
		system(cmd);
	}

	if (cpu_num3 > 0) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "echo " "%s" " > %s", mode,_CPU_3_FREQ_GOVERNOR);
		system(cmd);
	}
	printf("\n=================== function :%s finish======================\n",__func__);
	return 0;
}

//设置CPU 列表里面的频率，返回设置的频率
int set_curl_freq(int cpu_num, int freq_num)
{
	char cmd[512];
	char CPU_FREQ_SET_PATH[128];
	CPU_FREQ *cpu_freq;
	CPU_FREQ *p;
    int freq_set = 0;
    printf("\n=================== function :%s start======================\n",__func__);

    //选择CPU_num对应的频率链表
    switch(cpu_num){
        case 0:
            cpu_freq = &cpu_0;break;
        case 1:
            cpu_freq = &cpu_1;break;
        case 2:
            cpu_freq = &cpu_2;break;
        case 3:
            cpu_freq = &cpu_3;break;
    }

    p = cpu_freq;
	while(p) {
        if ((--freq_num) <= 0)
			break;
		if (p->next)
			p = p->next;
	}

    /* 设置CPU 频率*/
	freq_set = p->freq;
	memset(cmd, 0, sizeof(cmd));
	sprintf(CPU_FREQ_SET_PATH,_CPU_FREQ_SET,cpu_num);
	sprintf(cmd, "echo %d > %s", freq_set, CPU_FREQ_SET_PATH);
	printf("cpu: %d; set current frequency cmd is: %s\n",cpu_num,cmd);
	system(cmd);

	printf("\n=================== function :%s finish======================\n",__func__);
	return freq_set;
}


void *cpu_test(void *argv)
{
    char cmd[128];
    int test_flag=0;
    printf("=======  cpu frequency test starting   ========\n\n");

    /* 1、首先打印出每个CPU支持的CPU frequency table */
    /* get all freq */
    printf("********************get cpu frequency table start ***************\n");
	cpu_num0 = get_cpu_freq_table(0,&cpu_0);
	cpu_num1 = get_cpu_freq_table(1,&cpu_1);
//	cpu_num2 = get_cpu_freq_table(2,&cpu_2);
//	cpu_num3 = get_cpu_freq_table(3,&cpu_3);
    if(-1 == cpu_num0|| -1== cpu_num1||-1 ==cpu_num2 || -1 ==cpu_num3)
    {
        test_flag = -1;
        goto fail;
    }

	printf("********************get cpu frequency table finish***************\n\n");

    /* 2、然后打印出目前CPU运行的frequency */
    printf("********************get current cpu frequency start ***************\n");
    int cpu_0_freq,cpu_1_freq;
    cpu_0_freq = get_curl_freq(0);
    cpu_1_freq = get_curl_freq(1);
    if(cpu_0_freq == -1 || cpu_1_freq==-1)
    {
        test_flag = -1;
        goto fail;
    }
    printf("********************get current cpu frequency finish ***************\n\n");

    /* 3、设置CPU一个支持的CPU frequency */
     printf("********************set cpu frequency start ***************\n");
    /* set mode to userspace */
	test_flag = cpu_set_mode(_CPU_MODE_USER);
	int freq_get,freq_set =-1;
	int cpu_num = 0;                //选择CPU 0
	int freq_num = rand()%cpu_num0;       //随机选择CPU支持频率列表中的一项；可能出错
	freq_set = set_curl_freq(cpu_num, freq_num);
	printf("********************set cpu frequency finish ***************\n");

    /* 4、读取CPU的frequency， 查看是否与设置的一致*/
    freq_get = get_curl_freq(cpu_num);
    if(freq_set == freq_get)
    {
        printf("Cpu frequency set and get is equal; is %d MHz\n",freq_set/1000);
    }
    else{
        printf("Cpu frequency set and get is not equal; set failed\n");
        printf("Cpu frequency set is :%d MHz\n",freq_set);
        printf("Cpu frequency get is :%d MHz\n",freq_get);
        test_flag = -1;
        goto fail;
    }
    printf("=======  cpu frequency test success   ========\n");

    return (void*)test_flag;
    fail:
        printf("=======  cpu frequency test failed   ========\n");

        return (void*)test_flag;
}

//主函数启动cpu_test
int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "cpu_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    test_flag = (int)cpu_test(argv[0]);
    if(test_flag < 0)
    {
        strcpy(result,RESULT_FAIL);
        err_code = CPU_PROC_ERR;
    }
    send_msg_to_server(buf, result, err_code);
}


