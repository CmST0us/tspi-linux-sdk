// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rockx_task_handler.h"
#include "logger/log.h"
#include "nn_vision_rockx.h"
#include "RTAIDetectResults.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "RockxTaskHandler"


namespace rockchip {
namespace aiserver {

RockxTaskHandler::RockxTaskHandler() {
    mShmNNcontroller = new ShmNNController();
}

RockxTaskHandler::~RockxTaskHandler() {
    if (mShmNNcontroller != nullptr) {
        delete mShmNNcontroller;
        mShmNNcontroller = nullptr;
    }
}

int32_t RockxTaskHandler::processAIData(RTMediaBuffer *buffer) {
    if (buffer == NULL)
        return 0;

    void* nnResult = getAIDetectResults(buffer);
    if (nnResult != NULL) {
        postNNData(nnResult);
    }
    buffer->release();

    return 0;
}

int32_t RockxTaskHandler::processAIMatting(RTMediaBuffer *buffer) {

    return 0;
}

int32_t RockxTaskHandler::processAIFeature(RTMediaBuffer *buffer) {

    return 0;
}


void RockxTaskHandler::postNNData(void *nnResult) {
    std::lock_guard<std::mutex> lock(opMutex);
    auto   detectRes    = (RTRknnAnalysisResults*)(nnResult);
    bool   needUpate    = true;
    const char *nnName  = NULL;

    if (!detectRes || !(detectRes->count) || (detectRes->results == NULL))
        return;
    NNData nnData;
    int32_t size = detectRes->count;
    for (int32_t i = 0; i < size; i++) {
        RTRknnResult* result = &detectRes->results[i];
        RTNNDataType nnType = result->type;
        switch (nnType) {
          case RT_NN_TYPE_FACE:
            pushFaceDetectInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_FACE_DETECT;
            break;
          case RT_NN_TYPE_BODY:
            pushPoseBodyInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_POSE_BODY;
            break;
          case RT_NN_TYPE_LANDMARK:
            pushLandMarkInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_FACE_LANDMARK;
            break;
          case RT_NN_TYPE_FINGER:
            pushFingerDetectInfo(&nnData, result, 1);
            nnName = ROCKX_MODEL_POSE_FINGER;
            break;
          default:
            needUpate = false;
            break;
        }
    }

    if (needUpate) {
        std::string sendbuf;
        int res = setNNGeneralInfos(&nnData, &detectRes->results[0], 1);
        if (res >= 0) {
            nnData.set_model_name(nnName);
            nnData.SerializeToString(&sendbuf);
            nnData.ParseFromString(sendbuf);
            mShmNNcontroller->send(sendbuf);
        }
    }
}

void RockxTaskHandler::pushFaceDetectInfo(NNData *nnData, void *bufptr, int32_t size) {
    if (!bufptr || !nnData)
        return;
    auto nn_result_face_detect = (RTRknnResult *)(bufptr);
    FaceDetect *facedetect;
    for (int i = 0; i < size; i++) {
        auto face_detect_item = nn_result_face_detect->info_face.object;
        float score = face_detect_item.score;
        if (score < 0.5f) {
            LOG_DEBUG("face score %f\n", score);
            continue;
        }
        int x1 = face_detect_item.box.left;
        int y1 = face_detect_item.box.top;
        int x2 = face_detect_item.box.right;
        int y2 = face_detect_item.box.bottom;
        facedetect = nnData->add_face_detect();
        facedetect->set_left(x1);
        facedetect->set_top(y1);
        facedetect->set_right(x2);
        facedetect->set_bottom(y2);
        facedetect->set_score(score);
        // LOG_DEBUG("AIServer: FaceInfo Rect[%04d,%04d,%04d,%04d] score=%f\n", x1, y1, x2, y2, score);
    }
}

void RockxTaskHandler::pushPoseBodyInfo(NNData *nnData, void *bufptr, int32_t size) {
    if (!bufptr || !nnData)
        return;
    auto nn_result_pose_body = (RTRknnResult *)(bufptr);
    LandMark *landmark;
    for (uint32_t i = 0; i < size; i++) {
        auto keyPointsItem = nn_result_pose_body->info_body.object;
        landmark = nnData->add_landmark();
        for (int j = 0; j < keyPointsItem.count; j++) {
            Points *mPoint = landmark->add_points();
            mPoint->set_x(keyPointsItem.points[j].x);
            mPoint->set_y(keyPointsItem.points[j].y);
            LOG_DEBUG("  %s [%d, %d] %f\n", ROCKX_POSE_BODY_KEYPOINTS_NAME[j],
                     keyPointsItem.points[j].x, keyPointsItem.points[j].y,
                     keyPointsItem.score[j]);
        }
    }
}

void RockxTaskHandler::pushLandMarkInfo(NNData *nnData, void *bufptr, int32_t size) {
    if (!bufptr || !nnData)
        return;
    auto nn_result_land_mark = (RTRknnResult *)(bufptr);
    LandMark *landmark;
    for (uint32_t i = 0; i < size; i++) {
        landmark = nnData->add_landmark();
        auto face_landmark_item = nn_result_land_mark->info_landmark.object;
        for (int j = 0; j < face_landmark_item.landmarks_count; j++) {
            Points *mPoint = landmark->add_points();
            mPoint->set_x(face_landmark_item.landmarks[j].x);
            mPoint->set_y(face_landmark_item.landmarks[j].y);
        }
    }
}

void RockxTaskHandler::pushFingerDetectInfo(NNData *nnData, void *bufptr, int32_t size) {
    if (!bufptr)
        return;
}

int32_t RockxTaskHandler::setNNGeneralInfos(NNData *nnData, void *bufptr, int32_t size) {
    if (!bufptr || size <= 0)
        return -1;

    auto nn_result = (RTRknnResult *)(bufptr);
    int32_t type = nn_result->type;
    int32_t npu_w = 0, npu_h = 0;
    npu_w = nn_result->img_w;
    npu_h = nn_result->img_h;
    if (npu_w <= 0 || npu_h <= 0) {
        npu_w = 300;
        npu_h = 300;
    }
    nnData->set_nn_width(npu_w);
    nnData->set_nn_height(npu_h);
    nnData->set_model_type(type);
    return 0;
}

int32_t RockxTaskHandler::convertDetectType(int32_t detectType) {
    return detectType;
}

} // namespace aiserver
} // namespace rockchip
