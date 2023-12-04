// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "globle.h"

#include <stdarg.h>
#include <stdio.h>

#ifndef NDEBUG
static void LogPrintf(const char *prefix, const char *fmt, va_list vl) {
  char line[1024];
  fprintf(stderr, "%s", (char *)prefix);
  vsnprintf(line, sizeof(line), fmt, vl);
  fprintf(stderr, "%s", line);
}

void LOGD(const char *format, ...) {
  va_list vl;
  va_start(vl, format);
  LogPrintf("Debug - ", format, vl);
  va_end(vl);
}
#endif // #ifndef NDEBUG
