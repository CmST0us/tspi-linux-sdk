#!/bin/bash

delay=10
total=${1:-10000}
fudev=/dev/sda
CNT=/userdata/cfg/rockchip-test/reboot_cnt

if [ ! -e "/usr/bin/update" ]; then
	echo "Please check if have rktoolkit built\n"
fi

# check log on /userdata/recovery/last_log
# Reboots the device and wipes the user data partition.  This is
# sometimes called a "factory reset", which is something of a
# misnomer because the system partition is not restored to its
# factory state.

/usr/bin/update
#/usr/bin/update factory
#/usr/bin/update ota xxx

sync

