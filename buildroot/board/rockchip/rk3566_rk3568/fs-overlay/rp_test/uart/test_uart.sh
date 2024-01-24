#!/bin/bash

ttyArray=()
baudrates=(
1200
1800
2400
4800
9600
19200
38400
57600
115200
230400
460800
921600)

index=0
cd /sys/class/tty/

echo ==============================
echo All available uart port
echo ==============================

for tty in `ls -d tty*`; do
    if [ -e $tty/device ];then
        [ "$tty" == "ttyFIQ0" ] && continue
        echo $index: "/dev/$tty"
        ttyArray+=("/dev/$tty")
        let index++
    fi
done
#echo ${ttyArray[@]}

read -p "Select uart port num to test: " ttyindex
if echo $ttyindex | grep -vq [^0-9]; then
    if ((ttyindex >= 0 && ttyindex < ${#ttyArray[@]})); then
        index=0
        for baudrate in ${baudrates[@]}; do
            echo $index: $baudrate  
            let index++          
        done 
        read -p "Select uart baudrate: " baudrateindex
        if echo $baudrateindex | grep -vq [^0-9]; then
            if ((baudrateindex >= 0 && baudrateindex < ${#baudrates[@]})); then
                echo "Selected UART port: ${ttyArray[$ttyindex]}"
                echo "Selected baudrate: ${baudrates[$baudrateindex]}"
                /rp_test/uart/comtest ${ttyArray[$ttyindex]} ${baudrates[$baudrateindex]}
            else
                echo "Invalid baudrate selection"
            fi
        else
            echo "Invalid input"
        fi
    else
        echo "Invalid UART port selection"
    fi
else
    echo "Invalid input"
fi

