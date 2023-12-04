#!/bin/sh
ulimit -c unlimited
echo "/data/core-%p-%e" > /proc/sys/kernel/core_pattern
