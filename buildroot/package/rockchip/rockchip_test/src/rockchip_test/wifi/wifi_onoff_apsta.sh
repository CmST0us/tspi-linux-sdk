ssid=
password=
encrypt=psk
mode="station"
config_file="/data/wifi/wifi_configure.txt"
router_ip="192.168.168.1"
router_connted="no"
ap_ip="192.168.2.1"
ping_period="6"
retry="1"
onoff_test="1"
debug="1"
module_pid_file="/sys/bus/mmc/devices/mmc1:0001/mmc1:0001:1/device"
init_config=0
ERROR_FLAG=0
wifi_results_file="/data/wifi/wifi_results.txt"
wifi_scan_results_file="/data/wifi/wifi_scan_results.txt"

wifi_results_ok=
wifi_results_fail=

wpa_pid=
dhcpcd_pid=

NAME1=wpa_supplicant
DAEMON1=/usr/sbin/$NAME1
PIDFILE1=/var/run/$NAME1.pid

NAME2=hostapd
DAEMON2=/usr/sbin/$NAME2
PIDFILE2=/var/run/$NAME2.pid

NAME3=dnsmasq
DAEMON3=/usr/sbin/$NAME3
PIDFILE3=/var/run/$NAME3.pid

NAME4=dhcpcd
DAEMON4=/usr/sbin/$NAME4
PIDFILE4=/var/run/${NAME4}-wlan0.pid

alias check_wlan="ifconfig wlan0 2> /dev/null"
alias check_wpa="wpa_cli ping 2> /dev/null | grep PONG"
alias check_ap_connect="wpa_cli status 2> /dev/null | grep state=COMPLETED"
alias check_hostapd="hostapd_cli status 2> /dev/null | grep state=ENABLED"
alias check_dnsmasq="ps | grep -v grep | grep dnsmasq > /dev/null"
alias check_scan="iw wlan0 scan | grep SSID > $wifi_scan_results_file"

function end_script() {
if [ ${ERROR_FLAG} -ne 0 ]
then
    exit 11
else
    exit
fi
}

function check_in_loop() {
local cnt=1
while [ $cnt -lt $1 ]; do
    echo "check_in_loop processing..."
    case "$2" in
        check_wlan)
        check_wlan
        ;;
        check_hostapd)
        check_hostapd
        ;;
        check_dnsmasq)
        check_dnsmasq
        ;;
        check_wpa)
        check_wpa
        ;;
        check_ap_connect)
        check_ap_connect
        ;;
		check_scan)
		check_scan
        ;;
    esac
    if [ $? -eq 0 ];then
        return
    else
        cnt=$((cnt + 1))
        sleep 1
        continue
    fi
done

echo "check_in_loop fail!!"
#ERROR_FLAG=1
#end_script
}

function load_driver() {
if [ $1 = "0" ];then
	echo "removing driver if loaded"
else
	echo "start driver loading..."
	##########check wlan0############################
	echo "checking wlan0..."
	check_in_loop 15 check_wlan
	echo "wlan0 shows up
	"
fi
}

function stop_wifi() {

load_driver 0

echo "#########stoping wifi#####################"
#####stop wpa_supplicant hostapd dhcpcd dnsamas##
echo "Stopp prv wpa_supplicant first"
wpa_pid=`ps | grep wpa_supplicant | awk '{print $1}' | sed -n "1p"`
echo "wpa_pid: $wpa_pid"
kill $wpa_pid
sleep 1
echo "Stopp prv dhcpcd first"
dhcpcd_pid=`ps | grep dhcpcd | awk '{print $1}' | sed -n "1p"`
echo "dhcpcd_pid: $dhcpcd_pid"
kill $dhcpcd_pid
sleep 1
ifconfig wlan0 down
sleep 1
}

function wifi_bcm_init()
{

	echo 1 > /sys/class/rfkill/rfkill1/state
	if [ ! -f ${module_pid_file} ];then
		ifconfig wlan0 up
		sleep 2
	fi
}

