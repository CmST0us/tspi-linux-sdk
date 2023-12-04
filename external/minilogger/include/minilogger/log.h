// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef __MINILOG_LOG_H
#define __MINILOG_LOG_H

#ifdef __APPLE__
#define SECTION(X) section("__DATA,__" X)
#define SECTION_START(X) __asm("section$start$__DATA$__" X)
#define SECTION_END(X) __asm("section$end$__DATA$__" X)
#else
#define SECTION(X) section(X)
#define SECTION_START(X)
#define SECTION_END(X)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

struct minilog_debug_desc {
  const char *name;
  const char *file;
#define MINILOG_DEBUG_FLAG_DEFAULT (0)
#define MINILOG_DEBUG_FLAG_PRINT (1 << 0)
#define MINILOG_DEBUG_FLAG_ALIAS (1 << 1)
  unsigned int flags;
} __attribute__((aligned(8)));

int __minilog_log_init(const char *program, const char *debug, bool detach,
                     bool backtrace, const char *program_name,
                     const char *program_version);
void __minilog_log_cleanup(bool backtrace);
void __minilog_log_enable(struct minilog_debug_desc *start,
                        struct minilog_debug_desc *stop);

void minilog_info(const char *format, ...) __attribute__((format(printf, 1, 2)));
void minilog_warn(const char *format, ...) __attribute__((format(printf, 1, 2)));
void minilog_error(const char *format, ...) __attribute__((format(printf, 1, 2)));
void minilog_debug(const char *format, ...) __attribute__((format(printf, 1, 2)));

#define minilog_warn_once(fmt, ...)                                           \
  do {                                                                         \
    static bool printed;                                                       \
    if (!printed) {                                                            \
      minilog_warn(fmt, __VA_ARGS__);                                                  \
      printed = true;                                                          \
    }                                                                          \
  } while (0)

#define MINILOG_DEBUG_DEFINE(name)                                               \
  static struct minilog_debug_desc __debug_alias_##name                          \
      __attribute__((used, SECTION("__debug"), aligned(8))) = {                \
          #name, __FILE__, MINILOG_DEBUG_FLAG_ALIAS};

#define DBG(fmt, ...)                                                       \
  do {                                                                         \
    static struct minilog_debug_desc __minilog_debug_desc                          \
        __attribute__((used, SECTION("__debug"), aligned(8))) = {              \
            .file = __FILE__,                                                  \
            .flags = MINILOG_DEBUG_FLAG_DEFAULT,                                 \
    };                                                                         \
    if (__minilog_debug_desc.flags & MINILOG_DEBUG_FLAG_PRINT)                     \
      minilog_debug("%s:%s() " fmt, __FILE__, __FUNCTION__, __VA_ARGS__);              \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __MINILOG_LOG_H */
