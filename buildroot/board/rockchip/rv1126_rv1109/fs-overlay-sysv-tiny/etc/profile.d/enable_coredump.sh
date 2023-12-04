#!/bin/sh
ulimit -c unlimited
echo "/tmp/core-%p-%e" > /proc/sys/kernel/core_pattern
