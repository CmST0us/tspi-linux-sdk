#!/bin/bash

# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=rv1126-bat-emmc-tb
# Uboot image format type: fit(flattened image tree)
export RK_UBOOT_FORMAT_TYPE=fit
# Loader update spl from Uboot
export RK_LOADER_UPDATE_SPL=true
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rv1126_defconfig
# kernel defconfig fragment
export RK_KERNEL_DEFCONFIG_FRAGMENT="rv1126-tb.config rv1126-snapshot.config"
# Kernel dts
export RK_KERNEL_DTS=rv1126-snapshot
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/Image
# parameter for GPT table
export RK_PARAMETER=parameter-snapshot.txt
# Buildroot config
# export RK_CFG_BUILDROOT=rockchip_rv1126_rv1109_tb
# Recovery config
export RK_CFG_RECOVERY=
# ramboot config
export RK_CFG_RAMBOOT=rockchip_rv1126_snapshot
# ramboot idt config
export RK_RECOVERY_FIT_ITS=boot-tb.its
# Pcba config
export RK_CFG_PCBA=
# Build jobs
export RK_JOBS=12
# target chip
export RK_TARGET_PRODUCT=rv1126_rv1109
# Set ramboot image type
export RK_RAMBOOT_TYPE=ROMFS
# Define package-file for update.img
export RK_PACKAGE_FILE=rv1126-package-file-snapshot
