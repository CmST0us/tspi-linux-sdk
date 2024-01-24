#!/bin/bash -e

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

export mpp_syslog_perror=1

KERNEL_VERSION=$(cat /proc/version)
if [[ $KERNEL_VERSION =~ "4.4" ]]; then
    echo 0x100 > /sys/module/rk_vcodec/parameters/debug
else
    echo 0x100 > /sys/module/rk_vcodec/parameters/mpp_dev_debug
fi

echo performance | tee $(find /sys/ -name *governor) /dev/null || true

touch /dev/video-dec0
ln -sf /usr/lib/libmali.so /usr/lib/libMali.so.1

if [ -e "/usr/bin/chromium" ] ;
then
	chromium --no-sandbox --gpu-sandbox-start-early --ozone-platform=wayland --ignore-gpu-blacklist --enable-wayland-ime "file:///oem/SampleVideo_1280x720_5mb.mp4"
else
	echo "Please sure the config/rockchip_xxxx_defconfig include "chromium.config"........"
fi
echo "the governor is performance for now, please restart it........"
