#!/bin/sh
#
#

case "$1" in
  start)
        echo "Starting $0..."
        cd /oem/dds_client && ./dui_fespa dui_fespa.json &
        ;;
  stop)
        echo "Stop $0..."
        killall dui_fespa
        ;;
  restart|reload)
        killall dui
        cd /oem/dds_client && ./dui_fespa dui_fespa.json &
        ;;
  *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?
