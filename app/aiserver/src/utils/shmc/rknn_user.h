// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_RKNN_USER_H_
#define EASYMEDIA_RKNN_USER_H_

#ifdef USE_ROCKFACE
#include "rockface/rockface.h"
#endif

#ifdef USE_ROCKX
#include <rockx/rockx.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

using RknnCallBack = std::add_pointer<void(void* handler, int type, void *ptr, int size)>::type;
// using RknnHandler = std::add_pointer<void*>::type;

typedef struct Rect {
    int left;
    int top;
    int right;
    int bottom;
} Rect;

typedef struct {
#ifdef USE_ROCKFACE
  rockface_det_t base;
  rockface_attribute_t attr;
  rockface_landmark_t landmark;
  rockface_angle_t angle;
  rockface_feature_t feature;
#endif
#ifdef USE_ROCKX
  rockx_object_t object;
#endif
} FaceInfo;

typedef struct {
#ifdef USE_ROCKX
  rockx_face_landmark_t object;
#endif
} LandmarkInfo;

typedef struct {
#ifdef USE_ROCKFACE
  rockface_det_t base;
#endif
#ifdef USE_ROCKX
  rockx_keypoints_t object;
#endif
} BodyInfo;

typedef struct {
#ifdef USE_ROCKX
  rockx_keypoints_t object;
#endif
} FingerInfo;

typedef enum {
  SUCCESS = 0,
  FAILURE,
  TIMEOUT,
  UNKNOW,
} AuthorizedStatus;

typedef enum {
    NNRESULT_TYPE_NONE = -1,
    NNRESULT_TYPE_FACE = 0,
    NNRESULT_TYPE_FACE_PICTURE_UPLOAD,
    NNRESULT_TYPE_BODY,
    NNRESULT_TYPE_FINGER,
    NNRESULT_TYPE_LANDMARK,
    NNRESULT_TYPE_AUTHORIZED_STATUS,
} RknnResultType;

typedef struct _RknnResult{
    int img_w;
    int img_h;
    int64_t timeval;
    RknnResultType type;
    AuthorizedStatus status;
    union {
      BodyInfo body_info;
      FaceInfo face_info;
      LandmarkInfo landmark_info;
      FingerInfo finger_info;
    };
} RknnResult;

typedef struct _ShmNNData {
    int32_t      size;
    int64_t      timestamp;
    const char*  nn_model_name;
    RknnResult*  rknn_result;
} ShmNNData;

#ifdef __cplusplus
}
#endif

#endif // #ifndef EASYMEDIA_RKNN_USER_H_
