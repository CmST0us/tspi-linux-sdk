#!/bin/bash
#export GST_DEBUG=*:5
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/gstreamer-1.0

gst-launch-1.0 v4l2src device=/dev/video1 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink
#gst-launch-1.0 v4l2src device=/dev/video1 ! video/x-raw, format=NV12, width=640, height=480, framerate=30/1 ! kmssink
