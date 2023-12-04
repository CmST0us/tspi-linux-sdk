#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "math.h"

#include "vibrate_test.h"
//#define printf(format, args...) fprintf (stderr, format, args)

#define LOG_TAG "i2c_controller"

#define FRAME_LENGTH 128

#define DATA_TYPE float

/*const int g_mic_table[5] = {
7,1,8,2,9	 
};*/

//static FILE * fp_in;
//static FILE * fp_out[12];
static int test_result;

int check_vibration(int signal, int input[], int input_length)
{
	float min = 0.5;
	float max = 0.0;
	DATA_TYPE sum0l = 0.0;
	DATA_TYPE sum0 = 0.0;
	int ind = 1;
	int i = 0, k = 0, j = 0, offset = 0;
	int first_index;
	long count = 0;;

	printf("start to check vibration, signal is %d\n", signal);
	if(input == NULL || input_length < 44800)
	{
		printf("error in input pcm, content : %d , length less than one frame\n", input_length);
		return -21;
	}

	i += 20;
	// 读取文件的第一个sample，取其index最为比较的标准
	first_index = input[i];
	first_index = (first_index >> 8) & 0xf;
	//if(first_index != 0x07 && first_index != 0x01 && first_index != 0x08 && first_index != 0x02 && first_index != 0x09)
	//{
	//	printf("pass check\n");
	//	return 0;
	//}
	int filesize = (input_length - 80)/sizeof(DATA_TYPE);
	int noofframe = filesize/FRAME_LENGTH;

	// 靠近风口的声道特殊处理
	/*if(first_index == 3 | first_index == 9)
	{
		printf("this is a special channel");
		//fseek(fp_test,57600,SEEK_SET);
		i = 57600;
	}
	else*/
	{
		//printf("this is a normal channel");
		//fseek(fp_test,44800,SEEK_SET);
		//i = 44800;
		i = 44800;
	}
	int *temp_intp = (int *)malloc( FRAME_LENGTH * sizeof(int) );

	DATA_TYPE maxA = 0;
	//while(!feof(fp_test))
	while(i + FRAME_LENGTH < input_length)
	{
		count++;
		k = k + 1;
		if (k < noofframe - 10)
		{
			//fread(temp_intp,sizeof(int),FRAME_LENGTH,fp_test);
			for(j = i, offset = 0; offset < FRAME_LENGTH; j++, offset++)
		    {
			    temp_intp[offset] = input[j];
		    }
			i += FRAME_LENGTH;
			//printf("input_length : %d, i : %d", input_length, i);
			sum0l = sum0;
			sum0 = 0.0;			
			
			for (j = 0; j < FRAME_LENGTH; j++)
			{
				int tmp = temp_intp[j] >> 8;
				float temp_float;
				temp_float = (float)tmp/0x800000;

				DATA_TYPE temp = 0;
				if(temp_float>0)
				{
					temp = temp_float;
				}
				else
				{
					temp = -temp_float;
				}
				if(temp>maxA)
				{
					maxA = temp;

				}
				sum0 += (temp_float) * (temp_float);

			}

			if (k == 1)
			{
				sum0 = sum0;
			}
			else
			{
				sum0 = (1-0.99) * sum0 + 0.99 * sum0l;
			}

			if (sum0<min)
			{
				min = sum0;
			}
			if (sum0>max)
			{
				max = sum0;
			}

		}
		else
		{
			break;
		}
	}
	
	free(temp_intp);
	printf("max is %2.3f\tmin is %2.3f\trate is %2.3f\n", max, min, max/(min+0.1));

	if (max>80*(min+.1))
	{
		ind = 1;
	}
	else
	{
		ind = 0;
	}

	printf("result is %d\tmaxA is %2.3f \n",ind, maxA);
	return ind;

}

int* vibrateTestWr(int audio_data[], int audio_length)
{
	int *buf;  
	static int ccids[13];
	int signal = 0;
	int i = 0, ret = 0;
	int rlength = audio_length / 10;
	int pcmIndex[12];
	int *pcmInput[12];

        buf = audio_data;
	memset(pcmIndex, 0, sizeof(pcmIndex));
	memset(ccids, 0, sizeof(ccids));

	if(buf == NULL) {  
		return NULL;
	}
	printf("start to check audio , vibrationtest!!!\n");
	for(i = 0; i <12; i++)
	{
		pcmInput[i] = malloc(sizeof(int) * rlength);
		if(pcmInput[i] == NULL)
		{
			printf("pcmInput malloc failed \n");
			return NULL;
		}
	}
	
	printf("split audio begin\n");
	while(i < audio_length)
	{
        signal = (buf[i]>>8)&15;
		if(signal > 0 && signal <= 12)
        {
			signal--;
			if(pcmIndex[signal] < rlength)
			{	
                *(pcmInput[signal] + pcmIndex[signal]) = buf[i];
			    pcmIndex[signal] ++;
			}	
        }
		else 
		{
            printf("signal : %d , less than 1 or larger than 12 \n", signal);
		}	
		i++;
	}
	printf("split audio success\n");
	
	// 检查频谱
	for(i = 0; i < 12; i++)
	{
		ret = check_vibration(i, pcmInput[i], pcmIndex[i]);
		if(ret != 0) 
		{
			printf("check vibration fail\n");
			ccids[i] = 1;
		} else {
			printf("check vibration success\n");
		}
	}
	for(i = 0; i <12; i++)
	{
		free(pcmInput[i]);
	}
	
	return ccids;
}


