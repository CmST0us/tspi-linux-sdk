#!/bin/bash

MEM_SIZE=`free -m |grep "^Mem" | awk '{print $2}'`

echo ==============================
echo DDR info
echo =============================

# < 1G
if [ $MEM_SIZE -lt 512 ];then
    echo "DDR size: 128M"
#1G
elif [ $MEM_SIZE -lt 1024 ];then
    echo "DDR size: 1G"
#2G
elif [ $MEM_SIZE -lt 2048 ];then
    echo "DDR size: 2G"
#3G
elif [ $MEM_SIZE -lt 3072 ];then
    echo "DDR size: 3G"
#4G
elif [ $MEM_SIZE -lt 4096 ];then
    echo "DDR size: 4G"
#8G
elif [ $MEM_SIZE -lt 8192 ];then
    echo "DDR size: 8G"
#16G
elif [ $MEM_SIZE -lt 16384 ];then
    echo "DDR size: 16G"
# > 16G
else
    echo "Invalid memory size!!!!"
fi

if [ -e /sys/kernel/debug/clk/scmi_clk_ddr/clk_rate ]; then
    freq=`cat /sys/kernel/debug/clk/scmi_clk_ddr/clk_rate`
elif [ -e /sys/kernel/debug/clk/clk_scmi_ddr/clk_rate ]; then
    freq=`cat /sys/kernel/debug/clk/clk_scmi_ddr/clk_rate`
fi
echo DDR freq: $freq
