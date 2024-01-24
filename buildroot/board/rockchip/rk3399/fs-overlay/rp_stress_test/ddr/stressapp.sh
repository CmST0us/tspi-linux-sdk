#!/bin/bash

echo ============================================
echo "CPU stressapptest"
echo ============================================

RESULT_DIR=/var/rp_stress_test
RESULT_LOG=${RESULT_DIR}/stressapptest.log

mkdir -p ${RESULT_DIR}

#get free memory size
mem_avail_size=$(cat /proc/meminfo | grep MemAvailable | awk '{print $2}')
mem_test_size=$((mem_avail_size/1024/2))

#get cpu number
cpu_num=`nproc`
test_time=86400
cpu_temp_path=/stressapp_cpu_temp.txt
sleep_time=1800

echo "CPU core number: $cpu_num"
echo "DDR availabel size: $((mem_avail_size/1024))M"
echo "DDR test use: ${mem_test_size}M"
echo "Test time: $((test_time/60/60))H"
echo "Log save at: $RESULT_LOG"
echo "CPU temp save at: $cpu_temp_path"
echo "CPU temp get freq: ${sleep_time}s"

#run stressapptest_test
echo "*************************** DDR STRESSAPPTEST TEST 24H ***************************************"
echo "**run: stressapptest -s $test_time -i $cpu_num -C $cpu_num -W --stop_on_errors -M $mem_test_size -l $RESULT_LOG**"

stressapptest -s $86400 -i $cpu_num -C $cpu_num -W --stop_on_errors -M $mem_test_size -l $RESULT_LOG &

echo "************************** DDR STRESSAPPTEST START, LOG AT $RESULT_LOG ************************"

times=0
while [ 1 ]; do
    temp=`cat /sys/class/thermal/thermal_zone0/temp`
    echo "${times}: ${temp}" >> $cpu_temp_path
    sleep $sleep_time
done
