#include "audio/wave_reader.h"
#include "audio/wave_writer.h"
#include "rkaudio_preprocess.h"
#include "rkaudio_sed.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <assert.h>
#include <iostream>

#ifndef IN_SIZE
#define IN_SIZE 256
#endif

#define ENABLE_WAKEUP 0
#define ENABLE_COMMAND 0

#define Single_TEST 1
//#define Array_SET
typedef unsigned char SKV_BYTE;
#define ENABLE_DBG 0
//#define ENABLE_RX
#include <errno.h>
#include <dlfcn.h>


int test_cmd()
{
  printf("\nin cmd test\n");
	char* in_filename = (char*)"test_16k_2mic_cmd.wav";;
	char* out_filename = (char*)"test_16k_2mic_cmd_out.wav";
	// for wave reader
	wave_reader* wr;
	wave_reader_error rerror;

	// for wave writer
	wave_writer* ww;
	wave_writer_error werror;
	wave_writer_format format;

#if ENABLE_WAKEUP && ENABLE_COMMAND
	int num_wakeup = 0;
	int cmd_id = 0;
	float asr_score = 0.0f, cmd_score = 0.0f;
#endif
	int is_wakeup = 0;

	wr = wave_reader_open(in_filename, &rerror);
	if (!wr)	{
		printf("rerror=%d\n", rerror);
		return -1;
	}
	clock_t startTime, endTime;
	double Total_time = 0.0;
	double Total_sample = 0.0;
	float Spe_time = 0.0;

	printf("filename=%s format=%d num_channels=%d sample_rate=%d sample_bits=%d num_samples=%d\n",in_filename,wave_reader_get_format(wr),
		wave_reader_get_num_channels(wr),wave_reader_get_sample_rate(wr),wave_reader_get_sample_bits(wr),wave_reader_get_num_samples(wr));

	int mSampleRate = wave_reader_get_sample_rate(wr);
	int mBitPerSample = wave_reader_get_sample_bits(wr);
	int mNumChannel = wave_reader_get_num_channels(wr);
	int num_ref_channel = NUM_REF_CHANNEL;
	int num_src_channel = mNumChannel - NUM_REF_CHANNEL;
	printf("ref = %d, src=%d\n", num_ref_channel, num_src_channel);
	int read_size = IN_SIZE * mNumChannel * mBitPerSample / 8;
	SKV_BYTE* in = (SKV_BYTE*)malloc(read_size * sizeof(SKV_BYTE));
	short* out = (short*)malloc(IN_SIZE * 1 * sizeof(short));
	format.num_channels = 1;
	format.sample_rate = wave_reader_get_sample_rate(wr);
	format.sample_bits = wave_reader_get_sample_bits(wr);
	ww = wave_writer_open(out_filename, &format, &werror);
	if (!ww)
	{
		printf("werror=%d\n", werror);

		wave_reader_close(wr);
		return -1;
	}
	//rkaudio_asr_set_param(0.3, 0.72, 0.5);
    RKAUDIOParam param;
    memset(&param, 0, sizeof(RKAUDIOParam));
	param.model_en = RKAUDIO_EN_BF;
	param.aec_param = rkaudio_aec_param_init();
	param.bf_param = rkaudio_preprocess_param_init();
	param.rx_param = rkaudio_rx_param_init();
	void* st_ptr = NULL;

	st_ptr = rkaudio_preprocess_init(mSampleRate, mBitPerSample, num_src_channel, num_ref_channel, &param);

    if (st_ptr == NULL)
    {
		printf("Failed to create audio preprocess handle\n");
		return -1;
	}

	int cnt = 0, in_size, out_size, res, frame_cot = 0;
	startTime = clock();
	while (0 < (res = wave_reader_get_samples(wr, IN_SIZE, in)))
	{
		cnt++;
		in_size = res * (mBitPerSample / 16) * mNumChannel;
		out_size = rkaudio_preprocess_short(st_ptr, (short*)in, out, in_size, &is_wakeup);
		int targ_Doa = rkaudio_Doa_invoke(st_ptr);
		int targ_ang , targ_pth;
		//rkaudio_Cir_Doa_invoke(st_ptr, &targ_ang, &targ_pth);
		wave_writer_put_samples(ww, IN_SIZE, out);
		Total_sample = Total_sample + out_size;
#if ENABLE_WAKEUP && ENABLE_COMMAND

		if (frame_cot == 0 && is_wakeup == 1)
		{
			frame_cot = 250;/*设置10s的命令词等待时间*/
			num_wakeup = num_wakeup + 1;
			printf("%f, [%d], Hi, I am XiaoRui\n",  cnt*0.016, num_wakeup);
		}
		if (frame_cot > 0) {
			frame_cot--;
			rkaudio_preprocess_get_cmd_id(st_ptr, &cmd_score, &cmd_id);
			if (cmd_id != 0) {
			    printf("%f,, cmd=%d\n\n",  cnt*0.016, cmd_id);
				frame_cot = 0;
				is_wakeup = 0;
			} else if (frame_cot == 0) {
                is_wakeup = 0;
			}
		}
		if (cnt == 3750) {
			printf("stop\n");
		}
#elif ENABLE_WAKEUP
		if (is_wakeup)
		{
			num_wakeup = num_wakeup + 1;
			printf("[%d], Hi, I am Xiaorui, is_wakeup = %d\n", num_wakeup, is_wakeup);
			is_wakeup = 0;
			//printf("targ_ang is %d,targ_pth is %d\n", targ_ang, targ_pth);
		}
#endif
	}
	endTime = clock();
	Total_time = endTime - startTime;
	Spe_time = Total_sample / mSampleRate;
	printf("Finished, speech_time = %f, cost_time = %f\n", Spe_time, Total_time  / CLOCKS_PER_SEC);

	wave_writer_close(ww, &werror);
	wave_reader_close(wr);
	free(in);
	free(out);

	if (st_ptr)
		rkaudio_preprocess_destory(st_ptr);
	rkaudio_param_deinit(&param);

	return 0;
}

