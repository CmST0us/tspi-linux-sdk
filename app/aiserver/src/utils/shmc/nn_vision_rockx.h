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
 * author: hh@rock-chips.com
 *   date: 2020-5-19
 * module: video filter with rknn/rockx/rockface
 */
#ifndef SRC_RT_MEDIA_AV_FILTER_INCLUDE_RTMEDIAROCKX_H_
#define SRC_RT_MEDIA_AV_FILTER_INCLUDE_RTMEDIAROCKX_H_

#ifdef HAVE_ROCKX
#include <rockx/rockx.h>
#endif

#define ROCKX_MODEL_FACE_DETECT       "rockx_face_detect"
#define ROCKX_MODEL_FACE_LANDMARK     "rockx_face_landmark"
#define ROCKX_MODEL_POSE_BODY         "rockx_pose_body"
#define ROCKX_MODEL_POSE_BODY_V2      "rockx_pose_body_v2"
#define ROCKX_MODEL_POSE_FINGER       "rockx_pose_finger"
#define ROCKX_MODEL_FACE_GENDER_AGE   "rockx_face_gender_age"

typedef void (*RTNNCallBack) (void* handler, int32_t type, void *ptr, int32_t size);
typedef void* RTNNHandler;

typedef struct _RTNNInfoFace {
#ifdef HAVE_ROCKFACE
    rockface_det_t base;
    rockface_attribute_t attr;
    rockface_landmark_t landmark;
    rockface_angle_t angle;
    rockface_feature_t feature;
#endif

#ifdef HAVE_ROCKX
    rockx_object_t object;
#endif
} RTNNInfoFace;

typedef struct _RTNNInfoLandmark {
#ifdef HAVE_ROCKX
    rockx_face_landmark_t object;
#endif
} RTNNInfoLandmark;

typedef struct {
#ifdef HAVE_ROCKFACE
    rockface_det_t base;
#endif

#ifdef HAVE_ROCKX
    rockx_keypoints_t object;
#endif
} RTNNInfoBody;

typedef struct _RTNNInfoFinger {
#ifdef USE_ROCKX
    rockx_keypoints_t object;
#endif
} RTNNInfoFinger;

typedef enum _RTNNStatus {
    RT_NN_SUCCESS = 0,
    RT_NN_FAILURE,
    RT_NN_TIMEOUT,
    RT_NN_UNKNOW,
} RTNNStatus;

typedef enum _RTNNDataType {
    RT_NN_TYPE_NONE = -1,
    RT_NN_TYPE_FACE = 0,
    RT_NN_TYPE_BODY,
    RT_NN_TYPE_FINGER,
    RT_NN_TYPE_LANDMARK,
    RT_NN_TYPE_AUTHORIZED_STATUS,
} RTNNDataType;

typedef struct _RTRknnResult {
    int32_t index;
    int32_t img_w;
    int32_t img_h;
    int64_t timeval;
    RTNNDataType type;
    RTNNStatus   status;
    union {
        RTNNInfoBody     info_body;
        RTNNInfoFace     info_face;
        RTNNInfoLandmark info_landmark;
        RTNNInfoFinger   info_finger;
    };
} RTRknnResult;

typedef struct _RTRknnAnalysisResults {
    int32_t         count;
    RTRknnResult    *results;
} RTRknnAnalysisResults;

typedef struct {
    int32_t dataSize;
    int32_t width;
    int32_t height;
    int32_t format;
    int32_t angle;
    int32_t mirror;
    int32_t faceID;
    unsigned char *feature;
    int32_t featureLen;
} RTKKMattingFaceInfo;

typedef struct {
    int32_t faceCount;
    RTKKMattingFaceInfo *faceInfo;
} RTKKAIMattingResult;

#endif  // SRC_RT_MEDIA_AV_FILTER_INCLUDE_RTMEDIAROCKX_H_

