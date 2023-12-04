#!/bin/sh
#

killall ipc-daemon
killall mediaserver
killall storage_manager
killall netserver
killall dbserver
killall startup_app_ipc
killall ispserver
echo "Stop Application ..."

cnt=0
while [ 1 ];
do
	cnt=$(( cnt + 1 ))
	if [ $cnt -eq 8 ]; then
		echo "killall -9 mediaserver"
		killall -9 mediaserver
		sleep 0.1
		break
	fi
	ps|grep mediaserver|grep -v grep
	if [ $? -ne 0 ]; then
		echo "mediaserver exit"
		break
	else
		echo "mediaserver active"
	fi
	sleep 1
done

umount /userdata/media
