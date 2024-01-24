#!/bin/sh

/bin/mount -t devtmpfs devtmpfs /dev
/bin/mount -t proc proc /proc
/bin/mount -t sysfs sysfs /sys
/bin/mount -t tmpfs tmpfs /tmp

mkdir -p /dev/shm
mkdir -p /dev/pts

/bin/mount -t tmpfs tmpfs /dev/shm
/bin/mount -t devpts devpts /dev/pts