int test_aec()
{
#undef IN_SIZE
#define IN_SIZE 256

  printf("\nin aec test\n");
	//char* in_filename = (char*)"human.wav";;
	//char* out_filename = (char*)"human_out.wav";
 	char* in_filename = (char*)"rkaudio_test_2a1.wav";;
	char* out_filename = (char*)"rkaudio_test_2a1_out.wav";
	// for wave reader
	wave_reader* wr;
	wave_reader_error rerror;

	// for wave writer
	wave_writer* ww;
	wave_writer_error werror;
	wave_writer_format format;

	int is_wakeup = 0;
	wr = wave_reader_open(in_filename, &rerror);
	if (!wr)	{
		printf("rerror=%d\n", rerror);
		return -1;
	}
	clock_t startTime, endTime;
	double Total_time = 0.0;
	double Total_sample = 0.0;
	float Spe_time = 0.0;

	printf("filename=%s format=%d num_channels=%d sample_rate=%d sample_bits=%d num_samples=%d\n",in_filename,wave_reader_get_format(wr),
		wave_reader_get_num_channels(wr),wave_reader_get_sample_rate(wr),wave_reader_get_sample_bits(wr),wave_reader_get_num_samples(wr));

	int mSampleRate = wave_reader_get_sample_rate(wr);
	int mBitPerSample = wave_reader_get_sample_bits(wr);
	int mNumChannel = wave_reader_get_num_channels(wr);
	int num_ref_channel = NUM_REF_CHANNEL;
	int num_src_channel = mNumChannel - NUM_REF_CHANNEL;
	
	int read_size = IN_SIZE * mNumChannel * mBitPerSample / 8;
	SKV_BYTE* in = (SKV_BYTE*)malloc(read_size * sizeof(SKV_BYTE));
	short* out = (short*)malloc(IN_SIZE * 1 * sizeof(short));

	format.num_channels = 1;
	format.sample_rate = wave_reader_get_sample_rate(wr);
	format.sample_bits = wave_reader_get_sample_bits(wr);
	ww = wave_writer_open(out_filename, &format, &werror);
	if (!ww)
	{
		printf("werror=%d\n", werror);

		wave_reader_close(wr);
		return -1;
	}
	//rkaudio_asr_set_param(0.3, 0.72, 0.5);
    RKAUDIOParam param;
    memset(&param, 0, sizeof(RKAUDIOParam));
	param.model_en = RKAUDIO_EN_AEC | RKAUDIO_EN_BF;
	param.aec_param = rkaudio_aec_param_init();
	param.bf_param = rkaudio_preprocess_param_init();
	param.rx_param = rkaudio_rx_param_init();
	void* st_ptr = NULL;

	st_ptr = rkaudio_preprocess_init(mSampleRate, mBitPerSample, num_src_channel, num_ref_channel, &param);
    if (st_ptr == NULL)
    {
		printf("Failed to create audio preprocess handle\n");
		return -1;
	}

	int cnt = 0, in_size, out_size, res, frame_cot = 0;
	startTime = clock();
	while (0 < (res = wave_reader_get_samples(wr, IN_SIZE, in)))
	{
		cnt++;
		in_size = res * (mBitPerSample / 16) * mNumChannel;
		out_size = rkaudio_preprocess_short(st_ptr, (short*)in, out, in_size, &is_wakeup);
		int targ_Doa = rkaudio_Doa_invoke(st_ptr);
		int targ_ang , targ_pth;
		//rkaudio_Cir_Doa_invoke(st_ptr, &targ_ang, &targ_pth);
		wave_writer_put_samples(ww, IN_SIZE, out);
		Total_sample = Total_sample + out_size;
	}
	endTime = clock();
	Total_time = endTime - startTime;
	Spe_time = Total_sample / mSampleRate;
	printf("Finished, speech_time = %f, cost_time = %f\n", Spe_time, Total_time  / CLOCKS_PER_SEC);

	wave_writer_close(ww, &werror);
	wave_reader_close(wr);
	free(in);
	free(out);

	if (st_ptr)
		rkaudio_preprocess_destory(st_ptr);
	rkaudio_param_deinit(&param);

	return 0;
}

