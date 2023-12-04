/*
 *  bluetooth_test.c  --  bluetooth test application
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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

//*open 函数所需头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//error 相关头文件
#include <errno.h>
#include "bt_test.h"

//_IOW 所需头文件
#include <sys/ioctl.h>
//WIFEXITED 所需头文件
#include <sys/wait.h>

#define LOG_TAG "bt_test"
#include "common.h"

//*定义循环读取蓝牙地址时间间隔以及超时时间
#define PERIOD_TIME 1
#define BT_TIMEOUT_FIRMWARE 7

#define BT_PROC_ERR -30
#define BT_FIRMWARE_LOAD_ERR -31

#define WIFI_CHIP_TYPE_PATH "/sys/class/rkwifi/chip"
#define BT_ON 1
#define BT_OFF 0

#define BT_INIT_ADDR "00:00:00:00:00:00"

#ifndef HCI_DEV_ID
#define HCI_DEV_ID 0
#endif

enum WIFI_CHIP_TYPE_LIST{
    BT_UNKNOWN = -1,
	BCM4329 = 0,
	RTL8188CU,
	RTL8188EU,
	BCM4330,
	RK901,
	RK903,
	MT6620,
	RT5370,
    MT5931,
    RDA587x,
    RDA5990,
    RTK8723AS,
    RTK8723BS,
    RTK8723AU,
    RTK8723BU,
    BK3515,
    SOFIA_3GR,
};

static int rfkill_bt_id = -1;
static char rfkill_state_path[128];
static int bluetooth_power_status = 0;
static int chip_type;

//* 1、初始化rfkill，获取Bluetooth对应id
static int init_rfkill() {
    char path[64];
    char buf[16];
    int fd;
    int length_r;
    int id;
    printf("======================init rfkill start==================\n\n");
    for (id = 0; ; id++) {
        sprintf(path, "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("open(%s) failed: %s (%d)\n", path, strerror(errno), errno);
            printf("======================init rfkill failed==================\n\n");
            return -1;
        }
        length_r = read(fd, &buf, sizeof(buf));
        close(fd);
        if (length_r >= 9 && memcmp(buf, "bluetooth", 9) == 0) {
            rfkill_bt_id = id;
            break;
        }
    }
    printf("\t Bluetooth initialize id is :%d\n",rfkill_bt_id);
    sprintf(rfkill_state_path, "/sys/class/rfkill/rfkill%d/state", rfkill_bt_id);
    printf("======================init rfkill success==================\n\n");
    return 0;
}

//* 2、获取Bluetooth芯片名字
int bt_get_chip_name(char* name, int len)
{
    int fd = -1;
    int length_r = 0;
    char rfkill_name_path[128];
    int ret = -1;

    printf("================bt_get_chip_name start================\n\n");
    sprintf(rfkill_name_path, "/sys/class/rfkill/rfkill%d/name", rfkill_bt_id);
    fd = open(rfkill_name_path, O_RDONLY|O_NOCTTY|O_NDELAY,0);
    if (fd < 0) {
        printf("open(%s) failed: %s (%d)", rfkill_name_path, strerror(errno),errno);
        goto out;
    }

    length_r = read(fd, name, len);
    if (length_r < 0) {
        printf("read(%s) failed: %s (%d)", rfkill_name_path, strerror(errno),errno);
        goto out;
    }
    name[length_r-1] = '\0';
    close(fd);
    printf("================bt_get_chip_name success================\n");
    return 0;
out:
    if (fd >= 0) close(fd);
    printf("================bt_get_chip_name failed================\n");
    return -1;
}

//* 3、获取芯片类型
static int get_chip_type(char *bt_chip_name)

{
    int chip_type;
    char chip_type_name[64];
    memset(chip_type_name,0,sizeof(chip_type_name));
    printf("================bluetooth get_chip_type start================\n\n");
    if(!memcmp(bt_chip_name, "rk903",strlen("rk903"))) {
		chip_type = RK903;
		memcpy(chip_type_name,"RK903",strlen("RK903"));
	} else if(!memcmp(bt_chip_name, "mt6622",strlen("mt6622"))) {
		chip_type = MT5931;
		memcpy(chip_type_name,"MT5931",strlen("MT5931"));
	} else if(!memcmp(bt_chip_name, "rda587x",strlen("rda587x"))) {
		chip_type = RDA587x;
		memcpy(chip_type_name,"RDA587x",strlen("RDA587x"));
	} else if(!memcmp(bt_chip_name, "rda5990",strlen("rda5990"))) {
		chip_type = RDA5990;
		memcpy(chip_type_name,"RDA5990",strlen("RDA5990"));
	} else if(!memcmp(bt_chip_name, "rtk8723as",strlen("rtk8723as"))) {
		chip_type = RTK8723AS;
		memcpy(chip_type_name,"RTK8723AS",strlen("RTK8723AS"));
	} else if(!memcmp(bt_chip_name, "rtk8723bs",strlen("rtk8723bs"))) {
		chip_type = RTK8723BS;
		memcpy(chip_type_name,"RTK8723BS",strlen("RTK8723BS"));
	} else if(!memcmp(bt_chip_name, "rtk8723au",strlen("rtk8723au"))) {
		chip_type = RTK8723AU;
		memcpy(chip_type_name,"RTK8723AU",strlen("RTK8723AU"));
	} else if(!memcmp(bt_chip_name, "rtk8723bu",strlen("rtk8723bu"))) {
		chip_type = RTK8723BU;
		memcpy(chip_type_name,"RTK8723BU",strlen("RTK8723BU"));
    } else if(!memcmp(bt_chip_name, "bk3515",strlen("bk3515"))) {
        chip_type = BK3515;
        memcpy(chip_type_name,"BK3515",strlen("BK3515"));
	} else if(!memcmp(bt_chip_name, "Sofia-3gr",strlen("Sofia-3gr"))){
        chip_type = SOFIA_3GR;
        memcpy(chip_type_name,"SOFIA_3GR",strlen("SOFIA_3GR"));
	}
    else if (!memcmp(bt_chip_name, "rk903_26M",strlen("rk903_26M"))){
        chip_type = RK903;
        memcpy(chip_type_name,"RK903",strlen("RK903"));
    }
    else if (!memcmp(bt_chip_name, "rk903",strlen("rk903")))
        {
        chip_type = RK903;
        memcpy(chip_type_name,"RK903",strlen("RK903"));
    }
    else if (!memcmp(bt_chip_name, "ap6210",strlen("ap6210"))){
        chip_type = RK903;
        memcpy(chip_type_name,"RK903",strlen("RK903"));
    }
    else if (!memcmp(bt_chip_name, "ap6335",strlen("ap6330"))){
        chip_type = RK903;
        memcpy(chip_type_name,"RK903",strlen("RK903"));
    }
    else if (!memcmp(bt_chip_name, "ap6476",strlen("ap6476"))){
        chip_type = RK903;
        memcpy(chip_type_name,"RK903",strlen("RK903"));
    }
    else if (!memcmp(bt_chip_name, "ap6493",strlen("ap6493"))){
        chip_type = RK903;
        memcpy(chip_type_name,"RK903",strlen("RK903"));
    }
    else {
        printf("Not support BT chip, skip bt test.\n");
        chip_type = BT_UNKNOWN;
        memcpy(chip_type_name,"BT_UNKNOWN",strlen("BT_UNKNOWN"));
    }
    printf("chip type is: %s\n", chip_type_name);
    printf("================bluetooth get_chip_type finish================\n");
    return chip_type;
}

//* 4->1、关闭后台brcm_patchram_plus1进程
int close_brcm_pathcram_plus1(void)
{
    int test_flag = -1;
    char pid_buf[64];
    char cmd[64];
    FILE* pp;
    printf("====================function : %s start =================\n",__func__);
    pp = popen("ps |grep brcm_patchram_plus1|awk 'NR==1 {print $1}'","r");
    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s popen err%s\n",__func__,strerror(errno));
        return -1;
    }
    fgets(pid_buf,sizeof(pid_buf),pp);
    pclose(pp);
    printf("Get pid_buf is: \t %s\n",pid_buf);
    sprintf(cmd,"kill -9 %d",atoi(pid_buf));
    printf("cmd is: %s\n",cmd);
    system(cmd);
    printf("====================function : %s finish =================\n",__func__);
    return 0;
}

//* 4->2->1、获取hci0的蓝牙地址,确认固件是否加载成功，成功得到蓝牙地址，失败则无蓝牙地址
int get_bdaddr_test(void)
{
    int test_flag =-1;
    char bdaddr[64];
    char cmd[64];
    int count =0;
    printf("*******************function:%s start******************\n",__func__);
    FILE* pp = popen("hciconfig -a| grep \"BD Address:\"|awk '{print $3}'","r");
    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s popen err%s\n",__func__,strerror(errno));
        return -1;
    }
    memset(bdaddr,0,sizeof(bdaddr));
    fscanf(pp,"%s",bdaddr);
    pclose(pp);
    if(bdaddr[0]!='\0')
    {
       printf("Get bluetooth device address is: \t  %s\n",bdaddr);
       test_flag = 0;
    }
    else
    {
        //printf("Failed to load bluetooth firmware.\n");
        printf("loadind bluetooth firmware.....\n");
        test_flag = -1;
    }
    if(!memcmp(bdaddr,BT_INIT_ADDR,strlen(BT_INIT_ADDR)))
    {
        printf("hci0 interface create failed\n");
        test_flag = -1;
    }
    printf("*******************function:%s finish******************\n",__func__);
    return test_flag;
}

//* 4->2->2、循环读取hci0的蓝牙地址,确认固件是否加载成功，如果在30s内获取蓝牙地址，固件加载成功，否则失败
static int confirm_firmware_test(void)
{
    int period = PERIOD_TIME;
    int time_out = 0;
    int test_flag = 0;

    printf("*******************function:%s start******************\n",__func__);
    while(time_out < BT_TIMEOUT_FIRMWARE)
    {

        //每隔固定时间读取一次蓝牙地址
        sleep(period);
        if(!get_bdaddr_test()){
                break;
        }
        time_out +=PERIOD_TIME;
    }
    if(time_out > BT_TIMEOUT_FIRMWARE)
    {
        printf("Failed to load bluetooth firmware.\n");
        test_flag = -1;
    }
    printf("*******************function:%s finish******************\n",__func__);
    return test_flag;
}

//* 4->3、使用hciconfig hci0 up 激活蓝牙，然后使用hcitool dev查看是否激活
int activate_bt_test(void)
{
    int test_flag = -1;
    char bdaddr[64];
    char cmd[64];
    FILE* pp = NULL;

    printf("*******************function:%s start******************\n",__func__);
    system("hciconfig hci0 up");
    pp = popen("hcitool dev|grep hci0|awk '{print $2}'","r");
    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s popen err%s\n",__func__,strerror(errno));
        return -1;
    }
    memset(bdaddr,0,sizeof(bdaddr));
    fscanf(pp,"%s",bdaddr);
    pclose(pp);
    if(bdaddr[0]!='\0')
    {
        printf("Get bluetooth device address is :\t  %s\n",bdaddr);
        test_flag = 0;
    }
    else{
        printf("hci0 activates failed.\n");
        test_flag = -1;
    }
    printf("*******************function:%s finish******************\n",__func__);
    return test_flag;
}

//* 4->4、扫描周围的蓝牙设备，获取远端蓝牙设备地址,返回扫描得到的远端设备数量
int bt_scan_test(void)
{
    int test_flag = -1;
    char bdaddr[64];
    char cmd[64];
    int count =0;
    FILE* pp;
    printf("--------------------function: %s start-------------------\n",__func__);
    pp = popen("hcitool scan |awk 'NR>=2{print $1}'","r");
    //如果文件打开失败，则输出错误信息
    if (!pp)
    {
        printf("%s popen err%s\n",__func__,strerror(errno));
        return -1;
    }
    while(!feof(pp))
    {
        memset(bdaddr,0,sizeof(bdaddr));
        fscanf(pp,"%s",bdaddr);
        if((bdaddr[0]!='\0'))
        {
            printf("Get remote Bluetooth device address is : \t  %s\n",bdaddr);
            count++;
        }
        else if(!count)
        {
            printf("Failed to scan remote bluetooth.\n");
            return -1;
        }
    }
    pclose(pp);
    printf("--------------------function: %s success-------------------\n",__func__);
    return count;
}


//* 4、蓝牙测试主程序
int bt_test_bluez(void)
{
    int test_flag = -1;
    char cmd[128];
    int dev_cnt = 0;
    int status;
    //* 1) 关闭蓝牙，首先关闭后台brcm_patchram_plus1进程，然后给蓝牙下电
    test_flag = close_brcm_pathcram_plus1();
    if(test_flag<0)
        goto out;
    sprintf(cmd,"echo 0 > %s",rfkill_state_path);
    system(cmd);


    //* 2) 确定蓝牙关闭后，重新给蓝牙上电，加载固件
    sprintf(cmd,"echo 1 > %s",rfkill_state_path);
    system(cmd);
    //status = system("brcm_patchram_plus1 --enable_hci --no2bytes \
    //       --use_baudrate_for_download  --tosleep  200000 \
    //       --baudrate 1500000 --patchram /data/bcm4339a0.hcd /dev/ttyS1 &");
#ifdef PCBA_3308
    status = system("brcm_patchram_plus1 --enable_hci --no2bytes \
           --use_baudrate_for_download  --tosleep  200000 \
           --baudrate 1500000 --patchram /system/etc/firmware/BCM4345C0.hcd /dev/ttyS4 &");
#endif

#ifdef PCBA_PX3SE
    status = system("rtlbt  -n -s 115200 /dev/ttyS1 rtk_h5 > /data/cfg/hciattach.txt  2>&1 &");
#endif
#ifdef PCBA_3229GVA
    status = system("brcm_patchram_plus1 --enable_hci --no2bytes \
           --use_baudrate_for_download  --tosleep  200000 \
           --baudrate 1500000 --patchram /data/bcm4339a0.hcd /dev/ttyS1 &");
#endif
    test_flag = confirm_firmware_test();
    if(test_flag < 0)
        goto out;

    //* 3) 激活蓝牙，然后使用hcitool dev查看是否激活
    system("hciconfig hci0 up");
    test_flag = activate_bt_test();
    if(test_flag < 0)
        goto out;

    //* 4) 扫描周围的蓝牙设备,打印出设备地址
//    dev_cnt = bt_scan_test();
//    if(dev_cnt < 0)
//        goto out;
    printf("Get remote Bluetooth device number is: %d\n",dev_cnt);
    printf("==============function: %s success ================\n",__func__);
    return 0;
out:
    printf("==============function: %s failed ================\n",__func__);
    return -1;
}

//* Bluetooth test function
void *bt_test(void *argv)
{
    int bt_init_result,bt_setPower_result;
    char cmd[64];
    char bt_chip_name[64];
    int bt_chip_type;
    int ret = -1;
    printf("======================Bluetooth test start================\n\n");
    //sprintf(cmd,"aplay %s/bt_test_start.wav",AUDIO_PATH);
    //system(cmd);
    //system("aplay /data/cfg/test/bt_test_start.wav");
    //1、初始化rfkill，获取Bluetooth对应id
    ret = init_rfkill();
    if(ret < 0){
		printf("function: %s failed! %s\n",__func__,strerror(errno));
		goto fail;
	}
    printf("Bluetooth is /sys/class/rfkill/rfkill:%d\n",rfkill_bt_id);

    //2、获取Bluetooth芯片名字，chip name
    ret = bt_get_chip_name(bt_chip_name,sizeof(bt_chip_name));
    if(ret < 0){
		printf("function: %s failed! %s\n",__func__,strerror(errno));
		goto fail;
	}
	printf("Bluetooth chip name is %s\n",bt_chip_name);

    //3、获取芯片类型、chip type
    bt_chip_type = get_chip_type(bt_chip_name);
    if(bt_chip_type < 0){
		printf("function: %s failed! %s\n",__func__,strerror(errno));
		goto fail;
	}
    printf("Bluetooth chip type is: bt_chip_type = %d\n", bt_chip_type);

    //4、测试Bluetooth主程序
     ret = bt_test_bluez();
     if(ret < 0){
        printf("bluetooth_test main function: %s failed.\n",__func__);
		goto fail;
     }
    printf("======================Bluetooth test success================\n");
    system("busybox killall brcm_patchram_plus1");      //关闭固件加载程序
    return (void*)ret;
fail:
    printf("======================Bluetooth test failed================\n");
    system("busybox killall brcm_patchram_plus1");      //关闭固件加载程序
    return (void*)ret;
}

//*主程序入口
int main(int argc, char *argv[])
{
    int test_flag = 0,err_code = 0;
    char buf[COMMAND_VALUESIZE] = "bt_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;
    test_flag = (int)bt_test(argv[0]);
    if(test_flag < 0)
    {
        strcpy(result,RESULT_FAIL);
        err_code = BT_PROC_ERR;
        printf("===err_code = %d===\n",err_code);
    }
    if (err_code != 0)
        strcpy(result, RESULT_FAIL);
    send_msg_to_server(buf, result, err_code);
}


