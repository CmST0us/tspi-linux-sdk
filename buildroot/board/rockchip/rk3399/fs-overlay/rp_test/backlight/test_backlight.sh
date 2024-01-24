#!/bin/bash

brightness=255
increase=true

cd /sys/class/backlight

echo =======================================
echo write and read all backlight brightness
echo ======================================

trap 'onCtrlC' INT
function onCtrlC () {
    #move cursor down $row line
    printf "\33[${row}B";
    echo
    exit 0
}

while [ 1 ];do
    row=0
    result=""
    sleep 0.01s
    if [ $brightness -lt 10 ];then
        increase=false
        brightness=0
    fi

    if [ $brightness -gt 245 ];then
        increase=true
        brightness=255
    fi

    if $increase; then
        brightness=$((brightness-10))
    else
        brightness=$((brightness+10))
    fi

    for node in `ls .`; do
        [ ! -e $node/brightness ] && continue
 
        result="${result}write $node brightness: $brightness,  "
        echo $brightness > $node/brightness
        result="${result}read $node brightness: $brightness     \r\n"
        sleep 0.1s
        let row++
    done

    echo -ne "$result \r"
    printf "\33[${row}F";


done
