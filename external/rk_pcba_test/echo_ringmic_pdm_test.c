/*
 * Copyright (c) 2017 Rockchip Electronics Co. Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* flytek ringmic test require:
* 1. recored more than 10 seconds
* 2. start play before recording, stop record before stop playing
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "mic_test_Linux/vibrate_test.h"
#include "mic_test_Linux/record_test.h"

#define LOG_TAG "echo_ringmic_test"
#include "common.h"

#define AUDIO_CHANNAL_CNT 8
#define RINGMIC_BAD_ERROR 93 /* has bad mic */

#define DEBUG_ON 0
#if DEBUG_ON
	#define FLAGS O_WRONLY | O_CREAT | O_TRUNC
	#define MODE S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH
	void save_audio_file(char *path, char *buff, int len)
	{
		int fd = 0;
		int ret = 0;
		char shell_cmd[200] = {0};
		sprintf(shell_cmd, "rm -rf %s", path);
		system(shell_cmd);
		if ((fd = open(path, FLAGS, MODE)) == -1) {
			log_err("%s Create %s failed, %s\n", __func__,
				path, strerror(errno));
			return;
		}
		ret = write(fd, buff, len);
		if (ret != len) {
			log_err("%s Write %s failed, %s\n", __func__,
				path, strerror(errno));
			return;
		}
		log_info("%s file Saved successfully!\n", path);
		close(fd);
		return;
	}
#else
	void save_audio_file(char *paht, char *buff, int len);
#endif

int *bytesToInt(char *src, int *length)
{
	int value = 0, offset = 0, i = 0;
	int *ret = NULL;
	int int_len = 0;

	int_len = (*length / 4) + 1;
	ret = (int *)malloc(int_len * 4);
	if (!ret)
		return NULL;

	while ((offset < *length) && (*length - offset >= 4)) {
		value = (int)((src[offset] & 0xFF) |
			      ((src[offset + 1] & 0xFF) << 8) |
			      ((src[offset + 2] & 0xFF) << 16) |
			      ((src[offset + 3] & 0xFF) << 24));
		offset += 4;
		ret[i++] = value;
	}

	*length = int_len;
	return ret;
}
int linear_amplification(void *data, void *out, int bytes)
{
    int i = 0, j = 0;
	int mChannels = AUDIO_CHANNAL_CNT;
	for (i = 0; i < bytes / 2; ) {
		for (j = 0; j < mChannels; j++) {
			short raw = (*((short *)data + i + j));
			int tmp_data = (int)raw;
#if 1
			tmp_data = tmp_data << 4;
			if(tmp_data > 32767) {
				tmp_data = 32767;
			} else if(tmp_data < -32768) {
				tmp_data = -32768;
			}
#endif
			*((short *)out + i + j) = (short)tmp_data;
		}
		i += mChannels;
	}
    return 0;
}

int linear_amplification4(void *data, void *out, int bytes)
{
    int i = 0, j = 0;
	int mChannels = AUDIO_CHANNAL_CNT;
	for (i = 0; i < bytes / 2; ) {
		for (j = 0; j < mChannels; j++) {
			short raw = (*((short *)data + i + j));
			int tmp_data = (int)raw;
#if 1
			tmp_data = tmp_data << 4;
			if(tmp_data > 32767) {
				tmp_data = 32767;
			} else if(tmp_data < -32768) {
				tmp_data = -32768;
			}
#endif
			*((short *)out + i + j) = (short)tmp_data;
		}
		i += mChannels;
	}
    return 0;
}


int linear_amplification5(void *data, void *out, int bytes)
{
    int i = 0, j = 0;
	int mChannels = AUDIO_CHANNAL_CNT;
	for (i = 0; i < bytes / 2; ) {
		for (j = 0; j < mChannels; j++) {
			short raw = (*((short *)data + i + j));
			int tmp_data = (int)raw;
#if 1
			tmp_data = tmp_data << 6;
			if(tmp_data > 32767) {
				tmp_data = 32767;
			} else if(tmp_data < -32768) {
				tmp_data = -32768;
			}
#endif
			*((short *)out + i + j) = (short)tmp_data;
		}
		i += mChannels;
	}
    return 0;
}


