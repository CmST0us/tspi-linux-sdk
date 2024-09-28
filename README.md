# TSPI-Linux SDK

泰山派Linux SDK
基于Rockchip原厂 `rk4.1-202305` 版本

## 预编译固件
链接：https://pan.baidu.com/s/1yd-n9Xr-0quVid7sIeTM7Q?pwd=2653 
提取码：2653 


## 编译步骤
以下步骤针对Ubuntu 22.04编译环境

```bash
sudo apt install git-lfs
git clone --recursive https://github.com/CmST0us/tspi-linux-sdk.git
cd tspi-linux-sdk
git lfs pull

sudo apt update
sudo apt install -y git ssh make gcc libssl-dev liblz4-tool expect \
g++ patchelf chrpath gawk texinfo chrpath diffstat binfmt-support \
qemu-user-static live-build bison flex fakeroot cmake gcc-multilib \
g++-multilib unzip device-tree-compiler ncurses-dev python2
./build.sh init
./build.sh
```

## Kernel
版本 `5.10.198`

- 添加distroboot支持, 在配置中增加 `RK_KERNEL_BOOT_TYPE=distroboot`


## Device
- 添加 `tspi` 立创开发板泰山派 `4G_HUB2.0_ET100_HS4G_HUB2.0_ET100_HS` 扩展板的支持, 选择 `tspi-rk3566-ext39-ubuntu_defconfig`
- 添加 `tspi` 立创开发板泰山派 的支持，参考配置文件 `device/rockchip/.chips/rk3566_rk3568/tspi-rk3566-ubuntu_defconfig`
- 添加 `tspi` 立创开发板泰山派 Distroboot启动方法的支持, 参考配置文件 `device/rockchip/.chips/rk3566_rk3568/tspi-rk3566-ubuntu-distroboot_defconfig`


## Ubuntu
使用 Ubuntu-22.04
默认用户名密码: `neons`

### 串口
使用波特率 `115200`

### ADB
默认启动 `adb`, 支持USB单线连接到开发板.
```
adb shell
```

同时支持 `adb` 端口转发
```
adb forward tcp:2222 tcp:22
```

## Build

Build 基础Ubuntu镜像
**注意在构建Ubuntu时，需要输入电脑的root密码**
```
./build.sh init
./build.sh 
```

## Flash

### 使用 update.img

**注意带日期的文件才是最终的update.img**
```
mv rockdev/<带日期的update.img> rockdev/update.img
sudo ./rkflash.sh updateimg
```

### 使用分区小包
```
sudo ./rkflash.sh
```
