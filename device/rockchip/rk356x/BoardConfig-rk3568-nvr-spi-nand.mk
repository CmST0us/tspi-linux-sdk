#!/bin/bash

# Target arch
export RK_ARCH=arm64
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rk3568
# Uboot image format type: fit(flattened image tree)
export RK_UBOOT_FORMAT_TYPE=fit
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rockchip_linux_defconfig
# Kernel defconfig fragment
export RK_KERNEL_DEFCONFIG_FRAGMENT=rk3568_nvr.config
# Kernel dts   rk3568-nvr-demo-v10-linux   rk3568-evb1-ddr4-v10-linux
export RK_KERNEL_DTS=rk3568-nvr-demo-v10-linux-spi-nand
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_ZIMG=kernel/arch/arm64/boot/Image.lz4
# kernel image format type: fit(flattened image tree)
export RK_KERNEL_FIT_ITS=zboot.its
# parameter for GPT table
export RK_PARAMETER=parameter-buildroot-NVR-spi-nand-128M.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk356x_nvr_spi_nand
# Recovery config
export RK_CFG_RECOVERY=
# Recovery image format type: fit(flattened image tree)
export RK_RECOVERY_FIT_ITS=
# ramboot config
export RK_CFG_RAMBOOT=
# Pcba config
export RK_CFG_PCBA=
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rk356x
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=ubi
# Set debian version (debian10: buster, debian11: bullseye)
export RK_DEBIAN_VERSION=buster
# yocto machine
export RK_YOCTO_MACHINE=rockchip-rk3568-evb
# rootfs image path
export RK_ROOTFS_IMG=rockdev/rootfs.${RK_ROOTFS_TYPE}
# Set ramboot image type
export RK_RAMBOOT_TYPE=
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ubi
#OEM config
export RK_OEM_DIR=
# OEM build on buildroot
#export RK_OEM_BUILDIN_BUILDROOT=YES


#userdata config, if not define this, system will format by RK_USERDATA_FS_TYPE
export RK_USERDATA_DIR=userdata_empty
#
# RK_UBI_PAGE_SIZE and RK_UBI_BLOCK_SIZE MUST be defined if meet One of the following conditions:
#
# 1. define RK_OEM_DIR and undefine RK_OEM_BUILDIN_BUILDROOT
# 2. define RK_USERDATA_DIR
#
# Set ubifs page size, 2048(2KB) or 4096(4KB)
export RK_UBI_PAGE_SIZE=2048
#
# Set ubifs block size, 0x20000(128KB) or 0x40000(256KB)
export RK_UBI_BLOCK_SIZE=0x20000
#
# Set userdata partition size (byte) if define RK_USERDATA_DIR , 128M=0x3400000   256M=0xB400000,from 75M-127(255)
export RK_USERDATA_PARTITION_SIZE=0xB400000
#
# Set oem partition size (byte) if undefine RK_OEM_BUILDIN_BUILDROOT
# export RK_OEM_PARTITION_SIZE=0x6400000
#


#misc image
export RK_MISC=
#choose enable distro module
export RK_DISTRO_MODULE=
# Define pre-build script for this board
export RK_BOARD_PRE_BUILD_SCRIPT=app-build.sh

# Define package-file
export RK_PACKAGE_FILE=rk356x-package-file-spi-nand
