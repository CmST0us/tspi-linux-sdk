#!/bin/bash

cd /sys/class/mmc_host/
detect_flag=0

for mmc in `ls -d mmc*`; do

    [ ! -d ${mmc}/$mmc\:* ] && continue
    mmc_type=`cat $mmc/$mmc\:*/type` 
    if [ "$mmc_type" == "SD" ];then
        detect_flag=1
        echo "Detect SD card!!"
        echo =============================
        echo SD card info
        echo =============================
        cat /sys/kernel/debug/$mmc/ios
        
        #get sdcard mmc channl, and get mount info, such as: df -h | hrep mmcblk1
        mountinfo=`df -h |grep mmcblk${mmc: -1}`
        if [ "$mountinfo" == "" ];then
	    echo "SD card not auto mount! manual mounting..."

            mountpoint=/mnt/sdcard/
            [ ! -d $mountpint ] && mkdir $mountpoint
            if [ -e /dev/mmcblk${mmc: -1}p1 ]; then 
                mount /dev/mmcblk${mmc: -1}p1 $mountpoint
            elif [ -e /dev/mmcblk${mmc: -1} ];then
                mount /dev/mmcblk${mmc: -1}p1 $mountpoint
            else
                echo "SD card not found available partiniton! exit!!"
            fi
            df -h
        else
            mountpoint=$(echo $mountinfo | awk '{print $6}')
            echo "SD card mount point: " $mountpoint
        fi
        
        echo 
        echo =============================
        echo SD card write/read 1G data rate
        echo =============================
        echo Write rate:
        cmd="dd if=/dev/zero of=$mountpoint/test.out bs=512k count=2000"
        echo $cmd
        $cmd
        
        echo Clean cache for test read
        
        dmesg -n 1
        sync && echo 3 | tee /proc/sys/vm/drop_caches
        dmesg -n 8
        
        echo Read rate:
        cmd="dd if=$mountpoint/test.out of=/dev/zero bs=512k count=2000"
        echo $cmd
        $cmd
        
        rm -rf $mountpoint/test.out
    fi
done

[ "$detect_flag" == "0" ] && echo "SD card not found!"
