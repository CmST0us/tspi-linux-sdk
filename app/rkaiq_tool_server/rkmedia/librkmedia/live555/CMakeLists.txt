#
# Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

# vi: set noexpandtab syntax=cmake:

set(EASY_MEDIA_LIVE555_SOURCE_FILES)
set(EASY_MEDIA_LIVE555_LIBS)

option(LIVE555_SERVER "compile: live555 server" OFF)
if(LIVE555_SERVER)
  add_subdirectory(server)
endif()

set(EASY_MEDIA_SOURCE_FILES ${EASY_MEDIA_SOURCE_FILES}
                            ${EASY_MEDIA_LIVE555_SOURCE_FILES} PARENT_SCOPE)
set(EASY_MEDIA_DEPENDENT_LIBS ${EASY_MEDIA_DEPENDENT_LIBS}
                              ${EASY_MEDIA_LIVE555_LIBS} PARENT_SCOPE)
