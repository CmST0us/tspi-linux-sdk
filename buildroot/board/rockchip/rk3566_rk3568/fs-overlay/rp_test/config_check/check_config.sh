#!/bin/bash

# ANSI color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

function print_format()
{
    echo "============================"
    echo "$1"
    echo "============================"
}

function check_printf()
{
    if [ "$2" = "PASS" ]; then
        printf "%s %*s${GREEN}$2${NC}\n" "$1" $(($3 - ${#1})) ""
    else
        printf "%s %*s${RED}$2${NC}\n" "$1" $(($3 - ${#1})) ""
    fi
}


command_list=(
    "gdb"
    "bluetoothd"
    "bluetoothctl"
    "hciconfig"
    "create_ap"
    "aplay"
    "dropbear"
    "udhcpc"
    "dhcpd"
    "hostapd"
    "dnsmaq"
    "iptables"
    "wpa_supplicant"
    "wpa_cli"
    "iwlist"
    "iwconfig"
    "ntpd"
    "scp"
    "ftp"
    "telnetd"
    "ffmpeg"
    "stressapptest"
    "memtester"
    "stress"
    "pppd"
    "ntfs-3g"
)

env_list=(
    "QT_QPA_PLATFORM"
)

kernel_config_list=(
    "CONFIG_USB_F_RNDIS"
    "CONFIG_USB_SERIAL_CP210X"
    "CONFIG_USB_SERIAL_CH341"
    "CONFIG_SND_USB_AUDIO"
    "CONFIG_TOUCHSCREEN_USB_COMPOSITE"
    "CONFIG_INPUT_JOYSTICK"
    "CONFIG_USB_NET_DRIVERS"
    "CONFIG_EXFAT_FS"
    "CONFIG_NTFS_FS"
    "CONFIG_NTFS3_FS"
    "CONFIG_NETFILTER"
)

function check_command()
{
    echo
    print_format "Rootfs command check"
    for cmd in ${command_list[@]}; do
        if command -v "$cmd" &> /dev/null; then
            check_printf $cmd PASS 20
        else                                   
            check_printf $cmd FAIL 20
        fi
    done
}

function check_kernel_config()
{
    echo
    print_format "Kernel config check"
    
    if [ ! -e /proc/config.gz ];then
        echo "/proc/config.gz not found, exit!"
        exit
    fi
    for config in ${kernel_config_list[@]}; do
        if zcat /proc/config.gz | grep -wq "${config}=y"; then
            check_printf $config PASS 40
        else                                   
            check_printf $config FAIL 40
        fi
    done
}

function check_env()
{
    echo
    print_format "Environmental variable check"
    
    for env_value in ${env_list[@]}; do
        if env | grep -q "$env_value"; then
            check_printf "$env_value" PASS 20
        else
            check_printf "$env_value" FAIL 20
        fi
    done
}

#rootfs config check
check_command

#env variable check
check_env

#kernel config check
check_kernel_config


