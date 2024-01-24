/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/poll.h>
#include "rk_defines.h"
#include "rk_debug.h"
#include "rk_mpi_ai.h"
#include "rk_mpi_sys.h"
#include "rk_mpi_mb.h"

#include "test_comm_argparse.h"

static RK_BOOL gAiExit = RK_FALSE;
#define TEST_AI_WITH_FD 0

typedef struct _rkMpiAICtx {
    const char *srcFilePath;
    const char *dstFilePath;
    RK_S32      s32LoopCount;
    RK_S32      s32ChnNum;
    RK_S32      s32DeviceSampleRate;
    RK_S32      s32SampleRate;
    RK_S32      s32DeviceChannel;
    RK_S32      s32Channel;
    RK_S32      s32BitWidth;
    RK_S32      s32DevId;
    RK_S32      s32PeriodCount;
    RK_S32      s32PeriodSize;
    char       *chCardName;
    RK_S32      s32ChnIndex;
    RK_S32      s32DevFd;
} TEST_AI_CTX_S;

static AUDIO_SOUND_MODE_E ai_find_sound_mode(RK_S32 ch) {
    AUDIO_SOUND_MODE_E channel = AUDIO_SOUND_MODE_BUTT;
    switch (ch) {
      case 1:
        channel = AUDIO_SOUND_MODE_MONO;
        break;
      case 2:
        channel = AUDIO_SOUND_MODE_STEREO;
        break;
      default:
        RK_LOGE("channel = %d not support", ch);
        return AUDIO_SOUND_MODE_BUTT;
    }

    return channel;
}

static AUDIO_BIT_WIDTH_E ai_find_bit_width(RK_S32 bit) {
    AUDIO_BIT_WIDTH_E bitWidth = AUDIO_BIT_WIDTH_BUTT;
    switch (bit) {
      case 8:
        bitWidth = AUDIO_BIT_WIDTH_8;
        break;
      case 16:
        bitWidth = AUDIO_BIT_WIDTH_16;
        break;
      case 24:
        bitWidth = AUDIO_BIT_WIDTH_24;
        break;
      default:
        RK_LOGE("bitwidth(%d) not support", bit);
        return AUDIO_BIT_WIDTH_BUTT;
    }

    return bitWidth;
}

RK_S32 test_ai_poll_event(RK_S32 timeoutMsec, RK_S32 fd) {
    RK_S32 num_fds = 1;
    struct pollfd pollFds[num_fds];
    RK_S32 ret = 0;

    RK_ASSERT(fd > 0);
    memset(pollFds, 0, sizeof(pollFds));
    pollFds[0].fd = fd;
    pollFds[0].events = (POLLPRI | POLLIN | POLLERR | POLLNVAL | POLLHUP);

    ret = poll(pollFds, num_fds, timeoutMsec);

    if (ret > 0 && (pollFds[0].revents & (POLLERR | POLLNVAL | POLLHUP))) {
        RK_LOGE("fd:%d polled error", fd);
        return -1;
    }

    return ret;
}

RK_S32 test_open_device_ai(TEST_AI_CTX_S *ctx) {
    AUDIO_DEV aiDevId = ctx->s32DevId;
    AUDIO_SOUND_MODE_E soundMode;

    AIO_ATTR_S aiAttr;
    RK_S32 result;
    memset(&aiAttr, 0, sizeof(AIO_ATTR_S));

    if (ctx->chCardName) {
        snprintf(reinterpret_cast<char *>(aiAttr.u8CardName),
                 sizeof(aiAttr.u8CardName), "%s", ctx->chCardName);
    }

    aiAttr.soundCard.channels = ctx->s32DeviceChannel;
    aiAttr.soundCard.sampleRate = ctx->s32DeviceSampleRate;
    aiAttr.soundCard.bitWidth = AUDIO_BIT_WIDTH_16;

    AUDIO_BIT_WIDTH_E bitWidth = ai_find_bit_width(ctx->s32BitWidth);
    if (bitWidth == AUDIO_BIT_WIDTH_BUTT) {
        goto __FAILED;
    }
    aiAttr.enBitwidth = bitWidth;
    aiAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)ctx->s32SampleRate;
    soundMode = ai_find_sound_mode(ctx->s32Channel);
    if (soundMode == AUDIO_SOUND_MODE_BUTT) {
        goto __FAILED;
    }
    aiAttr.enSoundmode = soundMode;
    aiAttr.u32FrmNum = ctx->s32PeriodCount;
    aiAttr.u32PtNumPerFrm = ctx->s32PeriodSize;

    aiAttr.u32EXFlag = 0;
    aiAttr.u32ChnCnt = 2;

    result = RK_MPI_AI_SetPubAttr(aiDevId, &aiAttr);
    if (result != 0) {
        RK_LOGE("ai set attr fail, reason = %d", result);
        goto __FAILED;
    }

    result = RK_MPI_AI_Enable(aiDevId);
    if (result != 0) {
        RK_LOGE("ai enable fail, reason = %d", result);
        goto __FAILED;
    }

    return RK_SUCCESS;
