#!/bin/sh

case "$1" in
    start)

    echo "start rtl8723ds bluetooth server"
    bt_pcba_test &
    sleep 7

    echo "start rtl8723ds bluetooth hciconfig"
    hciconfig hci0 up &

    echo "start rtl8723ds bluetooth wifi config"
    /usr/libexec/bluetooth/bluetoothd -C -E -d -n &
    sleep 2
    gatt-service &

        ;;
    stop)
        echo -n "Stopping rtl8723ds bluez5_utils bluetooth server & app"
        killall gatt-service
        sleep 1
        killall bluetoothd
        sleep 1
        if busybox cat /sys/class/rkwifi/chip | grep RTL8723DS; then
        killall rtk_hciattach
        fi
        if busybox cat /sys/class/rkwifi/chip | grep AP6255; then
        killall brcm_patchram_plus1
        fi

        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
esac

exit $?

