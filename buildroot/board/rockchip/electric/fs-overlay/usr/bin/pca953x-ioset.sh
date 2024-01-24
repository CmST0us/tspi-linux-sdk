#! /bin/bash

COMPATIBLE=$(cat -v /proc/device-tree/compatible)

if [[ $(expr $COMPATIBLE : ".*rk3562") -ne 0 ]]; then
	CHIPNAME="rk3562"
elif [[ $(expr $COMPATIBLE : ".*rk3568") -ne 0 ]]; then
	CHIPNAME="rk3568"
else
    CHIPNAME="rk3358"
fi

if [[ x"$CHIPNAME" == x"rk3562" ]]; then
	value=495
	while [ $value -le 510 ]; do
		cd /sys/class/gpio
		echo $value > export
		echo out > gpio$value/direction
		echo 0 > gpio$value/value
		value=$(($value + 1))
	done
fi
