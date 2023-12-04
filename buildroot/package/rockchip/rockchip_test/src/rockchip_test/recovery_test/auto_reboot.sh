#!/bin/sh

delay=10
total=30000
CNT=/oem/rockchip_test/reboot_cnt

if [ ! -e "/oem/rockchip_test" ]; then
	echo "no /oem/rockchip_test"
	mkdir /oem/rockchip_test
fi

if [ ! -e "/oem/rockchip_test/auto_reboot.sh" ]; then
	cp /rockchip_test/recovery_test/auto_reboot.sh /oem/rockchip_test
fi

if [ -e "/oem/rockchip_test/auto_reboot.sh" ]; then
	cp /oem/rockchip_test/auto_reboot.sh /data/cfg/rockchip_test
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
    rm -rf /data/cfg/rockchip_test/auto_reboot.sh
	rm -rf /oem/rockchip_test/auto_reboot.sh
	rm -f $CNT
    exit 0
fi

echo $cnt > $CNT
echo "current cnt = $cnt"
echo "You can stop reboot by: echo off > /oem/rockchip_test/reboot_cnt"
sleep $delay
cnt=`cat $CNT`
if [ $cnt != "off" ]; then
	update
	reboot
else
	echo "Auto reboot is off"
	rm -rf /data/cfg/rockchip_test/auto_reboot.sh
	rm -rf /oem/rockchip_test/auto_reboot.sh
	rm -f $CNT
fi
exit 0
done
