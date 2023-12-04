// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_SOUND_H_
#define EASYMEDIA_SOUND_H_

#include <stddef.h>

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SAMPLE_FMT_NONE = -1,
    SAMPLE_FMT_U8,
    SAMPLE_FMT_S16,
    SAMPLE_FMT_S32,
    SAMPLE_FMT_FLT,
    SAMPLE_FMT_U8P,
    SAMPLE_FMT_S16P,
    SAMPLE_FMT_S32P,
    SAMPLE_FMT_FLTP,
    SAMPLE_FMT_G711A,
    SAMPLE_FMT_G711U,
    SAMPLE_FMT_NB
} SampleFormat;

typedef struct {
    SampleFormat fmt;
    int channels;
    int sample_rate;
    int nb_samples;
} SampleInfo;

#ifndef MAX_FILE_PATH_LEN
    #define MAX_FILE_PATH_LEN 256
    #define AI_TALKVQE_MASK_AEC 0x1
    #define AI_TALKVQE_MASK_ANR 0x2
    #define AI_TALKVQE_MASK_AGC 0x4
    #define AI_RECORDVQE_MASK_ANR 0x1
    #define AO_VQE_MASK_ANR 0x1
    #define AO_VQE_MASK_AGC 0x2
#endif

typedef enum
{
    VQE_MODE_AI_TALK,
    VQE_MODE_AI_RECORD,
    VQE_MODE_AO,
    VQE_MODE_BUTT
} VQE_MODE_E;

typedef struct rkVQE_CONFIG_S {
    VQE_MODE_E u32VQEMode;
    union {
        struct {
            uint32_t u32OpenMask;
            int32_t s32WorkSampleRate;
            int32_t s32FrameSample;
            char aParamFilePath[MAX_FILE_PATH_LEN];
        } stAiTalkConfig;
        struct {
            uint32_t u32OpenMask;
            uint32_t s32WorkSampleRate;
            uint32_t s32FrameSample;
            struct {
                float fPostAddGain; /* post-gain 0*/
                float fGmin;        /* spectral gain floor,unit:(dB),default:-30dB */
                float fNoiseFactor; /* noise suppression factor,default:0.98 */
            } stAnrConfig;
        } stAiRecordConfig;
        struct {
            uint32_t u32OpenMask;
            uint32_t s32WorkSampleRate;
            uint32_t s32FrameSample;
            char aParamFilePath[MAX_FILE_PATH_LEN];
        } stAoConfig;
    };
} VQE_CONFIG_S;

#ifdef __cplusplus
}
#endif

_API const char* SampleFmtToString(SampleFormat fmt);
_API SampleFormat StringToSampleFmt(const char* fmt_str);
_API bool SampleInfoIsValid(const SampleInfo& sample_info);
_API size_t GetSampleSize(const SampleInfo& sample_info);

#include <map>
#include <string>
namespace easymedia
{
    bool ParseSampleInfoFromMap(std::map<std::string, std::string>& params, SampleInfo& si);
    std::string _API to_param_string(const SampleInfo& si);
} // namespace easymedia

#endif // #ifndef EASYMEDIA_SOUND_H_
