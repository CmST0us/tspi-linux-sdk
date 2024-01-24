#!/bin/bash

delay=30
total=${1:-10000}
data_path=/userdata/rp_stress_test
CNT=${data_path}/reboot_cnt

if [ ! -e "$data_path" ]; then
	echo "no $data_path"
	mkdir -p $data_path
fi

if [ ! -e "${data_path}/auto_reboot.sh" ]; then
	cp /rp_stress_test/reboot/auto_reboot.sh $data_path
	echo $total > ${data_path}/reboot_total_cnt
    sync
fi

cp /rp_stress_test/reboot/S99-auto-reboot /etc/init.d/

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
    rm -rf ${data_path}/auto_reboot.sh
    rm -rf ${data_path}/reboot_total_cnt
    rm -f $CNT
    sync
    exit 0
fi

echo $cnt > $CNT
echo "current cnt = $cnt, total cnt = $total"
echo "You can stop reboot by: touch ${data_path}/stop"
sleep $delay
cnt=`cat $CNT`
if [ -e ${data_path}/stop ]; then
    sync
    if [ -e /sys/fs/pstore/console-ramoops-0 ]; then
        echo "check console-ramoops-o message"
        grep -q "Restarting system" /sys/fs/pstore/console-ramoops-0
        if [ $? -ne 0 -a $cnt -ge 2 ]; then
           echo "no found 'Restarting system' log in last time kernel message"
           echo "consider kernel crash in last time reboot test"
           echo "quit reboot test"
           rm -rf ${data_path}/auto_reboot.sh
           #rm -rf ${data_path}/reboot_total_cnt
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
    rm -rf ${data_path}/auto_reboot.sh
    #rm -rf ${data_path}/reboot_total_cnt
    #rm -f $CNT
    cat $CNT
    sync
fi
exit 0
done
