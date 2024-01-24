#!/bin/bash

echo ================================
echo CPU stress test
echo ================================

CPU_NUM=`nproc`
sleep_time=1800
cpu_temp_path="/stress_cpu_temp.txt"

killall stress > /dev/null 2>&1


echo "CPU core number: $CPU_NUM"
echo "Save CPU temp every ${sleep_time}s in ${cpu_temp_path}"
echo "" > $cpu_temp_path

echo "stress -c $CPU_NUM &"
stress -c $CPU_NUM &

times=0
while [ 1 ]; do
    temp=`cat /sys/class/thermal/thermal_zone0/temp`
    echo "${times}: ${temp}" >> $cpu_temp_path
    sleep $sleep_time
done
