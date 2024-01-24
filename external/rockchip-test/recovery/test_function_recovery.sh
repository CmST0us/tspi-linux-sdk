#!/bin/bash

delay=10
total=${1:-10000}
CNT=/userdata/rockchip-test/reboot_cnt

if [ ! -e "/usr/bin/update" ]; then
	echo "Please check if have rktoolkit built\n"
fi

# check log on /userdata/recovery/last_log
/usr/bin/update
sync

