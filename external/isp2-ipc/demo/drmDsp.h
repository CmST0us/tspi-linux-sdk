#ifndef __DRM_DSP_H__
#define __DRM_DSP_H__
#ifdef __cplusplus

extern "C"
{
#endif

#include <drm_fourcc.h>

int initDrmDsp();
int drmDspFrame(int width, int height, void* dmaFd, int fmt);
void deInitDrmDsp();
#ifdef __cplusplus
}
#endif

#endif

