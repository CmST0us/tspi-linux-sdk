/*
 *
 * Copyright 2010 Rockchip Electronics S.LSI Co. LTD
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
 */

/***************************************************************************************************
    File:
        vpu_macro.h
    Description:
        Macro definition in VPU module
    Author:
        Jian Huan
    Date:
        2010-11-23 21:45:57
 **************************************************************************************************/
#ifndef _VPU_MACRO_
#define _VPU_MACRO_

#include "vpu_type.h"

#define     VPU_OK                                  0
#define     VPU_ERR                                 1

#define     VDPU_BASE                               0x10104200

#define     VPU_MAX_FRAME_WIDTH                     1920
#define     VPU_MAX_FRAME_HEIGHT                    1088

#define     VPU_FRAME_NULL                          NULL

#define     DEC_RK70_REGISTERS                      159

#define     DEC_RK70_LITTLE_ENDIAN                  1
#define     DEC_RK70_BIG_ENDIAN                     0

#define     DEC_RK70_BUS_BURST_LENGTH_UNDEFINED     0
#define     DEC_RK70_BUS_BURST_LENGTH_4             4
#define     DEC_RK70_BUS_BURST_LENGTH_8             8
#define     DEC_RK70_BUS_BURST_LENGTH_16            16

#define  VPU_CODEC_DEC_H264      0
#define  VPU_CODEC_DEC_MPEG4      1
#define  VPU_CODEC_DEC_H263      2
#define  VPU_CODEC_DEC_JPEG      3
#define  VPU_CODEC_DEC_VC1      4
#define  VPU_CODEC_DEC_MPEG2      5
#define  VPU_CODEC_DEC_MPEG1      6
#define  VPU_CODEC_DEC_VP6      7
#define  VPU_CODEC_DEC_RV      8
#define  VPU_CODEC_DEC_VP7      9
#define  VPU_CODEC_DEC_VP8      10
#define  VPU_CODEC_DEC_AVS      11
#define  VPU_CODEC_DEC_SVC      12
#define  VPU_CODEC_DEC_VC2      13
#define  VPU_CODEC_DEC_MVC      14
#define  VPU_CODEC_DEC_THEORA     15
#define  VPU_CODEC_ENC_JPEG      16
#define  VPU_CODEC_ENC_H264      17
#define  VPU_CODEC_DEC_MJPEG     18
/*@jh: ∂ÓÕ‚∏Ωº”*/
#define     VPU_CODEC_DEC_DIVX3                     18
#define     VPU_CODEC_DEC_SORESONSPARKLE            19

#define     VPU_OUTPUT_FRAME_TYPE      0
#define     VPU_OUTPUT_TOP_FIELD_FIRST_TYPE    1
#define     VPU_OUTPUT_BOT_FIELD_FIRST_TYPE    2

/*
 * bit definition of ColorType in structure VPU_FRAME
 */
#define VPU_OUTPUT_FORMAT_TYPE_MASK                 (0x0000ffff)
#define VPU_OUTPUT_FORMAT_ARGB8888                  (0x00000000)
#define VPU_OUTPUT_FORMAT_ABGR8888                  (0x00000001)
#define VPU_OUTPUT_FORMAT_RGB888                    (0x00000002)
#define VPU_OUTPUT_FORMAT_RGB565                    (0x00000003)
#define VPU_OUTPUT_FORMAT_RGB555                    (0x00000004)
#define VPU_OUTPUT_FORMAT_YUV420_SEMIPLANAR         (0x00000005)
#define VPU_OUTPUT_FORMAT_YUV420_PLANAR             (0x00000006)
#define VPU_OUTPUT_FORMAT_YUV422_SEMIPLANAR         (0x00000007)
#define VPU_OUTPUT_FORMAT_YUV422_PLANAR             (0x00000008)
#define VPU_OUTPUT_FORMAT_YUV444                    (0x00000009)
#define VPU_OUTPUT_FORMAT_YCH420                    (0x0000000A)

#define VPU_OUTPUT_FORMAT_BIT_MASK                  (0x000f0000)
#define VPU_OUTPUT_FORMAT_BIT_8                     (0x00000000)
#define VPU_OUTPUT_FORMAT_BIT_10                    (0x00010000)
#define VPU_OUTPUT_FORMAT_BIT_12                    (0x00020000)
#define VPU_OUTPUT_FORMAT_BIT_14                    (0x00030000)
#define VPU_OUTPUT_FORMAT_BIT_16                    (0x00040000)

#define  VPU_PP_INPUT_FORMAT_YUV420_SEMIPLANAR  0
#define  VPU_PP_INPUT_FORMAT_YUV420_PLANAR   1
#define  VPU_PP_INPUT_FORMAT_YUV420_INTERLACE  2
#define  VPU_PP_INPUT_FORMAT_YUV422     3

#define  VPU_PP_SCALE_DISABLE      0
#define  VPU_PP_SCALE_ENABLE       1

#define  VPU_PP_ROTATE_DISABLE      0
#define  VPU_PP_ROTATE_ENABLE      1

#define  VPU_PP_DITHER_DISABLE      0
#define  VPU_PP_DITHER_ENABLE      1

#define  VPU_PP_DEBLK_DISABLE      0
#define  VPU_PP_DEBLK_ENABLE       1

#define  VPU_PP_DEINTERLACE_DISABLE     0
#define  VPU_PP_DEINTERLACE_ENABLE     1

#define     VPU_BITSTREAM_START_CODE        (0x524b5642)    /* RKVB, rockchip video bitstream */

#define     SWAP(_T_,A,B)       { _T_ tmp = A; A = B; B = tmp; }
#ifndef MIN
#define     MIN(X, Y)           ((X)<(Y)?(X):(Y))
#endif
#ifndef MAX
#define     MAX(X, Y)           ((X)>(Y)?(X):(Y))
#endif
#ifndef ABS
#define     ABS(X)              (((X) < 0) ? (-(X)) : (X))
#endif
#define     SIGN(X)             (((X)>0)?1:-1)
#define     BSWAP(a) \
    ((((a) & 0xff) << 24)  | (((a) & 0xff00) << 8) | \
           (((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))
#define     CLIP(X,AMIN,AMAX)   (((X)<(AMIN)) ? (AMIN) : ((X)>(AMAX)) ? (AMAX) : (X))
#define     MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))


//#define FRAME_COPY

//#define _VPU_TRACE_ENABLE

#ifdef _VPU_TRACE_ENABLE
#define LOG_TAG "VPU_TRACE"
#ifdef AVS40
#define ALOGV printf
#define ALOGD printf
#define ALOGI printf
#define ALOGW printf
#define ALOGE printf
#endif
#define VPU_TRACE(fmt,args...) //ALOGD(__FILE__ ":%d: " fmt, __LINE__ , ## args)
#define VPU_T(fmt, args...)
#else
#define VPU_TRACE(fmt, args...)
#define VPU_T(fmt,args...) //ALOGD(__FILE__ ":%d: " fmt, __LINE__ , ## args)
#endif


#endif /*_VPU_MACRO_*/
