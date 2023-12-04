/*
 * Copyright (C) 2019 Hertz Wang 1989wanghang@163.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see http://www.gnu.org/licenses
 *
 * Any non-GPL usage of this software or parts of this software is strictly
 * forbidden.
 *
 */

#ifndef NPU_UVC_SHARED_H_
#define NPU_UVC_SHARED_H_

#include <stdint.h>
#include <rockx.h>

#ifdef __cplusplus
extern "C" {
#endif

enum RK_NN_OUTPUT_TYPE {
  TYPE_INVALID_NPU_OUTPUT = -1,
  TYPE_RK_NPU_OUTPUT = 0,
  TYPE_RK_ROCKX_OUTPUT,
  TYPE_RK_FACE_SDK_OUTPUT
};

// all types of output shoule be redefined with attribute packed

// rk npu output
struct aligned_npu_output {
  uint8_t want_float;
  uint8_t is_prealloc;
  uint32_t index;
  uint32_t size;
  uint8_t buf[0];
} __attribute__((packed));

// rockx output
struct aligned_rockx_face_rect {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  uint8_t score[4]; // assert(sizeof(float) == 4);
} __attribute__((packed));

struct aligned_rockx_face_landmark {
  uint8_t landmarks_count;
  rockx_face_landmark_t* marks;
} __attribute__((packed));

struct aligned_rockx_face_gender_age {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
  uint8_t score[4]; // assert(sizeof(float) == 4);
  int32_t gender;
  int32_t age;
} __attribute__((packed));

// the width and height of npu processing image
struct npu_widthheight {
  uint32_t width;
  uint32_t height;
} __attribute__((packed));

// post process at host side
struct extra_jpeg_data {
  int64_t picture_timestamp;     // the time stamp of picture
  int64_t npu_outputs_timestamp; // the time stamp of npu outputs
  uint32_t npu_output_type;      // RK_NN_OUTPUT_TYPE
  int8_t model_identifier[64];
  struct npu_widthheight npuwh;
  uint32_t npu_output_size; // the length of follow outputs
  uint32_t npu_outputs_num; // the num of npu output
  uint8_t outputs[0];       // the buffer of npu outputs
} __attribute__((packed));

#ifdef __cplusplus
}
#endif

#endif