function initial_configure() {
if [ -f $config_file ];then
########load from txt##################
echo "reading from txt...."
	while read line ; do
	key=`echo $line | awk -F "=" '{print $1}'`
	val=`echo $line | awk -F "=" '{print $2}'`
    case "$key" in
		ssid)
		ssid=$val
		;;
		password)
		password=$val
		;;
		encrypt)
		encrypt=$val
		;;
		driver)
		driver=$val
		;;
		mode)
		mode=$val
		;;
		debug)
		debug=$val
		;;
		ping_period)
		ping_period=$val
		;;
		retry)
		retry=$val
		;;
        onoff_test)
		onoff_test=$val
		;;
	esac
	done < $config_file
	
	if [ "$ssid" = "" ];then
		init_config=1
		read -t 100 -p "Input ssid: " ssid
		if [ "$ssid" = "" ];then
				echo "wifi ssid not empty!!!"
				end_script
		else
				echo "wifi ssid: $ssid"
		fi
	fi

	if [ "$password" = "" ];then
		read -t 100 -p "Input password: " password
		if [ "$password" = "" ];then
				echo "wifi password not empty!!!"
				end_script
		else
				echo "wifi password: $password"
		fi
	fi

	if [ $init_config ]; then
echo "ssid=$ssid
password=$password
encrypt=psk
mode=station
onoff_test=1
retry=5
debug=1
"  > /data/wifi/wifi_configure.txt
	fi
	
else
	if [ "$ssid" = "" ];then
		init_config=1
		read -t 100 -p "Input ssid: " ssid
		if [ "$ssid" = "" ];then
				echo "wifi ssid not empty!!!"
				end_script
		else
				echo "wifi ssid: $ssid"
		fi
	fi

	if [ "$password" = "" ];then
		read -t 100 -p "Input password: " password
		if [ "$password" = "" ];then
				echo "wifi password not empty!!!"
				end_script
		else
				echo "wifi password: $password"
		fi
	fi

	if [ $init_config ]; then
		echo "ssid=$ssid
password=$password
encrypt=psk
mode=station
onoff_test=1
retry=5
debug=1
"  > /data/wifi/wifi_configure.txt
	fi
fi
echo "user set:
ssid=$ssid, key=$password, mode=$mode debug=$debug
4s to check your configure
"
if [ "`echo $password |wc -L`" -lt "8" ];then
echo "waring: password lentgh is less than 8, it is not fit for WPA-PSK"
fi

}

function usage() {
echo "
##################################################################
#usage:                                                          
#first choice:
#   write configure in /data/wifi/wifi_configure.txt
#second choice:
#   $0  \"ssid\" \"key\" \"mode\"                             
#   example:$0 $ssid $password $mode                               
#   version:1.4
##################################################################
"
}

function ping_test() {
if [ ! -f $wifi_results_file ];then
echo "
wifi_ok=0
wifi_fail=0
" > $wifi_results_file
fi

########load from txt##################
echo "reading from txt...."
while read line ; do
	key=`echo $line | awk -F "=" '{print $1}'`
	val=`echo $line | awk -F "=" '{print $2}'`
	echo $key $val
	case "$key" in
		wifi_ok)
		wifi_results_ok=$val
		;;
		wifi_fail)
		wifi_results_fail=$val
		;;
	esac
done < $wifi_results_file

#router_ip=`ifconfig wlan0 | grep "inet addr" | awk -F " " '{print $2}' | sed "s/addr://g"`
router_ip=`route -n | awk '{print $2}' | sed -n "3p"`
echo "
now going to ping router's ip: $router_ip for $ping_period seconds"
ping $router_ip -w $ping_period

if [ $? -eq 1 ];then
	echo "ping fail!! please check"
	echo "======================kernel log=========================" > /data/wifi/wifi_reboot_fail.txt
	dmesg >> /data/wifi/wifi_reboot_fail.txt
	echo "======================wpa_supplicant_fail log============" > /data/wifi/wpa_supplicant_fail.log
	cat /data/wifi/wpa_supplicant.log > /data/wifi/wpa_supplicant_fail.log
	rm /data/wifi/wpa_supplicant.log

	let wifi_results_fail+=1
	ERROR_FLAG=1
else

	echo "ping successfully"
	router_connted="yes"
	dmesg > /data/wifi/wifi_reboot_ok.txt

	let wifi_results_ok+=1
fi

echo "wifi_ok=$wifi_results_ok
wifi_fail=$wifi_results_fail
" > $wifi_results_file
}

