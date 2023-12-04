// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_LINK_CONFIG_H_
#define EASYMEDIA_LINK_CONFIG_H_

namespace easymedia
{

    enum LinkType
    {
        LINK_NONE,
        LINK_VIDEO,
        LINK_AUDIO,
        LINK_PICTURE,
        LINK_NNDATA
    };

    typedef struct linkvideo {
        void* buffer_ptr;
        unsigned int buffer_size;
        time_t timestamp;
        int nat_type;
    } linkvideo_s;

    typedef struct linkaudio {
        void* buffer_ptr;
        unsigned int buffer_size;
        time_t timestamp;
    } linkaudio_s;

    typedef struct linkpicture {
        void* buffer_ptr;
        unsigned int buffer_size;
        int type;
        const char* id;
    } linkpicture_s;

    typedef struct linknndata {
        int size;
        time_t timestamp;
        const char* nn_model_name;
        RknnResult* rknn_result;
    } linknndata_s;

    typedef struct linkcommon {
        int linktype;
        union {
            linkvideo_s video;
            linkaudio_s audio;
            linkpicture picture;
        };
    } linkcommon_s;

} // namespace easymedia

#endif // #ifndef EASYMEDIA_FLOW_H_
