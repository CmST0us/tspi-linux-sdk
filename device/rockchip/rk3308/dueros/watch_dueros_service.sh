#!/bin/sh

sleep 5

while true;do
    pid_num=`pidof duer_linux alsa_audio_main_service | wc -w`
    if [ $pid_num -lt 2 ] ;then
        echo "duer_linux|alsa_audio_main_service died, restart it."
        /oem/dueros_service.sh restart
    fi
    sleep 2
done
