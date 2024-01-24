#!/bin/bash

cd /sys/devices/platform/

for gpu in `ls -d *gpu*`; do
    echo $gpu
    if [ -e $gpu/gpuinfo ];then
        echo =============================
        echo GPU info
        echo ============================
        echo GPU model: `cat $gpu/gpuinfo`
        echo GPU load:  `cat $gpu/utilisation`%
    fi
done
