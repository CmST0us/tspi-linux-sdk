#!/bin/bash

echo =======================
echo RTC test
echo ======================


if [ -e /sys/class/rtc/rtc0 ];then
    echo RTC model: `cat /sys/class/rtc/rtc0/name`
    echo "RTC detect success"
    echo
    echo "Set system time: 2023-07-11 12:00:00"
    date --set="2023-07-11 12:00:00"
    echo 
    echo "Set rtc time: 2023-07-11 12:00:00"
    hwclock -w
    hwclock -r
    echo
    echo "Please restart the viewing time after 2 minutes of cut of power"
else
    echo "RTC detect faild"
fi
