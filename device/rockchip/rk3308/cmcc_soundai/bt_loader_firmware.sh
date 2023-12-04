#!/bin/sh

echo 0 > /sys/class/rfkill/rfkill0/state
sleep 2
echo 1 > /sys/class/rfkill/rfkill0/state
sleep 2

insmod /usr/lib/modules/hci_uart.ko
rtk_hciattach -n -s 115200 /dev/ttyS4 rtk_h5 &
hciconfig hci0 up

