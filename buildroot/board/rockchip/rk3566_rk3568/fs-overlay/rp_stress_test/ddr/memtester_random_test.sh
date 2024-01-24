#!/bin/bash

echo ================================
echo DDR test
echo ================================

CPU_NUM=`nproc`
MEM_SIZE=`free -m |grep "^Mem" | awk '{print $2}'`

#maybe not support free -m option
if [ ${#MEM_SIZE} -gt 6 ];then
    MEM_SIZE=$((MEM_SIZE/1024))
fi

echo "CPU NUMBER: $CPU_NUM"
echo "MEMERY SIZE: ${MEM_SIZE}M"


while true
do

# < 1G
if [ $MEM_SIZE -lt 512 ];then
        for i in $(seq 1 $CPU_NUM); do
                echo "exec memtester times $i"
                memtester 32M &
        done

#1G
elif [ $MEM_SIZE -lt 1024 ];then
	for i in $(seq 1 $CPU_NUM); do
    		echo "exec memtester times $i"
		memtester 32M &
	done

#2G
elif [ $MEM_SIZE -lt 2048 ];then
	for i in $(seq 1 $CPU_NUM); do
                echo "exec memtester times $i"
                memtester 64M &
        done

#4G
elif [ $MEM_SIZE -lt 4096 ];then
	for i in $(seq 1 $CPU_NUM); do
                echo "exec memtester times $i"
                memtester 128M &
	done
#8G
elif [ $MEM_SIZE -lt 8192 ];then
	for i in $(seq 1 $CPU_NUM); do
                echo "exec memtester times $i"
                memtester 256M &
        done

#16G
elif [ $MEM_SIZE -lt 16384 ];then
	for i in $(seq 1 $CPU_NUM); do
                echo "exec memtester times $i"
                memtester 512M &
        done

# > 16G
else
	for i in $(seq 1 $CPU_NUM); do
                echo "exec memtester times $i"
		memtester 512M &
	done
fi


sleep 5

busybox pidof memtester | busybox xargs kill

sleep 60

done


exit