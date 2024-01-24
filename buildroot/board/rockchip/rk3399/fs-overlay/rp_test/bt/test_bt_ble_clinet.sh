#!/bin/bash

echo ==========================
echo "BLE clinet start"
echo =========================
echo

echo =========================
echo "BLE info"
echo =========================
bluetoothctl show

echo "BLE clinet runing..."
bluetoothctl advertise on
