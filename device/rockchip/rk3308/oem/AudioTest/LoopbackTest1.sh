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

mkdir /tmp/loopbacktest1
rm -rf /tmp/loopbacktest1/$device$2
mkdir /tmp/loopbacktest1/$device$2
aplay ./r.wav&
for((i=1;i<=10;i++));  
do
j=$(expr $i % 20 + 1);   
echo $j;
arecord -D $1 -c $2 -r 16000 -d 3  --period-size 1024 --buffer-size 65536 -f S16_LE /tmp/loopbacktest1/$device$2/$j.wav
done
killall aplay  