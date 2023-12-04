#!/bin/sh

PROCESS=/data/dds_service.sh

softap_stop()
{
    echo softap_stoping

    killall dnsmasq || echo dnsmasq-exit
    ip addr delete 192.168.1.1 dev wlan1 || echo ip-addr-delete
    killall hostapd || echo hostapd-exit
    ifconfig wlan1 down || echo wlan1-down

    echo softap_stopped
}

dds_start()
{
    softap_stop
    #echo dds_start
    pidof demo_main || $PROCESS start
    
    gst-play-1.0 /data/aispeech_softap_lite/audio/connect_ok.mp3 
}

dds_stop()
{
    echo dds_stop
    #$PROCESS stop
}
wifiReadyAction()
{
    pidof demo_main || $PROCESS start
}
wifiUpAction()
{
    echo wifiUp
    dds_start
}
wifiDownAction()
{
    echo wifiDown
    dds_stop
}
wifiChangeAction()
{
    echo wifiChange
    dds_stop
    dds_start
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
                    wifiReadyAction
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

$PROCESS stop
checkwifistate
