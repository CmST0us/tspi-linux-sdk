#!/bin/sh
#
#

case "$1" in
  start)
        echo "Starting $0..."
        cd /oem/demo/ && ./demo_main &
        ;;
  stop)
        echo "Stop $0..."
        #killall dui_fespl
        ;;
  restart|reload)
        #killall dui_fespl
        cd /oem/demo/ && ./demo_main &
        ;;
  *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?
