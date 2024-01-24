#!/bin/sh
### BEGIN INIT INFO
# Provides:       usbdevice
# Required-Start: $local_fs $syslog
# Required-Stop:  $local_fs
# Default-Start:  S
# Default-Stop:   K
# Description:    Manage USB device functions
### END INIT INFO

case "$1" in
	start|stop|restart)
		/usr/bin/usbdevice $1
		;;
	*)
		echo "Usage: [start|stop|restart]" >&2
		exit 3
		;;
esac

:
