#!/bin/sh
#
killall -9 aicamera.sh
killall smart_display_service
ifconfig usb0 down
killall dbserver
killall aiserver
#sleep for aiserver deint over
sleep .6
killall uvc_app
killall ispserver
killall uac_app
sleep .1
while [[ true ]]; do
	PID=`busybox ps |grep uvc_app |grep -v grep | wc -l`
	if [ $PID -le 0 ]; then
		echo "uvc_app die, wake..."
		echo none > /sys/kernel/config/usb_gadget/rockchip/UDC
		rmdir /sys/kernel/config/usb_gadget/rockchip/functions/rndis.gs0
		break
	fi
done
killall -9 aiserver
echo "All Stop Application to suspend now..."
