#!/bin/bash

availabel_eth=()
exclude_interfaces=("lo" "wlan0" "dummy0")

function print_title()
{
    echo
    echo "============================================"
    echo "$1"
    echo "============================================"
}

print_title "All available ethernet interface"

function get_interface_ip()
{
    ipaddr=$(ip addr show "$1" | awk '/inet / {print $2}' | sed 's/\/[0-9]\+//')
    echo "$ipaddr"
}

function get_all_ethernet_interface()
{
    if_index=0
    for interface in /sys/class/net/*; do
        interface_name=$(basename "$interface")
        if [ -d "$interface" ] && [ "$(cat "$interface/type" 2>/dev/null)" = "1" ] && [[ ! " ${exclude_interfaces[@]} " =~ " ${interface_name} " ]]; then
            echo -n "$if_index $interface_name: "
            ipaddr=$(get_interface_ip $interface_name)
            
            if [ "$ipaddr" == "" ]; then
                echo "No ipaddress"
            else
                echo "$ipaddr"
            fi
            
            availabel_eth+=("$interface_name")
            let if_index++
        fi
    done
}

#$1 interface name
function get_interface_gateway()
{
    ip route show dev $1 | awk '/^default via/ {print $3}'
}

#$1 interface name
function test_interface_netstat()
{
    print_title "Test $1 gateway connect stat"

    gateway=$(get_interface_gateway $1)
    echo "ping -I $1 -c4 $gateway"
    ping -I $1 -c4 $gateway
    
    if [ $? == 0 ];then
        echo "Test $1 gateway connect successful!!"
    else
        echo "Test $1 gateway connect failded!!!"
    fi
}

function test_interface_dns_stat()
{
    print_title "Test $1 domain resolve test"            
    
    echo "ping -I $1 -c4  www.baidu.com"
    ping -I $1 -c4 www.baidu.com

    if [ $? == 0 ];then                                                         
        echo "Test $1 domain resolve successful!!"         
    else                                         
        echo "Test $1 domain resolve failded!!!"           
    fi
}

function test_interface_iperf3()
{
    print_title "Test $1 throughput"

    read -p "Please run \"iperf3 -s\" on server, and input server ip address:" server_ip

    while true; do
        ping -I $1 -c 1 $server_ip > /dev/null
        if [ $? -eq 0 ]; then
            break
        else
            echo "Server ip $server_ip can not be connected, please retry!"
            read -p "Input the server ip address again:" server_ip
        fi
    done
    
    interface_ip=$(get_interface_ip $1)                                                                   
    speed=$(cat /sys/class/net/$1/speed)   
 
    print_title "UDP upload throughput test"
    cmd="iperf3 -c $server_ip -B $interface_ip -u -i 1 -b ${speed}M -t 10"
    echo "$cmd"
    $cmd

    print_title "UDP download throughput test"
    cmd="iperf3 -c $server_ip -B $interface_ip -u -i 1 -b ${speed}M -t 10 -R"
    echo "$cmd"
    $cmd

    print_title "TCP upload throughput test"
    cmd="iperf3 -c $server_ip -B $interface_ip -i 1 -b ${speed}M -t 10"
    echo "$cmd"
    $cmd

    print_title "TCP download throughput test"
    cmd="iperf3 -c $server_ip -B $interface_ip -i 1 -b ${speed}M -t 10 -R"
    echo "$cmd"
    $cmd
    
}

get_all_ethernet_interface

[ ${#availabel_eth[@]} == 0 ] && exit -1

if [ ${#availabel_eth[@]} == 1 ];then
    index=0
else
    read -p "Select interface to test:" index
    
    if ! [[ $index =~ ^[0-9]+$ ]]; then
        echo "Invalid interface selection"
        exit -1
    fi
    index=$(($index))
    if ((index < 0 || index >= ${#availabel_eth[@]})); then
        echo "Invalid input"
        exit -1
    fi
fi

eth_interface=${availabel_eth[$index]}

if [ "$(get_interface_ip $eth_interface)" = "" ];then
    echo "Select interface $eth_interface not has ipaddress, exit!"
    exit
fi

echo "Select interface: $eth_interface"

test_interface_netstat $eth_interface

test_interface_dns_stat $eth_interface

test_interface_iperf3 $eth_interface
