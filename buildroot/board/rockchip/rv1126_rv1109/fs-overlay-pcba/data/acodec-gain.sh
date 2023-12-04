#0=0db, 1=6.6db, 2=13dB, 3=20db
amixer cset -c 0 name="ADC MIC Group 2 Left Volume" 3
amixer cset -c 0 name="ADC MIC Group 3 Left Volume" 3
amixer cset -c 0 name="ADC MIC Group 2 Right Volume" 3
amixer cset -c 0 name="ADC MIC Group 3 Right Volume" 3

#12=0dB 18=9db
amixer cset -c 0 name="ADC ALC Group 2 Left Volume" 16
amixer cset -c 0 name="ADC ALC Group 3 Left Volume" 16
amixer cset -c 0 name="ADC ALC Group 2 Right Volume" 16
amixer cset -c 0 name="ADC ALC Group 3 Right Volume" 16

#loop back gain 
amixer cset -c 0 name="ADC ALC Group 0 Left Volume"  17

# lineout gain
#0=490mv，1=890mv 
amixer cset -c 0 name='DAC LINEOUT Right Volume' 3
amixer cset -c 0 name='DAC LINEOUT Left Volume' 3

amixer cset -c 0 name='DAC HPMIX Left Volume' 1
amixer cset -c 0 name='DAC HPMIX Right Volume' 1

vol=70
amixer set  Master Playback $vol
