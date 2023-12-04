/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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

#ifndef FACE_LINE_TYPE_H_
#define FACE_LINE_TYPE_H_

#include <stdint.h>

typedef int32_t INT32;

typedef struct _FaceData {
  INT32 id;
  INT32 x;
  INT32 y;
  INT32 activate;
  INT32 width;
  INT32 height;
  INT32 left;
  INT32 top;
  INT32 right;
  INT32 bottom;
  float score;
} FaceData;

typedef struct _FaceAiData {
  FaceData *face_data;
  INT32 face_count;
} FaceAiData;


#endif // FACE_LINE_TYPE_H_
