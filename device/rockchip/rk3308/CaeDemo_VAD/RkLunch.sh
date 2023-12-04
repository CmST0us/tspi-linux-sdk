#cd /data
#wpa_supplicant -B -i wlan0 -c /data/cfg/wpa_supplicant.conf
#kill input-event-daemon
#input-event-daemon -v -c /data/input-event-daemon.conf

#echo 0x60 0x00ff0050 > /sys/kernel/debug/vad/reg
#echo 5 > /sys/class/gpio/export
#echo out > /sys/class/gpio/gpio5/direction
#echo 0> /sys/class/gpio/gpio5/value
#sleep 1
#echo 0 > /sys/class/gpio/gpio5/value

echo 0x60 0x40ff0168 > /sys/kernel/debug/vad/reg
echo 0x5c 0x000e2200 > /sys/kernel/debug/vad/reg
#echo 0x64 0x365fe663> /sys/kernel/debug/vad/reg
#echo 0x68 0x365f9343> /sys/kernel/debug/vad/reg
#echo 0x6c 0x2e3394b8> /sys/kernel/debug/vad/reg
ifconfig wlan0 down

ln -s /oem/* /data -f
cd /data && ./cae_sample &
