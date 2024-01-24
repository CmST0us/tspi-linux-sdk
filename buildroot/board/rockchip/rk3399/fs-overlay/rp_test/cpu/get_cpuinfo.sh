#!/bin/bash

echo ===================================
echo Number of CPU cores
echo ===================================

cat /proc/cpuinfo | grep -c processor

cd /sys/class/thermal/

echo
echo ===================================
echo SOC temperature
echo ===================================

for zone in `ls -d thermal_zone*`; do
    temp=$(cat "$zone/temp")
    trip_point_0_temp=$(cat "$zone/trip_point_0_temp" 2>/dev/null)
    trip_point_1_temp=$(cat "$zone/trip_point_1_temp" 2>/dev/null)
    trip_point_2_temp=$(cat "$zone/trip_point_2_temp" 2>/dev/null)

    echo "$zone: $temp"

    [ -n "$trip_point_0_temp" ] && echo "    trip_point_0_temp: $trip_point_0_temp"
    [ -n "$trip_point_1_temp" ] && echo "    trip_point_1_temp: $trip_point_1_temp"
    [ -n "$trip_point_2_temp" ] && echo "    trip_point_2_temp: $trip_point_2_temp"
done

echo
echo ===================================
echo CPU frequency
echo ===================================

cd /sys/devices/system/cpu/

for cpu in `ls -d cpu*`; do
    if [ -d "$cpu/cpufreq" ]; then
        cur_freq=$(cat "$cpu/cpufreq/cpuinfo_cur_freq")
        max_freq=$(cat "$cpu/cpufreq/cpuinfo_max_freq")
        min_freq=$(cat "$cpu/cpufreq/cpuinfo_min_freq")
        available_freq=$(cat "$cpu/cpufreq/scaling_available_frequencies")

        printf "CPU: %s, Current freq: %7s, Max freq: %7s, Min freq: %7s, Available freq: %s\n" \
            "$(basename "$cpu")" "$cur_freq" "$max_freq" "$min_freq" "$available_freq"
    fi
done

