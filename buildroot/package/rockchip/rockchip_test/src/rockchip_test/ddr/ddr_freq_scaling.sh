#!/bin/bash

DMC_PATH1=/sys/class/devfreq/dmc
DMC_PATH2=/sys/bus/platform/drivers/rk3399-dmc-freq/dmc/devfreq/dmc

if [ -d "$DMC_PATH1" ];then
    DMC_PATH=$DMC_PATH1
elif [ -d $DMC_PATH2 ];then
    DMC_PATH=$DMC_PATH2
else
    echo "non-existent dmc path,please check if enable dmc"
    exit
fi

echo "DMC_PATH:"$DMC_PATH

if [ "$#" -eq "1" ];then
    echo userspace > $DMC_PATH/governor
    echo $1 > $DMC_PATH/userspace/set_freq
    val=$(cat $DMC_PATH/cur_freq)
    echo "already change to" $val"Hz done."
    array=$(cat $DMC_PATH/available_frequencies)
    let j=${#array[@]}-1
    if [ "$val" -eq "${array[j]}" ];then
        echo "change frequency to available max frequency done."
    else
        echo "!!!warning!!!"
        echo "!!!warning!!! available max frequency is" ${array[j]}"Hz"
	echo "!!!warning!!! please check frequency" $val"Hz if you need."
        echo "!!!warning!!!"
    fi
    exit
else
    unset FREQS
    read -a FREQS < $DMC_PATH/available_frequencies

    RANDOM=$$$(date +%s)
    while true; do
      echo userspace > $DMC_PATH/governor
      FREQ=${FREQS[$RANDOM % ${#FREQS[@]} ]}
      echo "set ddr frequency to $FREQ"
      echo ${FREQ} > $DMC_PATH/userspace/set_freq
    done
fi
