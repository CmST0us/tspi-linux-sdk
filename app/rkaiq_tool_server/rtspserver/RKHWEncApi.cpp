//#define LOG_NDEBUG 0
#define LOG_TAG "RKHWEncApi"
#include <utils/Log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "RKHWEncApi.h"

typedef enum
{
    UNSUPPORT_PROFILE = -1,
    BASELINE_PROFILE = 66,
    MAIN_PROFILE = 77,
    HIGHT_PROFILE = 100,
} EncProfile;

/*
 * This enumeration is for levels. The value follows the level_idc in sequence
 * parameter set rbsp. See Annex A.
 * @published All
 */
typedef enum AVCLevel
{
    AVC_LEVEL_AUTO = 0,
    AVC_LEVEL1_B = 9,
    AVC_LEVEL1 = 10,
    AVC_LEVEL1_1 = 11,
    AVC_LEVEL1_2 = 12,
    AVC_LEVEL1_3 = 13,
    AVC_LEVEL2 = 20,
    AVC_LEVEL2_1 = 21,
    AVC_LEVEL2_2 = 22,
    AVC_LEVEL3 = 30,
    AVC_LEVEL3_1 = 31,
    AVC_LEVEL3_2 = 32,
    AVC_LEVEL4 = 40,
    AVC_LEVEL4_1 = 41,
    AVC_LEVEL4_2 = 42,
    AVC_LEVEL5 = 50,
    AVC_LEVEL5_1 = 51
} AVCLevel;

RKHWEncApi::RKHWEncApi() : mVpuCtx(NULL), mSpsPpsBuf(NULL), mSpsPpsLen(0), mFrameCount(0)
{
    ALOGD("RKHWEncApi enter");
}

RKHWEncApi::~RKHWEncApi()
{
    ALOGD("~RKHWEncApi enter");

    mVpuCtx->flush(mVpuCtx);
    if (mVpuCtx != NULL) {
        vpu_close_context(&mVpuCtx);
        free(mVpuCtx);
        mVpuCtx = NULL;
    }

    if (mOutputBuf != NULL) {
        free(mOutputBuf);
        mOutputBuf = NULL;
    }
    if (mSpsPpsBuf != NULL) {
        free(mSpsPpsBuf);
        mSpsPpsBuf = NULL;
    }
}

bool RKHWEncApi::init(EncCfgInfo* cfg)
{
    int32_t ret;
    EncParameter_t* params = NULL;

    mVpuCtx = (VpuCodecContext_t*)malloc(sizeof(VpuCodecContext_t));
    ret = vpu_open_context(&mVpuCtx);
    if (ret) {
        ALOGE("ERROR: Failed to open ctx, ErrCode %d", ret);
        return false;
    }

    mOutputBuf = (unsigned char*)malloc(cfg->width * cfg->height * 2);

    mVpuCtx->codecType = CODEC_ENCODER;
    mVpuCtx->videoCoding = OMX_RK_VIDEO_CodingAVC; // h264 default
    mVpuCtx->width = cfg->width;
    mVpuCtx->height = cfg->height;

    // set encoding parameters
    params = (EncParameter_t*)malloc(sizeof(EncParameter_t));
    memset(params, 0, sizeof(EncParameter_t));
    mVpuCtx->private_data = params; // private_data free at #vpu_close_context

    params->width = cfg->width;
    params->height = cfg->height;
    params->format = cfg->format;
    params->bitRate = cfg->bitRate;
    params->framerate = cfg->framerate;
    /* gop length */
    params->intraPicRate = cfg->IDRInterval * cfg->framerate;
    /* cabac mode */
    params->enableCabac = 0;
    params->cabacInitIdc = 0;
    /* profile level */
    params->profileIdc = BASELINE_PROFILE;
    params->levelIdc = AVC_LEVEL4_1;

    /* encoding quality: rc_mode & qp */
    params->rc_mode = cfg->rc_mode;
    params->qp = cfg->qp;

    ALOGD("encode params init settings:\n"
          "width = %d\n"
          "height = %d\n"
          "bitRate = %d\n"
          "framerate = %d\n"
          "format = %d\n"
          "enableCabac = %d,\n"
          "cabacInitIdc = %d,\n"
          "intraPicRate = %d,\n"
          "profileIdc = %d,\n"
          "levelIdc = %d,\n"
          "rc_mode = %d,\n",
          params->width, params->height, params->bitRate, params->framerate, params->format, params->enableCabac,
          params->cabacInitIdc, params->intraPicRate, params->profileIdc, params->levelIdc, params->rc_mode);

    ret = mVpuCtx->init(mVpuCtx, NULL, 0);
    if (ret) {
        ALOGE("ERROR: Failed to init ctx, ErrCode %d", ret);
        return false;
    }

    mVpuCtx->control(mVpuCtx, VPU_API_ENC_GETCFG, (void*)params);
    if (mVpuCtx->extradata != NULL && mVpuCtx->extradata_size < 2048) {
        mSpsPpsBuf = (unsigned char*)malloc(2048);
        memcpy(mSpsPpsBuf, mVpuCtx->extradata, mVpuCtx->extradata_size);
        mSpsPpsLen = mVpuCtx->extradata_size;
        ALOGD("get h264 sps_pps len %d", mSpsPpsLen);
    }

    return true;
}

