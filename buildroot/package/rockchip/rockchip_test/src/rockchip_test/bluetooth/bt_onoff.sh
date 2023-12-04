ERROR_FLAG=0

bt_results_ok=
bt_results_fail=

bt_results_file=/data/bt/bt_results.txt
bt_scan_results_file=/data/bt/bt_scan_results.txt

alias check_hci0="hciconfig | grep hci0"
alias check_scan="hcitool scan > $bt_scan_results_file"

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
        check_hci0)
        check_hci0
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
ERROR_FLAG=1
#end_script
}

function test_bt() {

killall brcm_patchram_plus1
echo 0 > /sys/class/rfkill/rfkill0/state
rm $bt_scan_results_file
touch $bt_scan_results_file

sleep 1
/usr/bin/bt_pcba_test
sleep 2

check_in_loop 10 check_hci0

hciconfig hci0 up

if [ ${ERROR_FLAG} -ne 0 ];then
	let bt_results_fail+=1
	dmesg >> /data/bt/bt_onoff_fail.txt
	ERROR_FLAG=0
else
	let bt_results_ok+=1
	dmesg > /data/bt/bt_onoff_ok.txt
fi

echo "bt_ok=$bt_results_ok
bt_fail=$bt_results_fail
" > $bt_results_file

}

function main2() {
while true; do
	test_bt
	sleep 5
	cnt=$((cnt + 1))
	echo "
	#######################################################
	##### bt has been tuned on/off for $cnt times...
	#####  ok: $bt_results_ok times
	#####  fail: $bt_results_fail times
	#######################################################
	"
done
}

mkdir /data/bt 2>/dev/null
touch /data/bt/bt_results.txt
touch /data/bt/bt_scan_results.txt
main2
