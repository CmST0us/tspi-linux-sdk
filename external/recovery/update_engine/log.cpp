// Copyright 2013 Google Inc. All Rights Reserved.
//
// Log - implemented using the standard Android logging mechanism

/*
 * Qutoing from system/core/include/log/log.h:
 * Normally we strip ALOGV (VERBOSE messages) from release builds.
 * You can modify this (for example with "#define LOG_NDEBUG 0"
 * at the top of your source file) to change that behavior.
 */
#ifndef LOG_NDEBUG
#ifdef NDEBUG
#define LOG_NDEBUG 1
#else
#define LOG_NDEBUG 0
#endif
#endif

#define LOG_BUF_SIZE 1024

#include "log.h"
#include <stdio.h>
#include <stdarg.h>


void InitLogging(int argc, const char* const* argv) {}

void Log(const char* file, int line, LogPriority level, const char* fmt, ...) {
  va_list ap;
  char buf[LOG_BUF_SIZE];
  va_start(ap, fmt);
  vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
  va_end(ap);

  switch(level) {
    case LOG_ERROR: printf("LOG_ERROR : %s\n", buf); break;
    case LOG_WARN: printf("LOG_WARN : %s\n", buf); break;
    case LOG_INFO: printf("LOG_INFO :%s \n", buf); break;
    case LOG_DEBUG:
      //#if LOG_NDEBUG
      printf("LOG_DEBUG :%s \n", buf);
      //#endif
      break;
    default :
      break;
  }
}
