#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/gstreamer-1.0
gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080 ! videoconvert ! video/x-raw,format=RGB ! autovideosink
MEDIA_NUM=$(ls /dev/media* | wc -l)

COMPATIBLE=$(cat /proc/device-tree/compatible)
if [[ $COMPATIBLE =~ "rk3588" ]]; then
  if [[ $MEDIA_NUM == 2 ]];then
gst-launch-1.0 v4l2src device=/dev/video20 ! video/x-raw,width=1920,height=1080 ! videoconvert ! video/x-raw,format=RGB ! autovideosink
  elif [[ $MEDIA_NUM == 4 ]];then
gst-launch-1.0 v4l2src device=/dev/video40 ! video/x-raw,width=1920,height=1080 ! videoconvert ! video/x-raw,format=RGB ! autovideosink 
  elif [[ $MEDIA_NUM == 6 ]];then
gst-launch-1.0 v4l2src device=/dev/video60 ! video/x-raw,width=1920,height=1080 ! videoconvert ! video/x-raw,format=RGB ! autovideosink
  fi
else
gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1920,height=1080 ! videoconvert ! video/x-raw,format=RGB ! autovideosink
fi
COMPATIBLE=${COMPATIBLE#rockchip,}

