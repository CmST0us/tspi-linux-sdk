#!/bin/sh

case "$1" in
	start|"")
		while true
		do
			list-iodomain.sh
			sleep $(( 60 * 2 ))
		done &
		;;
	restart|reload|force-reload)
		echo "Error: argument '$1' not supported" >&2
		exit 3
		;;
	stop|status)
		# No-op
		;;
	*)
		echo "Usage: [start|stop]" >&2
		exit 3
		;;
esac
