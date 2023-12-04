#!/bin/sh

sleep 3

while true;do
    pid_voip=`ps aux | grep cmcc_voip | grep -v grep | busybox awk '{print \$1}'`
    pid_client=`ps aux | grep sai_client | grep -v grep | busybox awk '{print \$1}'`
    pid_qplay=`ps aux | grep qplay_client | grep -v grep | busybox awk '{print \$1}'`
    if [ "$pid_client" == "" ];then
        echo "sai_client died, restart it."
        export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oem/lib
        ps -ef | grep -iE "sai_client" | grep -v "grep" | awk '{print $1}' | xargs kill -9
        #ps -ef | grep -iE "cmcc_voip" | grep -v "grep" | awk '{print $1}' | xargs kill -9
         
        sleep 1
        /oem/sai_client &
    
    fi
   
    if [ "$pid_voip" = "" ] ;then
	echo "voip died,restart it."
	ps -ef | grep -iE "cmcc_voip" | grep -v "grep" | awk '{print $1}' | xargs kill -9
	
	sleep 1
	/oem/cmcc_voip &
    fi
    
    if [ "$pid_qplay" = "" ] ;then
        echo "qplay died,restart it."
        ps -ef | grep -iE "qplay_client" | grep -v "grep" | awk '{print $1}' | xargs kill -9

        sleep 1
        /oem/qplay_client &
    fi

    sleep 2
done
