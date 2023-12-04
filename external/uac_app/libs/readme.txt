2020.9.14
libaec_wake_float.so: 音频回声消除算法
libagc.so: 音频自动增益
libanr.so: 音频去噪算法
libbeamform.so：波束算法

这几个库只支持最多2mic输入,如需支持大于2mic的输入时，
需先向rockchip申请，并重新出库。
算法库的使用，请参考文档《Rockchip_Developer_Guide_Linux_UACApp_CN.pdf》
的json配置章节，以及mic_recode_3a_usb_playback_demo.json
