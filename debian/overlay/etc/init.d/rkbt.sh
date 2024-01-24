#!/bin/bash -e
### BEGIN INIT INFO
# Provides:          rockchip
# Required-Start:
# Required-Stop:
# Default-Start:
# Default-Stop:
# Short-Description:
# Description:       Setup rockchip platform environment
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

init_rkwifibt() {
    case $1 in
        rk3288)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3399|rk3399pro)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3328)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3326|px30)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3128|rk3036)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3562)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3566)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3568)
	    /usr/bin/bt_pcba_test&
            ;;
        rk3588|rk3588s)
	    /usr/bin/bt_pcba_test&
            ;;
    esac
}

COMPATIBLE=$(cat /proc/device-tree/compatible)
if [[ $COMPATIBLE =~ "rk3288" ]];
then
    CHIPNAME="rk3288"
elif [[ $COMPATIBLE =~ "rk3328" ]]; then
    CHIPNAME="rk3328"
elif [[ $COMPATIBLE =~ "rk3399" && $COMPATIBLE =~ "rk3399pro" ]]; then
    CHIPNAME="rk3399pro"
    update_npu_fw
elif [[ $COMPATIBLE =~ "rk3399" ]]; then
    CHIPNAME="rk3399"
elif [[ $COMPATIBLE =~ "rk3326" ]]; then
    CHIPNAME="rk3326"
elif [[ $COMPATIBLE =~ "px30" ]]; then
    CHIPNAME="px30"
elif [[ $COMPATIBLE =~ "rk3128" ]]; then
    CHIPNAME="rk3128"
elif [[ $COMPATIBLE =~ "rk3562" ]]; then
    CHIPNAME="rk3562"
elif [[ $COMPATIBLE =~ "rk3566" ]]; then
    CHIPNAME="rk3566"
elif [[ $COMPATIBLE =~ "rk3568" ]]; then
    CHIPNAME="rk3568"
elif [[ $COMPATIBLE =~ "rk3588" ]]; then
    CHIPNAME="rk3588"
else
    CHIPNAME="rk3036"
fi
COMPATIBLE=${COMPATIBLE#rockchip,}
BOARDNAME=${COMPATIBLE%%rockchip,*}

# init rkwifibt
init_rkwifibt ${CHIPNAME}
