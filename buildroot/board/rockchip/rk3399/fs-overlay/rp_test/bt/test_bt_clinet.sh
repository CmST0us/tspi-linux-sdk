#!/bin/bash

echo ============================
echo "BT clinet test"
echo ===========================

[ ! -e /sys/class/bluetooth/hci0 ] && bt_init.sh

hciconfig hci0 up
hciconfig hci0 piscan

bluetoothctl show

echo =======================================
echo "Now you can connect this device......"
echo =======================================
bluetoothctl
