#!/bin/sh
### BEGIN INIT INFO
# Provides:       mount-all
# Required-Start:
# Required-Stop:
# Default-Start:  S
# Default-Stop:
# Description:    Mount all internal partitions in /etc/fstab
### END INIT INFO

case "$1" in
	start|"")
		mount-helper
		;;
	restart|reload|force-reload)
		echo "Error: argument '$1' not supported" >&2
		exit 3
		;;
	stop|status)
		# No-op
		;;
	*)
		echo "Usage: start" >&2
		exit 3
		;;
esac

:
