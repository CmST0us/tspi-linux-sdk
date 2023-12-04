#!/bin/bash
#export GST_DEBUG=*:5
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/aarch64-linux-gnu/gstreamer-1.0

COMPATIBLE=$(cat /proc/device-tree/compatible)
if [[ $COMPATIBLE =~ "rk3588" ]]; then
    gst-launch-1.0 v4l2src device=/dev/video8 ! video/x-raw,format=NV12,width=1920,height=1080, framerate=30/1 ! xvimagesink
else
    gst-launch-1.0 v4l2src device=/dev/video1 ! video/x-raw,format=NV12,width=1920,height=1080, framerate=30/1 ! xvimagesink
fi
COMPATIBLE=${COMPATIBLE#rockchip,}

echo "Start RKAIQ Camera Preview!"
gst-launch-1.0 v4l2src device=/dev/video1 ! video/x-raw,format=NV12,width=1920,height=1080, framerate=30/1 ! xvimagesink
#gst-launch-1.0 v4l2src device=/dev/video1 ! video/x-raw, format=NV12, width=640, height=480, framerate=30/1 ! kmssink
