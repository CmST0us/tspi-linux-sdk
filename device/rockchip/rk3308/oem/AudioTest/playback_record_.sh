#!/bin/bash

#example:./amic-loopback-no-vad.sh 16000(record rate) S16_LE(record format) hw:0,0(playback hardward)
#							 7(total times) /tmp/playback-sine-1khz-48k-16bit-2ch.wav(playback file)
rm /tmp/playback_record/ -rf
mkdir /tmp/playback_record
arecord -D hw:0,0 -c 8 -r $1 -d 10000 -f "$2"  --period-size 1024 --buffer-size 65536 -f S16_LE /tmp/playback_record/record_playback.wav &    
for((i=1;i<=$4 ;i++));  
do
j=$(expr $i % 9 + 1);   
echo $j;
aplay -D $3 $5 -d $j &
if [ $j -eq 5 ]
then
	killall arecord 
	killall aplay
	sleep 3
	aplay -D $3 $5 -d 100 &
	sleep 1
	echo "restart record..."
	arecord -D hw:0,0 -c 8 -r $1 -d 10000 -f "$2"  --period-size 1024 --buffer-size 65536 -f S16_LE /tmp/playback_record/playback_record.wav &
fi
sleep $j
killall aplay
sleep 1
done 
echo "end record..." 
killall arecord 