int preProcessBuffer(void *data, void *out, int bytes)
{
	int i = 0, j = 0;

	for (i = 0; i < bytes / 2 ; ) {
		for (j = 0; j < AUDIO_CHANNAL_CNT; j++) {
			int tmp = 0;
			short tmp_data = (*((short *)data + i + j));
			tmp = ((tmp_data) << 16 | ((j+1) << 8)) & 0xffffff00;
			*((int *)out + i + j) = tmp;
		}
		i += AUDIO_CHANNAL_CNT;
	}

	return 0;
}

static int add_channel(char *src, char **dst, int len)
{
	int fd = 0;
	int dst_len = 0;
	int ret = 0;

	dst_len = len * 2;
	*dst = (char *)malloc(dst_len);
	if (!*dst)
		return -ENOMEM;

	preProcessBuffer(src, *dst, len);

	return dst_len;
}
/*
 * This function includes a ring mic recording test
 * and vibration test, select the test type by flag.
 *    flag = 0:ring mic record test.
 *    flag = 1:ring mic vibration test.
 */
int ringmic_test(char *result, int flag)
{
	int *record_ret = NULL;
	int fd = 0;
	int rf_len = 0; /* record file length */
	int pre_len = 0;
	char *rf_buff = NULL; /* record file buffer */
	char *pre_buff = NULL;
	char *gain_buff = NULL;
	int *buffer = NULL;
	char path[128] = {0};
	int ret = 0, i = 0;

	/*
	 * According to flag choose to perform "recording test"
	 * or "vibration test". recording at least 10 seconds.
	 */
	if (flag) {
		log_info("Start vibration test.\n");
		sprintf(path, "%s", "/tmp/ringmic_vibration.pcm");
		/* Play the specified file, and recording. */
		system("killall -9 arecord");
		system("rm /tmp/ringmic_vibration.pcm");
		system("aplay /data/vibration.wav &");
		usleep(200000);
		system("arecord -D hw:0,0 -t raw -f S16_LE -c 8 -r 16000 -d 15 /tmp/ringmic_vibration.pcm");
	} else {
		log_info("Start record test.\n");
		sprintf(path, "%s", "/tmp/ringmic_record.pcm");
		/* Play the specified file, and recording. */
		system("killall -9 arecord");
		system("rm /tmp/ringmic_record.pcm");
		system("aplay /data/rectest_400hz.wav &");
		usleep(200000);
		system("arecord -D hw:0,0 -t raw -f S16_LE -c 8 -r 16000 -d 15 /tmp/ringmic_record.pcm");
	}
	system("killall -9 arecord");
	system("killall aplay");

	log_info("Parsing audio file...\n");
	fd = open(path, O_RDONLY);
	if (fd <= 0) {
		log_err("open %s failed(%s).\n", path, strerror(errno));
		return errno;
	}
	/* get length of the record file. */
	rf_len = lseek(fd, 0, SEEK_END);
	rf_buff = (char *)malloc(rf_len);
	if (!rf_buff) {
		close(fd);
		return -ENOMEM;
	}
	memset(rf_buff, 0, rf_len);
	/* Read from the beginning of the file. */
	lseek(fd, 0, SEEK_SET);
	ret = read(fd, rf_buff, rf_len);
	if (ret != rf_len) {
		log_err("read %s failed(%s).\n", path, strerror(errno));
		close(fd);
		free(rf_buff);
		return errno;
	}
	close(fd);

	/* Increase gain */
	gain_buff = (char *)malloc(rf_len);
	if (!gain_buff) {
		close(fd);
		free(rf_buff);
		return -ENOMEM;
	}
	memset(gain_buff, 0, rf_len);

	if (flag)   //vibration
	    linear_amplification4(rf_buff, gain_buff, rf_len);
    else    //record
        linear_amplification5(rf_buff, gain_buff, rf_len);

	memcpy(rf_buff, gain_buff, rf_len);
	free(gain_buff);
	FILE *fp;
	if (flag)
		fp = fopen("/tmp/gain-vib.pcm", "wb");
	else
		fp = fopen("/tmp/gain-rec.pcm", "wb");
	if (fp) {
		fwrite(rf_buff, 1, rf_len, fp);
		fclose(fp);
	}

	/* Add channel numbers to the original recording file */
	pre_len = add_channel(rf_buff, &pre_buff, rf_len);
	if (pre_len < 0) {
		free(rf_buff);
		return pre_len;
	}
	free(rf_buff);

        if (flag)
                fp = fopen("/tmp/addchan-vib.pcm", "wb");
        else
                fp = fopen("/tmp/addchan-rec.pcm", "wb");
        if (fp) {
                fwrite(pre_buff, 1, pre_len, fp);
                fclose(fp);
        }

#if 0
	buffer = (int *)pre_buffer;
	pre_len = (pre_len / 4);
#else
	buffer = bytesToInt(pre_buff, &pre_len);
	if (!buffer) {
		log_err("bytesToInt() failed!\n");
		free(pre_buff);
		return -ENOMEM;
	}
	free(pre_buff);

	if (flag)
                fp = fopen("/tmp/bytesToInt-vib.pcm", "wb");
        else
                fp = fopen("/tmp/bytesToInt-rec.pcm", "wb");
        if (fp) {
                fwrite((int *)buffer+16000, 4, pre_len - 32000, fp);
                fclose(fp);
        }
#endif

	/* Call library interface */
	if (!flag) {
		record_ret = recordTestWr((int *)buffer + 16000, pre_len - 32000);
		printf("\n");
		for (i = 0; i < AUDIO_CHANNAL_CNT; i++) {
			if (*(record_ret + i)) {
				log_info("recordTest:#%d mic is bad!\n", i);
				*result++ = 1;
			} else {
				log_info("recordTest:#%d mic is ok!\n", i);
				*result++ = 0;
			}
		}

		system("rm -rf /tmp/ringmic_record.pcm");
	} else {
		record_ret = vibrateTestWr((int *)buffer + 16000, pre_len - 32000);
		printf("\n");
		for (i = 0; i < AUDIO_CHANNAL_CNT; i++) {
			if (*(record_ret + i)) {
				log_info("vibrationTest:#%d mic is bad!\n", i);
				*result++ = 1;
			} else {
				log_info("vibrationTest:#%d mic is ok!\n", i);
				*result++ = 0;
			}
		}

		system("rm -rf /data/ringmic_vibration.pcm");
	}

	free(buffer);
	return 0;
}

