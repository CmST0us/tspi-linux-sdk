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
//WLAN test program
#include <stdio.h>

//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//error相关头文件
#include <errno.h>
#include <string.h>
#include "wlan_test.h"

#define LOG_TAG "wlan_test"
#include "common.h"
#define WLAN_PROC_ERR -45

#define WLAN_START_UP_COMMAND "ifconfig wlan0 up"
#define WLAN_MANAGE_START "wpa_supplicant -i wlan0 -c /etc/wpa_supplicant.conf &"
#define WLAN_SCAN_COMMAND "wpa_cli -i wlan0 scan 'wlan0'"
#define WLAN_SCAN_RESULT  "wpa_cli -i wlan0 scan_r | busybox awk 'NR>=3{print $5,$3}'"
#define SCAN_RESULT_FILE "/tmp/wlan_scan_result.txt"

#define AP_SUPPORT_CMD "iw list | busybox grep AP > %s"
#define AP_SUPPORT_FILE "/tmp/wlan_ap_result.txt"
#define SOFTAP_MSG "dmesg | grep SSID | tail -n 6 > %s"
#define MSG_FILE "/tmp/softap_start_msg.txt"

#ifdef PCBA_PX3SE
#define CONNECT_AP_CMD "wpa_supplicant -i wlan0 -c /data/wpa_conf &"
#define CONNECT_AP_STATUS "wpa_cli status > %s"
#define CONNECT_AP_RESULT_FILE "/tmp/wlan_connect_result.txt"

#define REPORT_SSID     "Rockchip" //指定扫描的的wifi ssid名称
#endif

//* 1、关闭后台wpa_supplicant进程
static int close_wpa_supplicant(void)
{
    int test_flag = -1;
    char pid_buf[64];
    char cmd[64];
    FILE* pp;
    printf("====================function : %s start =================\n",__func__);
    pp = popen("ps |grep wpa_supplicant|awk 'NR==1 {print $1}'","r");
    //pp = popen("ps | grep wpa_supplicant | grep -v grep","r");
    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s popen err%s\n",__func__,strerror(errno));
        return -1;
    }
    memset(pid_buf,0,sizeof(pid_buf));
    fgets(pid_buf,sizeof(pid_buf),pp);
    pclose(pp);
    printf("Get pid_buf is: \t %s\n",pid_buf);
    sprintf(cmd,"kill -9 %d",atoi(pid_buf));
    printf("cmd is: %s\n",cmd);
    system(cmd);
    printf("====================function : %s finish =================\n",__func__);
    return 0;
}

/*
 * RSSI Levels as used by notification icon
 *
 * Level 4  -55 <= RSSI
 * Level 3  -66 <= RSSI < -55
 * Level 2  -77 <= RSSI < -67
 * Level 1  -88 <= RSSI < -78
 * Level 0         RSSI < -88
 */
static int calc_rssi_lvl(int rssi)
{
	if (rssi >= -55)
		return 4;
	else if (rssi >= -66)
		return 3;
	else if (rssi >= -77)
		return 2;
	else if (rssi >= -88)
		return 1;
	else
		return 0;
}


/* 将扫描到的WiFi信息输出到path对应文件
 * 并显示第一个WiFi名字和信号强度
*/
int wlan_exec(const char *cmd, char *path)
{
    char result_buf[128];
    char ssid[128];
    int signal_level;
    int ch=0;
    int fd;

     printf("=================== function :%s start======================\n\n",__func__);

    memset(result_buf, 0, sizeof(result_buf));
    memset(ssid, 0, sizeof(ssid));
    //显示第一个WiFi信息
    FILE *pp = popen(cmd, "r");
    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s popen err%s\n",__func__,strerror(errno));
        return -1;
    }

#ifdef PCBA_PX3SE
    /*chad.ma add below*/
    if (path != NULL) {
        fd = open(path, O_CREAT | O_WRONLY	| O_TRUNC);
        if (fd <0) {
            log_err("open %s fail, errno = %d\n", path, errno);
            return errno;
        }
    }
    /*chad.ma add up*/
#endif

	while(!feof(pp))
	{
	    fscanf(pp,"%s",ssid);
	    fscanf(pp,"%d",&signal_level);
	    //根据信号强度判断WiFi是否正常工作
	    if(signal_level< -200||signal_level >=0)
        {
            printf("wlan scan result is null\n");
            return -1;
        }
#ifdef PCBA_PX3SE
        snprintf(result_buf,sizeof(result_buf),
                    "SSID:%s Signal Level:%d RSSI:%d\n",
                    ssid, signal_level, calc_rssi_lvl(signal_level));
        int w_len = write(fd, result_buf, strlen(result_buf));
		if (w_len <= 0) {
			log_err("Write %s fail, errno = %d\n", path, errno);
			return errno;
		}
#endif

	    printf("SSID is: %s,signal level is: %d\n",ssid,signal_level);
	    printf("SSID is: %s,rssi is: %d\n",ssid,calc_rssi_lvl(signal_level));
	}
	pclose(pp);

