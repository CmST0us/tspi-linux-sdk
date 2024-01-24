#!/bin/bash

echo =========================
echo Fan test
echo =========================

cd /proc/rp_power

fan_gpio=""
value=0

for gpio in `ls -d *fan*`;do
    fan_gpio="$gpio"
done

if [ ! "$fan_gpio" == "" ];then
    echo "Found fan ctrl gpio: $fan_gpio"
    while [ true ]; do
        echo -ne "Fan power state: $value \r"
        echo $value > /proc/rp_power/$fan_gpio
        let value=!value
        sleep 1s
    done
else
    echo "Current board not fund any fan ctrl gpio"
fi