int main()
{
	char buf[COMMAND_VALUESIZE] = {0};
	char result[COMMAND_VALUESIZE] = RESULT_PASS;
	unsigned char record_ret[AUDIO_CHANNAL_CNT] = {0};
	unsigned char vibration_ret[AUDIO_CHANNAL_CNT] = {0};
	char *start = NULL;
	int ispass = 1;
	int i = 0, ret = 0;

	system("amixer set Master Playback 50%");

	start = buf;
	memcpy(start, "vibration:", strlen("vibration:"));
	start = start + strlen("vibration:");
	ret = ringmic_test(vibration_ret, 1);
	if (ret) {
		memcpy(start, "error", strlen("error"));
		start += strlen("error");
		ispass = 0;
	} else {
		for (i = 0; i < AUDIO_CHANNAL_CNT; i++) {
			if (vibration_ret[i]) {
				ispass = 0;
				*(start++) = '1' + i;
			}
		}
	}
	*start++ = ';';
	sleep(1);

	memcpy(start, "record:", strlen("record:"));
	start = start + strlen("record:");
	ret = ringmic_test(record_ret, 0);
	if (ret) {
		memcpy(start, "error", strlen("error"));
		start += strlen("error");
		strcpy(result, RESULT_FAIL);
		ispass = 0;
	} else {
		for (i = 0; i < AUDIO_CHANNAL_CNT; i++) {
			if (record_ret[i]) {
				ispass = 0;
				*(start++) = '1' + i;
                *start++ = ' ';
			}
		}
	}
	*start++ = ';';

	if (!ispass) {
		strcpy(result, RESULT_FAIL);
		ret = -RINGMIC_BAD_ERROR;
	}

	send_msg_to_server(buf, result, ret);
	return 0;
}
