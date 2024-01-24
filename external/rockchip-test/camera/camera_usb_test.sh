#!/bin/bash
#export GST_DEBUG=*:5
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/gstreamer-1.0

v4l2-ctl --list-devices > /tmp/.v4l2_list
USB_VIDEO=($(awk '/usb/{getline a;print a}' /tmp/.v4l2_list))
echo "Found ${#USB_VIDEO[@]} USB Cameras"
rm /tmp/.v4l2_list

for i in USB_VIDEO
do
	eval value=\${${i}[@]}
	for j in $value
	do
	echo "Start Preview USB Camera Video Path $j By GStreamer"
	gst-launch-1.0 v4l2src device="$j" ! image/jpeg! jpegparse ! mppjpegdec ! waylandsink sync=false
	done
done
