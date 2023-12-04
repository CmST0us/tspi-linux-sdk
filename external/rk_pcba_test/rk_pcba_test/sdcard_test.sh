#!/bin/sh

SDCARD=mmcblk0
SDCARD_PATH=/dev/${SDCARD}
TIMEOUT=60
MOUNTPOINT=/tmp/sdcard

echo Start testing SDCARD:${SDCARD}

for i in `seq ${TIMEOUT}`;do
	echo Waiting for SDCARD inserted ... `expr ${TIMEOUT} - ${i}`
	sleep 1
	ls ${SDCARD_PATH} >/dev/null 2>&1 || continue

	echo SDCARD inserted...

	capacity=`cat /proc/partitions | grep ${SDCARD} -w | busybox awk '{printf $3}'`
	echo "${SDCARD}: ${capacity}"
	echo ${capacity} > /run/sd_capacity

	busybox mount | grep ${SDCARD_PATH} && exit 0

	mkdir -p ${MOUNTPOINT} 2>/dev/null

	for p in `ls ${SDCARD_PATH}*`;do
		echo Mounting ${p}...
		busybox mount ${p} ${MOUNTPOINT} || continue

		echo Mounted ${p}...
		busybox umount ${MOUNTPOINT}
		exit 0
	done

	echo Failed to mount SDCARD:${SDCARD}...
	exit 1
done

echo Timed out waiting for SDCARD:${SDCARD}...
exit 2
