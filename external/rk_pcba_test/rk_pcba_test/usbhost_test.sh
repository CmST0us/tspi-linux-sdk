#!/bin/sh

USBHOST=sda1
USBHOST_PATH=/dev/${USBHOST}
TIMEOUT=60
MOUNTPOINT=/tmp/usb_storage

echo Start testing USBHOST:${USBHOST}

for i in `seq ${TIMEOUT}`;do
	echo Waiting for USBHOST inserted ... `expr ${TIMEOUT} - ${i}`
	sleep 1
	ls ${USBHOST_PATH} >/dev/null 2>&1 || continue

	echo USBHOST inserted...

	capacity=`cat /proc/partitions | grep ${USBHOST} -w | busybox awk '{printf $3}'`
	echo "${USBHOST}: ${capacity}"
	echo ${capacity} > /run/usbhost_capacity

	busybox mount | grep ${USBHOST_PATH} && exit 0

	mkdir -p ${MOUNTPOINT} 2>/dev/null

	for p in `ls ${USBHOST_PATH}*`;do
		echo Mounting ${p}...
		busybox mount ${p} ${MOUNTPOINT} || continue

		echo Mounted ${p}...
		busybox umount ${MOUNTPOINT}
		exit 0
	done

	echo Failed to mount USBHOST:${USBHOST}...
	exit 1
done

echo Timed out waiting for USBHOST:${USBHOST}...
exit 2
