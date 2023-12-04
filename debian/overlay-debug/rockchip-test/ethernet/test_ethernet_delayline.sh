#!/bin/bash

if [ -e /sys/devices/platform/*.ethernet/phy_lb_scan ]; then
	echo "please check if enable the tool on drivers/net/ethernet/stmicro/stmmac/dwmac-rk-tool.c\n"
fi
echo 1000 > /sys/devices/platform/*.ethernet/phy_lb_scan