extern "C" int rkaudio_agc_short_process(void *st_ptr, short *in, short *out);
extern "C" void rkaudio_agc_short_destroy(void *st_ptr);
extern "C" void *rkaudio_agc_short_init(int frame_len, int sampling_rate, int chan, void* param_);

int test_agc()
{
#undef IN_SIZE
#define IN_SIZE 256
  
	printf("\nin aec test\n");
	  char* in_filename = (char*)"test_16k_1a1.wav";;
	  char* out_filename = (char*)"test_16k_1a1_out.wav";
	  // for wave reader
	  wave_reader* wr;
	  wave_reader_error rerror;
  
	  // for wave writer
	  wave_writer* ww;
	  wave_writer_error werror;
	  wave_writer_format format;
  
	  int is_wakeup = 0;
	  wr = wave_reader_open(in_filename, &rerror);
	  if (!wr)	  {
		  printf("rerror=%d\n", rerror);
		  return -1;
	  }
	  clock_t startTime, endTime;
	  double Total_time = 0.0;
	  double Total_sample = 0.0;
	  float Spe_time = 0.0;
  
	  printf("filename=%s format=%d num_channels=%d sample_rate=%d sample_bits=%d num_samples=%d\n",in_filename,wave_reader_get_format(wr),
		  wave_reader_get_num_channels(wr),wave_reader_get_sample_rate(wr),wave_reader_get_sample_bits(wr),wave_reader_get_num_samples(wr));
  
	  int mSampleRate = wave_reader_get_sample_rate(wr);
	  int mBitPerSample = wave_reader_get_sample_bits(wr);
	  int mNumChannel = wave_reader_get_num_channels(wr);
	  
	  int read_size = IN_SIZE * mNumChannel * mBitPerSample / 8;
	  SKV_BYTE* in = (SKV_BYTE*)malloc(read_size * sizeof(SKV_BYTE));
	  short* out = (short*)malloc(read_size * sizeof(short));
  
	  format.num_channels = mNumChannel;
	  format.sample_rate = wave_reader_get_sample_rate(wr);
	  format.sample_bits = wave_reader_get_sample_bits(wr);
	  ww = wave_writer_open(out_filename, &format, &werror);
	  if (!ww)
	  {
		  printf("werror=%d\n", werror);
  
		  wave_reader_close(wr);
		  return -1;
	  }
	  //rkaudio_asr_set_param(0.3, 0.72, 0.5);
	  void *param = rkaudio_agc_param_init();
	  void *st_ptr = rkaudio_agc_short_init(IN_SIZE, 44100, mNumChannel, param);
	  if (st_ptr == NULL)
	  {
		  printf("Failed to create audio preprocess handle\n");
		  return -1;
	  }
  	  free(param);
  	  
	  int cnt = 0, in_size, out_size, res, frame_cot = 0;
	  startTime = clock();
	  while (0 < (res = wave_reader_get_samples(wr, IN_SIZE, in)))
	  {
		  cnt++;
		  in_size = res * (mBitPerSample / 16) * mNumChannel;
		  out_size = rkaudio_agc_short_process(st_ptr, (short*)in, out);
		  wave_writer_put_samples(ww, IN_SIZE, out);
		  Total_sample = Total_sample + out_size;
	  }
	  endTime = clock();
	  Total_time = endTime - startTime;
	  Spe_time = Total_sample / mSampleRate;
	  printf("Finished, speech_time = %f, cost_time = %f\n", Spe_time, Total_time  / CLOCKS_PER_SEC);
  
	  wave_writer_close(ww, &werror);
	  wave_reader_close(wr);
	  free(in);
	  free(out);
  
	  if (st_ptr)
		  rkaudio_agc_short_destroy(st_ptr);
	  return 0;
}

