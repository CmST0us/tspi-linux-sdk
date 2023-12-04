#!/bin/sh

DIR_WIFI=/rockchip_test/wifi
RESULT_DIR=/data/cfg/rockchip_test/wifi

wifi_sta_connect()
{
	sh ${DIR_WIFI}/wifi_sta_connect.sh
}

wifi_sta_reboot()
{
	sh ${DIR_WIFI}/wifi_sta_reboot.sh
}

wifi_onoff()
{
	sh ${DIR_WIFI}/wifi_onoff_apsta.sh
}

wifi_test()
{
    echo "*****************************"
    echo "**                         **"
    echo "**      WIFI TEST          **"
    echo "**                         **"
    echo "*****************************"

    echo "*****************************"
    echo "WIFI Connect:             1"
    echo "Auto reboot:              2"
    echo "WIFI ON/OFF:              3"

    read -t 30 WIFI_CHOICE
    case ${WIFI_CHOICE} in
        1)
            wifi_sta_connect
            ;;
        2)
            wifi_sta_reboot
            ;;
        3)
            wifi_onoff
            ;;
        *)
            echo "not found wifi module."
            ;;
    esac
}

wifi_test
