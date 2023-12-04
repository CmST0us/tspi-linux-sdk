#!/bin/bash

# Target arch
export RK_ARCH=arm64
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rknpu-lion
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rk1808_linux_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk1808-compute-v10
# boot image type
export RK_BOOT_IMG=boot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm64/boot/Image
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk1808_compute_stick
# ramboot config
export RK_CFG_RAMBOOT=rockchip_rk1808_compute_stick
# Pcba config
export RK_CFG_PCBA=rockchip_rk1808_pcba
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rk1808
#choose enable distro module
export RK_DISTRO_MODULE=
