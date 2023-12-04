#!/bin/bash

device=$1_
                        
case "$1" in                   
        hw:0,0)                
          device=hw_0_0_       
          ;;                   
        hw:1,0)                
          device=hw_1_0_       
          ;;                   
        hw:7,0,0)                                                                                                  
          device=hw_7_0_0_                                                                                         
          ;;                                                                                                       
        hw:7,1,0)                                                                                                  
          device=hw_7_1_0_                                                                                         
          ;;                                                                                                       
esac        

mkdir /tmp/ChangeVolume

rm /tmp/ChangeVolume/$device$2.wav
gst-play-1.0 ./Track01.mp3&
arecord -D $1 -c $2 -r 16000 -d 10000  --period-size 128 --buffer-size 65536 -f S16_LE /tmp/ChangeVolume/$device$2.wav&
for((i=0;i<=50;i++));  
do   
k=$(expr $i \* 2); 
echo k=$k;
tinymix set "Master Playback Volume" $k
j=$(expr $i % 2 \* 3);
echo j=$j;
tinymix set "ADC MIC Group 0 Left Volume" $j
tinymix set "ADC MIC Group 0 Right Volume" $j
tinymix set "ADC MIC Group 1 Left Volume" $j
tinymix set "ADC MIC Group 1 Right Volume" $j
tinymix set "ADC MIC Group 2 Left Volume" $j
tinymix set "ADC MIC Group 2 Right Volume" $j
tinymix set "ADC MIC Group 3 Left Volume" $j
tinymix set "ADC MIC Group 3 Right Volume" $j

z=$(expr $i % 31); 
echo z=$z;     
tinymix set "ADC ALC Group 0 Left Volume" $z 
tinymix set "ADC ALC Group 0 Right Volume" $z
tinymix set "ADC ALC Group 1 Left Volume" $z
tinymix set "ADC ALC Group 1 Right Volume" $z
tinymix set "ADC ALC Group 2 Left Volume" $z
tinymix set "ADC ALC Group 2 Right Volume" $z
tinymix set "ADC ALC Group 3 Left Volume" $z
tinymix set "ADC ALC Group 3 Right Volume" $z
sleep 1
done

killall gst-play-1.0
killall arecord
   
