tinymix set "ADC MIC Group 1 Left Volume" 3
tinymix set "ADC MIC Group 2 Left Volume" 3
tinymix set "ADC MIC Group 2 Right Volume" 3

#tinymix set "ADC ALC Group 1 Left Volume" 28
#tinymix set "ADC ALC Group 1 Right Volume" 28
tinymix set "ADC ALC Group 2 Left Volume" 25 
tinymix set "ADC ALC Group 2 Right Volume" 25

arecord -c 4 -r 16000 -f S16_LE -d 1 -t raw /tmp/test.pcm
rm /tmp/test.pcm
echo 0x60 0x40ff0190 > /sys/kernel/debug/vad/reg
echo 0x5c 0x000e2020 > /sys/kernel/debug/vad/reg

wpa_supplicant -B -i wlan0 -c /oem/wpa_supplicant.conf
iflytekDemo &
