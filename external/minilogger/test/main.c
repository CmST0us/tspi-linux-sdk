// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <minilogger/backtrace.h>
#include <minilogger/log.h>

void crash() {
  volatile int i = *(int *)7;
  (void)i;
}

void foo2(void) {
  crash();
}

void foo1(void) {
  foo2();
}

int main(int argc, char **argv) {

  __minilog_log_init(argv[0], NULL, false, true, "trace_c", "1.1");

  foo1();

  return 0;
}
