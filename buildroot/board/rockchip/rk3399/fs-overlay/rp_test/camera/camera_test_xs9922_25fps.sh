#!/bin/bash
#export GST_DEBUG=*:5
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/aarch64-linux-gnu/gstreamer-1.0
MEDIA_NUM=$(ls /dev/media* | wc -l)

echo "Start 9922 Camera Preview!"
COMPATIBLE=$(cat /proc/device-tree/compatible)
if [[ $COMPATIBLE =~ "rk3588" ]]; then
	#systemctl start rkisp_demo.service
  if [[ $MEDIA_NUM == 1 ]]||[[ $MEDIA_NUM == 2 ]];then
	gst-launch-1.0 v4l2src device=/dev/video0  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video1  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink & 
	gst-launch-1.0 v4l2src device=/dev/video2  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink & 
	gst-launch-1.0 v4l2src device=/dev/video3  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &	
    
	#gst-launch-1.0 v4l2src device=/dev/video0  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert ！xvimagesink &
	#gst-launch-1.0 v4l2src device=/dev/video1  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert ！xvimagesink &
	#gst-launch-1.0 v4l2src device=/dev/video2  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert ！xvimagesink &
	#gst-launch-1.0 v4l2src device=/dev/video3  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert ！xvimagesink &
  elif [[ $MEDIA_NUM == 3 ]];then
	gst-launch-1.0 v4l2src device=/dev/video0  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video1  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink & 
	gst-launch-1.0 v4l2src device=/dev/video2  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink & 
	gst-launch-1.0 v4l2src device=/dev/video3  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video11  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video12  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video13  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video14  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
  fi
elif [[ $COMPATIBLE =~ "rk3568" ]]; then
	if [[ $MEDIA_NUM == 1 ]];then
	gst-launch-1.0 v4l2src device=/dev/video0  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	gst-launch-1.0 v4l2src device=/dev/video1  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink & 
	gst-launch-1.0 v4l2src device=/dev/video2  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink & 
	gst-launch-1.0 v4l2src device=/dev/video3  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink &
	fi
else
	gst-launch-1.0 v4l2src device=/dev/video-camera0  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert ！xvimagesink
    #gst-launch-1.0 v4l2src device=/dev/video-camera0 ! video/x-raw,format=NV12,width=1920,height=1080, framerate=30/1 ! xvimagesink
fi
COMPATIBLE=${COMPATIBLE#rockchip,}
