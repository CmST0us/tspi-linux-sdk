// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_RKNN_USER_H_
#define EASYMEDIA_RKNN_USER_H_

#include <type_traits>

#ifdef USE_ROCKFACE
    #include "rockface/rockface.h"
#endif
#ifdef USE_ROCKX
    #include <rockx/rockx.h>
#endif

#define RKNN_PICTURE_PATH_LEN (512)

using RknnCallBack = std::add_pointer<void(void* handler, int type, void* ptr, int size)>::type;
using RknnHandler = std::add_pointer<void*>::type;

typedef struct Rect {
    int left;
    int top;
    int right;
    int bottom;
} Rect;

typedef enum
{
    FACE_REG_NONE = -1,
    FACE_REG_RECOGNIZE = 0,
    FACE_REG_REGISTER,
} FaceRegType;

/* recognize_type: -1, Unknow; register_type: -1, register repeaded, -99
 * register failed */
typedef struct {
    FaceRegType type;
    int user_id;
    float similarity;
    char pic_path[RKNN_PICTURE_PATH_LEN];
} FaceReg;

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
    FaceReg face_reg;
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

typedef enum
{
    SUCCESS = 0,
    FAILURE,
    TIMEOUT,
    UNKNOW,
} AuthorizedStatus;

typedef enum
{
    NNRESULT_TYPE_NONE = -1,
    NNRESULT_TYPE_FACE = 0,
    NNRESULT_TYPE_FACE_PICTURE_UPLOAD,
    NNRESULT_TYPE_BODY,
    NNRESULT_TYPE_FINGER,
    NNRESULT_TYPE_LANDMARK,
    NNRESULT_TYPE_AUTHORIZED_STATUS,
    NNRESULT_TYPE_FACE_REG,
} RknnResultType;

typedef struct {
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

#endif // #ifndef EASYMEDIA_RKNN_USER_H_
