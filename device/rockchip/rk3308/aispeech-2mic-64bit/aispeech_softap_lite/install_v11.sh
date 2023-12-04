#!/bin/sh

#this script is used for rk3308 V11 board

if [ "$#" -ne 1 ]; then
    echo  "usage:\n\tinstall.sh board_version"; exit 1;
fi

if [ "$1" != "v11" ]; then
    echo "this script is used for rk3308 V11 board, please check whether your boadr version is rk3308_v11"; exit 1;
fi

dir=`pwd`

for item in `ls bin`; do
    chmod +x ${dir}/bin/${item}
done

#hardware-related configuration
board_startup_script=/data/RkLunch.sh

rm -rf ${board_startup_script}

#wpa configuration file
aispeech_wifi_cfg=/data/cfg/wpa_supplicant.conf

rm -rf ${aispeech_wifi_cfg}

content="arecord -D vad -c 8 -r 16000 -f S16_LE -d 1 -t raw /tmp/test.pcm
rm /tmp/test.pcm                                                
echo 0x60 0x40ff0040 > /sys/kernel/debug/vad/reg
echo 0x5c 0x000e2080 > /sys/kernel/debug/vad/reg

export  AISPEECH_WIFI_CFG=\"${aispeech_wifi_cfg}\"

#aispeech dui app information file
export  AISPEECH_DUIKIT_APP=\"${dir}/device/app.json\"

#aispeech dui device file
export  AISPEECH_DUIKIT_DEVICE=\"${dir}/device/device.json\"

#aispeech dui softap web server address
export  AISPEECH_SOFTAP_SERVER_PORT=\"8000\"

#aispeech dui softap configuration folder
export  AISPEECH_SOFTAP_DIR=\"/data/cfg\"

export  AISPEECH_DO_CONNECT_MP3=\"${dir}/audio/do_connect.mp3\"

export  AISPEECH_WIFI_OK_MP3=\"${dir}/audio/wifi_ok.mp3\"

export  AISPEECH_NEED_CONNECT_MP3=\"${dir}/audio/need_connect.mp3\"

export  AISPEECH_CONNECT_OK_MP3=\"${dir}/audio/connect_ok.mp3\"

export  AISPEECH_START_CONNECT_MP3=\"${dir}/audio/start_connect.mp3\"

export  PATH=${PATH}:${dir}/bin

if [ -f \${AISPEECH_WIFI_CFG} ]; then
    aispeech_player \${AISPEECH_WIFI_OK_MP3}
    aispeech_led  -m on 6
    wpa_supplicant -B -i wlan0 -c \${AISPEECH_WIFI_CFG} &
    dhcpcd &
    aispeech_player \${AISPEECH_DO_CONNECT_MP3}
else
    aispeech_player \${AISPEECH_NEED_CONNECT_MP3} &
    aispeech_led -m breath 1 -s 500
    ifconfig wlan0 down 
    aispeech_softap_server -s aiengine -p 12345678 start &
fi
aispeech_startup &
"
echo "${content}" > ${board_startup_script}

