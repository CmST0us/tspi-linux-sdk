/usr/libexec/bluetooth/bluetoothd --compat -n -d&
#sleep 1
sdptool add A2SNK
#sleep 1
hciconfig hci0 up
sleep 1
hciconfig hci0 piscan
sleep 1
hciconfig hci0 class 0x240404
#sleep 1
#hciconfig hci0 name 'sayinfo_bt'
#sleep 1
hciconfig hci0 down
#sleep 2
hciconfig hci0 up
#sleep 2
bluealsa --profile=a2dp-sink & 
sleep 1
bluealsa-aplay --profile-a2dp 00:00:00:00:00:00 &
