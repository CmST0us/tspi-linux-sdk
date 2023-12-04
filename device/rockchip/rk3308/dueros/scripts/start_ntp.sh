#!/bin/sh

/etc/init.d/S49ntp stop

while true
do
        ntpd=`ntpd -qgx4 202.108.6.95 58.220.207.226 47.92.108.218 202.112.29.82 120.25.108.11 182.92.12.11 115.28.122.198 \
                | grep 'ntpd: time'`
        if [[ "$ntpd" != "" ]]
        then
            touch /tmp/ntp_successful
            break
        fi
	sleep 1
done

#/usr/sbin/ntpd -p /run/ntpd.pid -g
/etc/init.d/S49ntp start
