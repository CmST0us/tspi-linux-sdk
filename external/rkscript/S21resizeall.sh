#!/bin/sh
### BEGIN INIT INFO
# Provides:       resize-all
# Required-Start: $local_fs
# Required-Stop:  $local_fs
# Default-Start:  S
# Default-Stop:
# Description:    Resize all internal mounted partitions
### END INIT INFO

case "$1" in
	start|"")
		resize-helper
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
