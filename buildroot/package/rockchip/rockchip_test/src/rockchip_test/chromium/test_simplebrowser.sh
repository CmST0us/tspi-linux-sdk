#!/bin/bash -e

echo performance | tee $(find /sys/ -name *governor)

if [ -e "/usr/lib/qt/examples/webenginewidgets/simplebrowser" ] ;
then
	cd /usr/lib/qt/examples/webenginewidgets/simplebrowser
	./simplebrowser
	#./simplebrowser https://www.baidu.com
	#./simplebrowser "file:///oem/SampleVideo_1280x720_5mb.mp4"
else
	echo "Please sure the config/rockchip_xxxx_defconfig include "chromium.config"........"
fi
echo "the governor is performance for now, please restart it........"
