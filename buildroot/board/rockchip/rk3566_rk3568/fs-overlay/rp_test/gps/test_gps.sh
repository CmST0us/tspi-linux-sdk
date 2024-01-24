#!/bin/bash

echo ==========================
echo GPS model test
echo ==========================

ttyArray=()
index=0

cd /sys/class/tty

for tty in `ls -d tty*`; do
    if [ -e $tty/irq ];then
        echo $index: "/dev/$tty"
        ttyArray+=("/dev/$tty")
        let index++
    fi
done

read -p "Select uart port num for GPS model: " ttyindex
if echo $ttyindex | grep -vq [^0-9]; then
    if ((ttyindex >= 0 && ttyindex < ${#ttyArray[@]})); then
        echo "Select uart: ${ttyArray[ttyindex]}"
        echo "Press Ctrl+X to exit recived GPS info"
        microcom ${ttyArray[ttyindex]}
    else
        echo "Invalid UART port selection"
    fi
else
    echo "Invalid input"
fi
