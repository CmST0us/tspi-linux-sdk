#!/bin/bash

echo =====================================
echo WIFI STA model test
echo =====================================

echo "All available wireless interface:"
cd /sys/class/net/

interfaces=()
index=0
wlan_node=""

for node in `ls -d *`;do
    if [ -e $node/wireless ];then
        echo $index: "$node"
        interfaces+=("$node")
        let index++

    fi
done

#echo ${interfaces[@]}

if [ ${#interfaces[@]} == 0 ];then
    echo "Not found any availabel wireless interface"
    exit 1
fi

if [ ${#interfaces[@]} == 1 ];then
    wlan_node=${interfaces[0]}
else
    read -p "Select wireless interface to test: " wlan_index
    if echo $wlan_index | grep -vq [^0-9]; then
        if ((wlan_index >= 0 && wlan_index < ${#interfaces[@]})); then
            wlan_node=${interfaces[${wlan_index}]}
        else
            echo "Invalid wireless interface selection"
        fi
    else
        echo "Invalid input"
    fi
fi

echo "Select wireless interface: $wlan_node"
intf_state=`cat /sys/class/net/wlan0/carrier`

if [ ! "$intf_state" = "1" ];then
    ifconfig $wlan_node up
fi

scan_result=$(iwlist $wlan_node scan)

ssids=$(echo "$scan_result" | grep -o 'ESSID:"[^"]\+"' | cut -d '"' -f 2)
frequencies=$(echo "$scan_result" | grep -o 'Frequency:[0-9.]\+ GHz' | cut -d ':' -f 2)

#get ssid max length
max_length=$(echo "$ssids" | awk '{ if (length > max) { max = length } } END { print max }')

num_results=$(wc -l <<< "$ssids")

echo =====================================
echo All available ssid and frequency
echo =====================================
for ((i=1; i<=num_results; i++)); do
    ssid=$(awk "NR==$i" <<< "$ssids")
    frequency=$(awk "NR==$i" <<< "$frequencies")
    [ "$frequency" == "" ] && continue
    [ "$ssid" == "" ] && continue
    printf "SSID: %-${max_length}s  Frequency: %s\n" "$ssid" "$frequency"
done

if [ $i -lt 3 ]; then
    echo "Not fount any available ssid!"
    exit
fi

if [ $# -lt 2 ];then
    read -p "Enter your wifi-ssid, please: " WIFISSID
    read -p "Enter your wifi-pwssword, please: " WIFIPWD
    if [ ${#WIFIPWD} -lt 8 ];then
        echo "Input length of password to short, must >= 8"
        exit 1
    fi
else
    WIFISSID=$1
    WIFIPWD=$2
fi

CONF=/tmp/wpa_supplicant.conf

cp /rp_test/wifi/wpa_supplicant.conf /tmp/
echo "connect to WiFi ssid: $WIFISSID, Passwd: $WIFIPWD"

if command -v "wpa_passphrase" > /dev/null;then
    wpa_passphrase $WIFISSID $WIFIPWD > $CONF
else
    sed -i "s/SSID/$WIFISSID/g" $CONF
    sed -i "s/PASSWORD/$WIFIPWD/g" $CONF
fi

killall wpa_supplicant
sleep 1
wpa_supplicant -B -i $wlan_node -c $CONF

# auto get ipaddress
timeout 5s udhcpc -i $wlan_node

ifconfig $wlan_node
