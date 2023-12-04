// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "minilogger/log.h"
#include "minilogger/backtrace.h"

#include <dlfcn.h>
#include <glib.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

typedef void (*sighandler_t)(int);

static const char *program_exec;
static const char *program_path;

/* This makes sure we always have a __debug section. */
MINILOG_DEBUG_DEFINE(dummy)

void minilog_info(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsyslog(LOG_INFO, format, ap);
  va_end(ap);
}

void minilog_warn(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsyslog(LOG_WARNING, format, ap);
  va_end(ap);
}

void minilog_error(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsyslog(LOG_ERR, format, ap);
  va_end(ap);
}

void minilog_debug(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  vsyslog(LOG_DEBUG, format, ap);
  va_end(ap);
}

static void signal_handler(int signo) {
  minilog_error("Aborting (signal %d) [%s]", signo, program_exec);
  show_backtrace();
  exit(EXIT_FAILURE);
}

static void signal_setup(sighandler_t handler) {
  struct sigaction sa;
  sigset_t mask;

  sigemptyset(&mask);
  sa.sa_handler = handler;
  sa.sa_mask = mask;
  sa.sa_flags = 0;
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGILL, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGPIPE, &sa, NULL);
}

#if !__APPLE__
extern struct minilog_debug_desc __start___debug[];
extern struct minilog_debug_desc __stop___debug[];
#endif

static char **enabled = NULL;

static bool is_enabled(struct minilog_debug_desc *desc) {
  int i;

  if (!enabled)
    return false;

  for (i = 0; enabled[i]; i++) {
    if (desc->name && g_pattern_match_simple(enabled[i], desc->name))
      return true;
    if (desc->file && g_pattern_match_simple(enabled[i], desc->file))
      return true;
  }

  return false;
}

void __minilog_log_enable(struct minilog_debug_desc *start,
                        struct minilog_debug_desc *stop) {
  struct minilog_debug_desc *desc;
  const char *name = NULL, *file = NULL;

  if (!start || !stop)
    return;

  for (desc = start; desc < stop; desc++) {
    if (desc->flags & MINILOG_DEBUG_FLAG_ALIAS) {
      file = desc->file;
      name = desc->name;
      continue;
    }

    if (file || name) {
      if (strcmp(desc->file, file) == 0) {
        if (!desc->name)
          desc->name = name;
      } else
        file = NULL;
    }

    if (is_enabled(desc))
      desc->flags |= MINILOG_DEBUG_FLAG_PRINT;
  }
}

int __minilog_log_init(const char *program, const char *debug, bool detach,
                     bool backtrace, const char *program_name,
                     const char *program_version) {
  static char path[PATH_MAX];
  int option = LOG_NDELAY | LOG_PID;

  program_exec = program;
  program_path = getcwd(path, sizeof(path));

  if (debug)
    enabled = g_strsplit_set(debug, ":, ", 0);

#if !__APPLE__
  __minilog_log_enable(__start___debug, __stop___debug);
#endif

  if (!detach)
    option |= LOG_PERROR;

  if (backtrace)
    signal_setup(signal_handler);

#if !__APPLE__
  openlog(basename(program), option, LOG_DAEMON);
#else
  openlog(g_path_get_basename(program), option, LOG_DAEMON);
#endif

  syslog(LOG_INFO, "%s version %s", program_name, program_version);

  return 0;
}

void __minilog_log_cleanup(bool backtrace) {
  syslog(LOG_INFO, "Exit");

  closelog();

  if (backtrace)
    signal_setup(SIG_DFL);

  g_strfreev(enabled);
}
