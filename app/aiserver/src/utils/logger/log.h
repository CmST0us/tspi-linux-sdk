// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _RK_LOGGER_H_
#define _RK_LOGGER_H_

#ifdef ENABLE_MINILOGGER
#include "minilogger/log.h"
#else
#define minilog_warn(...)
#define minilog_error(...)
#define minilog_info(...)
#define minilog_debug(...)
#define __minilog_log_init(...)
#endif

static int minilog_enable    = 1;
static int minilog_log_level = 2;

namespace rockchip {
namespace aiserver {

#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#ifndef LOG_TAG
#define LOG_TAG ""
#endif // LOG_TAG

#define LOG_INFO(format, ...)                                                  \
  do {                                                                         \
    if (minilog_log_level < LOG_LEVEL_INFO)                                \
      break;                                                                   \
    if (minilog_enable)                                                        \
      minilog_info("[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);  \
    else                                                                       \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_WARN(format, ...)                                                  \
  do {                                                                         \
    if (minilog_log_level < LOG_LEVEL_WARN)                                \
      break;                                                                   \
    if (minilog_enable)                                                        \
      minilog_warn("[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__);  \
    else                                                                       \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_ERROR(format, ...)                                                 \
  do {                                                                         \
    if (minilog_log_level < LOG_LEVEL_ERROR)                               \
      break;                                                                   \
    if (minilog_enable)                                                        \
      minilog_error("[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__); \
    else                                                                       \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

#define LOG_DEBUG(format, ...)                                                 \
  do {                                                                         \
    if (minilog_log_level < LOG_LEVEL_DEBUG)                               \
      break;                                                                   \
    if (minilog_enable)                                                        \
      minilog_debug("[%s][%s]:" format, LOG_TAG, __FUNCTION__, ##__VA_ARGS__); \
    else                                                                       \
      fprintf(stderr, "[%s][%s]:" format, LOG_TAG, __FUNCTION__,               \
              ##__VA_ARGS__);                                                  \
  } while (0)

} // namespace aiserver
} // namespace rockchip

#endif
