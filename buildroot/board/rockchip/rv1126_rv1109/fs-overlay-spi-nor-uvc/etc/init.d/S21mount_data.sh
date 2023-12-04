#!/bin/sh

case "$1" in
	start)
		/bin/busybox mount -t jffs2 /dev/mtdblock5 /userdata
		;;
	stop)
		/bin/busybox umount /data
                ;;
	*)
		echo "Usage: $0 {start|stop}"
		exit 1
		;;
esac
exit 0
