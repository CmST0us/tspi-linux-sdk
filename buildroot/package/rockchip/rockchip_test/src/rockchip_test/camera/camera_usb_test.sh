#!/bin/bash
#export GST_DEBUG=*:5
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/gstreamer-1.0
gst-launch-1.0 v4l2src device=/dev/video10 ! image/jpeg, width=1280, height=720, framerate=30/1 ! jpegparse ! mppjpegdec ! kmssink sync=false
