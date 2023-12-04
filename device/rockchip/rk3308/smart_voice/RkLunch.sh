cd /data/
amixer cset name='Master Volume' 120
amixer cset name='Speaker Volume' 255

aplay /usr/appresources/startup.wav &

cd ..

wpa_supplicant -B -i wlan0 -c /data/wpa_supplicant.conf
#./wakeWordAgent -e gpio &
#echo 1 > /sys/devices/platform/ff050000.i2c/i2c-1/1-0058/mode 
rm /tmp/media_player.ipc
touch /tmp/media_player.ipc
cd /oem
./sai_ledservice &
sleep 2
LD_LIBRARAY_PATH=. ./MediaPlayer_Service &
sleep 3
LD_LIBRARAY_PATH=. ./talking_droid /oem/sai_config /oem/sai_config 955 &
cd ..
