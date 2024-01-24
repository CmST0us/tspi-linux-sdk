#!/bin/bash
#export GST_DEBUG=*:5
killall rkisp_demo
/rockchip-test/camera/rkisp_demo.sh
sleep 1
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/gstreamer-1.0
MEDIA_NUM=$(ls /dev/media* | wc -l)
echo "Start RKAIQ Camera Preview!"
COMPATIBLE=$(cat /proc/device-tree/compatible)
if [[ $COMPATIBLE =~ "rk3588" ]]; then
  if [[ $MEDIA_NUM == 2 ]];then
    gst-launch-1.0 v4l2src device=/dev/video12 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
  elif [[ $MEDIA_NUM == 4 ]];then
    gst-launch-1.0 v4l2src device=/dev/video23 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
    gst-launch-1.0 v4l2src device=/dev/video32 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
  elif [[ $MEDIA_NUM == 6 ]];then
	gst-launch-1.0 v4l2src device=/dev/video34 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
	gst-launch-1.0 v4l2src device=/dev/video43 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
	gst-launch-1.0 v4l2src device=/dev/video52 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
  elif [[ $MEDIA_NUM == 8 ]];then
	gst-launch-1.0 v4l2src device=/dev/video45 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
	gst-launch-1.0 v4l2src device=/dev/video54 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
	gst-launch-1.0 v4l2src device=/dev/video63 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
	gst-launch-1.0 v4l2src device=/dev/video72 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink &
  fi
else
    gst-launch-1.0 v4l2src device=/dev/video-camera0 ! video/x-raw,format=NV12,width=640,height=480, framerate=30/1 ! waylandsink
fi
COMPATIBLE=${COMPATIBLE#rockchip,}
