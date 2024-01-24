#!/bin/bash

cd /sys/class/net/

for can in `ls -d can*`; do
    echo =================================
    echo Start test $can
    echo ================================

    ip link set $can down                               
    ip link set $can type can bitrate 100000 loopback on
    ip link set $can up
    killall candump
    candump $can &
    
    i=1                                               
    while [ $i -lt 11 ]                                          
    do                                                           
        echo "---------------------send data times $i-----------------------"
        cansend $can 123#11223344556677                        
        let i=$i+1                                           
        sleep 1s                                             
    done   
done
