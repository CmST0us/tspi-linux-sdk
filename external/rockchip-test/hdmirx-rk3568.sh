
media-ctl -d /dev/media0 --set-v4l2 '"rkisp-isp-subdev":0[fmt:UYVY2X8/1920x1080]'&&media-ctl -d /dev/media0 --set-v4l2 '"rkisp-isp-subdev":0[crop:(0,0)/1920x1080]' && gst-launch-1.0 v4l2src device=/dev/video0  !  video/x-raw,width=1920,height=1080,framerate=25/1  ! videoconvert !  autovideosink

