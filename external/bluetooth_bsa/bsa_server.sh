#!/bin/sh

hcd_file="BTFIRMWARE_PATH"
echo "hcd_file = $hcd_file"
ttys_dev="BT_TTY_DEV"
echo "ttys_dev = $ttys_dev"

case "$1" in
    start)

    killall brcm_patchram_plus1
    killall bsa_server
    check_not_exist.sh bsa_server

    echo 0 > /sys/class/rfkill/rfkill0/state
    sleep 1
    echo 1 > /sys/class/rfkill/rfkill0/state
    sleep 1

    mkdir -p /data/bsa/config/test_files/av
    cp /etc/bsa_file/* /data/bsa/config/test_files/av/
    cd /data/bsa/config
    echo "start broadcom bluetooth server bsa_sever"
    bsa_server -r 12 -p $hcd_file -d $ttys_dev -all=0 &
    #bsa_server -r 12 -b /data/bsa/btsnoop.log -p $hcd_file -d $ttys_dev > /data/bsa/bsa_log &

    echo "|----- bluetooth bsa server is open ------|"

        ;;
    stop)
        echo "Stopping broadcom bsa bluetooth server"
        killall bsa_server
        check_not_exist.sh bsa_server
        echo 0 > /sys/class/rfkill/rfkill0/state
        echo "|-----bluetooth bsa server is close-----|"

        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac

exit $?

