while [ 1 ]
do
        if ! pidof gatt-service; then
                break
        fi
        sleep 1
done

if pidof bluetoothd; then
        killall bluetoothd
fi
if pidof bluealsa; then
        killall bluealsa
fi

sleep 1
case "$1" in
        0)
	    hciconfig hci0 down
	    ;;
	1)
	    /usr/libexec/bluetooth/bluetoothd --compat -n &
            sdptool add A2SNK
            sleep 2
            hciconfig hci0 up
            sleep 1
            hciconfig hci0 piscan
            sleep 1
            hciconfig hci0 class 0x240404
            hciconfig hci0 down
            hciconfig hci0 up
            bluealsa --profile=a2dp-sink &
            sleep 1
            bluealsa-aplay --profile-a2dp 00:00:00:00:00:00 &
	    ;;
	*)
esac
exit 0
