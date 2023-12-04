#!/bin/sh
#
# setup configfs for adbd, usb mass storage and MTP....
# For kernel v4.4 usb configfs
#

UAC=uac2

USB_ATTRIBUTE=0x409
USB_GROUP=rockchip
USB_SKELETON=b.1

CONFIGFS_DIR=/sys/kernel/config
USB_CONFIGFS_DIR=${CONFIGFS_DIR}/usb_gadget/${USB_GROUP}
USB_STRINGS_DIR=${USB_CONFIGFS_DIR}/strings/${USB_ATTRIBUTE}
USB_FUNCTIONS_DIR=${USB_CONFIGFS_DIR}/functions
USB_CONFIGS_DIR=${USB_CONFIGFS_DIR}/configs/${USB_SKELETON}

function_init()
{
	mkdir ${USB_FUNCTIONS_DIR}/${UAC}.gs0
}

configfs_init()
{
	echo "Debug: configfs_init"
	mkdir /dev/usb-ffs

	mount -t configfs none ${CONFIGFS_DIR}
	mkdir ${USB_CONFIGFS_DIR} -m 0770
	echo 0x2207 > ${USB_CONFIGFS_DIR}/idVendor
	echo 0x0310 > ${USB_CONFIGFS_DIR}/bcdDevice
	echo 0x0200 > ${USB_CONFIGFS_DIR}/bcdUSB
	mkdir ${USB_STRINGS_DIR}   -m 0770
	SERIAL=`cat /proc/cpuinfo | grep Serial | awk '{print $3}'`
	if [ -z $SERIAL ];then
		SERIAL=0123456789ABCDEF
	fi
	echo $SERIAL > ${USB_STRINGS_DIR}/serialnumber
	echo "rockchip"  > ${USB_STRINGS_DIR}/manufacturer
	echo "rv1109"  > ${USB_STRINGS_DIR}/product

	function_init

	mkdir ${USB_CONFIGS_DIR}  -m 0770
	mkdir ${USB_CONFIGS_DIR}/strings/${USB_ATTRIBUTE}  -m 0770
}

parameter_init()
{
	if [ "$UAC" == "uac2" ]; then
		CONFIG_STRING=uac2
		echo "parameter_init ${CONFIG_STRING}"
	elif [ "$UAC" == "uac1" ];then
		CONFIG_STRING=uac1
		echo "parameter_init ${CONFIG_STRING}"
	else
		echo "parameter_init ${UAC} is invalid"
	fi
}

config_init()
{
	UAC_GS0=${USB_FUNCTIONS_DIR}/${UAC}.gs0
	echo 3 > ${UAC_GS0}/p_chmask
	echo 2 > ${UAC_GS0}/p_ssize
	echo 8000,16000,44100,48000 > ${UAC_GS0}/p_srate
        echo 1 > ${UAC_GS0}/p_feature_unit

	echo 3 > ${UAC_GS0}/c_chmask
	echo 2 > ${UAC_GS0}/c_ssize
	echo 8000,16000,44100,48000 > ${UAC_GS0}/c_srate
	echo 1 > ${UAC_GS0}/c_feature_unit
}

syslink_function()
{
	ln -s ${USB_FUNCTIONS_DIR}/$1 ${USB_CONFIGS_DIR}/f${USB_FUNCTIONS_CNT}
	let USB_FUNCTIONS_CNT=USB_FUNCTIONS_CNT+1
}

bind_functions()
{
	USB_FUNCTIONS_CNT=1

	if [ "$UAC" == "uac2" ]; then
		syslink_function uac2.gs0
	elif [ "$UAC" == "uac1" ];then
		syslink_function uac1.gs0
	else
		echo "parameter_init ${UAC} is invalid"
	fi

	echo ${CONFIG_STRING} > ${USB_CONFIGS_DIR}/strings/${USB_ATTRIBUTE}/configuration
}

program_kill()
{
	P_PID=`ps | grep $1 | grep -v grep | awk '{print $1}'`
	test -z ${P_PID} || kill -9 ${P_PID}
}

usb_device_stop()
{
	echo "none" > ${USB_CONFIGFS_DIR}/UDC
	program_kill adbd
	program_kill mtp-server
	ls ${USB_CONFIGS_DIR} | grep f[0-9] | xargs -I {} rm ${USB_CONFIGS_DIR}/{}
}

case "$1" in
start)
    echo "usb_config in $1"
	DIR=$(cd `dirname $0`; pwd)
	parameter_init
	if [ -z $CONFIG_STRING ]; then
		echo "$0: no function be selected"
		exit 0
	fi
	test -d ${USB_CONFIGFS_DIR} || configfs_init
	echo 0x0019 > ${USB_CONFIGFS_DIR}/idProduct
	config_init
	bind_functions
	sleep 1
	UDC=`ls /sys/class/udc/| awk '{print $1}'`
	echo $UDC > ${USB_CONFIGFS_DIR}/UDC
	;;
stop)
	usb_device_stop
	;;
restart|reload)
	# Do restart usb by udev
	echo "USB_FORCE_CHANGED" >> /tmp/.usb_config
	usb_device_stop
	sleep 1
	$0 start
	# Don't forget to clear "USB_FORCE_CHANGED"
	sed -i "/USB_FORCE_CHANGED/d" /tmp/.usb_config
	;;
*)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

exit 0
