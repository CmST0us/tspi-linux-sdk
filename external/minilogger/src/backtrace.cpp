// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "minilogger/backtrace.h"
#include "minilogger/log.h"

#ifdef __cplusplus
#include <cxxabi.h>
#endif
#include <dlfcn.h>
#include <glib.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef USE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <ucontext.h>
#else
#include <libgen.h>
#include <execinfo.h>
#endif

void show_backtrace(void) {
#ifdef USE_LIBUNWIND
  unw_cursor_t cursor;
  unw_context_t uc;
  unw_word_t ip, sp;
  char func_name_cache[4096];
  func_name_cache[sizeof(func_name_cache) - 1] = 0;
  unw_word_t unw_offset;
  unw_proc_info_t unw_proc;
  int frame_id = 0;
#ifdef __cplusplus
  int status;
#endif

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
  minilog_error("++++++++ backtrace ++++++++");
  while (unw_step(&cursor) > 0) {
    unw_get_reg(&cursor, UNW_REG_IP, &ip);
    unw_get_reg(&cursor, UNW_REG_SP, &sp);

    unw_get_proc_info(&cursor, &unw_proc);
    unw_get_proc_name(&cursor, func_name_cache, sizeof(func_name_cache) - 1,
                      &unw_offset);
#ifdef __cplusplus
    char *func_name = abi::__cxa_demangle(func_name_cache, 0, 0, &status);
#else
    const char *func_name = func_name_cache;
#endif
    minilog_error("Frame #%02d: (%s+0x%llx) [0x%llx]\r\n", frame_id,
                func_name ? func_name : func_name_cache,
                static_cast<unsigned long long>(unw_offset),
                static_cast<unsigned long long>(unw_proc.start_ip));
#ifdef __cplusplus
    if (func_name)
      free((void *)func_name);
#endif
    frame_id++;
  }
  minilog_error("+++++++++++++++++++++++++++");
#else
#define SIZE 100
  int nptrs;
  void *buffer[SIZE];
  char **strings;

  nptrs = backtrace(buffer, SIZE);
  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL) {
    perror("backtrace_symbols");
    exit(EXIT_FAILURE);
  }

  for (int j = 0; j < nptrs; j++) {
    minilog_error("%s\n", strings[j]);
  }

  free(strings);
#endif
}
