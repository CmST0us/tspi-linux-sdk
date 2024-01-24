#!/bin/bash -e

export mpp_syslog_perror=1

KERNEL_VERSION=$(cat /proc/version)
if [[ $KERNEL_VERSION =~ "4.4" ]]; then
    echo 0x100 > /sys/module/rk_vcodec/parameters/debug
else
    echo 0x100 > /sys/module/rk_vcodec/parameters/mpp_dev_debug
fi

echo performance | tee $(find /sys/ -name *governor)

if [ -e "/usr/lib/qt/examples/webenginewidgets/simplebrowser" ] ;
then
	cd /usr/lib/qt/examples/webenginewidgets/simplebrowser
	./simplebrowser
	#./simplebrowser https://www.baidu.com
	#./simplebrowser "file:///oem/SampleVideo_1280x720_5mb.mp4"
else
	echo "Please sure the config/rockchip_xxx_defconfig include "qtwebengine.config"........"
fi
echo "the governor is performance for now, please restart it........"
