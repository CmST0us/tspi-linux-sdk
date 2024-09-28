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
echo "All Stop Application ..."
