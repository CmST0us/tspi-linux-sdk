#!/bin/bash

#for andriod, default mount and output path
OUT_PATH=/data

#single file size, BLOCK_SIZE * BLOCK_COUNT (Byte)
BLOCK_SIZE=512
BLOCK_COUNT=10240

#file count, auto calculation
FILE_COUNT=0
#emmc avail size, auto claculation
AVAIL_SIZE=0

function get_emmc_avail_size()
{
	#for andriod, always output to /data
	AVAIL_SIZE=$(df | grep -w "$OUT_PATH" | awk '{print $4}')

	#for linux, if root > part, use root, otherwise use part
	if [ "X$AVAIL_SIZE" == "X" ];then
		
		root_size=$(df | grep -w "/" | awk '{print $4}')
		part_size=$(df | grep -w "mmcblk" | awk '{print $4}')

		if [ "X$part_size" == "X" ];then
			AVAIL_SIZE=$root_size
			return
		fi

		if [ $root_size -lt $part_size ];then
			OUT_PATH=$(df | grep -w "mmcblk" | awk '{print $6}')
			AVAIL_SIZE=$part_size
			echo "change OUT_PATH to $OUT_PATH"
		fi
	fi
}


function write_read_file()
{
	FILE_SIZE=$(($BLOCK_SIZE * $BLOCK_COUNT))
# Byte for MB
	FILE_SIZE=$(($FILE_SIZE / 1024 /1024))
# KB for MB
	AVAIL_SIZE=$(($AVAIL_SIZE / 1024))
	FILE_COUNT=$(($AVAIL_SIZE / $FILE_SIZE / 100))
	[ ! -d $OUT_PATH/rp_stress_path ] && mkdir -p $OUT_PATH/rp_stress_path

	echo "AVAIL_SIZE: ${AVAIL_SIZE}M"
	echo "FILE_SIZE: ${FILE_SIZE}M"
	echo "FILE_COUNT: ${FILE_COUNT}"
	echo "OUT_PATH: ${OUT_PATH}"

	while [ 1 ];
	do
		loop=0
		while [ ${loop} -lt $FILE_COUNT ];
		do
		        for i in $(seq 1 10); do
		                echo "dd if=/dev/zero of=$OUT_PATH/rp_stress_path/file${loop} bs=${BLOCK_SIZE} count=$BLOCK_COUNT"
		                dd if=/dev/zero of=$OUT_PATH/rp_stress_path/file${loop} bs=${BLOCK_SIZE} count=$BLOCK_COUNT
		                sync && echo 3 > /proc/sys/vm/drop_caches
		                dd if=/$OUT_PATH/rp_stress_path/file${loop} of=/dev/zero
		                sync && echo 3 > /proc/sys/vm/drop_caches
		                let loop=${loop}+1
		        done
		done
		rm -rf $OUT_PATH/rp_stress_path/*
		
		sleep 60
	done
}

get_emmc_avail_size
write_read_file

