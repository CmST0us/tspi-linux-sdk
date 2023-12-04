#!/bin/sh
#
#

case "$1" in
  start)
        echo "Starting $0..."
        cd /oem/dds_client && ./dui_fespl dui_fespl.json &
        ;;
  stop)
        echo "Stop $0..."
        killall dui_fespl
        ;;
  restart|reload)
        killall dui_fespl
        cd /oem/dds_client && ./dui_fespl dui_fespl.json &
        ;;
  *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?
