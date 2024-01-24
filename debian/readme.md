## Introduction

A set of shell scripts that will build GNU/Linux distribution rootfs image
for rockchip platform.

## Available Distro

* Debian 11 (Bullseye-X11 and Wayland)~~

```
sudo apt-get install binfmt-support qemu-user-static
sudo dpkg -i ubuntu-build-service/packages/*
sudo apt-get install -f
```

## Usage for 32bit Debian 11 (Bullseye-32)

### Building debian system from linaro

Building a base debian system by ubuntu-build-service from linaro.

```
	RELEASE=bullseye TARGET=base ARCH=armhf ./mk-base-debian.sh
```

Building a desktop debian system by ubuntu-build-service from linaro.

```
	RELEASE=bullseye TARGET=desktop ARCH=armhf ./mk-base-debian.sh
```

### Building overlay with rockchip audio/video hardware accelerated

- Building with overlay with rockchip debian rootfs:

```
	RELEASE=bullseye ARCH=armhf ./mk-rootfs.sh
```

- Building with overlay with rockchip debug debian rootfs:

```
	VERSION=debug ARCH=armhf ./mk-rootfs-bullseye.sh
```

### Creating roofs image

Creating the ext4 image(linaro-rootfs.img):

```
	./mk-image.sh
```

---

## Usage for 64bit Debian 11 (Bullseye-64)

### Building debian system from linaro

Building a base debian system by ubuntu-build-service from linaro.

```
	RELEASE=bullseye TARGET=desktop ARCH=arm64 ./mk-base-debian.sh
```

### Building overlay with rockchip audio/video hardware accelerated

- Building the rk-debian rootfs

```
	RELEASE=bullseye ARCH=arm64 ./mk-rootfs.sh
```

- Building the rk-debain rootfs with debug

```
	VERSION=debug ARCH=arm64 ./mk-rootfs-bullseye.sh
```

### Creating roofs image

Creating the ext4 image(linaro-rootfs.img):

```
	./mk-image.sh
```
---

## Cross Compile for ARM Debian

[Docker + Multiarch](http://opensource.rock-chips.com/wiki_Cross_Compile#Docker)

## Package Code Base

Please apply [those patches](https://github.com/rockchip-linux/rk-rootfs-build/tree/master/packages-patches) to release code base before rebuilding!

## License information

Please see [debian license](https://www.debian.org/legal/licenses/)

## FAQ

- noexec or nodev issue
noexec or nodev issue /usr/share/debootstrap/functions: line 1450:
../rootfs/ubuntu-build-service/bullseye-desktop-arm64/chroot/test-dev-null:
Permission denied E: Cannot install into target
...
mounted with noexec or nodev

Solution: mount -o remount,exec,dev xxx (xxx is the mount place), then rebuild it.
