#!/bin/bash
cat /userdata/cfg/rockchip/reboot_cnt
test_dir=/userdata/cfg/rockchip/power_lost
CNT=/userdata/cfg/rockchip/reboot_cnt
total=5000
delay=3
no=0

if [ ! -e "/userdata/cfg/rockchip" ]; then
	echo "no /userdata/cfg/rockchip"
	mkdir -p /userdata/cfg/rockchip
fi

if [ ! -e "/userdata/cfg/rockchip/power_lost_test.sh" ]; then
	cp /rockchip/flash_test/power_lost_test.sh /userdata/cfg/rockchip/
fi

if [ -e $CNT ]
then
    cnt=`cat $CNT`
	echo -e $(date)_power_up_$cnt >> /userdata/cfg/rockchip/power_lost_test.log
else
    echo reset power lost count.
    cnt=0
    echo 0 > $CNT
	rm /userdata/cfg/rockchip/power_lost_test.log
	echo "$(date) power lost test begin" >> /userdata/cfg/rockchip/power_lost_test.log
	mkdir -p $test_dir
fi

if [ $cnt -ge $total ]
then
	echo power loat test finished!!!!!!!.
	echo "off" > $CNT
	echo "do cleaning ..."
	rm /userdata/cfg/rockchip/power_lost_test.sh
	rm -rf /userdata/cfg/rockchip/power_lost
	rm -f $CNT
	exit 0
fi

echo "current cnt = $cnt, total cnt = $total"
echo "You can stop reboot by: echo off > /userdata/cfg/rockchip/reboot_cnt"
echo "power lost test loop $cnt" >> /userdata/cfg/rockchip/power_lost_test.log
sleep $delay
if [ $cnt != "off" ]; then
	echo "$cnt begin dd"
else
	echo "power lost test is off"
	rm /userdata/cfg/rockchip/power_lost_test.sh
	rm -rf /userdata/cfg/rockchip/power_lost
	rm -f $CNT
	exit 0
fi

let "cnt=$cnt+1"
echo $cnt > $CNT

rm -rf $test_dir/*
sync
time dd if=/dev/random of=$test_dir/test_src bs=1M count=2
while true; do
	echo "power lost test, loop $cnt"
	time dd if=$test_dir/test_src of=$test_dir/test_dst
	sync
	busybox diff $test_dir/test_src $test_dir/test_dst
	echo $?
	if [ $? -eq 0 ];then
		echo "compare equally"
		rm $test_dir/test_dst
		sync
	else
		echo "compare not equally" >> /userdata/cfg/rockchip/power_lost_test.log
		exit
	fi
	sleep 0.1
done
