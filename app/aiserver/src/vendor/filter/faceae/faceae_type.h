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
 * author: kevin.lin@rock-chips.com
 *   date: 2020-09-18
 * module: faceae
 */

#ifndef FACEAE_TYPE_H_
#define FACEAE_TYPE_H_

#include <stdint.h>

typedef int32_t INT32;

typedef struct _FaceAeInitInfo {
  INT32 face_npu_width;   // AI计算使用图像的宽
  INT32 face_npu_height;  // AI计算使用图像的高
  INT32 face_src_width;   //摄像头源数据的宽
  INT32 face_src_height;  //摄像头源数据的高
  float face_facedetect_score_shold; // AI人脸质量分阈值
  INT32 face_fast_move_frame_judge;  //人物移动防抖阈值
  INT32 face_zoom_speed;             //人脸跟踪ZOOM/PAN效果转换速度
} FaceAeInitInfo;
typedef struct _FaceAeRect {
  INT32 x;
  INT32 y;
  INT32 w;
  INT32 h;
} FaceAeRect;

typedef struct _FaceData {
  INT32 left;
  INT32 top;
  INT32 right;
  INT32 bottom;
  float score;
} FaceData;

typedef struct _FaceAeAiData {
  FaceData *face_data;
  int face_count;
} FaceAeAiData;


#endif // faceae_TYPE_H_
