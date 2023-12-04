#
# Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

# vi: set noexpandtab syntax=cmake:

option(LIVE555_SERVER_H264 "compile: live555 server with h264" OFF)
if(LIVE555_SERVER_H264)
  set(EASY_MEDIA_LIVE555_SERVER_SOURCE_FILES
      live555/server/h264_server_media_subsession.cc)
endif()

option(LIVE555_SERVER_H265 "compile: live555 server with h265" OFF)
if(LIVE555_SERVER_H265)
  set(EASY_MEDIA_LIVE555_SERVER_SOURCE_FILES
      ${EASY_MEDIA_LIVE555_SERVER_SOURCE_FILES}
      live555/server/h265_server_media_subsession.cc)
endif()

set(EASY_MEDIA_LIVE555_SERVER_SOURCE_FILES
    ${EASY_MEDIA_LIVE555_SERVER_SOURCE_FILES}
    live555/server/live555_media_input.cc
    live555/server/rtsp_server.cc
    live555/server/aac_server_media_subsession.cc
    live555/server/simple_server_media_subsession.cc
    live555/server/mp2_server_media_subsession.cc
    live555/server/mjpeg_server_media_subsession.cc
    live555/server/live555_server.cc
    live555/server/mjpeg_video_source.cc)

set(EASY_MEDIA_LIVE555_SOURCE_FILES
    ${EASY_MEDIA_LIVE555_SOURCE_FILES} ${EASY_MEDIA_LIVE555_SERVER_SOURCE_FILES}
    PARENT_SCOPE)
set(EASY_MEDIA_LIVE555_LIBS
    ${EASY_MEDIA_LIVE555_LIBS}
    ${LIVEMEDIA_LIBRARIES}
    PARENT_SCOPE)
