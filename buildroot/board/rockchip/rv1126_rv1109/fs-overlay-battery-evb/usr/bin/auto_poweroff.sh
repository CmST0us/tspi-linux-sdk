#!/bin/sh
#

function usb_detect() {
    cat /sys/class/extcon/usb2-phy/state | grep P=1
    if [ $? == 0 ]; then
        echo "USB power supply detected, unable to enter sleep"
        exit 0
    fi
}

boot_limit_time=90
no_client_time=5
rtmp_file=/tmp/rtmp_live
# In the first 90 seconds, shut down if only one client
current_time=`cat /proc/uptime | cut -d '.' -f1`
while [ $current_time -lt $boot_limit_time ]
do
    current_time=`cat /proc/uptime | cut -d '.' -f1`
    # echo "current_time is $current_time, boot_limit_time is $boot_limit_time"
    if [ -e "$rtmp_file" ]; then
        break
    fi
    sleep 1
done

if [ ! -e "$rtmp_file" ]; then
    echo "$boot_limit_time seconds without client access"
    usb_detect
    aplay /etc/go_to_sleep.wav &
    killall -3 rkipc
    exit 0
fi

echo "after $boot_limit_time seconds or rtmp file exist"
while [ 1 ]
do
    if [ ! -e "$rtmp_file" ]; then
        prev_time=`cat /proc/uptime | cut -d '.' -f1`
        while [ 1 ]
        do
            sleep 1
            current_time=`cat /proc/uptime | cut -d '.' -f1`
            interval_time=$((current_time-prev_time))
            echo "interval_time is $interval_time, no rtmp time is $no_client_time"
            if [ $interval_time -gt $no_client_time ]; then
		usb_detect
		aplay /etc/go_to_sleep.wav &
		killall -3 rkipc
		exit 0
            fi
            if [ -e "$rtmp_file" ]; then
                break
            fi
        done
    fi
    sleep 1
done
