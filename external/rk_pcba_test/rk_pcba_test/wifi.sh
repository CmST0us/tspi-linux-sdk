#!/sbin/busybox sh

module_path_8188eu=/res/8188eu.ko
module_path_8192cu=/res/8192cu.ko
module_path_rk903=/res/rkwifi.ko
module_path_rt5370=/res/rt5370sta.ko
module_path_mt7601=/res/mt7601sta.ko
module_path_mtPrealloc7601=/res/mtprealloc7601Usta.ko
module_path_8723au=/res/8723au.ko
module_path_8723as=/res/8723as.ko
module_path_wlan=/res/wlan.ko
module_path_esp8089=/system/lib/modules/esp8089.ko
result_file=/tmp/scan_result.txt
result_file2=/tmp/scan_result2.txt
chip_type_path=/sys/class/rkwifi/chip
driver_node=/sys/class/rkwifi/driver
pcba_node=/sys/class/rkwifi/pcba
version_path=/proc/version
module_path=$module_path_wlan
chip_broadcom=false
driver_buildin=false
interface_up=true
version=.3.0.36+
mt5931_kitkat=false
android_kitkat=false

jmax=3

if busybox cat $chip_type_path | busybox grep RK903; then
  module_path=$module_path_rk903
  chip_broadcom=true
  echo 1 > $pcba_node
fi

if busybox cat $chip_type_path | busybox grep RK901; then
  module_path=$module_path_rk903
  chip_broadcom=true
  echo 1 > $pcba_node
fi

if busybox cat $chip_type_path | busybox grep BCM4330; then
  module_path=$module_path_rk903
  chip_broadcom=true
  echo 1 > $pcba_node
fi

if busybox cat $chip_type_path | busybox grep RTL8188CU; then
  jmax=6
  module_path=$module_path_8192cu
fi

if busybox cat $chip_type_path | busybox grep RTL8188EU; then
  jmax=6
  module_path=$module_path_8188eu
fi

if busybox cat $chip_type_path | busybox grep RT5370; then
  jmax=6
  module_path=$module_path_rt5370
fi

if busybox cat $chip_type_path | busybox grep MT7601; then
  jmax=6
  module_path=$module_path_mt7601
  echo "mt7601 insmod pre-alloc driver & copy firmware"
  insmod "$module_path_mtPrealloc7601"
  busybox cp /system/etc/firmware/RT2870STA.dat /etc/firmware/
  #interface_up=false
fi

if busybox cat $chip_type_path | busybox grep RTL8723AU; then
  module_path=$module_path_8723au
fi

if busybox cat $chip_type_path | busybox grep RTL8723AS; then
  module_path=$module_path_8723as
fi  

if busybox cat $chip_type_path | busybox grep ESP8089; then
  module_path=$module_path_esp8089
fi

if busybox cat $version_path | busybox grep 3.0.36+; then
  echo "kernel version 3.0.36+"
  if [ -e $module_path$version ]; then
    module_path=$module_path$version
  fi
fi

if busybox ls /dev/wmtWifi | busybox grep wmtWifi; then
  echo "mt5931_kitkat=true"
  mt5931_kitkat=true
fi

if busybox ifconfig wlan0; then
  echo "android_kitkat=true"
  android_kitkat=true
fi

if busybox ls $driver_node; then
  echo "wifi driver is buildin"
  driver_buildin=true
fi

echo "touch $result_file"
busybox touch $result_file

j=0

echo "get scan results"
while [ $j -lt $jmax ]; 
do
    echo "insmod wifi driver"
    if [ $mt5931_kitkat = "true" ]; then
        echo "echo 1 > /dev/wmtWifi"
        echo 1 > /dev/wmtWifi
    else
      if [ $android_kitkat = "false" ]; then
        if [ $driver_buildin = "true" ]; then
          echo "echo 1 > $driver_node"
          echo 1 > "$driver_node"
        else
          echo "insmod $module_path"
          insmod "$module_path"
        fi
      fi
    fi
    if [ $? -ne 0 ]; then
        echo "insmod failed"
        exit 0
    fi

    echo "sleep 3s"
    busybox sleep 3

    if busybox ifconfig wlan0; then
        if [ $interface_up = "true" ]; then
            busybox ifconfig wlan0 up
        fi
        #if [ $? -ne 0 ]; then
        #    echo "ifconfig wlan0 up failed"
        #    exit 0
        #fi
    
        iwlist wlan0 scanning > $result_file
        if [ $chip_broadcom = "true" ]; then
            echo "sleep 3s"
            busybox sleep 3    
        fi
        iwlist wlan0 scanning last | busybox grep SSID > $result_file
        busybox cat $result_file
        iwlist wlan0 scanning last | busybox grep "Signal level" > $result_file2
        busybox cat $result_file2
        echo "success"
        exit 1
    fi

    echo "remove wifi driver"
    if [ $mt5931_kitkat = "true" ]; then
        echo "echo 0 > /dev/wmtWifi"
        echo 0 > /dev/wmtWifi
    else
      if [ $android_kitkat = "false" ]; then
        if [ $driver_buildin = "true" ]; then
          echo "echo 0 > $driver_node"
          echo 0 > "$driver_node"
        else
          echo "rmmod wlan"
          rmmod wlan
        fi
      fi
    fi
    busybox sleep 1
    
    j=$((j+1))
done

echo "wlan test failed"
exit 0

