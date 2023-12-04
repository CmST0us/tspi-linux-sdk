#/oem/acodec-gain.sh &
sleep 1
if [ -f "/data/cfg/wpa_supplicant.bk" ]; then 
        cp /data/cfg/wpa_supplicant.bk /data/cfg/wpa_supplicant.conf
fi
wpa_supplicant -B -i wlan0 -c /data/cfg/wpa_supplicant.conf &
#sleep 1

cvlc -q --no-interact --play-and-exit /oem/start.mp3 &

/oem/sayinfoos.sh start &

cd /oem
./bt_loader_firmware.sh &
#sleep 12
sleep 10
#./bt_start.sh &
#/oem/acodec-gain.sh &
if ! pidof wpa_supplicant; then
        cp /etc/wpa_supplicant.conf /data/cfg/wpa_supplicant.conf
        cp /etc/wpa_supplicant.conf /data/cfg/wpa_supplicant.bk
        wpa_supplicant -B -i wlan0 -c /data/cfg/wpa_supplicant.conf &
fi

