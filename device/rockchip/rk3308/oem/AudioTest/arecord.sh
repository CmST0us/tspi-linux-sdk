#!/bin/bash
gst-play-1.0 ./Track01.mp3 &
rm -rf /tmp/arecord
mkdir /tmp/arecord
arecord -D hw:0,0 --period-size=1024 --buffer-size=25600 -c 8 -r 16000 -f S16_LE -t raw -d $1  /tmp/arecord/test_16k_16bit_8ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=102400 -c 2 -r 16000 -f S16_LE -t raw -d $1 /tmp/arecord/test_16k_16bit_2ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 4 -r 16000 -f S16_LE -t raw -d $1  /tmp/arecord/test_16k_16bit_4ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=25600 -c 6 -r 16000 -f S16_LE -t raw -d $1  /tmp/arecord/test_16k_16bit_6ch_channel.pcm

arecord -D hw:0,0 --period-size=1024 --buffer-size=25600 -c 8 -r 16000 -f S32_LE -t raw -d $1  /tmp/arecord/test_16k_32bit_8ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=102400 -c 2 -r 16000 -f S32_LE -t raw -d $1 /tmp/arecord/test_16k_32bit_2ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 4 -r 16000 -f S32_LE -t raw -d $1  /tmp/arecord/test_16k_32bit_4ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=25600 -c 6 -r 16000 -f S32_LE -t raw -d $1  /tmp/arecord/test_16k_32bit_6ch_channel.pcm

arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 8 -r 32000 -f S16_LE -t raw -d $1  /tmp/arecord/test_32k_16bit_8ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=102400 -c 2 -r 32000 -f S16_LE -t raw -d $1 /tmp/arecord/test_32k_16bit_2ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 4 -r 32000 -f S16_LE -t raw -d $1  /tmp/arecord/test_32k_16bit_4ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 6 -r 32000 -f S16_LE -t raw -d $1  /tmp/arecord/test_32k_16bit_6ch_channel.pcm

arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 8 -r 32000 -f S32_LE -t raw -d $1  /tmp/arecord/test_32k_32bit_8ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=102400 -c 2 -r 32000 -f S32_LE -t raw -d $1 /tmp/arecord/test_32k_32bit_2ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 4 -r 32000 -f S32_LE -t raw -d $1  /tmp/arecord/test_32k_32bit_4ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 6 -r 32000 -f S32_LE -t raw -d $1  /tmp/arecord/test_32k_32bit_6ch_channel.pcm

arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 8 -r 48000 -f S16_LE -t raw -d $1  /tmp/arecord/test_48k_16bit_8ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=102400 -c 2 -r 48000 -f S16_LE -t raw -d $1 /tmp/arecord/test_48k_16bit_2ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 4 -r 48000 -f S16_LE -t raw -d $1  /tmp/arecord/test_48k_16bit_4ch_channel.pcm
arecord -D hw:0,0 --period-size=1024 --buffer-size=51200 -c 6 -r 48000 -f S16_LE -t raw -d $1  /tmp/arecord/test_48k_16bit_6ch_channel.pcm

arecord -D hw:0,0 --period-size=128 --buffer-size=51200 -c 8 -r 48000 -f S32_LE -t raw -d $1  /tmp/arecord/test_48k_32bit_8ch_channel.pcm
arecord -D hw:0,0 --period-size=128 --buffer-size=65536 -c 2 -r 48000 -f S32_LE -t raw -d $1 /tmp/arecord/test_48k_32bit_2ch_channel.pcm
arecord -D hw:0,0 --period-size=128 --buffer-size=51200 -c 4 -r 48000 -f S32_LE -t raw -d $1  /tmp/arecord/test_48k_32bit_4ch_channel.pcm
arecord -D hw:0,0 --period-size=128 --buffer-size=51200 -c 6 -r 48000 -f S32_LE -t raw -d $1  /tmp/arecord/test_48_32bit_6ch_channel.pcm
killall gst-play-1.0

