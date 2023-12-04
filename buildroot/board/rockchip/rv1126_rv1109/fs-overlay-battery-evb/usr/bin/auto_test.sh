#!/bin/sh
#

# wifi

echo "============================================"
echo "请观察屏幕是否有显示预览，验证摄像头和屏幕正常"
sleep 1
echo "测试开始" > /tmp/test.log
echo "-------------------------------------------"

echo "============================================"
echo "开始三秒录音测试"
arecord -D hw:1,0 -f S16_LE /tmp/test.wav -d 3 -r 16000 -c 2 &
aplay /etc/no_key.wav
sleep 3
echo "-------------------------------------------"

echo "============================================"
echo "开始三秒播放测试"
aplay -Dhw:0,0 -r 16000 -c 2 -f S16_LE /tmp/test.wav
echo "-------------------------------------------"

echo "============================================"
echo "开始LED闪烁和光敏测试，观测是否有变化"
cat /sys/bus/iio/devices/iio:device1/in_illuminance_input
echo 255 > /sys/class/leds/white/brightness
cat /sys/bus/iio/devices/iio:device1/in_illuminance_input
sleep 1
echo 0 > /sys/class/leds/white/brightness
sleep .5
echo 255 > /sys/class/leds/white/brightness
sleep .5
echo 0 > /sys/class/leds/white/brightness
echo "-------------------------------------------"

echo "============================================"
echo "接下来读三次RTC时间，有变化即可"
cat /proc/driver/rtc | grep rtc_time
sleep 1
cat /proc/driver/rtc | grep rtc_time
sleep 1
cat /proc/driver/rtc | grep rtc_time
echo "-------------------------------------------"

echo "============================================"
echo "接下来将挂载SD卡，会显示SD卡容量"
mkdir /userdata/sd
mount -t vfat /dev/mmcblk2p1 /userdata/sd
sleep 1
df -h | grep sd
sleep 2
echo "-------------------------------------------"


echo "============================================"
echo "接下来将连接wifi，默认连Rockchip，有可能连不上，请手动执行tb_start_wifi.sh wifi名称 wifi密码 true"
sleep 2
tb_start_wifi.sh Rockchip-guest RKguest2.4 true
ping baidu.com -c 3
echo "-------------------------------------------"
