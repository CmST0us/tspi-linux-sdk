#!/bin/bash

# Target arch
export RK_ARCH=arm
# Uboot defconfig
export RK_UBOOT_DEFCONFIG=evb-aarch32-rk3308
# Kernel defconfig
export RK_KERNEL_DEFCONFIG=rk3308_robot_aarch32_defconfig
# Kernel dts
export RK_KERNEL_DTS=rk3308-robot-aarch32
# boot image type
export RK_BOOT_IMG=zboot.img
# kernel image path
export RK_KERNEL_IMG=kernel/arch/arm/boot/zImage
# parameter for GPT table
export RK_PARAMETER=parameter-32bit.txt
# Buildroot config
export RK_CFG_BUILDROOT=rockchip_rk3308_robot32
# Recovery config
export RK_CFG_RECOVERY=rockchip_rk3308_robot_recovery
# ramboot config
export RK_CFG_RAMBOOT=
# Pcba config
export RK_CFG_PCBA=rockchip_rk3308_pcba
# Build jobs
export RK_JOBS=20
# target chip
export RK_TARGET_PRODUCT=rk3308
# Set rootfs type, including ext2 ext4 squashfs
export RK_ROOTFS_TYPE=squashfs
# rootfs image path
export RK_ROOTFS_IMG=rockdev/rootfs.${RK_ROOTFS_TYPE}
# Set oem partition type, including ext2 squashfs
export RK_OEM_FS_TYPE=ext2
# Set userdata partition type, including ext2, fat
export RK_USERDATA_FS_TYPE=ext2
#OEM config:
export RK_OEM_DIR=oem_empty
#userdata config
export RK_USERDATA_DIR=userdata_empty
#misc image
export RK_MISC=wipe_all-misc.img
#choose enable distro module
export RK_DISTRO_MODULE=