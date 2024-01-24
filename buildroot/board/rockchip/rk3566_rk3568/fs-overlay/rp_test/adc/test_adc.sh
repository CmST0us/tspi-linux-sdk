#!/bin/bash

echo =======================
echo ADC read value test        
echo =======================

cd /sys/bus/iio/devices/
row=0

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
    for device in `ls -d iio\:device*`;do
        result="${result}$device \n"
        let row++
        cd $device
        for chn in `ls in_voltage*_raw`;
        do
            value=`cat $chn`
            result="${result}    $chn value: $value \n"
            let row++
        done
    done

    echo -ne "$result \r"

    #move cursor up $row line
    printf "\33[${row}F";

    cd ..
    sleep 0.5s
done
