#!/bin/sh
#

check_linker()
{
	[ ! -L "$2" ] && ln -sf $1 $2
}

check_linker /userdata   /usr/www/userdata
check_linker /media/usb0 /usr/www/usb0
check_linker /mnt/sdcard /usr/www/sdcard

#set max socket buffer size to 1.5MByte
sysctl -w net.core.wmem_max=1572864

export enable_encoder_debug=0

# ispp using fbc420 mode to save ddr bandwidth
echo 1 > /sys/module/video_rkispp/parameters/mode

#vpu 600M, kernel default 600M
#echo 600000000 >/sys/kernel/debug/mpp_service/rkvenc/clk_core

ipc-daemon --no-mediaserver &
#ispserver -no-sync-db &

echo 77 > /sys/devices/platform/pwmleds/leds/PWM-IR/brightness

#export QT_QPA_FB_DRM=1
#export QT_QPA_PLATFORM=linuxfb:rotation=0
#QFacialGate -f 10000 &
