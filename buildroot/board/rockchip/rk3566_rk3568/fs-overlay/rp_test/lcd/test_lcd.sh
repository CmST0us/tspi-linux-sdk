#!/bin/bash

echo ==============================
echo LCD test
echo ==============================

cd /sys/class/drm

echo "All lcd list:"
printf "\t%-10s %-20s %-15s %-15s\n" "Name" "Connect status" "Connector id" "Default mode"
for card in `ls -d card0-*`;do
    [[ "$card" =~ "card0-Writeback" ]] && continue
    
    card_name=${card#*-} 
    connector_id=`modetest -c | grep $card_name -w | awk '{print $1}'`
    connect_status=`cat $card/status`

    if [ "$connect_status" == "connected" ]; then
        def_mode=`cat $card/modes | head -n 1`
    else
        def_mode=""
    fi

    printf "\t%-10s %-20s %-15s %-15s\n" "$card_name" "$connect_status" "$connector_id" "$def_mode"
    
done