bool RKHWEncApi::sendFrame(char* data, int32_t size, int64_t pts, int32_t flag)
{
    int32_t ret;
    EncInputStream_t aInput;

    if (!mVpuCtx) {
        ALOGE("Init first");
        return false;
    }

    memset(&aInput, 0, sizeof(EncInputStream_t));
    aInput.buf = (unsigned char*)data;
    aInput.bufPhyAddr = -1;
    aInput.size = size;
    aInput.timeUs = pts > 0 ? pts : VPU_API_NOPTS_VALUE;
    aInput.nFlags = flag;

    // TODO
    if (aInput.nFlags != 0 && aInput.size == 0) {
        aInput.size = 1;
    }

    ret = mVpuCtx->encoder_sendframe(mVpuCtx, &aInput);
    if (ret < 0) {
        ALOGE("Failed to send pkt, ErrCodec %d", ret);
        return false;
    } else if (aInput.size != 0) {
        return false; // retry again
    }

    // ALOGI("Send pkt size %d pts %lld flag %d", size, pts, flag);

    return true;
}

bool RKHWEncApi::sendFrame(int32_t fd, int32_t size, int64_t pts, int32_t flag)
{
    int32_t ret;
    EncInputStream_t aInput;

    if (!mVpuCtx) {
        ALOGE("Init first");
        return false;
    }

    memset(&aInput, 0, sizeof(EncInputStream_t));
    aInput.buf = NULL;
    aInput.bufPhyAddr = fd;
    aInput.size = size;
    aInput.timeUs = pts > 0 ? pts : VPU_API_NOPTS_VALUE;
    aInput.nFlags = flag;

    // TODO
    if (aInput.nFlags != 0 && aInput.size == 0) {
        aInput.size = 1;
    }

    ret = mVpuCtx->encoder_sendframe(mVpuCtx, &aInput);
    if (ret < 0) {
        ALOGE("Failed to send pkt, ErrCodec %d", ret);
        return false;
    } else if (aInput.size != 0) {
        return false; // retry again
    }

    // ALOGI("send pkt size %d pts %lld flag %d", size, pts, flag);

    return true;
}

bool RKHWEncApi::getOutStream(EncoderOut_t* encOut)
{
    int32_t ret;

    if (!mVpuCtx) {
        ALOGE("Init first");
        return false;
    }

    memset(encOut, 0, sizeof(EncoderOut_t));

    ret = mVpuCtx->encoder_getstream(mVpuCtx, encOut);
    if (ret < 0 || encOut->size == 0) {
        return false; // EOS_STREAM_REACHED;
    } else if (encOut->size > 0) {
        int32_t offset = 0;
        if (mFrameCount == 0 && mSpsPpsLen > 0) {
            // setup h264 sps_pps header flag
            memcpy(mOutputBuf, mSpsPpsBuf, mSpsPpsLen);
            encOut->size += mSpsPpsLen;
            offset = mSpsPpsLen;
        }

        memcpy(mOutputBuf + offset, "\x00\x00\x00\x01", 4);
        memcpy(mOutputBuf + offset + 4, encOut->data, encOut->size);
        encOut->size += 4;

        if (encOut->data != NULL) {
            free(encOut->data);
            encOut->data = mOutputBuf;
        }

        mFrameCount++;
        ALOGI("Get one frame_num %d size %d pts %lld keyFrame %d", mFrameCount, encOut->size, encOut->timeUs,
              encOut->keyFrame);

        return true;
    }

    return false; // retry again
}
