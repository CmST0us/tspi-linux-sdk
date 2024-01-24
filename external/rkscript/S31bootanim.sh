#!/bin/sh -e
### BEGIN INIT INFO
# Provides:          bootanim
# Required-Start:    mountvirtfs
# Required-Stop:
# Should-Start:
# Should-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Boot time animation
### END INIT INFO

PATH="/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin"

# Load default env variables from profiles
. /etc/profile

type bootanim >/dev/null && \
case "$1" in
	start|restart|reload)
		echo -n "starting bootanim... "
		bootanim start
		echo "done."
		;;
	stop)
		echo -n "stoping bootanim... "
		bootanim stop
		echo "done."
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 1
esac

exit 0
