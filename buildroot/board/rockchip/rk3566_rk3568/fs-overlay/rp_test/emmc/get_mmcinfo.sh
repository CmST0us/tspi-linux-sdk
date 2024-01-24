#!/bin/bash

cd /sys/class/mmc_host/

for mmc in `ls -d mmc*`; do

    [ ! -d ${mmc}/$mmc\:0001 ] && continue
    mmc_type=`cat $mmc/$mmc\:0001/type` 
    if [ "$mmc_type" == "MMC" ];then
        echo =============================
        echo EMMC info
        echo =============================
        cat /sys/kernel/debug/$mmc/ios
        
        echo 
        echo =============================
        echo EMMC write/read 1G data rate
        echo =============================
        
        echo Write rate:
        cmd="dd if=/dev/zero of=/test.out bs=512k count=2000"
        echo $cmd
        $cmd
        
        echo Clean cache for test read
        dmesg -n 1
        sync && echo 3 | tee /proc/sys/vm/drop_caches
        dmesg -n 8
        
        echo Read rate:
        cmd="dd if=/test.out of=/dev/zero bs=512k count=2000"
        echo $cmd
        $cmd
        
        rm -rf /test.out
    fi

done
