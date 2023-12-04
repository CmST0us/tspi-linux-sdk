libbdSPILAudioProc.so
md5:e5d3bf87f8c39dbf43b707227564317e

libbd_audio_vdev.so
md5:f7ab4d038d31313c84ea4416a558f41e

libbd_alsa_audio_client.so
md5:c68fe39322647b4bf8bb576ad19bb707

集成和使用说明：
1. push库到设备上
   adb push libbdSPILAudioProc.so /data
   adb push libbd_audio_vdev.so /data
   adb push libbd_alsa_audio_client.so /data
   adb push config_huamei_rk3308_4_1.lst /data
   adb push setup.sh /data
   adb push alsa_audio_main_service /data
   adb push alsa_audio_client_sample /data
   adb shell sync
2. 创建目录，修改权限
   adb shell;cd /data
   chmod +x alsa_audio_*
   chmod +x setup.sh
   mkdir -p /data/local/ipc
   chmod 777 /data/local/ipc
3. 运行main service
   ./setup.sh
   ./alsa_audio_main_service hw:0,0 &
   hw:0,0是对应的录音设备的声卡号和device号，也可以配置asound.conf，使用逻辑pcm设备名
4. 运行app,比如duer_linux, 需要添加/data目录到duer_linux的动态库链接路径中
   也可以运行我们的sample程序
   ./alsa_audio_client_sample
   在当前目录下会保存经过信号处理的录音文件dump_pcm.pcm，是双声道，16K，小端，16bit位深音频。

保存原始录音数据的方法:
	启动录音前运行：
	mkdir -p /data/local/aw.so_profile
	touch  /data/local/aw.so_profile/dump_switch
	touch /data/local/aw.so_profile/dump_switch_wakets
	mkdir -p /data/local/aud_rec/
	chmod 777 /data/local/aud_rec/
在/data/local/aud_rec目录下会保存4路麦克风数据和1路参考数据，一路识别数据，一路唤醒数据。
文件的数据格式都是： 16KHz、小端、16bit、单声道
   