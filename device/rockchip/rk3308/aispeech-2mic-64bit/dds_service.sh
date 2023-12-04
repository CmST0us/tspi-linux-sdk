#!/bin/sh
#
#

case "$1" in
  start)
        echo "Starting $0..."
        cd /oem/dds_client/demo && ./demo_main &
        ;;
  stop)
        echo "Stop $0..."
        killall demo_main
        ;;
  restart|reload)
        killall demo_main
        cd /oem/dds_client/demo && ./demo_main &
        ;;
  *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?
