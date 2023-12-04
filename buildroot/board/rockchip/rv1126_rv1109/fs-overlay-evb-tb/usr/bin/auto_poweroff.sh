#!/bin/sh
#

boot_limit_time=20
no_client_time=10
# In the first 20 seconds, shut down if only one client
client_num=`netstat -t | grep ESTABLISHED | wc -l`
current_time=`cat /proc/uptime | cut -d '.' -f1`
while [ $current_time -lt $boot_limit_time ]
do
    client_num=`netstat -t | grep ESTABLISHED | wc -l`
    current_time=`cat /proc/uptime | cut -d '.' -f1`
    echo "current_time is $current_time, client_num is $client_num"
    if [ $client_num -gt 1 ]; then
        break
    fi
    sleep 1
done

if [ $client_num -le 1 ]; then
    echo "$boot_limit_time seconds without client access"
    poweroff
fi

echo "after $boot_limit_time seconds, client_num is $client_num"
while [ 1 ]
do
    client_num=`netstat -t | grep ESTABLISHED | wc -l`
    echo "client_num is $client_num"
    if [ $client_num -le 1 ]; then
        prev_time=`cat /proc/uptime | cut -d '.' -f1`
        while [ 1 ]
        do
            sleep 1
            current_time=`cat /proc/uptime | cut -d '.' -f1`
            interval_time=$((current_time-prev_time))
            client_num=`netstat -t | grep ESTABLISHED | wc -l`
            echo "interval_time is $interval_time, no_client_time is $no_client_time"
            if [ $interval_time -gt $no_client_time ]; then
                poweroff
            fi
            if [ $client_num -gt 1 ]; then
                break
            fi
        done
    fi
    sleep 1
done
