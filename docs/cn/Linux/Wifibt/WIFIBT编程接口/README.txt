RK Linux平台WiFiBT的应用接口API，方便客户快速开发相关应用。

以下有几点说明：
1. 之前的libDeviceIo后续统一改为librkwifibt.so
  #注意：由于文档里面的相关deviceio的相关术语没有更新，所以对应关系为：
  libDeviceio.so 对应新的：librkwifibt.so
  deviceio_test 对应新的：rkwifibt_test
2. librkwifibt.so基于wpa_supplicant/Bluez开发；
3. librkwifibt.so目前支持常用的PROFILE: A2DP SINK/SOURCE蓝牙音乐、BLE MASTER/SLAVE主从模式、SPP、HFP通话(SCO OVER HCI);
4. HFP说明：如果要使用SCO OVER PCM接口，需要找原厂协助，Linux系统默认不支持；
5. 之前基于bsa的libDeviceIo不再维护，因为bsa本身就是原厂封装的库，也提供完整的demo，如需使用请自行研究或找原厂协助；

补丁说明：(请使用最新版本)
RKWIFIBT_APP_V1.1.7z  #库及测试代码

#用法参考：
蓝牙:
/11-Linux平台/WIFIBT编程接口/Rockchip_Developer_Guide_DeviceIo_Bluetooth_CN.pdf
/11-Linux平台/WIFIBT编程接口/最新蓝牙接口说明.txt
WiFi:
/11-Linux平台/WIFIBT编程接口/最新WIFI接口说明.txt

WiFi SOFTAP配网参考：
Rockchip_Developer_Guide_Network_Config_CN.pdf --- 3.3 Softap 配网
示例程序：
RKWIFIBT_APP_V1.1\test\rk_wifi_test.c
RKWIFIBT_APP_V1.1\test\softap\softap.c

#RKWIFIBT_APP说明：
include/ #头文件
lib64/   #64接口库
lib32/   #32接口库
test/    #API示例用法

Makefile 修改：CC 和 SYSROOT 改成你实际使用的！！！ 主要是这个目录的差别：rockchip_rk3326_64 
CC := /PATH/buildroot/output/rockchip_rk3326_64/host/bin/aarch64-buildroot-linux-gnu-gcc
SYSROOT := --sysroot=/PATH/buildroot/output/rockchip_rk3326_64/host/aarch64-buildroot-linux-gnu/sysroot

make会生成rkwifibt_test (具体可自行修改Makefile)


#运行
librkwifibt.so push到  usr/lib/
rkwifibt_test push 任意位置

WiFi测试: rkwifibt_test wificonfig  #WiFi测试/及相关配网测试
蓝牙测试：rkwifibt_test bluetooth   #蓝牙相关API测试


#蓝牙功能特别注意：
使用上述接口是请确保蓝牙功能正常，SDK集成一个蓝牙初始化bt_init.sh脚本，库启动时会依赖这个脚本去给蓝牙做初始化！
请确保配置正确的模组型号，参考文档/11-Linux平台/WIFIBT开发文档/Rockchip_Developer_Guide_Linux_WIFI_BT_CN.pdf的第2章节，问题排查参考第4.3章节
正常情况下，开机会有如下文件：
/usr/bin/bt_init.sh
#如果是Realtek WiFi：
#!/bin/sh

killall rtk_hciattach

echo 0 > /sys/class/rfkill/rfkill0/state
sleep 2
echo 1 > /sys/class/rfkill/rfkill0/state
sleep 2

insmod /usr/lib/modules/hci_uart.ko
rtk_hciattach -n -s 115200 BT_TTY_DEV rtk_h5 &
hciconfig hci0 up

如果是正基/海华(CYPRESS):
#!/bin/sh

killall brcm_patchram_plus1

echo 0 > /sys/class/rfkill/rfkill0/state
sleep 2
echo 1 > /sys/class/rfkill/rfkill0/state
sleep 2

brcm_patchram_plus1 --bd_addr_rand --enable_hci --no2bytes --use_baudrate_for_download  --tosleep  200000 --baudrate 1500000 --patchram  BTFIRMWARE_PATH BT_TTY_DEV &
hciconfig hci0 up