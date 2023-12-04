#!/bin/sh
#
# Setup qtwebengine environment.
#

case "$1" in
	start)
		# Create dummy video node for V4L2 VDA/VEA with rkmpp plugin
		echo dec > /dev/video-dec0
		echo enc > /dev/video-enc0
		;;
	stop)
		;;
	restart|reload)
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 1
esac

exit $?