int test_sed()
{
#undef IN_SIZE
#define IN_SIZE 256
	printf("\nin sed test\n");

	double Total_sample = 0.0;
	// 读取数据并处理
	clock_t startTime, endTime;
	/* For Debug  */
	int out_size = 0, in_size = 0, res = 0;
#if ENABLE_8k
	char* in_filename = (char*)"babycry_8k.wav";
	char* out_filename = (char*)"babycry_8k_out.wav";
#else
	char* in_filename = (char*)"test_16K_sed.wav";
	char* out_filename = (char*)"test_16K_sed_out.wav";
#endif

	// for wave reader
	wave_reader* wr;
	wave_reader_error rerror;

	// for wave writer
	wave_writer* ww;
	wave_writer_error werror;
	wave_writer_format format;

	// 读取输入音频
	wr = wave_reader_open(in_filename, &rerror);
	if (!wr) {
		printf("rerror=%d\n", rerror);
		return -1;
	}

	int mSampleRate = wave_reader_get_sample_rate(wr);
	int mBitPerSample = wave_reader_get_sample_bits(wr);
	int mNumChannel = wave_reader_get_num_channels(wr);

	// 输入检查
	if (mNumChannel > 1) {
		printf("This algorithm is a single channel algorithm and will run on the first channel of data\n");
	}

	// 每次读取数据大小    
	int read_size = IN_SIZE * mNumChannel * mBitPerSample / 8;
	SKV_BYTE* in = (SKV_BYTE*)malloc(read_size * sizeof(SKV_BYTE));
	int out_res_num = 5;
	short* out = (short*)malloc(IN_SIZE * out_res_num * sizeof(short));
	// 输出音频格式设置
	format.num_channels =5;
	format.sample_rate = mSampleRate;
	format.sample_bits = mBitPerSample;
	ww = wave_writer_open(out_filename, &format, &werror);
	// 输出音频建立失败
	if (!ww)
	{
		printf("werror=%d\n", werror);
		wave_reader_close(wr);
		return -1;
	}

	// 声音事件检测初始化
	RKAudioSedRes sed_res;
	RKAudioSedParam* sed_param = rkaudio_sed_param_init();
	void* st_sed = rkaudio_sed_init(mSampleRate, mBitPerSample, mNumChannel, sed_param);
	//char initres = rkaudio_sed_init_res(st_sed);
	rkaudio_sed_param_destroy(sed_param);
	if (st_sed == NULL) {
		printf("Failed to create baby cry handle\n");
		return -1;
	}

	startTime = clock();
	int cnt = 0;
	while (0 < (res = wave_reader_get_samples(wr, IN_SIZE, in)))
	{
		in_size = res * (mBitPerSample / 8) * mNumChannel;
		cnt++;
		out_size = rkaudio_sed_process(st_sed, (short*)in, in_size / 2, &sed_res);
		//float lsd_res =  rkaudio_sed_lsd_db(st_sed);
        if (out_size < 0)
			fprintf(stderr, "bcd process return error=%d\n", out_size);

		//printf("lsd=%d,snr=%d,bcd=%d,buz_res=%d,gbs_res=%d\n", sed_res.lsd_res, sed_res.snr_res, sed_res.bcd_res, sed_res.buz_res, sed_res.gbs_res);

		// 输出，测试用
		for (int i = 0; i < IN_SIZE; i++) {
			*(out + out_res_num * i)     = 10000 * sed_res.snr_res;
			*(out + out_res_num * i + 1) = 10000 * sed_res.lsd_res;
			*(out + out_res_num * i + 2) = 10000 * sed_res.bcd_res;
			*(out + out_res_num * i + 3) = 10000 * sed_res.buz_res;
			*(out + out_res_num * i + 4) = 10000 * sed_res.gbs_res;
		}
		wave_writer_put_samples(ww, IN_SIZE, out);
		Total_sample += in_size / 2 / mNumChannel;
		//if (cnt % 63 == 0)
		//	printf("cnt = %d\n", cnt);
	}
	endTime = clock();

	printf("Finished, speech_time = %f, cost_time = %f\n", \
		Total_sample / mSampleRate, (double)(endTime - startTime) / CLOCKS_PER_SEC);

	wave_writer_close(ww, &werror);
	wave_reader_close(wr);

	free(in);
	free(out);

	// 释放
	if (st_sed)
		rkaudio_sed_destroy(st_sed);

	return 0;
}


int main()
{
	test_aec();
	//test_agc();
	test_sed();
}
