#
# Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

# vi: set noexpandtab syntax=cmake:

set(EASY_MEDIA_FLOW_SOURCE_FILES
    flow/video_encoder_flow.cc
    flow/audio_encoder_flow.cc
    flow/decoder_flow.cc
    flow/file_flow.cc
    flow/filter_flow.cc
    flow/link_flow.cc
    flow/source_stream_flow.cc
    flow/muxer_flow.cc
    flow/audio_decoder_flow.cc
    flow/output_stream_flow.cc)

if(MOVE_DETECTION)
set(EASY_MEDIA_FLOW_SOURCE_FILES ${EASY_MEDIA_FLOW_SOURCE_FILES}
                                 flow/move_detection_flow.cc)
endif()

if(OCCLUSION_DETECTION)
set(EASY_MEDIA_FLOW_SOURCE_FILES ${EASY_MEDIA_FLOW_SOURCE_FILES}
                                 flow/occlusion_detection_flow.cc)
endif()

set(EASY_MEDIA_SOURCE_FILES ${EASY_MEDIA_SOURCE_FILES}
                            ${EASY_MEDIA_FLOW_SOURCE_FILES} PARENT_SCOPE)
