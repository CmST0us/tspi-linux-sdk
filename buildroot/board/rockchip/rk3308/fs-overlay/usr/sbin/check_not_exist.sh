#!/bin/sh

if [ $# -eq 0 ]; then
    echo missing process names!
    exit 1
fi

while true;do
	pid=`pidof $*`
	if [ "$pid" = "" ];then
		echo "$* died."
		break
	fi
	sleep 0.2
done

