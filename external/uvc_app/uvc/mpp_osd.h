/*
 * Copyright (C) 2019 Rockchip Electronics Co., Ltd.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL), available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __MPI_OSD_H__
#define __MPI_OSD_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>

//***************OSD*********************//
#define MPP_ENC_OSD_ENABLE 0

#if MPP_ENC_OSD_ENABLE
#define MJPEG_RGA_OSD_ENABLE 1  //can modify this
#define YUV_RGA_OSD_ENABLE 1  //can modify this
#else
#define MJPEG_RGA_OSD_ENABLE 0  //do not modify this
#define YUV_RGA_OSD_ENABLE 0  //do not modify this
#endif

#define FIX_MPP_WHITE_EDGE_VALUE 0 //reference  80
#define FIX_RGA_WHITE_EDGE_VALUE 0 //reference  120

#define OSD_REGIONS_CNT 8
#define PALETTE_TABLE_LEN 256
#define OSD_DATA_DEBUG 0

#define WIDTHBYTES(bits) ((((bits) + 31) >> 5) * 4)


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int int32_t;

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

enum BMP_TYPE {
    BMP_TYPE_NO_DEF = 0,
    BMP_TYPE_ARGB,
    BMP_TYPE_BGRA_1, // 0x00ffffff   ->transparency
    BMP_TYPE_BGRA_2, // 0x00xxxxxx -> transparency
};

typedef union
{
    struct
    {
        uint32_t data_32;
    };
    struct
    {
        uint8_t argb_a; // low bit
        uint8_t argb_r;
        uint8_t argb_g;
        uint8_t argb_b; // high bit
    };
    struct
    {
        uint8_t bgra_b; // low bit
        uint8_t bgra_g;
        uint8_t bgra_r;
        uint8_t bgra_a; // high bit
    };

}BMP_DATA;


typedef struct tagBITMAPFILEHEADER {
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER;

typedef struct  {
    uint8_t *buffer; //Content: ID of palette
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t width;
    uint32_t height;
    uint32_t inverse;
    uint32_t region_id; // max = 8.
    uint8_t enable;
} OsdRegionData;

typedef struct osd_data {
    int type;
    union {
        const char *image;
        //text_data_s text;
        //border_data_s border;
    };
    int width;
    int height;
    uint8_t *buffer;
    uint32_t size;
    int origin_x;
    int origin_y;
    int enable;
    uint32_t region_id; // max = 8.
    uint32_t *plt_table; //argb
    FILE *bmp_file;
    uint8_t *bmp_data;
    BITMAPFILEHEADER bmp_bit_head;
    BITMAPINFOHEADER bmp_info_head;
    enum BMP_TYPE bmp_type;
} osd_data_s;

enum {
    OSD_REGION_ID_TIMEDATE = 0,
    OSD_REGION_ID_LAYER1, // ID_CHANNEL
    OSD_REGION_ID_LAYER2, // ID_IMAGE
    OSD_REGION_ID_LAYER3, // ID_TEXT3-4
    OSD_REGION_ID_LAYER4, // ID_TEXT0
    OSD_REGION_ID_LAYER5, // ID_TEXT1  (2)
    OSD_REGION_ID_LAYER6, // ID_MASK0  (1-2-3)
    OSD_REGION_ID_LAYER7, // ID_TEXT5-6-7
};

enum OSD_REGION_TYPE {
    OSD_REGION_TYPE_PICTURE = 0,
    OSD_REGION_TYPE_DATETIME,
    OSD_REGION_TYPE_MAX,
};

//************************************//

typedef struct rkMB_IMAGE_INFO {
    uint32_t u32Width;
    uint32_t u32Height;
    uint32_t u32HorStride;
    uint32_t u32VerStride;
  //IMAGE_TYPE_E enImgType;
} MB_IMAGE_INFO_S;

#ifdef __cplusplus
}
#endif
#endif

