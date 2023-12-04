#ifndef __RK_PCBA_TEST_LED_H_
#define __RK_PCBA_TEST_LED_H_

#include <stdio.h>
#include <stdlib.h>

//error相关头文件
#include <string.h>
#include <errno.h>

//access所需头文件
 #include <unistd.h>

//open()相关头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define LED_BRIGHTNESS_FILE "/sys/class/leds/led%d/brightness"

#define LED_NUM 36

//* 定义LED开关
#define LED_ON "128"
#define LED_OFF "0"
#define AUDIO_PATH "/data/cfg/rk_pcba_test"
//#define PCBA_TEST_PATH "/data/cfg/rk_pcba_test"

int write_int(char const* path,char const* value);
int light_up_led_red(void);
int light_up_led_green(void);
int light_up_led_blue(void);


/*亮红灯1s，然后熄灭*/
int light_up_led_red(void)
{
    char buf[128];
    int led_num;

    for(led_num=2;led_num<=LED_NUM;)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        write_int(buf,LED_ON);
		led_num = led_num+3;
    }

    sleep(1);       //暂停1秒

    for(led_num=2;led_num<=LED_NUM;)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        write_int(buf,LED_OFF);
		led_num = led_num+3;
    }
	return 0;
}

/*亮绿灯1s，然后熄灭*/
int light_up_led_green(void)
{
    char buf[128];
    int led_num;

    for(led_num=1;led_num<=LED_NUM;)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        write_int(buf,LED_ON);
		led_num = led_num+3;
    }

    sleep(1);       //暂停1秒

    for(led_num=1;led_num<=LED_NUM;)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        write_int(buf,LED_OFF);
		led_num = led_num+3;
    }
	return 0;
}
/*亮绿灯1s，然后熄灭*/
int light_up_led_blue(void)
{
    char buf[128];
    int led_num;

    for(led_num=3;led_num<=LED_NUM;)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        write_int(buf,LED_ON);
		led_num = led_num+3;
    }

    sleep(1);       //暂停1秒

    for(led_num=3;led_num<=LED_NUM;)
    {
        sprintf(buf,LED_BRIGHTNESS_FILE,led_num);
        write_int(buf,LED_OFF);
		led_num = led_num+3;
    }
	return 0;
}

//打开LED亮度文件路径，并写入亮度值0-255
int write_int(char const* path,char const* value)
{
    int fd;
    char read_buffer[64],write_buffer[64];         //读缓存数据
    ssize_t length_r,length_w;                     //读写数据长度

    static int already_warned = 0;

    //1、打开文件亮度调节文件结点
    fd = open(path, O_RDWR|O_NOCTTY|O_NDELAY);
    if (fd >= 0)
    {
        char buffer[128];
        int bytes;
        length_r = read(fd, buffer, 64);
        printf("The current value of %s is : %d\n",path,atoi(buffer));   //打印读取的字节数与内容
        if (!memcmp(buffer,value,length_r)) //don't need to set value, because the value is same with the target
        {
            close(fd);
            return 0;
        }
        bytes = strlen(value)+1;       //+1是多一个字符串结束符 '\0'
        length_w = write(fd, value, bytes);
        printf("%d bytes is wrote to node %s, the current value is %s\n",length_w,path,value);

        close(fd);
        return length_w == -1 ? -1 : 0;
    }
    else
    {
        fprintf(stderr,"can't open led driver file,%s\n",path);
        if (already_warned == 0)
        {
            fprintf(stderr,"write_int failed to open led driver %s\n", path);
            already_warned = 1;
        }
        return -1;
    }
}

#endif // RK_PCBA_TEST_LED_H_INCLUDED
