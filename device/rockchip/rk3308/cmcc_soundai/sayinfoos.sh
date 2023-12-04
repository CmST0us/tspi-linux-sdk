#!/bin/sh

start_sayinfo_os()
{
	printf "========== Start  sayinfo os ===========================\n"

    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oem/lib
    export LD_LIBRARY_PATH=/oem/lib
    

    cp /oem/sai_config/sai.q /userdata/sai.q -rf
    cp /oem/sai_config/sai_sdk_release.log /userdata/sai_sdk_release.log -rf

    mount --bind /userdata/sai.q /oem/sai_config/sai.q
    mount --bind /userdata/sai_sdk_release.log /oem/sai_config/sai_sdk_release.log
    
    /oem/enable_ipv6.sh &
   
    sleep 1
    /oem/sai_client &
 
    sleep 1
    /oem/cmcc_voip &  
    
     sleep 1
    /oem/qplay_client &

     sleep 1
    #/oem/rsdlclient &
    
    /usr/bin/ota_demo &
    
    sleep 1
    /oem/watch_sai_service.sh &
}

stop_sayinfo_os()
{
	printf "========== Stop talk_droid =============================\n"
    
    ps -ef | grep -iE "watch_sai_service" | grep -v "grep" | awk '{print $1}' | xargs kill -9
    ps -ef | grep -iE "sai_client" | grep -v "grep" | awk '{print $1}' | xargs kill -9
    ps -ef | grep -iE "cmcc_voip" | grep -v "grep" | awk '{print $1}' | xargs kill -9
    ps -ef | grep -iE "qplay_client" | grep -v "grep" | awk '{print $1}' | xargs kill -9
}


case $1 in
	"start") start_sayinfo_os ;;
	"stop")  stop_sayinfo_os  ;;
	*) echo "param should be [start|stop]" ;;
esac
