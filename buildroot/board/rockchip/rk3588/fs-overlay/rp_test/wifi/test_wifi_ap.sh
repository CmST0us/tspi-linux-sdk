#/bin/bash

echo =================================
echo WIFI AP model test
echo =================================

board=`cat /proc/device-tree/model`
password="12345678"

echo "hotspot name: $board"
echo "password: 12345678"

killall dnsmasq

if [ "$1" == "wlan1" ];then
        #use wlan1 creat AP
        ifconfig wlan0 up
        iw phy0 interface add wlan1 type managed
        ifconfig wlan1 up
        create_ap  wlan1 eth0 ${board} ${password} --no-virt
else
        #use wlan0 creat AP
        create_ap  wlan0 eth0 ${board} ${password} --no-virt
fi
