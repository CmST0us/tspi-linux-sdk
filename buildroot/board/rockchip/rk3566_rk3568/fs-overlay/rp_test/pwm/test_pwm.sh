#!/bin/bash

echo "============================"
echo "PWM test"
echo "============================"

cd /sys/class/pwm

period=10000
duty_cycle=0
duty_cycle_step=100
polarity=normal
available_pwm=()

echo "All pwm channel parameter:"
echo "period: $period"
echo "duty_cycle: 0-10000"
echo "duty_cycle_step: $duty_cycle_step"
echo "polarity: $polarity"

echo "Available pwm channels:"

for pwmchannel in `ls -d pwm*`; do
    if [ ! -d "$pwmchannel/pwm0" ]; then
        echo 0 > ${pwmchannel}/export 2>/dev/null
        if [ ! -d "$pwmchannel/pwm0" ];then
            echo -e "\t$pwmchannel: is busy, maybe is backlight pwm or occupied by others"
            continue
        fi
    fi
    echo -e "\t$pwmchannel:" `ls -l $pwmchannel | awk '{print $11}' | xargs -n 1 readlink -f`

    echo "$period" > "$pwmchannel/pwm0/period"
    echo "$duty_cycle" > "$pwmchannel/pwm0/duty_cycle"
    echo "$polarity" > "$pwmchannel/pwm0/polarity"
    echo 1 > "$pwmchannel/pwm0/enable"
    available_pwm+=("$pwmchannel")
done
echo =================================

if [ ${#available_pwm[@]} = 0 ];then
    echo "No available pwm found,exit!"
    exit 1
fi

while true; do
    sleep 0.1s

    if [ $duty_cycle -le 10 ]; then
        increase=true
    elif [ $duty_cycle -ge 9990 ]; then
        increase=false
    fi

    if $increase; then
        duty_cycle=$((duty_cycle + $duty_cycle_step))
    else
        duty_cycle=$((duty_cycle - $duty_cycle_step))
    fi

    for pwm in "${available_pwm[@]}"; do
        echo "$duty_cycle" > "$pwm/pwm0/duty_cycle"
        echo -ne "Set $pwm duty_cycle: $duty_cycle \r"
    done
done

