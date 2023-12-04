ssid=
password=
encrypt=psk
mode="station"
config_file="/data/wifi_configure.txt"
router_ip="192.168.168.1"
router_connted="no"
ap_ip="192.168.2.1"
ping_period="6"
retry="1"
onoff_test="0"
debug="1"
module_pid_file="/sys/bus/mmc/devices/mmc1:0001/mmc1:0001:1/device"
init_config=0
ERROR_FLAG=0
wifi_results_file=/data/wifi_results.txt
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
    esac
    if [ $? -eq 0 ];then
        return
    else
        cnt=$((cnt + 1))
        sleep 1
        continue
    fi
done
##return here if no matter###
if [ "$2" = "check_eth" ];then
	return
fi

echo "fail!!"
ERROR_FLAG=1
end_script
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
}

function wifi_bcm_init()
{

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
onoff_test=0
retry=5
debug=1
"  > /data/wifi_configure.txt
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
onoff_test=0
retry=5
debug=1
"  > /data/wifi_configure.txt
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
	let wifi_results_fail+=1
else

	echo "ping successfully"
	router_connted="yes"
	dmesg > /data/wifi_reboot_ok.txt

	let wifi_results_ok+=1
fi

echo "wifi_ok=$wifi_results_ok
wifi_fail=$wifi_results_fail
" > $wifi_results_file
}

############start wpa_supplicant##########
function start_sta() {
echo "starting wpa_supplicant..."
ifconfig wlan0 0.0.0.0

mkdir /data/wpa_supplicant
echo "ctrl_interface=/data/wpa_supplicant
ap_scan=1

network={
	ssid=\"$ssid\"
	psk=\"$password\"
	key_mgmt=WPA-PSK
}
" > /data/wpa_supplicant.conf

if [ $debug -eq 1 ];then
	echo "ctrl_interface=/var/run/wpa_supplicant"
	/sbin/start-stop-daemon -S -m -p $PIDFILE1  -x $DAEMON1 -- -Dnl80211 -iwlan0 -c/data/wpa_supplicant.conf &
else
	echo "ctrl_interface=/var/run/wpa_supplicant debug0"
	/sbin/start-stop-daemon -S -m -p $PIDFILE1 -b -x $DAEMON1 -- -Dnl80211 -iwlan0 -c/data/wpa_supplicant.conf
fi

#check_in_loop 2 check_wpa
#check_in_loop 2 check_ap_connect
echo "start wpa_supplicant successfully!!"

sleep 2

############start dhcp#######################
echo "starting wifi dhcp..."
/sbin/dhcpcd wlan0
echo "ap connected!!"
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

function main() {
###########show usage first#####################
usage
initial_configure
stop_wifi
start_wifi

}

wifi_bcm_init
main
