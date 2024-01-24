#!/bin/bash

current_mode=`cat /sys/kernel/debug/usb/ci_hdrc.0/role`

if [ $current_mode == "gadget" ];then
	echo "----------------change to host mode-------------------------"
	#host
	echo host > /sys/kernel/debug/usb/ci_hdrc.0/role
else
	echo "----------------change to device mode-------------------------"
	#device
	echo gadget > /sys/kernel/debug/usb/ci_hdrc.0/role
fi
