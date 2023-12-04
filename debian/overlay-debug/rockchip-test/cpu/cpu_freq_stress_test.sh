#!/bin/bash

usage()
{
echo "Usage: cpu_freq_test.sh [test_second] [every_freq_stay_second]"
echo "example: cpu_freq_test.sh  3600 30"
echo "means cpu_freq_test.sh will run 1 hour and every cpu frequency stay 10s"
}

echo "test will run $1 seconds"
echo "every cpu frqeucny will stay $2 seconds"

#disalbe thermal
if [ -e /sys/class/thermal/thermal_zone0 ]; then
  echo user_space >/sys/class/thermal/thermal_zone0/policy
fi

if [ -e /sys/class/thermal/thermal_zone1 ]; then
  echo user_space > /sys/class/thermal/thermal_zone1/policy
fi

#caculate how many cpu core
cpu_cnt=`cat /proc/cpuinfo | grep processor | sort | uniq | wc -l`

stressapptest -s $1 --pause_delay 10 --pause_duration 1 -W --stop_on_errors -M 128&

RANDOM=$$$(date +%s)
time_cnt=0

while true; do
  if [ $time_cnt -ge $1 ]
  then
    echo "======TEST SUCCESSFUL, QUIT====="
    exit 0
  fi
  for d in /sys/devices/system/cpu/cpufreq/*; do
    echo userspace > $d/scaling_governor

    read -a FREQS < $d/scaling_available_frequencies
    FREQ=${FREQS[$RANDOM % ${#FREQS[@]} ]}
    echo Set $d freq to ${FREQ}
    echo ${FREQ} > $d/scaling_setspeed
    cur_freq=`cat $d/scaling_cur_freq`
    echo Get "***$d freq $cur_freq***"
  done
  sleep $2
  let "time_cnt=$time_cnt+$2"
done
