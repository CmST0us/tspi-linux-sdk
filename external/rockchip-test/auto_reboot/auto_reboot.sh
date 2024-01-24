#!/bin/bash

delay=8
total=${1:-10000}
CNT=/userdata/rockchip-test/reboot_cnt

if [ ! -e "/userdata//rockchip-test" ]; then
	echo "no /userdata/rockchip-test"
	mkdir -p /userdata/rockchip-test
fi

if [ ! -e "/userdata/rockchip-test/auto_reboot.sh" ]; then
	cp /rockchip-test/auto_reboot/auto_reboot.sh /userdata/rockchip-test
	echo $total > /userdata/rockchip-test/reboot_total_cnt
    sync
fi

while true
do

if [ -e $CNT ]
then
    cnt=`cat $CNT`
else
    echo reset Reboot count.
    echo 0 > $CNT
fi

echo  Reboot after $delay seconds.

let "cnt=$cnt+1"

if [ $cnt -ge $total ]
then
    echo AutoReboot Finisned.
    echo "off" > $CNT
    echo "do cleaning ..."
    rm -rf /userdata/rockchip-test/auto_reboot.sh
    rm -rf /userdata/rockchip-test/reboot_total_cnt
    rm -f $CNT
    sync
    exit 0
fi

echo $cnt > $CNT
echo "current cnt = $cnt, total cnt = $total"
echo "You can stop reboot by: echo off > /userdata/rockchip-test/reboot_cnt"
sleep $delay
cnt=`cat $CNT`
if [ $cnt != "off" ]; then
    sync
    if [ -e /sys/fs/pstore/console-ramoops-0 ]; then
        echo "check console-ramoops-o message"
        grep -q "Restarting system" /sys/fs/pstore/console-ramoops-0
        if [ $? -ne 0 -a $cnt -ge 2 ]; then
           echo "no found 'Restarting system' log in last time kernel message"
           echo "consider kernel crash in last time reboot test"
           echo "quit reboot test"
            rm -rf /userdata/rockchip-test/auto_reboot.sh
            rm -rf /userdata/rockchip-test/reboot_total_cnt
            sync
	   exit 1
        else
	   reboot
        fi
    else
	   reboot
    fi
else
    echo "Auto reboot is off"
    rm -rf /userdata/rockchip-test/auto_reboot.sh
    rm -rf /userdata/rockchip-test/reboot_total_cnt
    rm -f $CNT
    sync
fi
exit 0
done
