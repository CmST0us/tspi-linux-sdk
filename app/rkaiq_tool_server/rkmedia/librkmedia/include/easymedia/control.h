// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_CONTROL_H_
#define EASYMEDIA_CONTROL_H_

#include <stdint.h>

#include "image.h"
#include "rknn_user.h"

namespace easymedia
{

    typedef struct {
        const char* name;
        uint64_t value;
    } DRMPropertyArg;

    typedef struct {
        unsigned long int sub_request;
        int size;
        void* arg;
    } SubRequest;

    typedef struct {
        bool enable;
    } NNinputArg;

    typedef struct {
        bool enable;
    } FaceDetectArg;

    typedef struct {
        bool enable;
    } FaceCaptureArg;

    typedef struct {
        bool enable;
    } DrawFilterArg;

    typedef struct {
        bool enable;
        int interval;
        int duration;     /* second */
        float percentage; /* 0-100 */
        ImageRect rect;
    } BodyDetectArg;

    typedef enum
    {
        USER_ADD_CAM = 0,
        USER_ADD_PIC,
        USER_REG_PIC,
        USER_DEL,
        USER_CLR,
        USER_ENABLE,
    } FaceRegArgType;

    typedef struct {
        FaceRegArgType type;
        int user_id; /* it is used to delete user */
        char pic_path[RKNN_PICTURE_PATH_LEN];
        bool enable;
    } FaceRegArg;

    typedef struct {
        bool enable;
        int interval;
    } RockxFilterArg;

    enum
    {
        S_FIRST_CONTROL = 10000,
        S_SUB_REQUEST, // many devices have their kernel controls

        // DRM Display controls
        // ImageRect
        S_SOURCE_RECT = 10100,
        S_DESTINATION_RECT,
        S_SRC_DST_RECT,
        // ImageInfo
        G_PLANE_IMAGE_INFO,
        // int
        G_PLANE_SUPPORT_SCALE,
        // DRMPropertyArg
        S_CRTC_PROPERTY,
        S_CONNECTOR_PROPERTY,

        // V4L2 controls
        // any type
        S_STREAM_OFF = 10200,

        // ALSA controls
        // int
        S_ALSA_VOLUME = 10300,
        G_ALSA_VOLUME,
        S_VQE_ENABLE,
        S_VQE_ATTR,
        G_VQE_ATTR,

        // Through Guard controls
        // int
        S_ALLOW_THROUGH_COUNT = 10400,

        // ANR controls
        // int
        S_ANR_ON = 10500,
        G_ANR_ON,

        // RKNN controls
        // any type
        S_NN_CALLBACK = 10600,
        G_NN_CALLBACK,
        S_NN_HANDLER,
        G_NN_HANDLER,
        S_NN_DRAW_HANDLER,
        G_NN_DRAW_HANDLER,
        S_NN_INFO,
        G_NN_INFO,

        // Move Detection
        S_MD_ROI_ENABLE = 10700,
        G_MD_ROI_ENABLE,
        S_MD_ROI_RECTS,
        G_MD_ROI_RECTS,
        S_MD_SENSITIVITY,
        G_MD_SENSITIVITY,

        // MUXER controls
        S_START_SRTEAM = 10800,
        S_STOP_SRTEAM,
        G_MUXER_GET_STATUS,
        S_MUXER_FILE_DURATION,
        S_MUXER_FILE_PATH,
        S_MUXER_FILE_PREFIX,

        // Occlusion Detection
        S_OD_ROI_ENABLE = 10900,
        G_OD_ROI_ENABLE,
        S_OD_ROI_RECTS,
        G_OD_ROI_RECTS,
        S_OD_SENSITIVITY,
        G_OD_SENSITIVITY,
    };

} // namespace easymedia

#endif // #ifndef EASYMEDIA_CONTROL_H_
