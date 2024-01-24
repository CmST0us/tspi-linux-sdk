#!/bin/bash

echo ===========================
echo GPIO input test
echo ===========================

trap 'onCtrlC' INT
function onCtrlC () {
    #move cursor down $row line
    printf "\33[${row}B";
    exit 0
}

cd /proc/rp_gpio/

while [ true ]; do
    row=0
    result=""
    for gpio in `ls .`; do
        result="${result}Get $gpio value: `cat $gpio` \n"
        let row++
        sleep 0.1s
    done
    
    echo -ne "$result \r"
    printf "\33[${row}F";
    sleep 1s
done
