/*
 *  echo_audio_linein_test.c  --  Audio line in test application
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

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <assert.h>

#include <stdbool.h>
#include <signal.h>
#include "audio_test.h"

#define LOG_TAG "audio_linein_test"
#include "common.h"

#if 0
snd_pcm_t *open_sound_dev(snd_pcm_stream_t type)
{
    int err;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hw_params;
    unsigned int buffer_time;
    unsigned int period_time;
    unsigned int rate = 44100;

    if ((err = snd_pcm_open (&handle, "hw:0,0", type, 0)) < 0)     //打开设备，第2个参数是指使用默认的 设备名称
        return NULL;

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {  //分配参数
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                 snd_strerror (err));
        return NULL;
    }

    if ((err = snd_pcm_hw_params_any (handle, hw_params)) < 0) {  //初始化参数
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                 snd_strerror (err));
        return NULL;
    }

    if (snd_pcm_hw_params_get_buffer_time_max(hw_params, &buffer_time, 0) < 0) {
        fprintf(stderr, "Error snd_pcm_hw_params_get_buffer_time_max\n");
        log_err("Error snd_pcm_hw_params_get_buffer_time_max(%s)\n",snd_strerror (err));
    }

    if (buffer_time > 500000)  buffer_time = 500000;
    period_time = buffer_time / 4;

    log_info("buff_time = %d, period_time =%d \n",buffer_time, period_time);
    if (snd_pcm_hw_params_set_buffer_time_near(handle, hw_params, &buffer_time, 0) < 0) {
        fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near\n");
        log_err("Error snd_pcm_hw_params_set_buffer_time_near(%s)\n",snd_strerror (err));

        return NULL;
    }

    if (snd_pcm_hw_params_set_period_time_near(handle, hw_params, &period_time, 0) < 0) {
        fprintf(stderr, "Error snd_pcm_hw_params_set_buffer_time_near\n");
        log_err("Error snd_pcm_hw_params_set_buffer_time_near(%s)\n",snd_strerror (err));

        return NULL;
    }

    if ((err = snd_pcm_hw_params_set_access (handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) { //设置参数，数据存放方式是交叉存放SND_PCM_ACCESS_RW_INTERLEAVED（先放左声道再放右声道）
        fprintf (stderr, "cannot set access type (%s)\n",
                 snd_strerror (err));
        log_err("cannot set access type (%s)\n", snd_strerror (err));
        return NULL;
    }

    if ((err = snd_pcm_hw_params_set_format (handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n", //设置格式SND_PCM_FORMAT_S16_LE是16位的
                 snd_strerror (err));
        log_err("cannot set sample format (%s)\n", snd_strerror (err));
        return NULL;
    }

    if ((err = snd_pcm_hw_params_set_rate_near (handle, hw_params, &rate, 0)) < 0) { //设置采样率
        fprintf (stderr, "cannot set sample rate (%s)\n",
                 snd_strerror (err));
        log_err("cannot set sample rate (%s)\n", snd_strerror (err));
        return NULL;
    }

    if ((err = snd_pcm_hw_params_set_channels (handle, hw_params, 2)) < 0) { //设置通道数
        fprintf (stderr, "cannot set channel count (%s)\n",
                 snd_strerror (err));
        log_err("cannot set channel count (%s)\n",snd_strerror (err));
        return NULL;
    }

    if ((err = snd_pcm_hw_params (handle, hw_params)) < 0) { //把构造的结构体的数据写到硬件上去
        fprintf (stderr, "cannot set parameters (%s)\n",
                 snd_strerror (err));
        log_err("cannot set parameters (%s)\n",snd_strerror (err));
        return NULL;
    }

    snd_pcm_hw_params_free (hw_params); //释放含参数的结构体

    return handle;  //成功时返回句柄
}


void close_sound_dev(snd_pcm_t *handle) //关闭声音设备
{
    snd_pcm_close (handle);
}

snd_pcm_t *open_playback(void) //打开播放设备
{
    return open_sound_dev(SND_PCM_STREAM_PLAYBACK);
}

snd_pcm_t *open_capture(void) //打开录音设备
{
    return open_sound_dev(SND_PCM_STREAM_CAPTURE);
}

int main (int argc, char *argv[])
{
    int err;
    char buf[512];
    snd_pcm_t *playback_handle;
    snd_pcm_t *capture_handle;
    playback_handle = open_playback();// 打开播放设备

    if (!playback_handle) {
        fprintf (stderr, "cannot open for playback\n");
        log_err("cannot open for playback\n");
        return -1;
    }

    capture_handle = open_capture();//打开录音设备

    if (!capture_handle) {
        fprintf (stderr, "cannot open for capture\n");
        log_err("cannot open for capture\n");
        return -1;
    }

    if ((err = snd_pcm_prepare (playback_handle)) < 0) { //准备播放操作
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        log_err("cannot prepare audio interface for use (%s)\n",snd_strerror (err));
        return -1;
    }

    if ((err = snd_pcm_prepare (capture_handle)) < 0) { //准备录音操作
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                 snd_strerror (err));
        log_err("cannot prepare audio interface for use (%s)\n",snd_strerror (err));
        return -1;
    }

    while (1) {
        if ((err = snd_pcm_readi (capture_handle, buf, 128)) != 128) { //读操作（把数据读到buffer）
            fprintf (stderr, "read from audio interface failed (%s)\n",
                     snd_strerror (err));
            log_err("read from audio interface failed (%s)\n",snd_strerror (err));
            return -1;
        }

        if ((err = snd_pcm_writei (playback_handle, buf, 128)) != 128) { //写操作(把数据写到buffer)
            fprintf (stderr, "write to audio interface failed (%s)\n",
                     snd_strerror (err));
            log_err("write to audio interface failed (%s)\n",snd_strerror (err));
            return -1;
        }
    }

    snd_pcm_close (playback_handle);//关闭播放设备
    snd_pcm_close (capture_handle);//关闭录音设备
    return 0;
}
#endif

#define LINEIN_TIME 20

static bool gIslinein = false;

int switch_to_linein(bool sw_to_linein)
{
    int ret = 0;
    int status = 0;

    if (sw_to_linein) {
        log_info("-----------> swtich to line in <-----------\n");
        status = system("/tmp/switch_inoutput.sh li");
    } else {
        log_info("-----------> swtich to mic in <-----------\n");
        status = system("/tmp/switch_inoutput.sh mi");
    }

    if (status == -1) {
        log_info("system cmd run error...\n");
        ret = -1;
    }

    return ret;
}

//*Audio line in test
void *audio_line_in_test(void *argv)
{
    char cmd[128];

    fprintf(stderr,"=========function :%s start=============\n",__func__);

    gIslinein = true;
    switch_to_linein(gIslinein);

    //指定Card 0，device 0，录音10秒
    fprintf(stderr,"recording\n ");
    sprintf(cmd,"arecord -D hw:0,0 -f S16_LE -c 2 -r 44100 -d %d %s/%s", \
                LINEIN_TIME,TEST_RESULT_SAVE_PATH,AUDIO_LINEIN_FILE);
    system(cmd);

    fprintf(stderr,"playing\n ");
    sprintf(cmd,"aplay -f S16_LE -c 2 -r 44100 -d %d %s/%s", \
                LINEIN_TIME,TEST_RESULT_SAVE_PATH,AUDIO_LINEIN_FILE); //放音
    system(cmd);

    //remove line-in test file
    sprintf(cmd,"rm %s/%s",TEST_RESULT_SAVE_PATH,AUDIO_LINEIN_FILE);
    system(cmd);
    fprintf(stderr,"=========Audio Line in test finished=============\n");
}

//* 信号处理函数，在结束进程前，删除录音的pcm文件
static int del_line_in_pcm(int sign_no)
{
    char cmd[128];

    printf("====================function : %s start =================\n",__func__);

    sprintf(cmd,"%s/%s",TEST_RESULT_SAVE_PATH,AUDIO_LINEIN_FILE);
    if(0 == access(cmd,F_OK)){
       remove(cmd);
    }

    printf("====================function : %s finished =================\n",__func__);
    exit(0);
}



/*主函数入口*/
int main(int argc, char *argv[])
{
    int delay_t = 0,err_code = 0;
	struct timeval t1, t2;
	char buf[COMMAND_VALUESIZE] = "audio_linein_test";
    char result[COMMAND_VALUESIZE] = RESULT_PASS;

    system("amixer cset name='Master Playback Volume' 80%");
	log_info("audio record test start...\n");

	//* 注册信号处理函数
	signal(SIGTERM, (__sighandler_t)del_line_in_pcm);

	gettimeofday(&t1, NULL);
    while(1)
    {
        audio_line_in_test(argv[0]);
        gettimeofday(&t2, NULL);
		delay_t = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
		if (delay_t > MANUAL_TEST_TIMEOUT) {
			log_warn("audio record test end, timeout 60s\n");
			err_code = AUDIO_LINEIN_TIMEOUT;
			//break;
		}
    }

    if (gIslinein) {
        gIslinein = false;
        switch_to_linein(gIslinein);    //acodec switch to mic-->input
    }

    if (err_code)
		strcpy(result, RESULT_FAIL);
    send_msg_to_server(buf, result, err_code);
    return 0;
}

