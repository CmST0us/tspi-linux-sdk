#!/bin/bash -e

export DISPLAY=:0.0
#export GST_DEBUG=*:5
#export GST_DEBUG_FILE=/tmp/2.txt

KERNEL_VERSION=$(cat /proc/version)
if [[ $KERNEL_VERSION =~ "4.4" ]]; then
    echo 0x100 > /sys/module/rk_vcodec/parameters/debug
else
    echo 0x100 > /sys/module/rk_vcodec/parameters/mpp_dev_debug
fi

echo performance | tee $(find /sys/ -name *governor) /dev/null || true
chromium --no-sandbox file:///usr/local/test.mp4

echo "the governor is performance for now, please restart it........"
