#!/bin/sh

enable_ipv6() {
while [ 1 ]
do
        status=`wpa_cli status | grep wpa_state | awk -F '=' '{print $2}'`
        if  [ $status = COMPLETED ]; then
                echo 2 > /proc/sys/net/ipv6/conf/wlan0/accept_ra
                echo 1 > /proc/sys/net/ipv6/conf/wlan0/autoconf
                echo 1 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6
                echo 0 > /proc/sys/net/ipv6/conf/wlan0/disable_ipv6
                sleep 2
                prex=`route -A inet6  | grep wlan0 | awk -F ' ' '{print $1}' |  awk -F '/' '{print $1}' | sed -n 1p`
                route -A inet6 add ::/0 gw ${prex}"1"
                break
        fi
done
}

enable_ipv6

while [ 1 ]
do
        status=`wpa_cli status | grep wpa_state | awk -F '=' '{print $2}'`
        if  [ $status != COMPLETED ]; then
                ip -6 addr flush dev wlan0
		route -A inet6
                enable_ipv6
                sleep 1
        fi
        sleep 2;
done