__FAILED:
    return RK_FAILURE;
}

RK_S32 test_init_mpi_ai(TEST_AI_CTX_S *params) {
    RK_S32 result;

    result =  RK_MPI_AI_EnableChn(params->s32DevId, params->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ai enable channel fail, aoChn = %d, reason = %x", params->s32ChnIndex, result);
        return RK_FAILURE;
    }

#if TEST_AI_WITH_FD
    // open fd immediate after enable chn will be better.
    params->s32DevFd = RK_MPI_AI_GetFd(params->s32DevId, params->s32ChnIndex);
    RK_LOGI("ai (devId: %d, chnId: %d), selectFd:%d", params->s32DevId, params->s32ChnIndex, params->s32DevFd);
#endif

    result = RK_MPI_AI_EnableReSmp(params->s32DevId, params->s32ChnIndex,
                                  (AUDIO_SAMPLE_RATE_E)params->s32SampleRate);
    if (result != 0) {
        RK_LOGE("ai enable channel fail, reason = %x, aoChn = %d", result, params->s32ChnIndex);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

RK_S32 test_deinit_mpi_ai(TEST_AI_CTX_S *params) {
    RK_MPI_AI_DisableReSmp(params->s32DevId, params->s32ChnIndex);
    RK_S32 result = RK_MPI_AI_DisableChn(params->s32DevId, params->s32ChnIndex);
    if (result != 0) {
        RK_LOGE("ai disable channel fail, reason = %d", result);
        return RK_FAILURE;
    }

    result =  RK_MPI_AI_Disable(params->s32DevId);
    if (result != 0) {
        RK_LOGE("ai disable fail, reason = %d", result);
        return RK_FAILURE;
    }

    return RK_SUCCESS;
}

void* sendDataThread(void * ptr) {
    TEST_AI_CTX_S *params = reinterpret_cast<TEST_AI_CTX_S *>(ptr);

    RK_S32 result = 0;
    RK_S32 s32MilliSec = -1;
    AUDIO_FRAME_S frame;

    if (params->dstFilePath) {
        AUDIO_SAVE_FILE_INFO_S save;
        save.bCfg = RK_TRUE;
        save.u32FileSize = 1024;
        snprintf(save.aFilePath, sizeof(save.aFilePath), "%s", params->dstFilePath);
        snprintf(save.aFileName, sizeof(save.aFileName), "%s", "cap_out.pcm");
        RK_MPI_AI_SaveFile(params->s32DevId, params->s32ChnIndex, &save);
    }

    while (!gAiExit) {
#if TEST_AI_WITH_FD
        test_ai_poll_event(-1, params->s32DevFd);
#endif
        result = RK_MPI_AI_GetFrame(params->s32DevId, params->s32ChnIndex, &frame, RK_NULL, s32MilliSec);
        if (result == 0) {
            void* data = RK_MPI_MB_Handle2VirAddr(frame.pMbBlk);
            // get the length of valid data in this frame
            RK_U64 len = RK_MPI_MB_GetLength(frame.pMbBlk);
            RK_LOGV("data = %p, len = %lld", data, len);
            RK_MPI_AI_ReleaseFrame(params->s32DevId, params->s32ChnIndex, &frame, RK_NULL);
        }
    }

    return RK_NULL;
}

RK_S32 unit_test_mpi_ai(TEST_AI_CTX_S *ctx) {
    RK_S32 i = 0;
    TEST_AI_CTX_S params[AI_MAX_CHN_NUM];
    pthread_t tidSend[AI_MAX_CHN_NUM];
    pthread_t tidComand[AI_MAX_CHN_NUM];

    if (test_open_device_ai(ctx) != RK_SUCCESS) {
        goto __FAILED;
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        memcpy(&(params[i]), ctx, sizeof(TEST_AI_CTX_S));
        params[i].s32ChnIndex = i;
        params[i].s32DevFd = -1;

        test_init_mpi_ai(&params[i]);
        pthread_create(&tidSend[i], RK_NULL, sendDataThread, reinterpret_cast<void *>(&params[i]));
    }

    for (i = 0; i < ctx->s32ChnNum; i++) {
        pthread_join(tidSend[i], RK_NULL);
        pthread_join(tidComand[i], RK_NULL);
        test_deinit_mpi_ai(&params[i]);
    }

    return RK_SUCCESS;
__FAILED:

    return RK_FAILURE;
}

static void mpi_ai_test_show_options(const TEST_AI_CTX_S *ctx) {
    RK_PRINT("cmd parse result:\n");
    RK_PRINT("input  file name      : %s\n", ctx->srcFilePath);
    RK_PRINT("output file name      : %s\n", ctx->dstFilePath);
    RK_PRINT("loop count            : %d\n", ctx->s32LoopCount);
    RK_PRINT("channel number        : %d\n", ctx->s32ChnNum);
    RK_PRINT("open sound rate       : %d\n", ctx->s32DeviceSampleRate);
    RK_PRINT("record data rate      : %d\n", ctx->s32SampleRate);
    RK_PRINT("sound card channel    : %d\n", ctx->s32DeviceChannel);
    RK_PRINT("output channel        : %d\n", ctx->s32Channel);
    RK_PRINT("bit_width             : %d\n", ctx->s32BitWidth);
    RK_PRINT("period_count          : %d\n", ctx->s32PeriodCount);
    RK_PRINT("period_size           : %d\n", ctx->s32PeriodSize);
    RK_PRINT("sound card name       : %s\n", ctx->chCardName);
    RK_PRINT("device id             : %d\n", ctx->s32DevId);
}

static const char *const usages[] = {
    "./rk_mpi_ai_test [--device_rate rate] [--device_ch ch] [--out_rate rate] [--out_ch ch]...",
    NULL,
};

int main(int argc, const char **argv) {
    RK_S32          i;
    RK_S32          s32Ret;
    TEST_AI_CTX_S  *ctx;

    ctx = reinterpret_cast<TEST_AI_CTX_S *>(malloc(sizeof(TEST_AI_CTX_S)));
    memset(ctx, 0, sizeof(TEST_AI_CTX_S));

    ctx->srcFilePath        = RK_NULL;
    ctx->dstFilePath        = RK_NULL;
    ctx->s32LoopCount       = 1;
    ctx->s32ChnNum          = 1;
    ctx->s32BitWidth        = 16;
    ctx->s32PeriodCount     = 4;
    ctx->s32PeriodSize      = 1024;
    ctx->chCardName         = RK_NULL;
    ctx->s32DevId           = 0;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("basic options:"),

        OPT_INTEGER('\0', "device_rate", &(ctx->s32DeviceSampleRate),
                    "the sample rate of open sound card.  <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "device_ch", &(ctx->s32DeviceChannel),
                    "the number of sound card channels. <required>.", NULL, 0, 0),
        OPT_INTEGER('\0', "out_ch", &(ctx->s32Channel),
                    "the channels of out data. <required>", NULL, 0, 0),
        OPT_INTEGER('\0', "out_rate", &(ctx->s32SampleRate),
                    "the sample rate of out data. <required>", NULL, 0, 0),
        OPT_STRING('o', "output", &(ctx->dstFilePath),
                    "output file name, e.g.(./ai). default(NULL).", NULL, 0, 0),
        OPT_INTEGER('n', "loop_count", &(ctx->s32LoopCount),
                    "loop running count. can be any count. default(1)", NULL, 0, 0),
        OPT_INTEGER('c', "channel_count", &(ctx->s32ChnNum),
                    "the count of adec channel. default(1).", NULL, 0, 0),
        OPT_INTEGER('\0', "bit", &(ctx->s32BitWidth),
                    "the bit width of open sound card, range(8, 16, 24), default(16)", NULL, 0, 0),
        OPT_INTEGER('\0', "period_size", &(ctx->s32PeriodSize),
                    "the period size for open sound card, default(1024)", NULL, 0, 0),
        OPT_INTEGER('\0', "period_count", &(ctx->s32PeriodCount),
                    "the period count for open sound card, default(4)", NULL, 0, 0),
        OPT_STRING('\0', "sound_card_name", &(ctx->chCardName),
                    "the sound name for open sound card, default(NULL)", NULL, 0, 0),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "\nselect a test case to run.",
                                 "\nuse --help for details.");

    argc = argparse_parse(&argparse, argc, argv);
    mpi_ai_test_show_options(ctx);

    if (ctx->s32Channel <= 0
        || ctx->s32SampleRate <= 0
        || ctx->s32DeviceSampleRate <= 0
        || ctx->s32DeviceChannel <= 0) {
        argparse_usage(&argparse);
        goto __FAILED;
    }

    RK_MPI_SYS_Init();

    for (i = 0; i < ctx->s32LoopCount; i++) {
        RK_LOGI("start running loop count  = %d", i);
        s32Ret = unit_test_mpi_ai(ctx);
        if (s32Ret != RK_SUCCESS) {
            goto __FAILED;
        }
        RK_LOGI("end running loop count  = %d", i);
    }

__FAILED:
    if (ctx) {
        free(ctx);
        ctx = RK_NULL;
    }

    RK_MPI_SYS_Exit();
    return 0;
}
