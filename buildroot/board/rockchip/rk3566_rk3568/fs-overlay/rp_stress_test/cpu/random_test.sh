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
sleep_time=1800

echo "CPU core number: $cpu_num"
echo "DDR availabel size: $((mem_avail_size/1024))M"
echo "DDR test use: ${mem_test_size}M"
echo "Log save at: $RESULT_LOG"

while [ true ]; do
	exec_time=$(((RANDOM % 10 + 1)*60))
	sleep_time=$(((RANDOM % 10 + 1)*60))
	#run stressapptest_test
	echo "*************************** DDR STRESSAPPTEST TEST 24H ***************************************"
	echo "**run: stressapptest -s $exec_time -i $cpu_num -C $cpu_num -W --stop_on_errors -M $mem_test_size -l $RESULT_LOG**"
	
	stressapptest -s $exec_time -i $cpu_num -C $cpu_num -W --stop_on_errors -M $mem_test_size -l $RESULT_LOG 
	
	echo "************************** DDR STRESSAPPTEST START, LOG AT $RESULT_LOG ************************"
	sleep $sleep_time
done
