#ifndef __RKVPU_ENC_API_H__
#define __RKVPU_ENC_API_H__

#include "vpu_api.h"

#define BUFFERFLAG_EOS 0x00000001

class RKHWEncApi
{
  public:
    RKHWEncApi();
    ~RKHWEncApi();

    typedef struct EncCfgInfo {
        int32_t width;
        int32_t height;
        int32_t format; /* input yuv format */
        int32_t IDRInterval;
        int32_t rc_mode;   /* 0 - VBR mode; 1 - CBR mode; 2 - FIXQP mode */
        int32_t bitRate;   /* target bitrate */
        int32_t framerate; /* target framerate */
        int32_t qp;        /* coding quality, from 1~51 */
    } EncCfgInfo_t;

    bool init(EncCfgInfo* cfg);

    // send video frame to encoder only, async interface
    bool sendFrame(char* data, int32_t size, int64_t pts, int32_t flag);
    bool sendFrame(int32_t fd, int32_t size, int64_t pts, int32_t flag);
    // get encoded video packet from encoder only, async interface
    bool getOutStream(EncoderOut_t* encOut);

  private:
    VpuCodecContext* mVpuCtx;
    unsigned char* mOutputBuf;
    unsigned char* mSpsPpsBuf;
    int32_t mSpsPpsLen;

    int32_t mFrameCount;
};

#endif // __RKVPU_ENC_API_H__