############start wpa_supplicant##########
function start_sta() {
ifconfig wlan0 up
sleep 3
echo "starting wpa_supplicant..."
ifconfig wlan0 0.0.0.0

echo "ctrl_interface=/var/run/wpa_supplicant
ap_scan=1

network={
	ssid=\"$ssid\"
	psk=\"$password\"
	key_mgmt=WPA-PSK
}
" > /data/cfg/wpa_supplicant.conf

if [ $debug -eq 1 ];then
	echo "ctrl_interface=/var/run/wpa_supplicant"
	/sbin/start-stop-daemon -S -m -p $PIDFILE1  -x $DAEMON1 -- -Dnl80211 -iwlan0 -c/data/cfg/wpa_supplicant.conf -ddddddddd > /data/wifi/wpa_supplicant.log &
else
	echo "ctrl_interface=/var/run/wpa_supplicant debug0"
	#/sbin/start-stop-daemon -S -m -p /usr/bin/wpa_supplicant -b -x /var/run/wpa_supplicant.pid -- -Dnl80211 -iwlan0 -c/data/wpa_supplicant.conf
	/sbin/start-stop-daemon -S -m -p $PIDFILE1 -b -x $DAEMON1 -- -Dnl80211 -iwlan0 -c/data/cfg/wpa_supplicant.conf
fi

sleep 2

############start dhcp#######################
echo "starting wifi dhcp..."
/sbin/dhcpcd wlan0
echo "ap connected!!"

check_in_loop 6 check_wpa
check_in_loop 6 check_ap_connect
echo "start wpa_supplicant successfully!!"
}

function start_wifi() {

echo "########starting wifi#####################"
###############load wifi driver##################
load_driver 1
#####start wpa_supplicant hostapd dhcpcd dnsamasq bridge##

if [ "${mode}" = "station" ]; then
	start_sta
	ping_test wlan0
elif [ "${mode}" = "ap" ]; then
	#start_ap
	#start_dnsmasq
    echo "ap is started!!"
else
	echo "bad mode!"
	end_script
fi

}

function wifi_onoff_loop() {
echo "
#####################################################
#####begin to turn on/off wifi
#####################################################
"
sleep 1
local cnt=0
while true; do
	start_wifi
	sleep 2
	stop_wifi
	sleep 5
	cnt=$((cnt + 1))
	echo "
	#####################################################
	#####wifi has been tuned on/off for $cnt times...
	#####################################################
	"
    if [ ${ERROR_FLAG} -ne 0 ]
    then
        exit $cnt
    fi
done
echo "wifi on/off test passed!!"
end_script
}

function main() {
###########show usage first#####################
usage
initial_configure
stop_wifi

if [ $onoff_test -eq 1 ]; then
##############wifi on/off loop begin#############
	wifi_onoff_loop
else
########start station or ap #####################
	start_wifi
    #end_script
fi

}

function test_wifi() {

rm $wifi_scan_results_file
touch $wifi_scan_results_file

echo "ifconfig wlan0 down ..."
ifconfig wlan0 down
sleep 2
echo "ifconfig wlan0 up ..."
ifconfig wlan0 up
sleep 2

echo "checking wlan0..."
check_in_loop 15 check_wlan
echo "wlan0 shows up"

echo "checking wlan0 scan ..."
check_in_loop 15 check_scan

wifi_ssid=`cat $wifi_scan_results_file | sed -n "1p" | cut -f 2 -d :`
echo "wifi ssid: $wifi_ssid"

if [ $wifi_ssid -eq 0 ];then
	echo "wifi failed !!!!!"
	dmesg >> /data/wifi/wifi_onoff_fail.txt

	let wifi_results_fail+=1
else
	echo "wifi successfully"
	router_connted="yes"
	dmesg > /data/wifi/wifi_onoff_ok.txt

	let wifi_results_ok+=1
fi

echo "wifi_ok=$wifi_results_ok
wifi_fail=$wifi_results_fail
" > $wifi_results_file

}

function main2() {
while true; do
	test_wifi
	sleep 5
	cnt=$((cnt + 1))
	echo "
	#######################################################
	##### wifi has been tuned on/off for $cnt times...
	#####  ok: $wifi_results_ok times
	#####  fail: $wifi_results_fail times
	#######################################################
	"
done
}

wifi_bcm_init
mkdir /data/wifi/ 2>/dev/null
touch /data/wifi/wifi_results.txt
touch /data/wifi/wifi_scan_results.txt
main2
