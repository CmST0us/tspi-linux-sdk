#0=0db, 1=6.6db, 2=13dB, 3=20db
amixer cset -c 0 name="ADC MIC Group 2 Left Volume" 3
amixer cset -c 0 name="ADC MIC Group 3 Left Volume" 3
amixer cset -c 0 name="ADC MIC Group 2 Right Volume" 3
amixer cset -c 0 name="ADC MIC Group 3 Right Volume" 3

#12=0dB 18=9db
amixer cset -c 0 name="ADC ALC Group 2 Left Volume" 12
amixer cset -c 0 name="ADC ALC Group 3 Left Volume" 12
amixer cset -c 0 name="ADC ALC Group 2 Right Volume" 12
amixer cset -c 0 name="ADC ALC Group 3 Right Volume" 12

#loop back gain 
amixer cset -c 0 name="ADC ALC Group 1 Left Volume" 18

# lineout gain
#0=490mv，1=890mv
amixer cset -c 0 name='DAC LINEOUT Right Volume' 2
amixer cset -c 0 name='DAC LINEOUT Left Volume' 2

amixer cset -c 0 name='DAC HPMIX Left Volume' 1
amixer cset -c 0 name='DAC HPMIX Right Volume' 1

vol=80
if [ -f "/userdata/cfg/volume.conf" ]; then
    vol=`cat /userdata/cfg/volume.conf`
    echo "cat volume.conf $vol"
    if [ -z "$vol" ]; then 
        echo "vol is empty"
        vol=80
    fi

    if [ -n "$vol" ]; then 
        echo "vol is not empty,vol=$vol"
    fi

    if [ $vol -gt 80 ]; then
        echo "vol >80 force vol=80"
        echo 80 > /userdata/cfg/volume.conf
        vol=80
    fi
else
    echo "no volume.conf file"
    echo 80 > /userdata/cfg/volume.conf
fi

echo "===!!!vol=$vol"
amixer set  Master Playback $vol