#ifdef PCBA_PX3SE
    close(fd);
#endif
	printf("\n=================== function :%s finish======================\n",__func__);
	return 0;
}

void *wlan_test(void *argv)
{
    int test_flag=-1;
    char cmd[128];
    printf("===================Wlan test start======================\n");
    //sprintf(cmd,"aplay %s/wlan_test_start.wav",AUDIO_PATH);
    //system(cmd);
    //system("aplay /data/cfg/test/wlan_test_start.wav");

    //1、先关闭后台wpa_supplicant进程，然后先卸载WLAN驱动固件，然后重新加载驱动固件
    test_flag = close_wpa_supplicant();
    system("echo 1 > /sys/class/rkwifi/driver");

    //2、开启wlan0端口
    int status = system(WLAN_START_UP_COMMAND);

    /* add by chad ma 20180629 below judge*/
    if(status == -1) {
        printf("system run error ....\n");
        goto fail;
    } else {
        log_info("exit status value = [0x%x]\n", status);
        if (WIFEXITED(status))
        {
            if (0 == WEXITSTATUS(status))
            {
                log_info("run WLAN_START_UP_COMMAND successfully.\n");
                //ret = 0;
            }
            else
            {
                system("rfkill unblock wifi");
            }
        }
        else
        {
            log_info("exit status = [%d]\n", WEXITSTATUS(status));

            //ret = -1;
        }
    }
    sleep(2);

    //3、启动无线网卡管理程序wpa_supplicant
    system(WLAN_MANAGE_START);
    sleep(2);

    //4、扫描无线网络
    system(WLAN_SCAN_COMMAND);
    sleep(3);

    //5、显示出WiFi信息
    test_flag = wlan_exec(WLAN_SCAN_RESULT,SCAN_RESULT_FILE);
    if(test_flag<0)
    {
        //第一次scan失败，再执行一遍;
        system(WLAN_SCAN_COMMAND);
        sleep(2);
        test_flag = wlan_exec(WLAN_SCAN_RESULT,SCAN_RESULT_FILE);
        if(test_flag<0)
        goto fail;
        //return -1;
    }

    printf("===================Wlan test success ======================\n");
    system("ifconfig wlan0 down");
    system("busybox killall wpa_supplicant");      //关闭无线管理程序wpa_supplicant
    return (void*)test_flag;
fail:
    printf("===================Wlan test failed======================\n");
    system("ifconfig wlan0 down");
    system("busybox killall wpa_supplicant");      //关闭无线管理程序wpa_supplicant
    return (void*)test_flag;
}

int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char read_buf[128];
    int find_flag = 0;
    FILE *fp;
    char buf[COMMAND_VALUESIZE] = "wlan_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    test_flag = (int)wlan_test(argv[0]);
    if(test_flag < 0)
    {
        strcpy(result,RESULT_FAIL);
        err_code = WLAN_PROC_ERR;
    }

#ifdef PCBA_PX3SE
    //从扫描结果保存文件中，扫描指定ssid的signal
    fp = fopen(SCAN_RESULT_FILE, "r");
    memset(read_buf, 0 ,sizeof(read_buf));
     //如果文件打开失败，则输出错误信息
    if (!fp)
    {
        printf("%s fopen err:%s\n",__func__,strerror(errno));
        return -1;
    }

    while(!feof(fp))
    {
        fgets(read_buf,sizeof(read_buf),fp);
        if(strstr(read_buf,REPORT_SSID)!= NULL)
        {
            find_flag = 1;
            //注意：read_buf会将一行的回车符也读进去，这里要对read_buf进行改造
            int len = strlen(read_buf);
            if ( len < 128) {
                //assert(len < 128);
                read_buf[len - 1] = '\0';
                printf("\n ###Found!!! %s ###\n",read_buf);
                break;
            } else {
                printf("%s ERROR!!! read to much from one line\n",__func__);
            }
        }
    }
    fclose(fp);
    //将保存结果复制到buf中
    if (find_flag) {
        strcat(buf, ": ");
        strcat(buf, read_buf);
    }

    //删除扫描文件
    //remove(SCAN_RESULT_FILE);
    printf("\n $$ %s :not found %s $$\n",buf , REPORT_SSID);
#endif
    send_msg_to_server(buf, result, err_code);
}
