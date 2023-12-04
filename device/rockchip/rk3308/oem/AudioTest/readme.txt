need bash support

the format cmd
1, arecord.sh 2(单位S)
2, ChangeVolume.sh 2mic_loopback 3(声道数)
3, LoopbackTest.sh 2mic_loopback 3
4, LoopbackTest1.sh 2mic_loopback 3

5  amic-loopback-no-vad.sh 16000(record rate) S16_LE(record format) hw:0,0(playback hardward)
#							 7(total times) /tmp/playback-sine-1khz-48k-16bit-2ch.wav(playback file)