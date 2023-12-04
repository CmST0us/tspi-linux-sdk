#!/bin/bash
#export GST_DEBUG=*:5
export DISPLAY=:0.0
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/aarch64-linux-gnu/gstreamer-1.0
#export GST_DEBUG=ispsrc:5
#export GST_DEBUG_FILE=/tmp/2.txt
echo "Start RKISP Camera Preview!"
gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,format=NV12,width=1920,height=1080, framerate=30/1 ! xvimagesink
