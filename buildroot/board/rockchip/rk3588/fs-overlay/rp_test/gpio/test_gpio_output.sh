#!/bin/bash

echo ============================
echo GPIO output test
echo ============================

cd /proc/rp_gpio/
value=0

trap 'onCtrlC' INT
function onCtrlC () {
    #move cursor down $row line
    printf "\33[${row}B";
    echo
    exit 0
}

while [ 1 ]; do
    row=0
    result=""
    for gpio in `ls .`; do
        result="${result}Set $gpio value: $value"
        echo $value > $gpio
        result="${result}, Get $gpio value: `cat $gpio` \n"
        sleep 0.1s
        let row++
    done

    echo -ne "$result \r"
    printf "\33[${row}F";

    if [ $value == 0 ];then
        value=1
    else
        value=0
    fi


    sleep 1s
done
