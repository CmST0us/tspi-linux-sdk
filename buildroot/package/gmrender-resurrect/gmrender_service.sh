#!/bin/sh

PROCESS=gmediarender
NAME=rockchip

dlna_start()
{
    echo dlna_start
    $PROCESS -f $NAME -d
}

dlna_stop()
{
    echo dlna_stop
    killall $PROCESS
}

wifiUpAction()
{
    echo wifiUp
    dlna_start
}
wifiDownAction()
{
    echo wifiDown
    dlna_stop
}
wifiChangeAction()
{
    echo wifiChange
    dlna_stop
    dlna_start
}
wifiRequestingIp()
{
    echo wifiRequestingIp
}

checkwifistate()
{
    local flag=0
    local last_ip_address=0
    while true
    do
        wpa_state=`wpa_cli -iwlan0 status | grep wpa_state | awk -F '=' '{printf $2}'`
        ip_address=`wpa_cli -iwlan0 status | grep ip_address | awk -F '=' '{printf $2}'`

        if [ "${wpa_state}"x = "COMPLETED"x ];then
            if [ "${ip_address}"x != ""x ] && [ "${ip_address}"x != "0.0.0.0"x ];then
                if [ $flag -eq 0 ];then
                    flag=1
                    wifiUpAction
                elif [ "${ip_address}"x != "${last_ip_address}"x ];then
                    flag=1
                    wifiChangeAction
                else
                    flag=1
                fi
            else
               flag=0
               wifiRequestingIp
            fi
        else
            if [ $flag -eq 1 ];then
               flag=0
               wifiDownAction
            fi
        fi
        sleep 3
        last_ip_address="${ip_address}"
    done
}

killall $PROCESS
checkwifistate
