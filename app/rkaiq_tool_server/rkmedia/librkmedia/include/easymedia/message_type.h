// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MESSAGE_TYPE_H_
#define EASYMEDIA_MESSAGE_TYPE_H_

#define MSG_INFO_MASK 0x000000
#define MSG_WARN_MASK 0x100000
#define MSG_ERROR_MASK 0x200000

typedef enum
{
    MSG_FLOW_EVENT_INFO_UNKNOW = MSG_INFO_MASK,
    MSG_FLOW_EVENT_INFO_EOS,
    MSG_FLOW_EVENT_INFO_MOVEDETECTION,
    MSG_FLOW_EVENT_INFO_OCCLUSIONDETECTION,
    MSG_FLOW_EVENT_WARN_UNKNOW = MSG_WARN_MASK,
    MSG_FLOW_EVENT_ERROR_UNKNOW = MSG_ERROR_MASK,
} MessageId;

typedef struct {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
} MoveDetecInfo;

typedef struct {
    unsigned short info_cnt;
    unsigned short ori_width;
    unsigned short ori_height;
    unsigned short ds_width;
    unsigned short ds_height;
    MoveDetecInfo data[4096];
} MoveDetectEvent;

typedef struct {
    unsigned short x;
    unsigned short y;
    unsigned short w;
    unsigned short h;
    unsigned short occlusion;
} OcclusionDetecInfo;

typedef struct {
    unsigned short info_cnt;
    unsigned short img_width;
    unsigned short img_height;
    OcclusionDetecInfo data[10];
} OcclusionDetectEvent;

typedef enum
{
    MESSAGE_TYPE_FIFO = 0,
    MESSAGE_TYPE_LIFO,
    MESSAGE_TYPE_UNIQUE
} MessageType;

#endif // #ifndef EASYMEDIA_MESSAGE_TYPE_H_
