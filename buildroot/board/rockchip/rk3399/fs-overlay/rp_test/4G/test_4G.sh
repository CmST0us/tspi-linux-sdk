#!/bin/bash

echo "================================"
echo "4G test"
echo "================================"


if [ ! -e /dev/ttyUSB3 ];then
	echo "/dev/ttyUSB* not found, exit!!!"
	exit -1
fi

sh /etc/ppp/peers/quectel-pppd.sh
sleep 4s

echo `route del default`

echo `route add default dev ppp0`
sleep 3s
echo `netstat -nr`

if [ $? == 0 ];then

ping -c 6 -I ppp0 www.baidu.com

fi


