#! /bin/sh
#/pcba/acodec-gain.sh &
/data/echo_pcbatest_server &
#/data/echo_laohua_test &
aplay /userdata/Mute.wav &
sleep 1
/data/acodec-gain.sh &
sleep 1


