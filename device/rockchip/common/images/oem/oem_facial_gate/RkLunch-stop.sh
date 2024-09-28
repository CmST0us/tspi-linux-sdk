#!/bin/sh
#

killall ipc-daemon
killall storage_manager
killall netserver
killall dbserver
killall ispserver
echo "Stop Application ..."

cnt=0
while [ 1 ];
do
	cnt=$(( cnt + 1 ))
	if [ $cnt -eq 8 ]; then
		echo "killall QFacialGate"
		killall QFacialGate
		sleep 0.1
		break
	fi
	ps|grep QFacialGate|grep -v grep
	if [ $? -ne 0 ]; then
		echo "QFacialGate exit"
		break
	else
		echo "QFacialGate active"
	fi
	sleep 1
done
